#ifndef MAINLOOP_HPP
#define MAINLOOP_HPP

#include <GLFW/glfw3.h>
#include "simulation.hpp"

struct AppState{
  Simulation simulation;
  bool is_mouse_captured;
  bool is_paused;
  bool gui_visible;
  float simulation_speed = 1.0f;
  double mouse_x = 0.0;
  double mouse_y = 0.0;
};

void mainloop(GLFWwindow *window);

#endif
