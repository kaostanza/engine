#include "model.hpp"
#include "assimp/Importer.hpp"
#include "assimp/material.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "texture2D.hpp"
#include <chrono>
#include <cstring>
#include <memory>
#include <stdexcept>

void Model::load_model(const std::string &path, const ModelBuilder &builder) {
  Assimp::Importer importer;
  auto start = std::chrono::high_resolution_clock::now();
  const aiScene *scene =
      importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    throw std::runtime_error(
        "Erreur: (ASSIMP) Impossible de load le modele: " + path + "\n -> " +
        importer.GetErrorString() + "\n");
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Load time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     start)
                   .count()
            << "ms\n";

  this->dir = path.substr(0, path.find_last_of('/'));
  this->process_node(scene->mRootNode, scene, builder);
}

void Model::process_node(aiNode *node, const aiScene *scene,
                         const ModelBuilder &builder) {
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    this->meshes.push_back(this->process_mesh(mesh, scene, builder));
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++)
    this->process_node(node->mChildren[i], scene, builder);
}

Mesh Model::process_mesh(aiMesh *mesh, const aiScene *scene,
                         const ModelBuilder &builder) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture2D *> textures;

  // process vertices
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    vertex.position.x = mesh->mVertices[i].x;
    vertex.position.y = mesh->mVertices[i].y;
    vertex.position.z = mesh->mVertices[i].z;

    vertex.normal.x = mesh->mNormals[i].x;
    vertex.normal.y = mesh->mNormals[i].y;
    vertex.normal.z = mesh->mNormals[i].z;

    if (mesh->mTextureCoords[0]) {
      vertex.texture_coordinate.x = mesh->mTextureCoords[0][i].x;
      vertex.texture_coordinate.y =
          (builder.flip_y ? -1 : 1) * mesh->mTextureCoords[0][i].y;
    } else {
      vertex.texture_coordinate = glm::vec2(0.0f);
    }

    vertices.push_back(std::move(vertex));
  }

  // process indices
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }

  // process textures
  aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
  std::vector<Texture2D *> diffuse_maps =
      this->load_material_textures(material, aiTextureType_DIFFUSE);
  std::vector<Texture2D *> specular_maps =
      this->load_material_textures(material, aiTextureType_SPECULAR);

  textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
  textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

  return Mesh(vertices, indices, textures);
}

std::vector<Texture2D *> Model::load_material_textures(aiMaterial *material,
                                                       aiTextureType type) {
  std::vector<Texture2D *> textures;
  for (unsigned int i = 0; i < material->GetTextureCount(type); i++) {
    aiString path;
    material->GetTexture(type, i, &path);

    if (std::find_if(this->loaded_textures.begin(), this->loaded_textures.end(),
                     [path](const auto &t) {
                       return !std::strcmp(path.C_Str(),
                                           t->get_local_path().data());
                     }) != this->loaded_textures.end()) {
      // std::cout << "Already loaded texture! " << path.C_Str() << std::endl;
      continue;
    }

    Texture2DBuilder builder;
    switch (type) {
    case aiTextureType_DIFFUSE:
      builder.type = TextureType::DIFFUSE;
      break;
    case aiTextureType_SPECULAR:
      builder.type = TextureType::SPECULAR;
      break;
    default:
      break;
    }

    auto tex_start = std::chrono::high_resolution_clock::now();
    std::unique_ptr<Texture2D> texture = std::make_unique<Texture2D>(
        (this->dir + "/" + path.C_Str()).data(), builder);
    auto tex_end = std::chrono::high_resolution_clock::now();
    std::cout << "Texture load time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     tex_end - tex_start)
                     .count()
              << "ms (" << path.C_Str() << ")\n";

    textures.push_back(texture.get());
    this->loaded_textures.push_back(std::move(texture));
  }
  return textures;
}
