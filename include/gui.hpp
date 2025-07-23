#ifndef GUI_H
#define GUI_H

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "simulation.hpp"
#include "mainloop.hpp"

#define UI_WIDTH         350.0f
#define MIN_SLIDER_WIDTH 150.0f

void initialize_imgui(GLFWwindow *window);
void render_gui(AppState &app);
void render_body_editor(CelestialBody &body, int index);
void render_simulation_stats(AppState &app, double frame_time);
void render_camera_info(Camera &camera);

#endif