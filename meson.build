project('solarsim', ['cpp', 'c'],
  version : '0.1.0',
  default_options : [
    'cpp_std=c++20',
    'c_std=c17',
    'buildtype=release',
    'default_library=static',
    'wrap_mode=forcefallback'
  ])

system = host_machine.system()
is_windows = system == 'windows' or system == 'msys' or system == 'cygwin'

if is_windows
  opengl_dep = declare_dependency(link_args : ['-lopengl32'])
else
  opengl_dep = dependency('gl', required : true)
endif

glfw_dep  = dependency('glfw3', version : '>=3.4')
glm_dep   = dependency('glm', version : '>=0.9.9.8')
imgui_dep = dependency('imgui', default_options : [
  'glfw=enabled',
  'opengl=enabled',
  'freetype=disabled',
  'dx9=disabled',
  'dx10=disabled',
  'dx11=disabled',
  'dx12=disabled',
  'metal=disabled',
  'sdl2_renderer=disabled',
  'sdl3_renderer=disabled',
  'sdl3_gpu=disabled',
  'vulkan=disabled',
  'webgpu=disabled',
  'sdl2=disabled',
  'sdl3=disabled',
  'osx=disabled',
  'win=disabled',
  'allegro5=disabled',
])

inc = include_directories(
  'include',
  'vendor/glad/include',
)

app_sources = files(
  'src/main.cpp',
  'src/callbacks.cpp',
  'src/gui.cpp',
  'src/simulation.cpp',
  'src/mainloop.cpp',
  'src/shaders.cpp'
)

glad_sources = files('vendor/glad/src/glad.c')

program_install_subdir = 'dist'

executable('solarsim',
  app_sources,
  glad_sources,
  include_directories : inc,
  dependencies        : [opengl_dep, glfw_dep, glm_dep, imgui_dep],
  link_args           : is_windows ? ['-mwindows'] : [],
  install             : true,
  install_dir         : program_install_subdir
)

install_subdir(
  'shaders',
  install_dir : program_install_subdir,
)

