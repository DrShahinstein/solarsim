#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <thread>
#include <iostream>
#include "mainloop.hpp"
#include "helper.hpp"
#include "simulation.hpp"
#include "window.h"

struct Vertex {
  glm::vec3 position;
  glm::vec3 color;
};

void mainloop(GLFWwindow *window) {
  // shader arrangements
  const char *vertex_shader_src   = load_shader("shaders/vertex_shader.glsl");
  const char *fragment_shader_src = load_shader("shaders/fragment_shader.glsl");

  if (!vertex_shader_src || !fragment_shader_src) {
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
  glDeleteShader(fragment_shader);
  delete[] vertex_shader_src;
  delete[] fragment_shader_src;

  // init simulation
  Simulation simulation;
  simulation.reset_to_solar_system();
  glfwSetWindowUserPointer(window, &simulation);

  // camera setup
  static glm::vec3 camera_pos(0.0f, 0.0f, 5.0f);
  static glm::vec3 camera_front(0.0f, 0.0f, -1.0f);
  static glm::vec3 camera_up(0.0f, 1.0f, 0.0f);
  static float yaw = -90.0f;
  static float pitch = 0.0f;

  glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
    camera_callback(window, key, scancode, action, mods, camera_pos, camera_front, camera_up);

    if (action == GLFW_PRESS) {
        Simulation* simulation = static_cast<Simulation*>(glfwGetWindowUserPointer(window));
        if (!simulation) return;
        
        if (key == GLFW_KEY_R) simulation->reset_to_solar_system();
        else if (key == GLFW_KEY_B) {
            CelestialBody black_hole;
            black_hole.position = glm::dvec3(camera_pos);
            black_hole.velocity = glm::dvec3(0.0);
            black_hole.mass = 10.0; // 10 solar masses
            black_hole.radius = 0.05;
            black_hole.color = glm::vec3(0.0, 1.0, 0.0);
            black_hole.is_black_hole = true;
            simulation->add_body(black_hole);
        }

        else if (key == GLFW_KEY_UP) {
            // increase planet sizes
            for (auto& body : simulation->get_bodies()) {
                if (!body.is_black_hole) {
                    body.radius *= 1.2;
                }
            }
        }

        else if (key == GLFW_KEY_DOWN) {
            // decrease planet sizes
            for (auto& body : simulation->get_bodies()) {
                if (!body.is_black_hole) {
                    body.radius *= 0.8;
                }
            }
        }}
  });

  // setup mouse
  glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos) {
    mouse_callback(window, xpos, ypos, yaw, pitch, camera_front);
  });
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // 3d grid
  const int grid_size = 20;
  const float grid_spacing = 0.5f;
  std::vector<Vertex> grid_vertices;

  for (int i = -grid_size; i <= grid_size; ++i) {
    grid_vertices.push_back(
        {{i * grid_spacing, 0.0f, -grid_size * grid_spacing},
         {0.3f, 0.3f, 0.3f}});

    grid_vertices.push_back({{i * grid_spacing, 0.0f, grid_size * grid_spacing},
                             {0.3f, 0.3f, 0.3f}});

    grid_vertices.push_back(
        {{-grid_size * grid_spacing, 0.0f, i * grid_spacing},
         {0.3f, 0.3f, 0.3f}});

    grid_vertices.push_back({{grid_size * grid_spacing, 0.0f, i * grid_spacing},
                             {0.3f, 0.3f, 0.3f}});
  }

  // VAO/VBO for grid
  unsigned int grid_VAO, grid_VBO;
  glGenVertexArrays(1, &grid_VAO);
  glGenBuffers(1, &grid_VBO);

  glBindVertexArray(grid_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, grid_VBO);
  glBufferData(GL_ARRAY_BUFFER, grid_vertices.size() * sizeof(Vertex), grid_vertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));
  glEnableVertexAttribArray(1);

  // VAO/VBO for planets
  unsigned int bodies_VAO, bodies_VBO;
  glGenVertexArrays(1, &bodies_VAO);
  glGenBuffers(1, &bodies_VBO);

  glBindVertexArray(bodies_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, bodies_VBO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));
  glEnableVertexAttribArray(1);

  glEnable(GL_DEPTH_TEST);

  // frame limiter
  const int FPS = 60;
  const auto frame_duration = std::chrono::milliseconds(1000 / FPS);

  // loop
  while (!glfwWindowShouldClose(window)) {
    auto frame_start = std::chrono::steady_clock::now();

    esc_quit(window);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    simulation.update(1.0 / (float)FPS);
    const auto &bodies = simulation.get_bodies();

    std::vector<Vertex> body_vertices;
    for (const auto &body : bodies) {
      body_vertices.push_back({glm::vec3(body.position), body.color});
    }

    glBindBuffer(GL_ARRAY_BUFFER, bodies_VBO);
    glBufferData(GL_ARRAY_BUFFER, body_vertices.size() * sizeof(Vertex), body_vertices.data(), GL_DYNAMIC_DRAW);

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view =
        glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);

    glUseProgram(shader_program);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE, &view[0][0]);
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, &model[0][0]);

    glBindVertexArray(grid_VAO);
    glDrawArrays(GL_LINES, 0, grid_vertices.size());

    glPointSize(8.0f);
    glBindVertexArray(bodies_VAO);
    glDrawArrays(GL_POINTS, 0, body_vertices.size());

    glfwSwapBuffers(window);
    glfwPollEvents();

    auto frame_end = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(frame_duration - (frame_end - frame_start));
  }

  // cleanup
  glDeleteVertexArrays(1, &grid_VAO);
  glDeleteBuffers(1, &grid_VBO);
  glDeleteVertexArrays(1, &bodies_VAO);
  glDeleteBuffers(1, &bodies_VBO);
  glDeleteProgram(shader_program);
}
