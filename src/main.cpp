#include "glad/glad.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
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

enum class RenderMode {
  None = 0,
  Normal,
  Depth_Linear,
  Depth_NonLinear,
};

// SCREEN + FOV
int WIDTH = 1920;
int HEIGHT = 1080;
constexpr float DEFAULT_FOV = 45.0f;

// FLY CAMERA
FlyCamera P_CAMERA(glm::vec3(0.0, 0.0, 3.0), DEFAULT_FOV);
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

void process_input(GLFWwindow *window);
void resize_window_callback(GLFWwindow *window, int x, int y);
void mouse_callback(GLFWwindow *window, double x, double y);
void scroll_callback(GLFWwindow *window, double x, double y);

void redefine_projection_matrix();

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

    Shader normal_shader_program;
    normal_shader_program.add_shader<VertexShader>(
        "../src/shaders/model_vertex.glsl");

    normal_shader_program.add_shader<FragmentShader>(
        "../src/shaders/normal.frag.glsl");
    normal_shader_program.link();

    Shader outline_model_shader_program;
    outline_model_shader_program.add_shader<VertexShader>(
        "../src/shaders/model_vertex.glsl");
    outline_model_shader_program.add_shader<FragmentShader>(
        "../src/shaders/outline_models.frag.glsl");
    outline_model_shader_program.link();

    Shader *shader_in_use = &model_shader_program;

    Model sponza("../assets/models/backpack/backpack.obj");

    // Wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // Default mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // VSYNC (1 = ON, 0 = OFF)
    glfwSwapInterval(1);

    unsigned int frame_c = 0;
    double fps = 165.0f;

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    const char *depth_options[] = {"None", "Normal", "Depth_Linear",
                                   "Depth_Non-Linear"};
    bool wireframe_mode = false;
    RenderOptions render_options;
    int depth_mode_option = 0;

    while (glfwWindowShouldClose(window) == 0) {
      LAST_TIME = TIME;
      TIME = glfwGetTime();
      DELTA = TIME - LAST_TIME;

      if (frame_c++ % 30 == 0)
        fps = 1 / DELTA;

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      {
        ImGui::Begin("Kaos Engine");
        float fov = P_CAMERA.get_fov();

        if (ImGui::CollapsingHeader("Debug")) {
          ImGui::Text("FPS: %f", fps);
        }

        if (ImGui::CollapsingHeader("Rendering")) {
          if (ImGui::Checkbox("Wireframe:", &wireframe_mode)) {
            if (wireframe_mode)
              glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else
              glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
          }

          if (ImGui::Combo("Depth Mode", &depth_mode_option, depth_options,
                           sizeof((depth_options)) /
                               sizeof(depth_options[0]))) {

            switch (static_cast<RenderMode>(depth_mode_option)) {
            case RenderMode::None:
              shader_in_use = &model_shader_program;
              break;
            case RenderMode::Normal:
              shader_in_use = &normal_shader_program;
              break;
            case RenderMode::Depth_Linear:
              shader_in_use = &linear_depth_program;
              break;
            case RenderMode::Depth_NonLinear:
              shader_in_use = &non_linear_depth_program;
              break;
            }
          }

          bool outlining;
          if (ImGui::Checkbox("Outline", &outlining)) {
            if (outlining) {
              render_options = RenderOptions{
                  true, Outline{glm::vec3(1.0, 0.0, 0.0), glm::vec3(1.01f)}};
            } else {
              render_options = RenderOptions();
            }
          }
        }

        if (ImGui::CollapsingHeader("Player")) {
          auto pos = P_CAMERA.get_position();
          ImGui::Text("Position:");
          if (ImGui::DragFloat3("##pos", &pos.x, 0.1f)) {
            P_CAMERA.set_position(pos);
          }

          if (ImGui::TreeNode("Camera")) {
            if (ImGui::SliderFloat("FOV", &fov, 1.0f, 45.0f)) {
              std::cout << "New FOV is " << fov << std::endl;
              P_CAMERA.set_fov(fov);
              redefine_projection_matrix();
            }

            if (ImGui::SliderFloat("Near", &NEAR_PLANE, 0.1f, 1000.0f))
              redefine_projection_matrix();

            if (ImGui::SliderFloat("Far", &FAR_PLANE, 0.1f, 10000.0f))
              redefine_projection_matrix();

            ImGui::TreePop();
          }
        }

        if (ImGui::CollapsingHeader("Lightning")) {
          if (ImGui::TreeNode("Point Lights")) {
            uint i = 0;
            for (auto &l : point_lights) {
              std::string light_label = "Light " + std::to_string(i);

              if (ImGui::TreeNode(light_label.c_str())) {
                ImGui::Text("Position:");
                ImGui::DragFloat3("##pos", &l.position.x, 0.1f);

                ImGui::Text("Ambient:");
                ImGui::DragFloat3("##amb", &l.info.ambient.x, 0.01f);

                ImGui::Text("Specular:");
                ImGui::DragFloat3("##spec", &l.info.specular.x, 0.01f);

                ImGui::Text("Diffuse:");
                ImGui::DragFloat3("##diff", &l.info.diffuse.x, 0.01f);

                ImGui::Text("Attenuation:");
                ImGui::DragFloat("Constant", &l.attenuation_info.constant,
                                 0.01f);
                ImGui::DragFloat("Linear", &l.attenuation_info.linear, 0.01f);
                ImGui::DragFloat("Quadratic", &l.attenuation_info.quadratic,
                                 0.001f);

                if (ImGui::Button(("Delete##" + std::to_string(i)).c_str())) {
                  point_lights.erase(point_lights.begin() + i);
                  ImGui::TreePop();
                  break;
                }

                ImGui::TreePop();
              }
              i++;
            }

            if (ImGui::Button("Add Point Light")) {
              point_lights.push_back(PointLight{});
            }

            ImGui::TreePop();
          }
          // Directional Lights
          if (ImGui::TreeNode("Directional Lights")) {
            uint i = 0;
            for (auto &l : directionnal_lights) {
              std::string light_label = "Dir Light " + std::to_string(i);

              if (ImGui::TreeNode(light_label.c_str())) {
                ImGui::Text("Direction:");
                ImGui::DragFloat3("##dir", &l.direction.x, 0.1f);

                ImGui::Text("Ambient:");
                ImGui::DragFloat3("##amb", &l.info.ambient.x, 0.01f);

                ImGui::Text("Specular:");
                ImGui::DragFloat3("##spec", &l.info.specular.x, 0.01f);

                ImGui::Text("Diffuse:");
                ImGui::DragFloat3("##diff", &l.info.diffuse.x, 0.01f);

                if (ImGui::Button(("Delete##" + std::to_string(i)).c_str())) {
                  directionnal_lights.erase(directionnal_lights.begin() + i);
                  ImGui::TreePop();
                  break;
                }

                ImGui::TreePop();
              }
              i++;
            }

            if (ImGui::Button("Add Directional Light")) {
              directionnal_lights.push_back(DirectionalLight{});
            }

            ImGui::TreePop();
          }

          // Spot Lights
          if (ImGui::TreeNode("Spot Lights")) {
            uint i = 0;
            for (auto &l : spot_lights) {
              std::string light_label = "Spot Light " + std::to_string(i);

              if (ImGui::TreeNode(light_label.c_str())) {
                ImGui::Text("Position:");
                ImGui::DragFloat3("##pos", &l.position.x, 0.1f);

                ImGui::Text("Direction:");
                ImGui::DragFloat3("##dir", &l.direction.x, 0.1f);

                ImGui::Text("Ambient:");
                ImGui::DragFloat3("##amb", &l.info.ambient.x, 0.01f);

                ImGui::Text("Specular:");
                ImGui::DragFloat3("##spec", &l.info.specular.x, 0.01f);

                ImGui::Text("Diffuse:");
                ImGui::DragFloat3("##diff", &l.info.diffuse.x, 0.01f);

                ImGui::Text("Attenuation:");
                ImGui::DragFloat("Constant", &l.attenuation_info.constant,
                                 0.01f);
                ImGui::DragFloat("Linear", &l.attenuation_info.linear, 0.01f);
                ImGui::DragFloat("Quadratic", &l.attenuation_info.quadratic,
                                 0.001f);

                ImGui::Text("Area:");
                ImGui::DragFloat("Inner", &l.inner_cut_off, 0.01f, 100.0f);
                ImGui::DragFloat("Outer", &l.outer_cut_off, 0.01f, 100.0f);

                if (ImGui::Button(("Delete##" + std::to_string(i)).c_str())) {
                  spot_lights.erase(spot_lights.begin() + i);
                  ImGui::TreePop();
                  break;
                }

                ImGui::TreePop();
              }
              i++;
            }

            if (ImGui::Button("Add Spot Light")) {
              spot_lights.push_back(SpotLight{});
            }

            ImGui::TreePop();
          }
        }

        ImGui::End();
      }

      process_input(window);

      glClearColor(0, 0, 0, 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
              GL_STENCIL_BUFFER_BIT);

      VIEW = P_CAMERA.looking_at();

      // Backpack
      shader_in_use->use();

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

      // Drawing the model
      sponza.set_render_options(render_options);
      sponza.draw(
          *shader_in_use,
          Transform{VIEW, PROJECTION,
                    glm::translate(glm::scale(IDENTITY, glm::vec3(1.0f)),
                                   glm::vec3(0.0, 0.0, 0.0))});

      GLenum gl_error;
      if ((gl_error = glGetError()) != GL_NO_ERROR) {
        std::cout << "Erreur: OpenGL\n"
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
  P_CAMERA.set_fov(
      std::clamp(P_CAMERA.get_fov() - static_cast<float>(y), 1.0f, 45.0f));
  PROJECTION =
      glm::perspective(glm::radians(static_cast<float>(P_CAMERA.get_fov())),
                       static_cast<float>(WIDTH) / static_cast<float>(HEIGHT),
                       NEAR_PLANE, FAR_PLANE);
}

void redefine_projection_matrix() {
  PROJECTION =
      glm::perspective(glm::radians(static_cast<float>(P_CAMERA.get_fov())),
                       static_cast<float>(WIDTH) / static_cast<float>(HEIGHT),
                       NEAR_PLANE, FAR_PLANE);
}
