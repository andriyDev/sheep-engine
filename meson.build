
project('sheep', 'cpp')

inc = include_directories('include/')

glfw_dep = dependency('glfw3')
gl_dep = dependency('gl')
glew_dep = dependency('glew')

executable('sheep', 'src/main.cpp', dependencies: [ glfw_dep, gl_dep, glew_dep ], include_directories: inc)
