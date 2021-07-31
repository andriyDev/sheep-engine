
rc_inc = include_directories('include')

executable('resource_converter', [
  'src/gltf_mesh.cpp',
  'src/main.cpp',
  'src/resources/transit/mesh.cpp',
  'src/resources/transit/transit_write.cpp',
  join_paths(meson.source_root(), 'src/utility/json.cpp'),
  join_paths(meson.source_root(), 'src/resources/skeleton.cpp'),
], include_directories: [
  inc,
  rc_inc,
], dependencies: [
  absl_dep,
  glm_dep,
  glog_dep,
  json_dep,
])
