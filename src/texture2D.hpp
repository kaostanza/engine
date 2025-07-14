#pragma once

#include <iostream>
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "glad/glad.h"
#include "stb_image.h"

class Texture2D {
public:
  enum class WrapMode : GLenum {
    Repeat = GL_REPEAT,
    ClampToEdge = GL_CLAMP_TO_EDGE
  };
  enum class FilterMode : GLenum { Linear = GL_LINEAR, Nearest = GL_NEAREST };
  enum class Format : GLenum { RGB = GL_RGB, RGBA = GL_RGBA };

  Texture2D(const char *file_path, WrapMode wrap_s = WrapMode::Repeat,
            WrapMode wrap_t = WrapMode::Repeat,
            FilterMode min_filter = FilterMode::Linear,
            FilterMode mag_filter = FilterMode::Linear,
            Format format = Format::RGB)
      : wrap_s_(wrap_s), wrap_t_(wrap_t), min_filter_(min_filter),
        mag_filter_(mag_filter), format_(format) {
    this->data =
        stbi_load(file_path, &this->width, &this->height, &this->nrChannels, 0);

    if (this->data == nullptr) {
      std::cerr << "Erreur: Impossible de charger la texture avec stbi: "
                << file_path << "\n";
    }

    glGenTextures(1, &this->id);
  }

  unsigned int get_id() const { return this->id; }

  void bind() {
    glBindTexture(GL_TEXTURE_2D, this->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    static_cast<GLenum>(wrap_s_));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    static_cast<GLenum>(wrap_t_));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    static_cast<GLenum>(min_filter_));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    static_cast<GLenum>(mag_filter_));
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLenum>(format_), width, height,
                 0, static_cast<GLenum>(format_), GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    data = nullptr;
  }

private:
  unsigned int id;
  int width;
  int height;
  int nrChannels;
  unsigned char *data = nullptr;

  WrapMode wrap_s_, wrap_t_;
  FilterMode min_filter_, mag_filter_;
  Format format_;
};
