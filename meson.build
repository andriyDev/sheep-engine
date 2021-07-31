
project('sheep', 'cpp', default_options: ['cpp_std=c++17'])

absl_dep = dependency('absl', modules: [
  'absl::flat_hash_map',
  'absl::flat_hash_set',
  'absl::status',
  'absl::statusor',
  'absl::flags',
  'absl::flags_parse',
  'absl::flags_usage',
])
gl_dep = dependency('gl')
glm_dep = dependency('glm')
glew_dep = dependency('glew')
glfw_dep = dependency('glfw3', modules: [ 'glfw' ])
glog_dep = dependency('glog')
json_dep = dependency('nlohmann_json')
png_dep = dependency('PNG', modules: [ 'PNG::PNG' ])

inc = include_directories('include/')

executable('sheep', [
  'src/engine.cpp',
  'src/main.cpp',
  'src/nodes/camera.cpp',
  'src/nodes/mesh_renderer.cpp',
  'src/nodes/skinned_mesh_renderer.cpp',
  'src/nodes/node.cpp',
  'src/nodes/transform.cpp',
  'src/nodes/utility.cpp',
  'src/resources/transit/mesh.cpp',
  'src/resources/transit/transit.cpp',
  'src/resources/mesh_formats/obj_mesh.cpp',
  'src/resources/renderable_mesh.cpp',
  'src/resources/resource.cpp',
  'src/resources/shader.cpp',
  'src/resources/skeleton.cpp',
  'src/resources/skinned_mesh.cpp',
  'src/resources/texture.cpp',
  'src/resources/texture_formats/png_texture.cpp',
  'src/systems/input_system.cpp',
  'src/systems/render_system.cpp',
  'src/systems/super_system.cpp',
  'src/systems/system.cpp',
  'src/utility/json.cpp',
  'src/utility/scope_cleanup.cpp',
  'src/world.cpp'
], dependencies: [
  absl_dep,
  gl_dep,
  glm_dep,
  glew_dep,
  glfw_dep,
  glog_dep,
  json_dep,
  png_dep,
], include_directories: inc)

subdir('tools')
