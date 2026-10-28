#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <algorithm>
#include <iostream>
#include "mainloop.hpp"
#include "callbacks.hpp"
#include "shaders.hpp"
#include "simulation.hpp"
#include "gui.hpp"
#include "window.h"
#include "camera.hpp"

struct Vertex {
  glm::vec3 position;
  glm::vec3 color;
};

struct CalestialBodyData {
  glm::vec3 position;
  glm::vec3 color;
  float radius;
  float mass;
  float event_horizon;
  int is_black_hole;
};

struct BodyUniforms {
  GLint position;
  GLint radius;
  GLint color;
  GLint mass;
  GLint event_horizon;
  GLint is_black_hole;
};

struct ShaderUniforms {
  GLint camera_pos;
  GLint camera_front;
  GLint camera_up;
  GLint camera_right;
  GLint aspect_ratio;
  GLint num_bodies;
  GLint light_index;
  GLint lighting_enabled;
  GLint raymarch_steps;
  BodyUniforms bodies[MAX_BODIES];
};

ShaderUniforms get_uniform_locations(unsigned int shader_program) {
  ShaderUniforms uniforms{};

  uniforms.camera_pos       = glGetUniformLocation(shader_program, "camera_pos");
  uniforms.camera_front     = glGetUniformLocation(shader_program, "camera_front");
  uniforms.camera_up        = glGetUniformLocation(shader_program, "camera_up");
  uniforms.camera_right     = glGetUniformLocation(shader_program, "camera_right");
  uniforms.aspect_ratio     = glGetUniformLocation(shader_program, "aspect_ratio");
  uniforms.num_bodies       = glGetUniformLocation(shader_program, "num_bodies");
  uniforms.light_index      = glGetUniformLocation(shader_program, "light_index");
  uniforms.lighting_enabled = glGetUniformLocation(shader_program, "lighting_enabled");
  uniforms.raymarch_steps   = glGetUniformLocation(shader_program, "raymarch_steps");

  for (int i = 0; i < MAX_BODIES; i++) {
    std::string index = "bodies[" + std::to_string(i) + "]";
    uniforms.bodies[i].position       = glGetUniformLocation(shader_program, (index + ".position").c_str());
    uniforms.bodies[i].radius         = glGetUniformLocation(shader_program, (index + ".radius").c_str());
    uniforms.bodies[i].color          = glGetUniformLocation(shader_program, (index + ".color").c_str());
    uniforms.bodies[i].mass           = glGetUniformLocation(shader_program, (index + ".mass").c_str());
    uniforms.bodies[i].event_horizon  = glGetUniformLocation(shader_program, (index + ".event_horizon").c_str());
    uniforms.bodies[i].is_black_hole  = glGetUniformLocation(shader_program, (index + ".is_black_hole").c_str());
  }

  return uniforms;
}

