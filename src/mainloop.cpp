#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <chrono>
#include <thread>
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
};

void mainloop(GLFWwindow *window) {
  // shaders
  auto shader_program = create_shader_program();

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
  AppState app{.simulation = simulation,
               .camera = &camera,
               .is_mouse_captured = true,
               .is_paused = false,
               .gui_visible = false,
               .simulation_speed = 1.0f,
               .lastX = SCREEN_WIDTH / 2.0,
               .lastY = SCREEN_HEIGHT / 2.0,
               .first_mouse = true};
  glfwSetWindowUserPointer(window, &app);

  // input callbacks
  glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
    AppState *app = static_cast<AppState *>(glfwGetWindowUserPointer(window));
    ImGuiIO &io = ImGui::GetIO();

    if (io.WantCaptureKeyboard) {
      ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
      return;
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      if (app->is_mouse_captured) {
        glfwGetCursorPos(window, &app->lastX, &app->lastY);
        app->is_mouse_captured = false;
        app->gui_visible = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        ImGui::GetIO().MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
      } else {
        app->is_mouse_captured = true;
        app->gui_visible = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        app->first_mouse = true;
      }
    }

    if (action == GLFW_PRESS && app->is_mouse_captured) {
      switch (key) {
      case GLFW_KEY_R:
        app->simulation.reset_to_solar_system();
        break;
      case GLFW_KEY_B: {
        CelestialBody new_body;
        new_body.position = glm::dvec3(app->camera->m_position + app->camera->m_front * 1.0f);
        new_body.velocity = glm::dvec3(0.0);
        new_body.mass = app->gui_props.body_editor.mass;
        new_body.radius = app->gui_props.body_editor.radius;
        new_body.color = app->gui_props.body_editor.color;
        new_body.is_black_hole = app->gui_props.body_editor.is_black_hole;
        app->simulation.add_body(new_body);
        break;
      }
      case GLFW_KEY_P:
        app->is_paused = !app->is_paused;
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
    if (app->is_mouse_captured) {
      if (app->first_mouse) {
        app->lastX = xpos;
        app->lastY = ypos;
        app->first_mouse = false;
      }
      float xoffset = xpos - app->lastX;
      float yoffset = app->lastY - ypos;
      app->lastX = xpos;
      app->lastY = ypos;
      app->camera->process_mouse_movement(xoffset, yoffset);
    }
  });

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glEnable(GL_DEPTH_TEST);

  glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
  });

  // game-loop timing (=> decouples fps from physics updates)
  const double dt = 1.0 / 100;
  double current_time = glfwGetTime();
  double accumulator = 0.0;

  // frame limiting
  using Clock = std::chrono::high_resolution_clock;
  constexpr double TARGET_FRAME_TIME = 1.0 / 60.0; // 60 FPS
  auto frame_start = Clock::now();

  // loop
  while (!glfwWindowShouldClose(window)) {
    double new_time = glfwGetTime();
    double frame_time = new_time - current_time;
    current_time = new_time;

    if (frame_time > 0.25) {
      frame_time = 0.25;
    }
    accumulator += frame_time;

    AppState *app_ptr = static_cast<AppState *>(glfwGetWindowUserPointer(window));

    // camera
    glfwPollEvents();
    if (app_ptr->is_mouse_captured) {
      process_input(window, *app_ptr->camera, static_cast<float>(frame_time), &app);
    }

    // physics update
    while (accumulator >= dt) {
      if (!app_ptr->is_paused) {
        app_ptr->simulation.update(dt * app_ptr->simulation_speed);
        app_ptr->simulation.remove_marked_bodies(); // erase mass=0 bodies 
      }
      accumulator -= dt;
    }

    // rendering
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto &bodies = app_ptr->simulation.get_bodies();
    float aspect_ratio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

    glUseProgram(shader_program);
    glUniform3fv(glGetUniformLocation(shader_program, "camera_pos"), 1, glm::value_ptr(app_ptr->camera->m_position));
    glUniform3fv(glGetUniformLocation(shader_program, "camera_front"), 1, glm::value_ptr(app_ptr->camera->m_front));
    glUniform3fv(glGetUniformLocation(shader_program, "camera_up"), 1, glm::value_ptr(app_ptr->camera->m_up));
    glUniform3fv(glGetUniformLocation(shader_program, "camera_right"), 1, glm::value_ptr(app_ptr->camera->m_right));
    glUniform1f(glGetUniformLocation(shader_program, "aspect_ratio"), aspect_ratio);
    glUniform1i(glGetUniformLocation(shader_program, "num_bodies"), bodies.size());
    glUniform1i(glGetUniformLocation(shader_program, "lighting_enabled"), app_ptr->gui_props.lighting_enabled);

    for (size_t i = 0; i < bodies.size(); i++) {
      std::string index = "bodies[" + std::to_string(i) + "]";
      CalestialBodyData body_data = {glm::vec3(bodies[i].position),
                                     bodies[i].color,
                                     static_cast<float>(bodies[i].radius)};
      glUniform3fv(glGetUniformLocation(shader_program, (index + ".position").c_str()), 1, glm::value_ptr(body_data.position));
      glUniform1f(glGetUniformLocation(shader_program, (index + ".radius").c_str()), body_data.radius);
      glUniform3fv(glGetUniformLocation(shader_program, (index + ".color").c_str()), 1, glm::value_ptr(body_data.color));
    }

    glBindVertexArray(quad_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    if (app_ptr->gui_visible) {
      render_gui(*app_ptr);
    } else {
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    glfwSwapBuffers(window);

    auto frame_end = Clock::now();
    auto elapsed_time = std::chrono::duration<double>(frame_end - frame_start).count();
    double sleep_duration = TARGET_FRAME_TIME - elapsed_time;

    if (sleep_duration > 0) {
      std::this_thread::sleep_for(std::chrono::duration<double>(sleep_duration * 0.9));

      auto spin_start = Clock::now();
      do {
        std::this_thread::yield();
      } while (std::chrono::duration<double>(Clock::now() - spin_start).count() < sleep_duration * 0.1);
    }

    frame_start = Clock::now();
  }

  // cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glDeleteVertexArrays(1, &quad_VAO);
  glDeleteBuffers(1, &quad_VBO);
  glDeleteProgram(shader_program);
}
