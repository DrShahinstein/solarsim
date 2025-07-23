#ifndef MAINLOOP_HPP
#define MAINLOOP_HPP

#include <GLFW/glfw3.h>
#include "simulation.hpp"
#include "camera.hpp"

struct AppState{
  Simulation simulation;
  Camera *camera;
  bool is_mouse_captured;
  bool is_paused;
  bool gui_visible;
  float simulation_speed = 1.0f;
  double lastX;
  double lastY;
  bool first_mouse;
  struct {
    bool show_help;
    bool show_stats;
    bool show_caminfo;
  } gui_props;
};

void mainloop(GLFWwindow *window);

#endif
