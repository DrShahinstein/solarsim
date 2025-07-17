#ifndef HELPER_HPP
#define HELPER_HPP

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include "camera.hpp"
#include "mainloop.hpp"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
std::string load_shader(const char *shader_path);
void process_input(GLFWwindow *window, Camera &camera, float delta_time, AppState *app);

#endif