#pragma once

#include "glad/glad.h"
#include "glm/detail/type_mat.hpp"
#include <cstdio>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "light.hpp"

struct VertexShader {};
struct FragmentShader {};

class Shader {
public:
  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;

  Shader() : id(glCreateProgram()) {}
  ~Shader() { this->deinit(); }
  void deinit();

  template <typename ShaderType> void add_shader(const char *file_path);
  template <typename T>
  void set_uniform(const char *uniform_name, const T &value) const;
  template <typename T>
  void set_uniform_struct(std::string_view uniform_struct_name,
                          const T &value) const;
  void link();
  void use() const;
  void delete_shaders();

private:
  unsigned int id;
  std::vector<unsigned int> shader_ids;

  template <typename ShaderType> int add_shader_impl() const;
  template <typename T>
  void set_uniform_impl(const int uniform_location, const T &value) const;
};

template <typename ShaderType> void Shader::add_shader(const char *file_path) {
  std::ifstream file(file_path, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    throw std::runtime_error("Erreur: impossible d'ouvrir le fichier " +
                             std::string(file_path));
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size + 1, 0); // sentinel '\0'
  if (!file.read(buffer.data(), size)) {
    throw std::runtime_error("Erreur: lecture échouée pour " +
                             std::string(file_path));
  }

  GLuint id = glCreateShader(add_shader_impl<ShaderType>());
  const char *src = buffer.data();
  glShaderSource(id, 1, &src, nullptr);
  glCompileShader(id);

  GLint success;
  glGetShaderiv(id, GL_COMPILE_STATUS, &success);

  if (!success) {
    char info_log[512];
    glGetShaderInfoLog(id, 512, nullptr, info_log);
    glDeleteShader(id);
    throw std::runtime_error("Erreur: Impossible de compiler le shader " +
                             std::string(file_path) + ": " + info_log);
  }

  glAttachShader(this->id, id);
  shader_ids.push_back(id);
}

template <> inline int Shader::add_shader_impl<FragmentShader>() const {
  return GL_FRAGMENT_SHADER;
}

template <> inline int Shader::add_shader_impl<VertexShader>() const {
  return GL_VERTEX_SHADER;
}

template <typename T>
void Shader::set_uniform(const char *uniform_name, const T &value) const {
  const auto uniform_location = glGetUniformLocation(this->id, uniform_name);
  if (uniform_location == -1) {
    std::cerr << "Erreur: Impossible de trouver l'uniform " << uniform_name
              << "\n";
    return;
  }

  set_uniform_impl(uniform_location, value);
}

template <>
inline void Shader::set_uniform_impl<int>(const int location,
                                          const int &value) const {
  glUniform1i(location, value);
}
template <>
inline void
Shader::set_uniform_impl<unsigned int>(const int location,
                                       const unsigned int &value) const {
  glUniform1ui(location, value);
}

template <>
inline void Shader::set_uniform_impl<float>(const int location,
                                            const float &value) const {
  glUniform1f(location, value);
}

template <>
inline void Shader::set_uniform_impl<double>(const int location,
                                             const double &value) const {
  glUniform1f(location, value);
}

template <>
inline void Shader::set_uniform_impl<glm::vec3>(const int location,
                                                const glm::vec3 &value) const {
  glUniform3fv(location, 1, &value[0]);
}

template <>
inline void Shader::set_uniform_impl<glm::mat4>(const int location,
                                                const glm::mat4 &value) const {
  glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
}

template <>
inline void
Shader::set_uniform_struct<LightInfo>(std::string_view uniform_struct_name,
                                      const LightInfo &value) const {
  char buffer[64];
  int name_len = uniform_struct_name.length();
  const char *name_ptr = uniform_struct_name.data();

  snprintf(buffer, sizeof(buffer), "%.*s.ambient", name_len, name_ptr);
  this->set_uniform(buffer, value.ambient);

  snprintf(buffer, sizeof(buffer), "%.*s.specular", name_len, name_ptr);
  this->set_uniform(buffer, value.specular);

  snprintf(buffer, sizeof(buffer), "%.*s.diffuse", name_len, name_ptr);
  this->set_uniform(buffer, value.diffuse);
}

template <>
inline void
Shader::set_uniform_struct<PointLight>(std::string_view uniform_struct_name,
                                       const PointLight &value) const {
  std::string uniform_struct_name_s(uniform_struct_name);
  this->set_uniform((uniform_struct_name_s + ".position").c_str(),
                    value.position);

  this->set_uniform((uniform_struct_name_s + ".linear").c_str(),
                    value.attenuation_info.linear);
  this->set_uniform((uniform_struct_name_s + ".quadratic").c_str(),
                    value.attenuation_info.quadratic);
  this->set_uniform((uniform_struct_name_s + ".constant").c_str(),
                    value.attenuation_info.constant);

  this->set_uniform_struct(uniform_struct_name_s, value.info);
}

template <>
inline void
Shader::set_uniform_struct<SpotLight>(std::string_view uniform_struct_name,
                                      const SpotLight &value) const {
  std::string uniform_struct_name_s(uniform_struct_name);
  this->set_uniform((uniform_struct_name_s + ".position").c_str(),
                    value.position);
  this->set_uniform((uniform_struct_name_s + ".direction").c_str(),
                    value.direction);
  this->set_uniform((uniform_struct_name_s + ".inner_cut_off").c_str(),
                    value.inner_cut_off);
  this->set_uniform((uniform_struct_name_s + ".outer_cut_off").c_str(),
                    value.outer_cut_off);
  this->set_uniform_struct(uniform_struct_name_s, value.info);
}

template <>
inline void Shader::set_uniform_struct<DirectionalLight>(
    std::string_view uniform_struct_name, const DirectionalLight &value) const {
  std::string uniform_struct_name_s(uniform_struct_name);
  this->set_uniform((uniform_struct_name_s + ".direction").c_str(),
                    value.direction);
  this->set_uniform_struct(uniform_struct_name_s, value.info);
}
