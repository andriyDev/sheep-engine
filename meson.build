
project('sheep', 'cpp', default_options: ['cpp_std=c++17'])

inc = include_directories('include/')

glfw_dep = dependency('glfw3')
gl_dep = dependency('gl')
glew_dep = dependency('glew')

executable('sheep', [
  'src/main.cpp',
  'src/mesh.cpp',
  'src/resource.cpp',
  'src/shader.cpp'
], dependencies: [ glfw_dep, gl_dep, glew_dep ], include_directories: inc)
