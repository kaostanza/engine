#include "glad/glad.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <fontconfig/fontconfig.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <iostream>
#include <ostream>

#include "glm/detail/func_trigonometric.hpp"
#include "glm/detail/type_mat.hpp"
#include "glm/detail/type_vec.hpp"
#include "shader.hpp"
#include "texture2D.hpp"

// SCREEN + FOV
float WIDTH = 800.0;
float HEIGHT = 600.0;
double FOV = 45.0;

// FLY CAMERA
float X_POS = WIDTH / 2;
float Y_POS = HEIGHT / 2;
float CAMERA_YAW = -90.0;
float CAMERA_PITCH = 0;
auto UP = glm::vec3(0.0, 1.0, 0.0);
auto CAMERA_POSITION = glm::vec3(0.0, 0.0, 3.0);
auto CAMERA_TARGET = glm::vec3(0.0, 0.0, 0.0);
auto CAMERA_DIRECTION = CAMERA_TARGET - CAMERA_POSITION;
auto CAMERA_FRONT = glm::vec3(0.0, 0.0, -1);
glm::vec3 CAMERA_RIGHT = glm::normalize(glm::cross(CAMERA_FRONT, UP));

// Time related stuff
float TIME = 0;
float LAST_TIME = 0;
float DELTA = 0;

// NORMAL OBJECTS
const float normal_cube_vertices[]{
    // bottom face
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

const float light_cube_vertices[] = {
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

const glm::vec3 normal_cubes_positions[] = {glm::vec3(0.0, 0.0, 0.0)};
const glm::vec3 light_cubes_positions[] = {glm::vec3(1.2, 1.0, 2.0)};

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

  if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  auto model = glm::mat4(1.0);
  auto view = glm::lookAt(CAMERA_POSITION, CAMERA_POSITION + CAMERA_FRONT, UP);
  auto projection = glm::perspective(glm::radians(static_cast<float>(FOV)),
                                     WIDTH / HEIGHT, 0.1f, 100.0f);

  Texture2D texture_container("../assets/textures/container.jpg");
  Texture2D texture_smiley(
      "../assets/textures/awesomeface.png", Texture2D::WrapMode::Repeat,
      Texture2D::WrapMode::Repeat, Texture2D::FilterMode::Linear,
      Texture2D::FilterMode::Linear, Texture2D::Format::RGBA);

  shader_program.use();
  shader_program.set_uniform("model", model);
  shader_program.set_uniform("view", view);
  shader_program.set_uniform("projection", projection);
  shader_program.set_uniform("lightColor", glm::vec3{1.0f, 1.0f, 1.0f});

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

    const auto direction = glm::vec3(std::cos(glm::radians(CAMERA_YAW)),
                                     std::sin(glm::radians(CAMERA_PITCH)),
                                     std::sin(glm::radians(CAMERA_YAW)));
    CAMERA_FRONT = glm::normalize(direction);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_container.get_id());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_smiley.get_id());

    glBindVertexArray(VAO);
    shader_program.use();
    view = glm::lookAt(CAMERA_POSITION, CAMERA_POSITION + CAMERA_FRONT, UP);
    shader_program.set_uniform("view", view);

    projection = glm::perspective(glm::radians(static_cast<float>(FOV)),
                                  WIDTH / HEIGHT, 0.1f, 100.0f);
    shader_program.set_uniform("projection", projection);
    for (const auto &position : normal_cubes_positions) {
      shader_program.set_uniform("model",
                                 glm::translate(glm::mat4(1.0f), position));
      glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned int),
                     GL_UNSIGNED_INT, nullptr);
    }

    glBindVertexArray(LIGHT_VAO);
    light_shader_program.use();
    light_shader_program.set_uniform("view", view);
    light_shader_program.set_uniform("projection", projection);
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
    CAMERA_POSITION += CAMERA_FRONT * glm::vec3(cam_speed);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    CAMERA_POSITION -= CAMERA_FRONT * glm::vec3(cam_speed);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    CAMERA_POSITION -=
        glm::vec3(cam_speed) * glm::normalize(glm::cross(CAMERA_FRONT, UP));
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    CAMERA_POSITION +=
        glm::vec3(cam_speed) * glm::normalize(glm::cross(CAMERA_FRONT, UP));
  }
}

void resize_window_callback(GLFWwindow *window, int width, int height) {
  WIDTH = width;
  HEIGHT = height;
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double x, double y) {
  auto x_offset = x - X_POS;
  auto y_offset = y - Y_POS;

  X_POS = x;
  Y_POS = y;

  const auto sensitivity = 0.01;
  x_offset *= sensitivity;
  y_offset *= -sensitivity;

  CAMERA_YAW += x_offset;
  CAMERA_PITCH = std::clamp(CAMERA_PITCH + y_offset, -89.9, 89.9);
}

void scroll_callback(GLFWwindow *window, double x, double y) {
  FOV = std::clamp(FOV - y, 1.0, 45.0);
}
