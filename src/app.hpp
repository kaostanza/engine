#pragma once

#include <GLFW/glfw3.h>

#include <climits>
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <type_traits>

enum SystemPriority {
  Once = INT_MIN,
  Update = 100,
};

struct System {
  SystemPriority priority;
  void (*func)(entt::registry &);
};

class App;
class Plugin;

class App {
public:
  App();
  ~App();

  template <typename T> void add_plugin() {
    static_assert(std::is_base_of_v<Plugin, T>, "T must derive Plugin trait");
    static_assert(!std::is_same<Plugin, T>(),
                  "Plugin itself cannot be used because purely virtual");
    T plugin;
    plugin.load(*this);
  }
  template <typename T> void remove_plugin() {
    static_assert(std::is_base_of_v<Plugin, T>, "T must derive Plugin trait");
    static_assert(!std::is_same<Plugin, T>(),
                  "Plugin itself cannot be used because purely virtual");

    T plugin;
    plugin.unload(*this);
  }

  void add_system(System system) { _systems.push_back(system); };
  void remove_system(System system) {
    auto it = std::find_if(
        _systems.begin(), _systems.end(),
        [system](const System &sys) { return sys.func == system.func; });

    if (it != _systems.end())
      _systems.erase(it);
  }

  bool is_running();
  void run();

private:
  // basically our ECS world
  entt::registry _world;
  // Always sorted by priority
  std::vector<System> _systems;

  // OpenGL + glfw related
  GLFWwindow *_window;
  int _window_width;
  int _window_height;
};

// Basically the equivalent of the trait Plugin in bevy
// Purely virtual
class Plugin {
public:
  virtual ~Plugin() = default;
  virtual void load(App &app) = 0;
  virtual void unload(App &app) = 0;

protected:
  Plugin() = default;
};
