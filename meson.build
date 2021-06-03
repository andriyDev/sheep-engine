
project('sheep', 'cpp', default_options: ['cpp_std=c++17'])

absl_dep = dependency('absl', modules: [
  'absl::flat_hash_map',
  'absl::flat_hash_set',
  'absl::status',
  'absl::statusor',
])
glfw_dep = dependency('glfw3')
gl_dep = dependency('gl')
glew_dep = dependency('glew')
glog_dep = dependency('glog')

inc = include_directories('include/')

executable('sheep', [
  'src/engine.cpp',
  'src/main.cpp',
  'src/mesh.cpp',
  'src/mesh_formats/obj_mesh.cpp',
  'src/nodes/camera.cpp',
  'src/nodes/mesh_renderer.cpp',
  'src/nodes/node.cpp',
  'src/nodes/transform.cpp',
  'src/nodes/utility.cpp',
  'src/resource.cpp',
  'src/shader.cpp',
  'src/systems/input_system.cpp',
  'src/systems/render_system.cpp',
  'src/systems/super_system.cpp',
  'src/systems/system.cpp',
  'src/world.cpp'
], dependencies: [
  absl_dep,
  gl_dep,
  glew_dep,
  glfw_dep,
  glog_dep,
], include_directories: inc)
