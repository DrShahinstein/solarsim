#ifndef GUI_H
#define GUI_H

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void initialize_imgui(GLFWwindow *window);
void render_gui(AppState &app);

#endif