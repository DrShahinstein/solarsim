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
#include "helper.hpp"
#include "simulation.hpp"
#include "gui.hpp"
#include "window.h"

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
  // shader arrangements
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
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << info_log << std::endl;
  }

  unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_src, NULL);
  glCompileShader(fragment_shader);

  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << info_log << std::endl;
  }

  unsigned int shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);

  if (!success) {
    glGetProgramInfoLog(shader_program, 512, NULL, info_log);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << info_log << std::endl;
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader); // shaders end

  // full-screen quad for ray marching
  float quad_vertices[] = {
      -1.0f, 1.0f,  // top-left
      -1.0f, -1.0f, // bottom-left
      1.0f,  -1.0f, // bottom-right

      -1.0f, 1.0f,  // top-left
      1.0f,  -1.0f, // bottom-right
      1.0f,  1.0f   // top-right
  };

  unsigned int quad_VAO, quad_VBO;
  glGenVertexArrays(1, &quad_VAO);
  glGenBuffers(1, &quad_VBO);
    
  glBindVertexArray(quad_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, quad_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // prepare app
  initialize_imgui(window);
  Simulation simulation;
  simulation.reset_to_solar_system();
  AppState app {
    .simulation = simulation,
    .is_mouse_captured = true,
    .is_paused = false,
    .gui_visible = false,
    .simulation_speed = 1.0f
  };
  glfwSetCursorPos(window, SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0);
  glfwSetWindowUserPointer(window, &app);

  // camera setup
  static glm::vec3 camera_pos(0.0f, 0.0f, 3.0f);
  static glm::vec3 camera_front(0.0f, 0.0f, -1.0f);
  static glm::vec3 camera_up(0.0f, 1.0f, 0.0f);
  static float yaw = -90.0f;
  static float pitch = 0.0f;
  static bool reset_mouse_position = true;

  glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
    AppState *app = static_cast<AppState *>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      if (app->is_mouse_captured) {
        glfwGetCursorPos(window, &app->mouse_x, &app->mouse_y);
        app->is_mouse_captured = false;
        app->gui_visible = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetCursorPos(window, app->mouse_x, app->mouse_y);
      } else {
        app->is_mouse_captured = true;
        app->gui_visible = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPos(window, SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0);
        reset_mouse_position = true;
      }
    }

    if (app->is_mouse_captured) camera_callback(window, key, scancode, action, mods, camera_pos, camera_front, camera_up);
    if (action == GLFW_PRESS && app->is_mouse_captured) {
      switch (key)
      {
      case GLFW_KEY_R:
        app->simulation.reset_to_solar_system();
        break;

      case GLFW_KEY_B: {
        CelestialBody black_hole;
        black_hole.position = glm::dvec3(camera_pos);
        black_hole.velocity = glm::dvec3(0.0);
        black_hole.mass = 10.0;
        black_hole.radius = 0.05;
        black_hole.color = glm::vec3(0.0, 1.0, 0.0);
        black_hole.is_black_hole = true;
        app->simulation.add_body(black_hole);
        break;
      }

      case GLFW_KEY_UP:
        for (auto &body : app->simulation.get_bodies()) {
          if (!body.is_black_hole) {
            body.radius *= 1.2;
          }
        }
        break;

      case GLFW_KEY_DOWN:
        for (auto &body : app->simulation.get_bodies()) {
          if (!body.is_black_hole) {
            body.radius *= 0.8;
          }
        }
        break;

      default:
        break;
      }
    }
    });

  // setup mouse
  glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos) {
    AppState *app = static_cast<AppState *>(glfwGetWindowUserPointer(window));
    
    if (app->is_mouse_captured) {
      mouse_callback(window, xpos, ypos, yaw, pitch, camera_front, reset_mouse_position);
    }
  });

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glEnable(GL_DEPTH_TEST);

  // frame limiter
  const int FPS = 60;
  const auto frame_duration = std::chrono::milliseconds(1000 / FPS);

  // loop
  while (!glfwWindowShouldClose(window)) {
    auto frame_start = std::chrono::steady_clock::now();

    AppState *app = static_cast<AppState *>(glfwGetWindowUserPointer(window));

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (!app->is_paused) app->simulation.update(1.0 / (float)FPS * app->simulation_speed);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto &bodies = app->simulation.get_bodies();

    // calculate camera vectors
    glm::vec3 camera_right  = glm::normalize(glm::cross(camera_front, camera_up));
    glm::vec3 camera_up_vec = glm::cross(camera_right, camera_front);
    float aspect_ratio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

    glUseProgram(shader_program);
    glUniform3fv(glGetUniformLocation(shader_program, "camera_pos"), 1, glm::value_ptr(camera_pos));
    glUniform3fv(glGetUniformLocation(shader_program, "camera_front"), 1, glm::value_ptr(camera_front));
    glUniform3fv(glGetUniformLocation(shader_program, "camera_up"), 1, glm::value_ptr(camera_up_vec));
    glUniform3fv(glGetUniformLocation(shader_program, "camera_right"), 1, glm::value_ptr(camera_right));
    glUniform1f(glGetUniformLocation(shader_program, "aspect_ratio"), aspect_ratio);
    glUniform1i(glGetUniformLocation(shader_program, "num_bodies"), bodies.size());

    for (size_t i = 0; i < bodies.size(); i++) {
      std::string index = "bodies[" + std::to_string(i) + "]";
      CalestialBodyData body_data = {
        glm::vec3(bodies[i].position),
        bodies[i].color,
        static_cast<float>(bodies[i].radius)
      };

      glUniform3fv(glGetUniformLocation(shader_program, (index + ".position").c_str()), 1, glm::value_ptr(body_data.position));
      glUniform1f(glGetUniformLocation(shader_program, (index + ".radius").c_str()), body_data.radius);
      glUniform3fv(glGetUniformLocation(shader_program, (index + ".color").c_str()), 1, glm::value_ptr(body_data.color));
    }

    glBindVertexArray(quad_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    if (app->gui_visible) {
      render_gui(*app);
    } else {
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    glfwSwapBuffers(window);
    glfwPollEvents();

    auto frame_end = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(frame_duration - (frame_end - frame_start));
  }

  // cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glDeleteVertexArrays(1, &quad_VAO);
  glDeleteBuffers(1, &quad_VBO);
  glDeleteProgram(shader_program);
}
