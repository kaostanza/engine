#include "glad/glad.h"
#include <GL/gl.h>

#include <GLFW/glfw3.h>
#include <fontconfig/fontconfig.h>
#include <iostream>
#include <stdexcept>

#include "app.hpp"

void App::run() {
  while (is_running()) {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    for (const auto &sys : _systems) {
      sys.func(_world);
    }

    GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR) {
      std::cerr << "Erreur: Impossible de render\n"
                << "-> " << gl_error << std::endl;
      gl_error = glGetError();
    }

    glfwSwapBuffers(_window);
    glfwPollEvents();
  }
}

bool App::is_running() { return glfwWindowShouldClose(_window) == 0; }

// OpenGL implementation
App::App() {
  _window_width = 1920;
  _window_height = 1080;

  if (FcInit() == 0) {
    throw std::runtime_error("Erreur: Impossible d'init fontconfig\n");
  }

  if (glfwInit() == 0) {
    throw std::runtime_error("Erreur: Impossible d'init glfw\n");
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  _window = glfwCreateWindow(_window_width, _window_height, "LearnOpenGL",
                             nullptr, nullptr);

  if (_window == nullptr) {
    throw std::runtime_error(
        "Erreur: Impossible de créer une Window avec OpenGL\n");
  }

  glfwMakeContextCurrent(_window);

  if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) ==
      0) {
    throw std::runtime_error("Erreur: Impossible de load via glad\n");
  }
}
App::~App() {
  glfwTerminate();
  FcFini();
}
