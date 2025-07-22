#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct LightInfo {
  glm::vec3 ambient;
  glm::vec3 specular;
  glm::vec3 diffuse;
};

struct PointLight {
  glm::vec3 position;

  LightInfo info;
};

struct DirectionalLight {
  glm::vec3 direction;
  LightInfo info;
};

struct SpotLight {
  glm::vec3 position;
  glm::vec3 direction;
  float inner_cut_off;
  float outer_cut_off;

  LightInfo info;
};
