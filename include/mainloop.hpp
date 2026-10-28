#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "simulation.hpp"
#include "camera.hpp"

struct InputState {
  bool mouse_captured = true;
  double last_x = 0.0;
  double last_y = 0.0;
  bool first_mouse = true;
};

struct GuiState {
  bool visible = false;
  bool show_help = false;
  bool show_stats = false;
  bool show_camera_info = false;
  bool lighting_enabled = true;
  int raymarch_steps = 128;
  struct {
    float mass = 0.1f;
    float radius = 0.1f;
    glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f);
    bool is_black_hole = false;
  } body_editor;
};

struct AppState {
  Simulation simulation;
  Camera *camera = nullptr;
  bool paused = false;
  float simulation_speed = 1.0f;
  InputState input;
  GuiState gui;
};

void mainloop(GLFWwindow *window);
