project('solarsim', ['cpp', 'c'],
  version : '0.1.0',
  default_options : [
    'cpp_std=c++20',
    'c_std=c17',
    'buildtype=release',
    'default_library=static'
  ])

system = host_machine.system()
is_windows = system == 'windows' or system == 'msys' or system == 'cygwin'
is_linux = system == 'linux'
is_darwin = system == 'darwin'

if is_windows
  opengl_dep = declare_dependency(link_args : ['-lopengl32'])
elif is_darwin
  app_frameworks = ['-framework', 'OpenGL', '-framework', 'Cocoa', 
                    '-framework', 'IOKit', '-framework', 'CoreVideo']
  opengl_dep = declare_dependency(link_args : app_frameworks)
else
  opengl_dep = dependency('gl', required : true)
endif

if is_windows
  glfw_inc = include_directories('glfw/include')
  glfw_lib = meson.current_source_dir() / 'glfw' / 'lib-mingw' / 'glfw3.dll'
  glfw_dep = declare_dependency(
    include_directories : glfw_inc,
    link_args           : [glfw_lib],
    dependencies        : [opengl_dep]
  )
elif is_darwin
  glfw_dep = dependency('glfw3', required : false)
  if not glfw_dep.found()
    glfw_path = '/opt/homebrew/lib'  # For ARM Macs
    if not run_command('test', '-d', glfw_path, check: false).returncode() == 0
      glfw_path = '/usr/local/lib'   # For Intel Macs
    endif
    glfw_dep = declare_dependency(
      link_args : ['-L' + glfw_path, '-lglfw'],
      dependencies : [opengl_dep]
    )
  endif
else
  glfw_dep = dependency('glfw3', required : true)
endif

linux_deps = []
if is_linux
  linux_deps = [
    dependency('x11'),
    dependency('xi'),
    dependency('xrandr'),
    dependency('xinerama'),
    dependency('xcursor')
  ]
endif

inc = include_directories(
  'include',
  'glad/include',
  'imgui',
  'imgui/backends',
)

glm_inc = include_directories('glm', is_system: true)

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
  dependencies        : [opengl_dep, glfw_dep] + linux_deps,
  link_args           : is_windows ? ['-mwindows'] : [],
  install             : true,
  install_dir         : program_install_subdir
)

if is_windows
  install_data(
    meson.current_source_dir() / 'glfw' / 'lib-mingw' / 'glfw3.dll',
    install_dir : program_install_subdir
  )
endif

install_subdir(
  'shaders',
  install_dir : program_install_subdir,
)