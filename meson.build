project('solarsim', ['cpp', 'c'],
  version : '0.1.0',
  default_options : ['cpp_std=c++20', 'c_std=c17'])

is_windows = host_machine.system() == 'windows' or \
             host_machine.system() == 'msys' or \
             host_machine.system() == 'cygwin'

if is_windows
  opengl_dep = declare_dependency(link_args : ['-lopengl32'])
  message('Detected Windows, linking with opengl32.lib')
else
  opengl_dep = dependency('opengl', required : true)
  message('Non-Windows platform, using standard OpenGL dependency')
endif

if is_windows
  glfw_inc = include_directories('glfw/include')
  glfw_lib = meson.current_source_dir() / 'glfw' / 'lib-mingw' / 'glfw3.dll'
  glfw_dep = declare_dependency(
    include_directories : glfw_inc,
    link_args           : [glfw_lib],
    dependencies        : [opengl_dep]
  )

else
  glfw_dep = dependency('glfw3', required : true)
  message('Using system-installed GLFW')
endif

inc = include_directories(
  'include',
  'glad/include',
  'imgui',
  'imgui/backends',
)

glm_inc = include_directories('glm')

app_sources = files(
  'src/main.cpp',
  'src/helper.cpp',
  'src/gui.cpp',
  'src/simulation.cpp',
  'src/mainloop.cpp',
)

glad_sources = files('glad/src/glad.c')

imgui_sources = files(
  'imgui/imgui.cpp',
  'imgui/imgui_draw.cpp',
  'imgui/imgui_widgets.cpp',
  'imgui/imgui_tables.cpp',
  'imgui/backends/imgui_impl_glfw.cpp',
  'imgui/backends/imgui_impl_opengl3.cpp',
)

program_install_subdir = 'app'

executable('solarsim',
  app_sources,
  glad_sources,
  imgui_sources,
  include_directories : [inc, glm_inc],
  dependencies        : [opengl_dep, glfw_dep],
  link_args           : ['-mwindows'],
  install             : true,
  install_dir         : program_install_subdir
)

if is_windows
  install_data(meson.current_source_dir() / 'glfw' / 'lib-mingw' / 'glfw3.dll',
    install_dir : program_install_subdir
  )
endif

install_subdir('shaders',
  install_dir : program_install_subdir
)