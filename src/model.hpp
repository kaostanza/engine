#pragma once

#include "assimp/scene.h"
#include "mesh.hpp"
#include "texture2D.hpp"

struct ModelBuilder {
  bool flip_y = true;
};

class Model {
public:
  Model(const char *path, const ModelBuilder builder = {}) {
    this->load_model(path, builder);
  }
  ~Model() {
    for (Texture2D *tex_ptr : loaded_textures)
      delete tex_ptr;
  }
  void draw(Shader &shader) {
    for (const Mesh &mesh : this->meshes)
      mesh.draw(shader);
  };

private:
  std::vector<Mesh> meshes;
  std::vector<Texture2D *> loaded_textures;
  std::string dir;

  void load_model(const std::string &path, const ModelBuilder &builder);
  void process_node(aiNode *node, const aiScene *scene,
                    const ModelBuilder &builder);
  Mesh process_mesh(aiMesh *mesh, const aiScene *scene,
                    const ModelBuilder &builder);
  std::vector<Texture2D *> load_material_textures(aiMaterial *mat,
                                                  aiTextureType type);
};
