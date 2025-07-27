#pragma once

#include "glad/glad.h"
#include "stb_image.h"
#include <stdexcept>

enum WrapMode : GLenum { Repeat = GL_REPEAT, ClampToEdge = GL_CLAMP_TO_EDGE };
enum FilterMode : GLenum { Linear = GL_LINEAR, Nearest = GL_NEAREST };
enum TextureType { DIFFUSE, SPECULAR, EMISSION };

struct Texture2DBuilder {
  WrapMode wrap_s = WrapMode::Repeat;
  WrapMode wrap_t = WrapMode::Repeat;
  FilterMode min_filter = FilterMode::Linear;
  FilterMode mag_filter = FilterMode::Linear;
  TextureType type = TextureType::DIFFUSE;
};

class Texture2D {
public:
  explicit Texture2D(const char *file_path,
                     const Texture2DBuilder &builder = {}) {
    unsigned char *data =
        stbi_load(file_path, &this->width, &this->height, &this->nrChannels, 0);

    if (!data) {
      throw std::runtime_error(
          "Erreur: impossible de charger la texture avec stbi: " +
          std::string(file_path));
    }

    GLenum format;
    switch (this->nrChannels) {
    case 1:
      format = GL_RED;
      break;
    case 3:
      format = GL_RGB;
      break;
    case 4:
      format = GL_RGBA;
      break;
    default:
      throw std::runtime_error(" Erreur: format non supporté " +
                               std::string(file_path));
    };

    glGenTextures(1, &this->id);

    glBindTexture(GL_TEXTURE_2D, this->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    static_cast<GLint>(builder.wrap_s));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    static_cast<GLint>(builder.wrap_t));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    static_cast<GLint>(builder.min_filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    static_cast<GLint>(builder.mag_filter));
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), width, height, 0,
                 static_cast<GLenum>(format), GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    this->path = std::string(file_path);
    this->type = builder.type;
  };

  void deinit() { glDeleteTextures(1, &this->id); }

  void bind() const { glBindTexture(GL_TEXTURE_2D, this->id); }

  unsigned int get_id() const { return this->id; }
  const std::string &get_path() const { return this->path; }
  std::string get_local_path() const {
    return this->path.substr(this->path.find_last_of("/") + 1,
                             this->path.size());
  }
  TextureType get_type() const { return this->type; }

private:
  unsigned int id;
  int width;
  int height;
  int nrChannels;

  TextureType type;
  std::string path;
};
