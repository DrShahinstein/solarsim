#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include "helper.hpp"
#include "window.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void esc_quit(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

const char *load_shader(const char *shader_path) {
  std::ifstream shader_file(shader_path);

  if (!shader_file) {
    std::cerr << "Could not open shader file: " << shader_path << std::endl;
    return nullptr;
  }

  std::stringstream shader_stream;
  shader_stream << shader_file.rdbuf();

  std::string shader_code = shader_stream.str();
  char *shader_code_cstr = new char[shader_code.size() + 1];

  std::strcpy(shader_code_cstr, shader_code.c_str());
  return shader_code_cstr;
}

void camera_callback(GLFWwindow *window, int key, int scancode, int action, int mods, glm::vec3 &camera_pos, glm::vec3 &camera_front, glm::vec3 &camera_up) {
  static float speed = 0.1f;

  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    glm::vec3 camera_right =
        glm::normalize(glm::cross(camera_front, camera_up));

    if (key == GLFW_KEY_W)
      camera_pos += speed * camera_front;
    if (key == GLFW_KEY_S)
      camera_pos -= speed * camera_front;
    if (key == GLFW_KEY_A)
      camera_pos -= speed * camera_right;
    if (key == GLFW_KEY_D)
      camera_pos += speed * camera_right;
    if (key == GLFW_KEY_SPACE)
      camera_pos += speed * camera_up;
    if (key == GLFW_KEY_LEFT_SHIFT)
      camera_pos -= speed * camera_up;
  }
}

// Add mouse callback
void mouse_callback(GLFWwindow *window, double xpos, double ypos, float &yaw, float &pitch, glm::vec3 &camera_front) {
  static float lastX = SCREEN_WIDTH / 2.0f;
  static float lastY = SCREEN_HEIGHT / 2.0f;
  static bool first_mouse = true;

  if (first_mouse) {
    lastX = xpos;
    lastY = ypos;
    first_mouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;
  lastX = xpos;
  lastY = ypos;

  const float sensitivity = 0.1f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;
  yaw += xoffset;
  pitch += yoffset;

  if (pitch > 89.0f)  pitch = 89.0f;
  if (pitch < -89.0f) pitch = -89.0f;

  glm::vec3 front;
  front.x      = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y      = sin(glm::radians(pitch));
  front.z      = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  camera_front = glm::normalize(front);
}