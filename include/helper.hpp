#ifndef HELPER_HPP
#define HELPER_HPP

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void camera_callback(GLFWwindow *window, int key, int scancode, int action, int mods, glm::vec3 &camera_pos, glm::vec3 &camera_front, glm::vec3 &camera_up);
void mouse_callback(GLFWwindow *window, double xpos, double ypos, float &yaw, float &pitch, glm::vec3 &camera_front, bool &reset_mouse_position);
std::string load_shader(const char *shader_path);

#endif