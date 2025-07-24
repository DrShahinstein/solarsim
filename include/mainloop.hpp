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
  float simulation_speed=1.0f;
  double lastX;
  double lastY;
  bool first_mouse;
  struct {
    bool show_help;
    bool show_stats;
    bool show_caminfo;
    bool lighting_enabled=true;
    struct {
      float mass=0.1f;
      float radius=0.1f;
      glm::vec3 color=glm::vec3(1.0f, 0.0f, 0.0f); // red
      bool is_black_hole=false;
    } body_editor;
  } gui_props;
};

void mainloop(GLFWwindow *window);

#endif
