
rc_inc = include_directories('include')

executable('resource_converter', [
  'src/main.cpp',
  'src/resources/transit/mesh.cpp',
  'src/resources/transit/transit_write.cpp',
], include_directories: [
  inc,
  rc_inc,
], dependencies: [
  absl_dep,
  glm_dep,
  glog_dep,
  json_dep,
])
