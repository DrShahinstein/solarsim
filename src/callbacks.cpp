#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "callbacks.hpp"
#include "window.h"
#include "camera.hpp"
#include "mainloop.hpp"

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
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