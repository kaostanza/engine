#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const auto UP = glm::vec3(0.0, 1.0, 0.0);

class FlyCamera {
public:
  FlyCamera();
  FlyCamera(glm::vec3 position_, double fov_)
      : position(position_), fov(fov_) {}
  ~FlyCamera() = default;

  glm::vec3 get_position() const { return this->position; }
  double get_fov() const { return this->fov; }
  float get_yaw() const { return this->yaw; }
  float get_pitch() const { return this->pitch; }

  void set_position(glm::vec3 pos) { this->position = pos; }
  void set_fov(double fov_) { this->fov = fov_; }
  void set_yaw(float yaw_) { this->yaw = yaw_; }
  void set_pitch(float pitch_) { this->pitch = pitch_; }

  glm::mat4 looking_at() const {
    return glm::lookAt(this->position, this->position + this->forward(), UP);
  };

  glm::vec3 forward() const {
    return glm::normalize(glm::vec3(std::cos(glm::radians(this->yaw)),
                                    std::sin(glm::radians(this->pitch)),
                                    std::sin(glm::radians(this->yaw))));
  };
  glm::vec3 right() const {
    return glm::normalize(glm::cross(forward(), glm::vec3(0.0, 1.0, 0.0)));
  };

private:
  glm::vec3 position;
  glm::vec3 direction;
  double fov;
  float yaw = -90.0;
  float pitch = 0;
};
