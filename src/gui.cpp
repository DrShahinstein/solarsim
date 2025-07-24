#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <sstream>
#include <chrono>
#include "gui.hpp"
#include "mainloop.hpp"
#include "simulation.hpp"

void initialize_imgui(GLFWwindow *window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  float xscale, yscale;
  glfwGetWindowContentScale(window, &xscale, &yscale);
  float dpi_scale = (xscale + yscale) * 0.5f;

  ImGuiIO& io = ImGui::GetIO();
  io.FontGlobalScale = dpi_scale;
  ImGui::GetStyle().ScaleAllSizes(dpi_scale);

  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, false);
  ImGui_ImplOpenGL3_Init("#version 330");
}

void render_body_editor(CelestialBody &body, int index) {
  std::string header = "Body " + std::to_string(index);

  if (ImGui::CollapsingHeader(header.c_str())) {
    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.6f);

    float mass = static_cast<float>(body.mass);
    if (ImGui::SliderFloat("Mass", &mass, 1e-8f, 100.0f, "%.8f", ImGuiSliderFlags_Logarithmic)) {
      body.mass = static_cast<double>(mass);
    }

    float radius = static_cast<float>(body.radius);
    if (ImGui::SliderFloat("Radius", &radius, 0.01f, 2.0f)) {
      body.radius = static_cast<double>(radius);
    }

    float position[3] = {static_cast<float>(body.position.x),
                         static_cast<float>(body.position.y),
                         static_cast<float>(body.position.z)};
    ImGui::InputFloat3("Position", position, "%.3f");
    body.position = glm::dvec3(position[0], position[1], position[2]);

    float velocity[3] = {static_cast<float>(body.velocity.x),
                         static_cast<float>(body.velocity.y),
                         static_cast<float>(body.velocity.z)};
    ImGui::InputFloat3("Velocity", velocity, "%.6f");
    body.velocity = glm::dvec3(velocity[0], velocity[1], velocity[2]);

    ImGui::ColorEdit3("Color", &body.color[0]);
    ImGui::Checkbox("Black Hole", &body.is_black_hole);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
    if (ImGui::Button(("Delete##" + std::to_string(index)).c_str())) body.mass = 0;
    ImGui::PopStyleColor(2);
    ImGui::PopItemWidth();
  }
}

void render_simulation_stats(AppState &app, double frame_time) {
  ImGui::Begin("Performance Stats");

  ImGui::Text("Frame Time: %.3f ms (%.1f FPS)", frame_time * 1000.0, 1.0 / frame_time);
  ImGui::Text("Physics Steps: %zu", app.simulation.get_bodies().size() * app.simulation.get_bodies().size());

  size_t body_size = app.simulation.get_bodies().size() * sizeof(CelestialBody);
  ImGui::Text("Memory: %.2f KB", body_size / 1024.0f);

  ImGui::Separator();
  ImGui::Text("Camera Position:");
  ImGui::Text("X: %.2f, Y: %.2f, Z: %.2f", app.camera->m_position.x, app.camera->m_position.y, app.camera->m_position.z);

  ImGui::Separator();
  ImGui::Text("Integrator: Velocity Verlet");
  ImGui::Text("Time Step: %.4f s", 1.0 / 100 * app.simulation_speed);

  ImGui::End();
}

void render_camera_info(Camera &camera) {
  ImGui::Begin("Camera Settings");

  float speed = camera.m_movement_speed;
  if (ImGui::SliderFloat("Move Speed", &speed, 0.1f, 50.0f)) {
    camera.m_movement_speed = speed;
  }

  float sensitivity = camera.m_mouse_sensitivity;
  if (ImGui::SliderFloat("Look Sensitivity", &sensitivity, 0.01f, 1.0f)) {
    camera.m_mouse_sensitivity = sensitivity;
  }

  ImGui::Separator();
  ImGui::Text("Front: %.2f, %.2f, %.2f", camera.m_front.x, camera.m_front.y, camera.m_front.z);
  ImGui::Text("Up:    %.2f, %.2f, %.2f", camera.m_up.x, camera.m_up.y, camera.m_up.z);
  ImGui::Text("Right: %.2f, %.2f, %.2f", camera.m_right.x, camera.m_right.y, camera.m_right.z);
  ImGui::End();
}

