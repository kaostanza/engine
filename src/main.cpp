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
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <algorithm>
#include <iostream>
#include <ostream>

#include "camera.hpp"
#include "light.hpp"
#include "model.hpp"
#include "shader.hpp"

enum class DepthMode {
  None = 0,
  Linear = 1,
  NonLinear = 2,
};

// SCREEN + FOV
int WIDTH = 800;
int HEIGHT = 600;
double FOV = 45.0;

// FLY CAMERA
FlyCamera P_CAMERA(glm::vec3(0.0, 0.0, 3.0), FOV);
float X_POS = static_cast<float>(WIDTH) / 2;
float Y_POS = static_cast<float>(HEIGHT) / 2;
float NEAR_PLANE = 0.1f;
float FAR_PLANE = 100.0f;
bool IS_FOCUS = true;
bool IS_NEW_FOCUS = true;

// MATRIX TRANSFORM
const auto IDENTITY = glm::mat4(1.0);
auto VIEW = P_CAMERA.looking_at();
auto PROJECTION =
    glm::perspective(glm::radians(static_cast<float>(P_CAMERA.get_fov())),
                     static_cast<float>(WIDTH) / static_cast<float>(HEIGHT),
                     NEAR_PLANE, FAR_PLANE);

// Time related stuff
double TIME = 0;
double LAST_TIME = 0;
double DELTA = 0;

