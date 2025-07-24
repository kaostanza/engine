#include "glad/glad.h"
#include "glm/detail/func_trigonometric.hpp"
#include "glm/detail/type_mat.hpp"
#include "glm/detail/type_vec.hpp"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <fontconfig/fontconfig.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <iostream>
#include <ostream>

#include "camera.hpp"
#include "shader.hpp"
#include "texture2D.hpp"

// SCREEN + FOV
int WIDTH = 800;
int HEIGHT = 600;
double FOV = 45.0;

// FLY CAMERA
FlyCamera p_camera(glm::vec3(0.0, 0.0, 3.0), FOV);
float X_POS = static_cast<float>(WIDTH) / 2;
float Y_POS = static_cast<float>(HEIGHT) / 2;
// Time related stuff
double TIME = 0;
double LAST_TIME = 0;
double DELTA = 0;

// Basic k_linear + quadratic + constant for pointlight attenuation
constexpr float k_constant = 1;
constexpr float k_linear = 0.09f;
constexpr float k_quadratic = 0.032f;

// NORMAL OBJECTS;
const float normal_cube_vertices[]{
    // bottom face // COLOR         // TEXTURE
    -0.5, -0.5, -0.5, 1.0, 0.0, 0.0, 0.0, 0.0, // bottom left
    0.5, -0.5, -0.5, 0.0, 1.0, 0.0, 1.0, 0.0,  // bottom right
    -0.5, 0.5, -0.5, 0.0, 0.0, 1.0, 0.0, 1.0,  // top left
    0.5, 0.5, -0.5, 1.0, 0.0, 0.0, 1.0, 1.0,   // top right
    // top face
    -0.5, -0.5, 0.5, 1.0, 0.0, 0.0, 0.0, 0.0, // bottom left
    0.5, -0.5, 0.5, 0.0, 1.0, 0.0, 1.0, 0.0,  // bottom right
    -0.5, 0.5, 0.5, 0.0, 0.0, 1.0, 0.0, 1.0,  // top left
    0.5, 0.5, 0.5, 1.0, 0.0, 0.0, 1.0, 1.0,   // top right
};

float light_cube_vertices[] = {
    // bottom face
    -0.5, -0.5, -0.5, 1.0, 1.0, 1.0, 0.0, 0.0, // bottom left
    0.5, -0.5, -0.5, 1.0, 1.0, 1.0, 1.0, 0.0,  // bottom right
    -0.5, 0.5, -0.5, 1.0, 1.0, 1.0, 0.0, 1.0,  // top left
    0.5, 0.5, -0.5, 1.0, 1.0, 1.0, 1.0, 1.0,   // top right
    // top face
    -0.5, -0.5, 0.5, 1.0, 1.0, 1.0, 0.0, 0.0, // bottom left
    0.5, -0.5, 0.5, 1.0, 1.0, 1.0, 1.0, 0.0,  // bottom right
    -0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 0.0, 1.0,  // top left
    0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 1.0, 1.0,   // top right
};

const unsigned int indices[] = {
    // bottom face
    0,
    1,
    3,
    3,
    2,
    0,
    // top face
    4,
    5,
    7,
    7,
    6,
    4,
    // left face
    0,
    4,
    2,
    2,
    6,
    4,
    // right face
    1,
    5,
    3,
    3,
    7,
    5,
    // back face
    6,
    2,
    3,
    3,
    7,
    6,
    // front face
    4,
    0,
    1,
    1,
    5,
    4,
};

const glm::vec3 normal_cubes_positions[] = {
    glm::vec3(0.0, 0.0, 0.0), glm::vec3(2.0),      glm::vec3(-2.0),
    glm::vec3(-2, 2, 0),      glm::vec3(2, -2, 0), glm::vec3(10, 0, 10)};
glm::vec3 light_cubes_positions[] = {
    glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f, 2.0f, -12.0f), glm::vec3(0.0f, 0.0f, -3.0f)};
glm::vec3 base_light_cubes_positions[] = {
    glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f, 2.0f, -12.0f), glm::vec3(0.0f, 0.0f, -3.0f)};

void processInput(GLFWwindow *window);
void resize_window_callback(GLFWwindow *window, int x, int y);
void mouse_callback(GLFWwindow *window, double x, double y);
void scroll_callback(GLFWwindow *window, double x, double y);

