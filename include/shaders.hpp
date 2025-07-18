#ifndef SHADERS_HPP
#define SHADERS_HPP

#include <string>

std::string load_shader(const char *shader_path);
unsigned int create_shader_program();

#endif