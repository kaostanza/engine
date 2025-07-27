#pragma once

#include "assimp/scene.h"
#include "mesh.hpp"
#include <memory>

struct ModelBuilder {
  bool flip_y = true;
};

class Model {
public:
  Model(const char *path, const ModelBuilder builder = {}) {
    this->load_model(path, builder);
  }
  void draw(Shader &shader) {
    for (const Mesh &mesh : this->meshes) {
      mesh.draw(shader);
    }
  };

private:
  std::vector<Mesh> meshes;
  std::vector<std::unique_ptr<Texture2D>> loaded_textures;
  std::string dir;

  void load_model(const std::string &path, const ModelBuilder &builder);
  void process_node(aiNode *node, const aiScene *scene,
                    const ModelBuilder &builder);
  Mesh process_mesh(aiMesh *mesh, const aiScene *scene,
                    const ModelBuilder &builder);
  std::vector<Texture2D *> load_material_textures(aiMaterial *mat,
                                                  aiTextureType type);
};
