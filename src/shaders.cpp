#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <iostream>
#include "shaders.hpp"
#include "simulation.hpp"

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <mach-o/dyld.h>
#endif

constexpr const char *VERTEX_SHADER = "vertex_shader.glsl";
constexpr const char *FRAGMENT_SHADER = "fragment_shader.glsl";

std::filesystem::path executable_directory() {
#if defined(_WIN32)
  char buffer[1024];
  GetModuleFileNameA(nullptr, buffer, sizeof(buffer));
  return std::filesystem::path(buffer).parent_path();
#elif defined(__APPLE__) && defined(__MACH__)
  char buffer[1024];
  uint32_t size = sizeof(buffer);
  _NSGetExecutablePath(buffer, &size);
  return std::filesystem::path(buffer).parent_path();
#else
  std::error_code error;
  const auto executable_path = std::filesystem::read_symlink("/proc/self/exe", error);
  if (error) return std::filesystem::current_path();
  return executable_path.parent_path();
#endif
}

std::string shader_path(const char *filename) {
  const auto directory = executable_directory();
  const std::filesystem::path candidates[] = {
      directory / "shaders" / filename,
      directory / ".." / "shaders" / filename,
      std::filesystem::current_path() / "shaders" / filename,
  };

  for (const auto &candidate : candidates) {
    std::error_code error;
    if (std::filesystem::exists(candidate, error)) {
      return candidate.string();
    }
  }

  return (directory / "shaders" / filename).string();
}

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
  std::string vertex_shader_str   = load_shader(shader_path(VERTEX_SHADER).c_str());
  std::string fragment_shader_str = inject_constants(load_shader(shader_path(FRAGMENT_SHADER).c_str()));
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
