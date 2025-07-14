#pragma once

#include "glad/glad.h"
#include "glm/detail/type_mat.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <vector>

struct VertexShader {};
struct FragmentShader {};

class Shader {
public:
  Shader();
  ~Shader();
  void deinit();

  template <typename ShaderType> void add_shader(const char *file_path);
  template <typename T>
  void set_uniform(const char *uniform_name, const T &value) const;
  void link();
  void use() const;
  void delete_shaders();

private:
  unsigned int id;
  std::vector<int> shader_ids;

  template <typename ShaderType> int add_shader_impl() const;
  template <typename T>
  void set_uniform_impl(const int uniform_location, const T &value) const;
};

template <typename ShaderType> void Shader::add_shader(const char *file_path) {
  std::ifstream file(file_path, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    std::cerr << "Erreur: impossible d'ouvrir le fichier " << file_path << "\n";
  }

  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size + 1, 0); // sentinel '\0'
  if (!file.read(buffer.data(), size)) {
    std::cerr << "Erreur: lecture échouée pour " << file_path << "\n";
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
    std::cerr << "Erreur: Impossible de compiler le shader: " << file_path
              << "\n-> " << info_log << "\n";
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
inline void Shader::set_uniform_impl<float>(const int location,
                                            const float &value) const {
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
