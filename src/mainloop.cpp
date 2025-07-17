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
#include <chrono>
#include "mainloop.hpp"
#include "helper.hpp"
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
  std::string vertex_shader_str   = load_shader("shaders/vertex_shader.glsl");
  std::string fragment_shader_str = load_shader("shaders/fragment_shader.glsl");
  const char *vertex_shader_src   = vertex_shader_str.c_str();
  const char *fragment_shader_src = fragment_shader_str.c_str();

  if (vertex_shader_str.empty() || fragment_shader_str.empty()) {
    std::cerr << "Shaders could not be loaded\n";
    return;
  }

  unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_src, NULL);
  glCompileShader(vertex_shader);
  int success;
  char info_log[512];
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << info_log << std::endl;
  }

  unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_src, NULL);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << info_log << std::endl;
  }

  unsigned int shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shader_program, 512, NULL, info_log);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
  }
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

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

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      if (app->is_mouse_captured) {
        glfwGetCursorPos(window, &app->lastX, &app->lastY);
        app->is_mouse_captured = false;
        app->gui_visible = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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
        CelestialBody black_hole;
        black_hole.position      = glm::dvec3(app->camera->m_position + app->camera->m_front * 1.0f);
        black_hole.velocity      = glm::dvec3(0.0);
        black_hole.mass          = 100.0;
        black_hole.radius        = 0.2;
        black_hole.color         = glm::vec3(0.0, 1.0, 0.0);
        black_hole.is_black_hole = true;
        app->simulation.add_body(black_hole);
        break;
      }
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

  // game-loop timing (=> decouples fps from physics updates)
  double t = 0.0;
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
      }
      accumulator -= dt;
      t += dt;
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