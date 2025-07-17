#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

// default cam values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;

class Camera {
public:
  // attributes
  glm::vec3 position;
  glm::vec3 front;
  glm::vec3 up;
  glm::vec3 right;
  glm::vec3 worldup;
  // euler angles
  float yaw;
  float pitch;
  // options
  float movement_speed;
  float mouse_sensitivity;

  Camera(glm::vec3 position_ = glm::vec3(0.0f, 0.0f, 3.0f),
         glm::vec3 up_ = glm::vec3(0.0f, 1.0f, 0.0f), float yaw_ = YAW,
         float pitch_ = PITCH)
      : front(glm::vec3(0.0f, 0.0f, -1.0f)), movement_speed(SPEED),
        mouse_sensitivity(SENSITIVITY) {
    position = position_;
    worldup = up_;
    yaw = yaw_;
    pitch = pitch_;
    update_camera_vectors();
  }

  void process_keyboard(CameraMovement direction, float delta_time) {
    float velocity = movement_speed * delta_time;
    if (direction == CameraMovement::FORWARD)
      position += front * velocity;
    if (direction == CameraMovement::BACKWARD)
      position -= front * velocity;
    if (direction == CameraMovement::LEFT)
      position -= right * velocity;
    if (direction == CameraMovement::RIGHT)
      position += right * velocity;
    if (direction == CameraMovement::UP)
      position += worldup * velocity;
    if (direction == CameraMovement::DOWN)
      position -= worldup * velocity;
  }

  void process_mouse_movement(float xoffset, float yoffset, bool constrain_pitch = true) {
    xoffset *= mouse_sensitivity;
    yoffset *= mouse_sensitivity;
    yaw += xoffset;
    pitch += yoffset;

    if (constrain_pitch) {
      if (pitch > 89.0f)
        pitch = 89.0f;
      if (pitch < -89.0f)
        pitch = -89.0f;
    }

    update_camera_vectors();
  }

private:
  void update_camera_vectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(front);
    right = glm::normalize(glm::cross(front, worldup));
    up = glm::normalize(glm::cross(right, front));
  }
};

#endif