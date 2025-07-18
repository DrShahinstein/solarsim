#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include "shaders.hpp"

#define VERTEX_SHADER   "shaders/vertex_shader.glsl"
#define FRAGMENT_SHADER "shaders/fragment_shader.glsl"

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

unsigned int create_shader_program() {
  std::string vertex_shader_str   = load_shader(VERTEX_SHADER);
  std::string fragment_shader_str = load_shader(FRAGMENT_SHADER);
  const char *vertex_shader_src   = vertex_shader_str.c_str();
  const char *fragment_shader_src = fragment_shader_str.c_str();

  if (vertex_shader_str.empty() || fragment_shader_str.empty()) {
    std::cerr << "Shaders could not be loaded\n";
    return 0;
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

  return shader_program;
}