void render_help_window() {
  ImGui::Begin("Controls Help", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::Text("Camera Controls:");
  ImGui::BulletText("WASD: Move horizontally");
  ImGui::BulletText("Space/Shift: Move vertically");
  ImGui::BulletText("Mouse: Look around");
  ImGui::BulletText("ESC: Toggle mouse capture");
  ImGui::Separator();
  ImGui::Text("Simulation Controls:");
  ImGui::BulletText("R: Reset to solar system");
  ImGui::BulletText("B: Add black hole at cursor");
  ImGui::BulletText("P: Toggle pause");
  ImGui::End();
}

double calc_frame_time() {
  static auto last_time = std::chrono::high_resolution_clock::now();
  auto now = std::chrono::high_resolution_clock::now();
  double elapsed = std::chrono::duration<double>(now - last_time).count();
  last_time = now;
  return elapsed;
}

void render_gui(AppState &app) {
  ImGui::SetNextWindowSize(ImVec2(UI_WIDTH, 0), ImGuiCond_FirstUseEver);
  ImGui::Begin("Simulation Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::Checkbox("Pause Simulation", &app.is_paused);
  ImGui::SameLine();
  if (ImGui::Button("Reset")) {
    app.simulation.reset_to_solar_system();
    app.simulation.setG(DEFAULT_G);
  }

  ImGui::SliderFloat("Simulation Speed", &app.simulation_speed, 0.1f, 1000.0f, "%.1f x", ImGuiSliderFlags_Logarithmic);

  float G = static_cast<float>(app.simulation.getG());
  if (ImGui::SliderFloat("G Constant", &G, 1e-7f, 1e-3f, "%.8f", ImGuiSliderFlags_Logarithmic)) {
    app.simulation.setG(static_cast<double>(G));
  }

  ImGui::Checkbox("Enable lighting", &app.gui_props.lighting_enabled);

  ImGui::Separator();
  ImGui::Text("Show Windows:");
  ImGui::SameLine();
  ImGui::Checkbox("Performance", &app.gui_props.show_stats);
  ImGui::SameLine();
  ImGui::Checkbox("Help", &app.gui_props.show_help);
  ImGui::SameLine();
  ImGui::Checkbox("Camera", &app.gui_props.show_caminfo);

  ImGui::Separator();
  if (ImGui::CollapsingHeader("Celestial Bodies", ImGuiTreeNodeFlags_DefaultOpen)) {
    auto &bodies = app.simulation.get_bodies();
    for (size_t i = 0; i < bodies.size(); i++) {
      ImGui::PushID(i);
      render_body_editor(bodies[i], static_cast<int>(i));
      ImGui::PopID();
    }
  }

  if (ImGui::CollapsingHeader("Add Body")) {
    static CelestialBody new_body;
    new_body.mass          = app.gui_props.body_editor.mass;
    new_body.radius        = app.gui_props.body_editor.radius;
    new_body.color         = app.gui_props.body_editor.color;
    new_body.is_black_hole = app.gui_props.body_editor.is_black_hole;
    new_body.position      = app.camera->m_position + app.camera->m_front * 1.0f;
    new_body.velocity      = glm::dvec3(0.0, 0.0, 0.0);

    if (ImGui::SliderFloat("Mass", &app.gui_props.body_editor.mass, 1e-8f, 100.0f, "%.8f", ImGuiSliderFlags_Logarithmic)) {
      new_body.mass = app.gui_props.body_editor.mass;
    }
    if (ImGui::SliderFloat("Radius", &app.gui_props.body_editor.radius, 0.01f, 2.0f)) {
      new_body.radius = app.gui_props.body_editor.radius;
    }
    if (ImGui::ColorEdit3("Color", &app.gui_props.body_editor.color[0])) {
      new_body.color = app.gui_props.body_editor.color;
    }

    ImGui::Checkbox("Black Hole", &app.gui_props.body_editor.is_black_hole);
    new_body.is_black_hole = app.gui_props.body_editor.is_black_hole;

    if (ImGui::Button("Add")) {
      app.simulation.add_body(new_body);
    }
  }

  if (app.gui_props.show_stats) {
    render_simulation_stats(app, calc_frame_time());
  }

  if (app.gui_props.show_help) {
    render_help_window();
  }

  if (app.gui_props.show_caminfo) {
    render_camera_info(*app.camera);
  }

  ImGui::End();
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