int main() {
  if (FcInit() == 0) {
    std::cerr << "Erreur: Impossible d'init fontconfig\n";
    return 1;
  }
  if (glfwInit() == 0) {
    std::cout << "Erreur: Impossible d'init glfw\n";
    return 1;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  const auto window =
      glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);

  if (window == nullptr) {
    std::cout << "Erreur: Impossible de créer une Window avec OpenGL\n";
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, resize_window_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) ==
      0) {
    std::cout << "Erreur: Impossible de load via glad\n";
    return 1;
  }

  Shader shader_program;
  shader_program.add_shader<VertexShader>("../src/shaders/vertex.glsl");
  shader_program.add_shader<FragmentShader>("../src/shaders/fragment.glsl");

  shader_program.link();

  Shader light_shader_program;
  light_shader_program.add_shader<VertexShader>("../src/shaders/vertex.glsl");
  light_shader_program.add_shader<FragmentShader>(
      "../src/shaders/light_fragment.glsl");
  light_shader_program.link();

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);

  unsigned int VBO;
  glGenBuffers(1, &VBO);
  unsigned int EBO;
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(normal_cube_vertices),
               &normal_cube_vertices[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0],
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        static_cast<void *>(0));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        reinterpret_cast<void *>(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        reinterpret_cast<void *>(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  Texture2DBuilder rgba;
  rgba.format = Format::RGBA;

  Texture2DBuilder rgb;
  rgb.format = Format::RGB;

  Texture2D container_diffuse_map("../assets/textures/container2.png", rgba);
  Texture2D container_specular_map("../assets/textures/container2_specular.png",
                                   rgba);
  Texture2D container_emission_map("../assets/textures/matrix.jpg", rgb);

  unsigned int LIGHT_VAO;
  glGenVertexArrays(1, &LIGHT_VAO);

  unsigned int LIGHT_VBO;
  glGenBuffers(1, &LIGHT_VBO);
  unsigned int LIGHT_EBO;
  glGenBuffers(1, &LIGHT_EBO);

  glBindVertexArray(LIGHT_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, LIGHT_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(light_cube_vertices),
               &light_cube_vertices[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, LIGHT_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0],
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        static_cast<void *>(0));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        reinterpret_cast<void *>(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        reinterpret_cast<void *>(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  auto model = glm::mat4(1.0);
  auto view = p_camera.looking_at();
  auto projection = glm::perspective(
      glm::radians(static_cast<float>(FOV)),
      static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 100.0f);

  auto spot_light_diffuse = glm::vec3(0.5f);
  auto spot_light_specular = glm::vec3(0.5f);
  auto spot_light_ambient = glm::vec3(0.01f);

  auto directionnal_light_diffuse = glm::vec3(0.5f);
  auto directionnal_light_specular = glm::vec3(1.0f);
  auto directionnal_light_ambient = glm::vec3(0.2f);

  auto point_light_diffuse = glm::vec3(1.0f);
  auto point_light_specular = glm::vec3(0.5f);
  auto point_light_ambient = glm::vec3(0.01f);

  shader_program.use();
  shader_program.set_uniform("model", model);
  shader_program.set_uniform("view", view);
  shader_program.set_uniform("projection", projection);

  light_shader_program.use();
  light_shader_program.set_uniform("model", model);
  light_shader_program.set_uniform("view", view);
  light_shader_program.set_uniform("projection", projection);

  // Wireframe mode
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  // Default mode
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glEnable(GL_DEPTH_TEST);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  while (glfwWindowShouldClose(window) == 0) {
    LAST_TIME = TIME;
    TIME = glfwGetTime();
    DELTA = TIME - LAST_TIME;
    // std::cout << "FPS: " << 1 / DELTA << std::endl;

    processInput(window);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    container_diffuse_map.bind();

    glActiveTexture(GL_TEXTURE1);
    container_specular_map.bind();

    // make light cube move
    const float radius = 105.0;

    for (unsigned int i = 0;
         i < sizeof(light_cubes_positions) / sizeof(light_cubes_positions[0]);
         i++) {
      auto &base_pos = base_light_cubes_positions[i];
      auto &pos = light_cubes_positions[i];
      const double x = base_pos.x + sin(TIME + i) * radius;
      const double z = base_pos.z + cos(TIME + i) * radius;

      pos.x = static_cast<float>(x);
      pos.z = static_cast<float>(z);
    }

    view = p_camera.looking_at();
    projection = glm::perspective(
        glm::radians(static_cast<float>(p_camera.get_fov())),
        static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 100.0f);

    glBindVertexArray(VAO);
    shader_program.use();
    shader_program.set_uniform("view", view);
    shader_program.set_uniform("view_pos", p_camera.get_position());
    shader_program.set_uniform_struct(
        "spot_light",
        SpotLight{p_camera.get_position(), p_camera.forward(),
                  static_cast<float>(glm::cos(glm::radians(12.5))),
                  static_cast<float>(glm::cos(glm::radians(17.5))),
                  LightInfo{
                      spot_light_ambient,
                      spot_light_specular,
                      spot_light_diffuse,
                  },
                  AttenuationInfo{
                      1.0f,
                      0.09f,
                      0.032f,
                  }});

    shader_program.set_uniform_struct(
        "directionnal_light", DirectionalLight{glm::vec3(0, -1, 0), // direction
                                               LightInfo{
                                                   directionnal_light_ambient,
                                                   directionnal_light_specular,
                                                   directionnal_light_diffuse,
                                               }});

    int i = 0;
    for (const auto &point_light_position : light_cubes_positions) {
      std::string struct_name = "point_lights[" + std::to_string(i) + "]";
      shader_program.set_uniform_struct(struct_name,
                                        PointLight{point_light_position,
                                                   LightInfo{
                                                       point_light_ambient,
                                                       point_light_specular,
                                                       point_light_diffuse,
                                                   },

                                                   AttenuationInfo{
                                                       1.0f,
                                                       0.09f,
                                                       0.032f,
                                                   }

                                        });

      i++;
    }

    shader_program.set_uniform("material.diffuse", 0);
    shader_program.set_uniform("material.specular", 1);
    shader_program.set_uniform("material.emission", 2);
    shader_program.set_uniform("material.shininess", 32.0f);

    shader_program.set_uniform("projection", projection);
    for (const auto &position : normal_cubes_positions) {
      float rotation_speed = 0.5f + abs(position.x + position.y) * 0.3f;
      glm::vec3 rotation_axis = glm::normalize(
          glm::vec3(sin(position.x), cos(position.y), sin(position.z)));

      auto model_ =
          glm::rotate(glm::translate(glm::mat4(1.0), position),
                      static_cast<float>(TIME) * rotation_speed, rotation_axis);

      shader_program.set_uniform("model", model_);
      glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned int),
                     GL_UNSIGNED_INT, nullptr);
    }

    glBindVertexArray(LIGHT_VAO);

    light_shader_program.use();
    light_shader_program.set_uniform("view", view);
    light_shader_program.set_uniform("projection", projection);
    light_shader_program.set_uniform("lightColor", glm::vec3(1.0));
    for (const auto &position : light_cubes_positions) {
      light_shader_program.set_uniform(
          "model", glm::translate(glm::mat4(1.0f), position));
      glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned int),
                     GL_UNSIGNED_INT, nullptr);
    }

    const auto gl_error = glGetError();
    if (gl_error != GL_NO_ERROR) {
      std::cout << "Erreur: Impossible de render\n"
                << "-> " << gl_error << std::endl;
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteVertexArrays(1, &LIGHT_VAO);
  glDeleteBuffers(1, &LIGHT_VBO);
  glDeleteBuffers(1, &LIGHT_EBO);

  container_diffuse_map.deinit();
  shader_program.deinit();
  light_shader_program.deinit();
  glfwTerminate();
  FcFini();
  std::cout << "Fin du programme\n";
  return 0;
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, 1);
  }

  if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }

  if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }

  const auto speed = 10.0;
  const auto cam_speed = DELTA * speed;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    auto pos = p_camera.get_position();
    pos += p_camera.forward() * glm::vec3(static_cast<float>(cam_speed));
    p_camera.set_position(pos);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    auto pos = p_camera.get_position();
    pos -= p_camera.forward() * glm::vec3(static_cast<float>(cam_speed));
    p_camera.set_position(pos);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    auto pos = p_camera.get_position();
    pos -= p_camera.right() * glm::vec3(static_cast<float>(cam_speed));
    p_camera.set_position(pos);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    auto pos = p_camera.get_position();
    pos += p_camera.right() * glm::vec3(static_cast<float>(cam_speed));
    p_camera.set_position(pos);
  }
}

void resize_window_callback(GLFWwindow *, int width, int height) {
  WIDTH = width;
  HEIGHT = height;
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *, double x, double y) {
  auto x_offset = x - X_POS;
  auto y_offset = y - Y_POS;

  X_POS = static_cast<float>(x);
  Y_POS = static_cast<float>(y);

  const auto sensitivity = 0.01;
  x_offset *= sensitivity;
  y_offset *= -sensitivity;
  p_camera.set_yaw(p_camera.get_yaw() + static_cast<float>(x_offset));
  p_camera.set_pitch(std::clamp(
      p_camera.get_pitch() + static_cast<float>(y_offset), -89.9f, 89.9f));
}

void scroll_callback(GLFWwindow *, double, double y) {
  p_camera.set_fov(std::clamp(p_camera.get_fov() - y, 1.0, 45.0));
}
