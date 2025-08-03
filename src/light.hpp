#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Basic K_LINEAR + quadratic + constant for pointlight attenuation
constexpr float K_CONSTANT = 1;
constexpr float K_LINEAR = 0.09f;
constexpr float K_QUADRATIC = 0.032f;

struct LightInfo {
  glm::vec3 ambient;
  glm::vec3 specular;
  glm::vec3 diffuse;
};

struct AttenuationInfo {
  float constant = K_CONSTANT;
  float linear = K_LINEAR;
  float quadratic = K_QUADRATIC;
};

struct PointLight {
  glm::vec3 position;

  LightInfo info;
  AttenuationInfo attenuation_info;
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
  AttenuationInfo attenuation_info;
};
