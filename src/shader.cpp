#include "shader.hpp"

void Shader::deinit() {
  delete_shaders();
  glDeleteProgram(this->id);
}

void Shader::delete_shaders() { this->shader_ids.clear(); }

void Shader::link() {
  auto success = 0;
  glLinkProgram(this->id);
  glGetProgramiv(this->id, GL_LINK_STATUS, &success);

  if (success == 0) {
    char info_log[512];
    glGetProgramInfoLog(this->id, 512, nullptr, info_log);
    std::cout << "Erreur: Impossible de link le shader program\n"
              << "-> " << info_log << std::endl;
  }

  delete_shaders();
}

void Shader::use() const { glUseProgram(this->id); }
