#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include "shader.hpp"
#include "texture2D.hpp"

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texture_coordinate;
};

class Mesh {
public:
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture2D *> textures;

  Mesh(std::vector<Vertex> _vertices, std::vector<unsigned int> _indices,
       std::vector<Texture2D *> _textures)
      : vertices(std::move(_vertices)), indices(std::move(_indices)),
        textures(std::move(_textures)) {
    this->setup();
  }
  // to do (maybe ok idk) no its fine
  ~Mesh() = default;

  void draw(Shader &shader) const;

private:
  unsigned int VAO, VBO, EBO;
  void setup();
};
