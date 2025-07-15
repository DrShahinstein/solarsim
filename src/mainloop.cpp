#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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

struct CalestialBodyData {
  glm::vec3 position;
  glm::vec3 color;
  float radius;
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

  // full-screen quad for ray marching
  float quad_vertices[] = {
    -1.0f,  1.0f, // top-left
    -1.0f, -1.0f, // bottom-left
    1.0f, -1.0f,  // bottom-right
        
    -1.0f,  1.0f, // top-left
    1.0f, -1.0f,  // bottom-right
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

    // calculate camera vectors
    glm::vec3 camera_target = camera_pos + camera_front;
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

    glfwSwapBuffers(window);
    glfwPollEvents();

    auto frame_end = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(frame_duration - (frame_end - frame_start));
  }

  // cleanup
  glDeleteVertexArrays(1, &quad_VAO);
  glDeleteBuffers(1, &quad_VBO);
  glDeleteProgram(shader_program);
}
