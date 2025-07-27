#include "mesh.hpp"
#include "shader.hpp"
#include "texture2D.hpp"
#include <cstddef>

void Mesh::draw(Shader &shader) const {
  for (unsigned int i = 0; i < textures.size(); ++i) {
    glActiveTexture(GL_TEXTURE0 + i);
    textures[i]->bind();
    if (textures[i]->get_type() == DIFFUSE) {
      shader.set_uniform("material.diffuse", static_cast<int>(i));
    } else if (textures[i]->get_type() == SPECULAR) {
      shader.set_uniform("material.specular", static_cast<int>(i));
    }
  }
  glBindVertexArray(this->VAO);
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->indices.size()),
                 GL_UNSIGNED_INT, 0);

  glBindVertexArray(0);
  glActiveTexture(GL_TEXTURE0);
}

void Mesh::setup() {
  glGenVertexArrays(1, &this->VAO);
  glGenBuffers(1, &this->VBO);
  glGenBuffers(1, &this->EBO);

  glBindVertexArray(this->VAO);

  glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
  glBufferData(
      GL_ARRAY_BUFFER,
      static_cast<unsigned int>(this->vertices.size() * sizeof(Vertex)),
      &this->vertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      static_cast<unsigned int>(this->indices.size() * sizeof(unsigned int)),
      &this->indices[0], GL_STATIC_DRAW);

  // Map position of vertex;
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        static_cast<void *>(0));
  // Map normal of vertex
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, normal)));
  // map Texture coordinate of vertex
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(
      2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      reinterpret_cast<void *>(offsetof(Vertex, texture_coordinate)));

  glBindVertexArray(0);
}