// Basic K_LINEAR + quadratic + constant for pointlight attenuation
// constexpr float K_CONSTANT = 1;
// constexpr float K_LINEAR = 0.09f;
// constexpr float K_QUADRATIC = 0.032f;

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
    std::vector<PointLight> point_lights{};
    std::vector<DirectionalLight> directionnal_lights{};
    std::vector<SpotLight> spot_lights{};

    Shader non_linear_depth_program;
    non_linear_depth_program.add_shader<VertexShader>(
        "../src/shaders/model_vertex.glsl");
    non_linear_depth_program.add_shader<FragmentShader>(
        "../src/shaders/non_linear_depth.frag.glsl");
    non_linear_depth_program.link();

    Shader linear_depth_program;
    linear_depth_program.add_shader<VertexShader>(
        "../src/shaders/model_vertex.glsl");
    linear_depth_program.add_shader<FragmentShader>(
        "../src/shaders/linear_depth.frag.glsl");
    linear_depth_program.link();
    linear_depth_program.use();
    linear_depth_program.set_uniform("near", NEAR_PLANE);
    linear_depth_program.set_uniform("far", FAR_PLANE);

    Shader model_shader_program;
    model_shader_program.add_shader<VertexShader>(
        "../src/shaders/model_vertex.glsl");
    model_shader_program.add_shader<FragmentShader>(
        "../src/shaders/model_fragment.glsl");
    model_shader_program.link();

    Shader *shader_in_use = &model_shader_program;

    Model sponza("../assets/models/sponza/scene.gltf");

    // Wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // Default mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // VSYNC (1 = ON, 0 = OFF)
    glfwSwapInterval(1);

    unsigned int frame_c = 0;
    const auto radius = 10.0;

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    const char *depth_options[] = {"None", "Linear", "Non-Linear"};
    bool wireframe_mode = false;
    int depth_mode_option = 0;

    while (glfwWindowShouldClose(window) == 0) {
      LAST_TIME = TIME;
      TIME = glfwGetTime();
      DELTA = TIME - LAST_TIME;

      const auto fps = 1 / DELTA;
      if (frame_c++ % 30 == 0)
        std::cout << "FPS: " << fps << std::endl;

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      {
        ImGui::Begin("Kaos Engine");

        if (ImGui::Checkbox("Wireframe:", &wireframe_mode)) {
          if (wireframe_mode)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
          else
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        if (ImGui::Combo("Depth Mode", &depth_mode_option, depth_options,
                         sizeof((depth_options)) / sizeof(depth_options[0]))) {

          switch (static_cast<DepthMode>(depth_mode_option)) {
          case DepthMode::None:
            shader_in_use = &model_shader_program;
            break;
          case DepthMode::Linear:
            shader_in_use = &linear_depth_program;
            break;
          case DepthMode::NonLinear:
            shader_in_use = &non_linear_depth_program;
            break;
          }
        }

        ImGui::End();
      }

      process_input(window);

      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      VIEW = P_CAMERA.looking_at();

      // make point lights move in a circle
      for (unsigned int i = 0; i < point_lights.size(); i++) {
        point_lights[i].position.x = static_cast<float>(sin(TIME + i) * radius);
        point_lights[i].position.z = static_cast<float>(cos(TIME + i) * radius);
      }

      // Backpack
      shader_in_use->use();

      // Give the transform matrix
      shader_in_use->set_uniform("view", VIEW);
      shader_in_use->set_uniform("projection", PROJECTION);
      shader_in_use->set_uniform(
          "model", glm::translate(glm::scale(IDENTITY, glm::vec3(0.01f)),
                                  glm::vec3(0.0, 0.0, 0.0)));

      shader_in_use->set_uniform("camera_pos", P_CAMERA.get_position());

      // Give light settings (disabled for now)
      shader_in_use->set_uniform(
          "point_lights_count", static_cast<unsigned int>(point_lights.size()));
      shader_in_use->set_uniform(
          "directionnal_lights_count",
          static_cast<unsigned int>(directionnal_lights.size()));
      shader_in_use->set_uniform("spot_lights_count",
                                 static_cast<unsigned int>(spot_lights.size()));

      unsigned int i = 0;
      for (const auto &point_light : point_lights) {
        auto str = "point_lights[" + std::to_string(i) + "]";
        shader_in_use->set_uniform_struct(str.data(), point_light);
        i++;
      }

      i = 0;
      for (const auto &directionnal_light : directionnal_lights) {
        auto str = "directionnal_lights[" + std::to_string(i) + "]";
        shader_in_use->set_uniform_struct(str.data(), directionnal_light);
        i++;
      }

      i = 0;
      for (const auto &spot_light : spot_lights) {
        auto str = "spot_lights[" + std::to_string(i) + "]";
        shader_in_use->set_uniform_struct(str.data(), spot_light);
        i++;
      }

      // give the camera position for lightining calculation
      shader_in_use->set_uniform("material.shininess", 32.0f);
      shader_in_use->set_uniform("material.emission", 2);
      sponza.draw(*shader_in_use);

      const auto gl_error = glGetError();
      if (gl_error != GL_NO_ERROR) {
        std::cout << "Erreur: Impossible de render\n"
                  << "-> " << gl_error << std::endl;
      }

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
    IS_FOCUS = true;
    IS_NEW_FOCUS = true;
  }

  if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    IS_FOCUS = false;
  }

  const auto speed = 10.0;
  const auto cam_speed = DELTA * speed;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    auto pos = P_CAMERA.get_position();
    pos += P_CAMERA.forward() * glm::vec3(static_cast<float>(cam_speed));
    P_CAMERA.set_position(pos);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    auto pos = P_CAMERA.get_position();
    pos -= P_CAMERA.forward() * glm::vec3(static_cast<float>(cam_speed));
    P_CAMERA.set_position(pos);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    auto pos = P_CAMERA.get_position();
    pos -= P_CAMERA.right() * glm::vec3(static_cast<float>(cam_speed));
    P_CAMERA.set_position(pos);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    auto pos = P_CAMERA.get_position();
    pos += P_CAMERA.right() * glm::vec3(static_cast<float>(cam_speed));
    P_CAMERA.set_position(pos);
  }
}

void resize_window_callback(GLFWwindow *, int width, int height) {
  WIDTH = width;
  HEIGHT = height;
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *, double x, double y) {
  if (!IS_FOCUS || IS_NEW_FOCUS) {
    X_POS = static_cast<float>(x);
    Y_POS = static_cast<float>(y);
    IS_NEW_FOCUS = false;
    return;
  }

  auto x_offset = x - X_POS;
  auto y_offset = y - Y_POS;

  X_POS = static_cast<float>(x);
  Y_POS = static_cast<float>(y);

  const auto sensitivity = 0.01;
  x_offset *= sensitivity;
  y_offset *= -sensitivity;
  P_CAMERA.set_yaw(P_CAMERA.get_yaw() + static_cast<float>(x_offset));
  P_CAMERA.set_pitch(std::clamp(
      P_CAMERA.get_pitch() + static_cast<float>(y_offset), -89.9f, 89.9f));
}

void scroll_callback(GLFWwindow *, double, double y) {
  P_CAMERA.set_fov(std::clamp(P_CAMERA.get_fov() - y, 1.0, 45.0));
  PROJECTION =
      glm::perspective(glm::radians(static_cast<float>(P_CAMERA.get_fov())),
                       static_cast<float>(WIDTH) / static_cast<float>(HEIGHT),
                       NEAR_PLANE, FAR_PLANE);
}
