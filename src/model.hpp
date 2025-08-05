#pragma once

#include "assimp/scene.h"
#include "mesh.hpp"
#include "shader.hpp"
#include "texture2D.hpp"
#include <glm/ext/matrix_transform.hpp>

struct ModelBuilder {
  bool flip_y = true;
};

struct Outline {
  glm::vec3 color;
  glm::vec3 scale;
};

struct Transform {
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 model;
};

struct RenderOptions {
  bool outline_enabled = false;
  Outline outline = Outline{glm::vec3(1.0), glm::vec3(1.0)};
};

class Model {
public:
  Model(const char *path, const ModelBuilder builder = {}) {
    this->load_model(path, builder);

    _outline.add_shader<VertexShader>("../src/shaders/model_vertex.glsl");
    _outline.add_shader<FragmentShader>(
        "../src/shaders/outline_models.frag.glsl");
    _outline.link();
  }
  ~Model() {
    for (Texture2D *tex_ptr : loaded_textures)
      delete tex_ptr;
  }

  void draw(const Shader &shader, const Transform &transfrom) {
    if (_options.outline_enabled) {
      glStencilFunc(GL_ALWAYS, 1, 0xFF);
      glStencilMask(0xFF);
      glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    }

    shader.set_uniform("view", transfrom.view);
    shader.set_uniform("projection", transfrom.projection);
    shader.set_uniform("model", transfrom.model);

    for (const Mesh &mesh : this->meshes)
      mesh.draw(shader);

    if (_options.outline_enabled) {
      glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
      glStencilMask(0x00);
      _outline.use();

      _outline.set_uniform("view", transfrom.view);
      _outline.set_uniform("projection", transfrom.projection);
      _outline.set_uniform("model",
                           glm::scale(transfrom.model, _options.outline.scale));
      _outline.set_uniform("outline_color", _options.outline.color);

      for (const Mesh &mesh : this->meshes)
        mesh.draw_without_texture();
      glStencilMask(0xFF);
      glStencilFunc(GL_ALWAYS, 1, 0xFF);
    }

    shader.use();
  }

  void set_render_options(RenderOptions options) { _options = options; }

private:
  std::vector<Mesh> meshes;
  std::vector<Texture2D *> loaded_textures;
  std::string dir;

  RenderOptions _options;
  Shader _outline;

  void load_model(const std::string &path, const ModelBuilder &builder);
  void process_node(aiNode *node, const aiScene *scene,
                    const ModelBuilder &builder);
  Mesh process_mesh(aiMesh *mesh, const aiScene *scene,
                    const ModelBuilder &builder);
  std::vector<Texture2D *> load_material_textures(aiMaterial *mat,
                                                  aiTextureType type);
};
