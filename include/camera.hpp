#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

const float YAW_DEFAULT = -90.0f;
const float PITCH_DEFAULT = 0.0f;
const float SPEED_DEFAULT = 2.5f;
const float SENSITIVITY_DEFAULT = 0.1f;

class Camera {
public:
  // attributes
  glm::vec3 m_position;
  glm::vec3 m_front;
  glm::vec3 m_up;
  glm::vec3 m_right;
  glm::vec3 m_worldup;
  // euler angles
  float m_yaw;
  float m_pitch;
  // options
  float m_movement_speed;
  float m_mouse_sensitivity;

  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW_DEFAULT,
         float pitch = PITCH_DEFAULT)
      : m_front(glm::vec3(0.0f, 0.0f, -1.0f)), m_movement_speed(SPEED_DEFAULT),
        m_mouse_sensitivity(SENSITIVITY_DEFAULT) {
    m_position = position;
    m_worldup = up;
    m_yaw = yaw;
    m_pitch = pitch;
    update_camera_vectors();
  }

  void process_keyboard(CameraMovement direction, float delta_time) {
    float velocity = m_movement_speed * delta_time;
    if (direction == CameraMovement::FORWARD)
      m_position += m_front * velocity;
    if (direction == CameraMovement::BACKWARD)
      m_position -= m_front * velocity;
    if (direction == CameraMovement::LEFT)
      m_position -= m_right * velocity;
    if (direction == CameraMovement::RIGHT)
      m_position += m_right * velocity;
    if (direction == CameraMovement::UP)
      m_position += m_worldup * velocity;
    if (direction == CameraMovement::DOWN)
      m_position -= m_worldup * velocity;
  }

  void process_mouse_movement(float xoffset, float yoffset, bool constrain_pitch = true) {
    xoffset *= m_mouse_sensitivity;
    yoffset *= m_mouse_sensitivity;
    m_yaw += xoffset;
    m_pitch += yoffset;

    if (constrain_pitch) {
      if (m_pitch > 89.0f)  m_pitch = 89.0f;
      if (m_pitch < -89.0f) m_pitch = -89.0f;
    }

    update_camera_vectors();
  }

private:
  void update_camera_vectors() {
    glm::vec3 front_vec;
    front_vec.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front_vec.y = sin(glm::radians(m_pitch));
    front_vec.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front_vec);
    m_right = glm::normalize(glm::cross(m_front, m_worldup));
    m_up = glm::normalize(glm::cross(m_right, m_front));
  }
};

#endif