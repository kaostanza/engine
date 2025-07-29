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
#include "light.hpp"
#include "model.hpp"
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

// MATRIX TRANSFORM
const auto identity = glm::mat4(1.0);
auto view = p_camera.looking_at();
auto projection = glm::perspective(
    glm::radians(static_cast<float>(p_camera.get_fov())),
    static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 100.0f);

// Time related stuff
double TIME = 0;
double LAST_TIME = 0;
double DELTA = 0;

// Basic k_linear + quadratic + constant for pointlight attenuation
constexpr float k_constant = 1;
constexpr float k_linear = 0.09f;
constexpr float k_quadratic = 0.032f;

void process_input(GLFWwindow *window);
void resize_window_callback(GLFWwindow *window, int x, int y);
void mouse_callback(GLFWwindow *window, double x, double y);
void scroll_callback(GLFWwindow *window, double x, double y);

int main() {
  if (FcInit() == 0) {
    std::cerr << "Erreur: Impossible d'init fontconfig\n";
    return 1;
  }
  if (glfwInit() == 0) {
    std::cerr << "Erreur: Impossible d'init glfw\n";
    return 1;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  const auto window =
      glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);

  if (window == nullptr) {
    std::cerr << "Erreur: Impossible de créer une Window avec OpenGL\n";
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, resize_window_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) ==
      0) {
    std::cerr << "Erreur: Impossible de load via glad\n";
    return 1;
  }

  // To run destructor before we reach glfwTerminate() at the end of main
  // (avoid seg fault)
  {

    std::vector<PointLight> point_lights{PointLight{glm::vec3(5.0f, 0.0f, 5.0f),
                                                    LightInfo{
                                                        glm::vec3(0.1f),
                                                        glm::vec3(0.5f),
                                                        glm::vec3(1.0f),
                                                    },

                                                    AttenuationInfo{
                                                        k_constant,
                                                        k_linear,
                                                        k_quadratic,
                                                    }

    }};
    std::vector<DirectionalLight> directionnal_lights;
    std::vector<SpotLight> spot_lights;

    Model backpack("../assets/models/backpack/backpack.obj");

    Shader model_shader_program;
    model_shader_program.add_shader<VertexShader>(
        "../src/shaders/model_vertex.glsl");
    model_shader_program.add_shader<FragmentShader>(
        "../src/shaders/model_fragment.glsl");
    model_shader_program.link();

    // Wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // Default mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // VSYNC (1 = ON, 0 = OFF)
    glfwSwapInterval(1);

    unsigned int frame_c = 0;
    double smoothed_fps = 165.0f;
    const auto alpha = 0.1f;
    const auto radius = 10.0;

    while (glfwWindowShouldClose(window) == 0) {
      LAST_TIME = TIME;
      TIME = glfwGetTime();
      DELTA = TIME - LAST_TIME;

      const auto fps = 1 / DELTA;
      smoothed_fps = alpha * fps + (1.0f - alpha) * smoothed_fps;
      if (frame_c++ % 30 == 0)
        std::cout << "FPS: " << smoothed_fps << std::endl;

      process_input(window);

      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      view = p_camera.looking_at();

      // make point lights move in a circle
      for (unsigned int i = 0; i < point_lights.size(); i++) {
        point_lights[i].position.x = static_cast<float>(sin(TIME + i) * radius);
        point_lights[i].position.z = static_cast<float>(cos(TIME + i) * radius);
      }

      // Backpack
      model_shader_program.use();

      // Give the transform matrix
      model_shader_program.set_uniform("view", view);
      model_shader_program.set_uniform("projection", projection);
      model_shader_program.set_uniform(
          "model", glm::translate(glm::scale(identity, glm::vec3(1.0f)),
                                  glm::vec3(0.0, 0.0, 0.0)));

      model_shader_program.set_uniform("camera_pos", p_camera.get_position());

      // Give light settings (disabled for now)
      model_shader_program.set_uniform(
          "point_lights_count", static_cast<unsigned int>(point_lights.size()));
      model_shader_program.set_uniform(
          "directionnal_lights_count",
          static_cast<unsigned int>(directionnal_lights.size()));
      model_shader_program.set_uniform(
          "spot_lights_count", static_cast<unsigned int>(spot_lights.size()));

      unsigned int i = 0;
      for (const auto &point_light : point_lights) {
        auto str = "point_lights[" + std::to_string(i) + "]";
        model_shader_program.set_uniform_struct(str.data(), point_light);
        i++;
      }

      i = 0;
      for (const auto &directionnal_light : directionnal_lights) {
        auto str = "directionnal_lights[" + std::to_string(i) + "]";
        model_shader_program.set_uniform_struct(str.data(), directionnal_light);
        i++;
      }

      i = 0;
      for (const auto &spot_light : spot_lights) {
        auto str = "spot_lights[" + std::to_string(i) + "]";
        model_shader_program.set_uniform_struct(str.data(), spot_light);
        i++;
      }

      // give the camera position for lightining calculation
      model_shader_program.set_uniform("material.shininess", 32.0f);
      model_shader_program.set_uniform("material.emission", 2);
      backpack.draw(model_shader_program);

      const auto gl_error = glGetError();
      if (gl_error != GL_NO_ERROR) {
        std::cout << "Erreur: Impossible de render\n"
                  << "-> " << gl_error << std::endl;
      }

      glfwSwapBuffers(window);
      glfwPollEvents();
    }
  }

  glfwTerminate();
  FcFini();
  std::cout << "Fin du programme\n";
  return 0;
}

void process_input(GLFWwindow *window) {
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
  glm::perspective(glm::radians(static_cast<float>(p_camera.get_fov())),
                   static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f,
                   100.0f);
}
