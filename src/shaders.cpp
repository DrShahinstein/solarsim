#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include "shaders.hpp"
#include "simulation.hpp"

constexpr const char *VERTEX_SHADER = "shaders/vertex_shader.glsl";
constexpr const char *FRAGMENT_SHADER = "shaders/fragment_shader.glsl";

std::string load_shader(const char *shader_path) {
  std::ifstream shader_file(shader_path);
  if (!shader_file) {
    std::cerr << "Could not open shader file: " << shader_path << std::endl;
    return "";
  }
  std::stringstream shader_stream;
  shader_stream << shader_file.rdbuf();
  return shader_stream.str();
}

std::string inject_constants(std::string source) {
  const std::string defines = "\n#define MAX_BODIES " + std::to_string(MAX_BODIES);
  const size_t version_end = source.find('\n');
  if (version_end == std::string::npos) return source;
  source.insert(version_end, defines);
  return source;
}

unsigned int compile_shader(GLenum type, const char *source, const char *label) {
  unsigned int shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  int success;
  char info_log[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, nullptr, info_log);
    std::cerr << "ERROR::SHADER::" << label << "::COMPILATION_FAILED\n" << info_log << std::endl;
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

unsigned int create_shader_program() {
  std::string vertex_shader_str   = load_shader(VERTEX_SHADER);
  std::string fragment_shader_str = inject_constants(load_shader(FRAGMENT_SHADER));
  const char *vertex_shader_src   = vertex_shader_str.c_str();
  const char *fragment_shader_src = fragment_shader_str.c_str();

  if (vertex_shader_str.empty() || fragment_shader_str.empty()) {
    std::cerr << "Shaders could not be loaded\n";
    return 0;
  }

  unsigned int vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_src, "VERTEX");
  if (vertex_shader == 0) return 0;

  unsigned int fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_src, "FRAGMENT");
  if (fragment_shader == 0) {
    glDeleteShader(vertex_shader);
    return 0;
  }

  unsigned int shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);

  int success;
  char info_log[512];
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shader_program, 512, nullptr, info_log);
    std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
    glDeleteProgram(shader_program);
    shader_program = 0;
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return shader_program;
}
