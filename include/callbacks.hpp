#pragma once

#include <GLFW/glfw3.h>
#include "camera.hpp"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void process_input(GLFWwindow *window, Camera &camera, float delta_time);
