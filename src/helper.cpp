#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "helper.hpp"
#include "window.h"
#include "camera.hpp"
#include "mainloop.hpp"

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

std::string load_shader(const char *shader_path) {
  std::ifstream shader_file(shader_path);
  if (!shader_file) {
    std::cerr << "Could not open shader file: " << shader_path << std::endl;
    return "";
  }
  std::stringstream shader_stream;
  shader_stream << shader_file.rdbuf();
  return shader_stream.str();
}

void process_input(GLFWwindow *window, Camera &camera, float delta_time, AppState *app) {
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.process_keyboard(CameraMovement::FORWARD, delta_time);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.process_keyboard(CameraMovement::BACKWARD, delta_time);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.process_keyboard(CameraMovement::LEFT, delta_time);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.process_keyboard(CameraMovement::RIGHT, delta_time);
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    camera.process_keyboard(CameraMovement::UP, delta_time);
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    camera.process_keyboard(CameraMovement::DOWN, delta_time);
}