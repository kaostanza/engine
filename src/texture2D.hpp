#pragma once

#include "glad/glad.h"
#include <stdexcept>
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "stb_image.h"

enum WrapMode : GLenum { Repeat = GL_REPEAT, ClampToEdge = GL_CLAMP_TO_EDGE };
enum FilterMode : GLenum { Linear = GL_LINEAR, Nearest = GL_NEAREST };
enum Format : GLenum { RGB = GL_RGB, RGBA = GL_RGBA };

struct Texture2DBuilder {
  WrapMode wrap_s = WrapMode::Repeat;
  WrapMode wrap_t = WrapMode::Repeat;
  FilterMode min_filter = FilterMode::Linear;
  FilterMode mag_filter = FilterMode::Linear;
  Format format = Format::RGBA;
};

class Texture2D {
public:
  Texture2D(const Texture2D &) = delete;
  Texture2D &operator=(const Texture2D &) = delete;

  Texture2D(Texture2D &&other) noexcept
      : id(other.id), width(other.width), height(other.height),
        nrChannels(other.nrChannels) {}

  explicit Texture2D(const char *file_path,
                     const Texture2DBuilder &builder = {}) {
    unsigned char *data =
        stbi_load(file_path, &this->width, &this->height, &this->nrChannels, 0);

    if (!data) {
      throw std::runtime_error(
          "Erreur: impossible de charger la texture avec stbi: " +
          std::string(file_path));
    }

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
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(builder.format), width,
                 height, 0, static_cast<GLenum>(builder.format),
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
  };

  ~Texture2D() { this->deinit(); }
  void deinit() { glDeleteTextures(1, &this->id); }

  void bind() const { glBindTexture(GL_TEXTURE_2D, this->id); }

  unsigned int get_id() const { return this->id; }

private:
  unsigned int id;
  int width;
  int height;
  int nrChannels;
};