void mainloop(GLFWwindow *window) {
  // shaders
  auto shader_program = create_shader_program();
  ShaderUniforms uniforms = get_uniform_locations(shader_program);

  // fs quad setup
  float quad_vertices[] = {-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
                           -1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 1.0f};
  unsigned int quad_VAO, quad_VBO;
  glGenVertexArrays(1, &quad_VAO);
  glGenBuffers(1, &quad_VBO);
  glBindVertexArray(quad_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, quad_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // initializations
  Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
  initialize_imgui(window);
  Simulation simulation;
  simulation.reset_to_solar_system();
  AppState app{.simulation = simulation, .camera = &camera, .simulation_speed = 1.0f};
  glfwSetWindowUserPointer(window, &app);

  // input callbacks
  glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
    AppState *app = static_cast<AppState *>(glfwGetWindowUserPointer(window));
    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      if (app->input.mouse_captured) {
        glfwGetCursorPos(window, &app->input.last_x, &app->input.last_y);
        app->input.mouse_captured = false;
        app->gui.visible = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        ImGui::GetIO().MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
      } else {
        app->input.mouse_captured = true;
        app->gui.visible = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        app->input.first_mouse = true;
      }
      return;
    }

    if (io.WantCaptureKeyboard) {
      return;
    }

    if (action == GLFW_PRESS && app->input.mouse_captured) {
      switch (key) {
      case GLFW_KEY_R:
        app->simulation.reset_to_solar_system();
        break;
      case GLFW_KEY_B: {
        if (app->simulation.get_bodies().size() >= MAX_BODIES) break;
        CelestialBody new_body{};
        new_body.position = glm::dvec3(app->camera->m_position + app->camera->m_front * 1.0f);
        new_body.velocity = glm::dvec3(0.0);
        new_body.mass = app->gui.body_editor.mass;
        new_body.radius = app->gui.body_editor.radius;
        new_body.color = app->gui.body_editor.color;
        new_body.is_black_hole = app->gui.body_editor.is_black_hole;
        new_body.is_star = app->gui.body_editor.is_star;
        app->simulation.add_body(new_body);
        break;
      }
      case GLFW_KEY_P:
        app->paused = !app->paused;
        break;
      case GLFW_KEY_C:
        if (!app->simulation.get_bodies().empty()) {
          app->simulation.get_bodies().pop_back();
        }
        break;
      }
    }
  });

  glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos) {
    AppState *app = static_cast<AppState *>(glfwGetWindowUserPointer(window));
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
    if (app->input.mouse_captured) {
      if (app->input.first_mouse) {
        app->input.last_x = xpos;
        app->input.last_y = ypos;
        app->input.first_mouse = false;
      }
      float xoffset = xpos - app->input.last_x;
      float yoffset = app->input.last_y - ypos;
      app->input.last_x = xpos;
      app->input.last_y = ypos;
      app->camera->process_mouse_movement(xoffset, yoffset);
    }
  });

  glfwSetCursorEnterCallback(window, [](GLFWwindow *window, int entered) {
    ImGui_ImplGlfw_CursorEnterCallback(window, entered);
  });

  glfwSetScrollCallback(window, [](GLFWwindow *window, double xoffset, double yoffset) {
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
  });

  glfwSetCharCallback(window, [](GLFWwindow *window, unsigned int codepoint) {
    ImGui_ImplGlfw_CharCallback(window, codepoint);
  });

  glfwSetWindowFocusCallback(window, [](GLFWwindow *window, int focused) {
    ImGui_ImplGlfw_WindowFocusCallback(window, focused);
  });

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSwapInterval(1);
  glEnable(GL_DEPTH_TEST);

  glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
  });

  // game-loop timing (=> decouples fps from physics updates)
  const double dt = 1.0 / 100;
  constexpr int MAX_STEPS_PER_FRAME = 2000;
  double current_time = glfwGetTime();
  double accumulator = 0.0;

  // loop
  while (!glfwWindowShouldClose(window)) {
    double new_time = glfwGetTime();
    double frame_time = new_time - current_time;
    current_time = new_time;

    if (frame_time > 0.25) {
      frame_time = 0.25;
    }
    AppState *app_ptr = static_cast<AppState *>(glfwGetWindowUserPointer(window));
    accumulator += frame_time * app_ptr->simulation_speed;

    // camera
    glfwPollEvents();
    if (app_ptr->input.mouse_captured) {
      process_input(window, *app_ptr->camera, static_cast<float>(frame_time), &app);
    }

    // physics update
    int steps = 0;
    while (accumulator >= dt && steps < MAX_STEPS_PER_FRAME) {
      if (!app_ptr->paused) {
        app_ptr->simulation.update(dt);
        app_ptr->simulation.remove_marked_bodies();
      }
      accumulator -= dt;
      ++steps;
    }
    if (steps == MAX_STEPS_PER_FRAME) {
      accumulator = 0.0;
    }

    // rendering
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto &bodies = app_ptr->simulation.get_bodies();
    int framebuffer_width, framebuffer_height;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    float aspect_ratio = framebuffer_height > 0
                             ? static_cast<float>(framebuffer_width) / static_cast<float>(framebuffer_height)
                             : 1.0f;
    glViewport(0, 0, framebuffer_width, framebuffer_height);

    glUseProgram(shader_program);
    glUniform3fv(uniforms.camera_pos, 1, glm::value_ptr(app_ptr->camera->m_position));
    glUniform3fv(uniforms.camera_front, 1, glm::value_ptr(app_ptr->camera->m_front));
    glUniform3fv(uniforms.camera_up, 1, glm::value_ptr(app_ptr->camera->m_up));
    glUniform3fv(uniforms.camera_right, 1, glm::value_ptr(app_ptr->camera->m_right));
    glUniform1f(uniforms.aspect_ratio, aspect_ratio);
    glUniform1i(uniforms.num_bodies, static_cast<int>(std::min(bodies.size(), static_cast<size_t>(MAX_BODIES))));
    glUniform1i(uniforms.lighting_enabled, app_ptr->gui.lighting_enabled);
    glUniform1i(uniforms.raymarch_steps, app_ptr->gui.raymarch_steps);

    int light_index = -1;
    for (size_t i = 0; i < bodies.size(); i++) {
      if (bodies[i].is_star) {
        light_index = static_cast<int>(i);
        break;
      }
    }
    glUniform1i(uniforms.light_index, light_index);

    for (size_t i = 0; i < bodies.size() && i < MAX_BODIES; i++) {
      CalestialBodyData body_data = {glm::vec3(bodies[i].position),
                                     bodies[i].color,
                                     static_cast<float>(bodies[i].radius),
                                     static_cast<float>(bodies[i].mass),
                                     static_cast<float>(bodies[i].event_horizon_radius(app.simulation.getG()) * BLACK_HOLE_VISUAL_SCALE),
                                     bodies[i].is_black_hole ? 1 : 0};

      glUniform3fv(uniforms.bodies[i].position, 1, glm::value_ptr(body_data.position));
      glUniform1f(uniforms.bodies[i].radius, body_data.radius);
      glUniform3fv(uniforms.bodies[i].color, 1, glm::value_ptr(body_data.color));
      glUniform1i(uniforms.bodies[i].is_black_hole, body_data.is_black_hole);
      glUniform1f(uniforms.bodies[i].mass, body_data.mass);
      glUniform1f(uniforms.bodies[i].event_horizon, body_data.event_horizon);
    }

    glBindVertexArray(quad_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    if (app_ptr->gui.visible) {
      render_gui(*app_ptr);
    } else {
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    glfwSwapBuffers(window);
  }

  // cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glDeleteVertexArrays(1, &quad_VAO);
  glDeleteBuffers(1, &quad_VBO);
  glDeleteProgram(shader_program);
}
