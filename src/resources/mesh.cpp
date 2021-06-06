
#include "resources/mesh.h"

#include <memory>

#include "resources/resource.h"
#include "utility/status.h"

absl::StatusOr<std::shared_ptr<RenderableMesh>> RenderableMesh::Load(
    const Details& details) {
  ASSIGN_OR_RETURN(const std::shared_ptr<Mesh> source_mesh, details.mesh.Get());

  if (source_mesh->triangles.size() > 0 &&
      source_mesh->small_triangles.size() > 0) {
    return absl::FailedPreconditionError(
        "Source mesh contains both large- and small-indexed triangles.");
  }
  std::shared_ptr<RenderableMesh> new_mesh(new RenderableMesh());
  new_mesh->vertex_attribute_count = 5;
  new_mesh->use_small_indexing = source_mesh->triangles.size() == 0;
  new_mesh->elements =
      (new_mesh->use_small_indexing ? source_mesh->small_triangles.size()
                                    : source_mesh->triangles.size()) *
      3;
  new_mesh->buffers.resize(2);

  glGenVertexArrays(1, &new_mesh->vao);
  glBindVertexArray(new_mesh->vao);
  glGenBuffers(2, new_mesh->buffers.data());
  glBindBuffer(GL_ARRAY_BUFFER, new_mesh->buffers[0]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(Mesh::Vertex) * source_mesh->vertices.size(),
               source_mesh->vertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
                        (void*)offsetof(Mesh::Vertex, position));
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
                        (void*)offsetof(Mesh::Vertex, texCoord));
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
                        (void*)offsetof(Mesh::Vertex, normal));
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
                        (void*)offsetof(Mesh::Vertex, tangent));
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
                        (void*)offsetof(Mesh::Vertex, bitangent));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_mesh->buffers[1]);
  if (new_mesh->use_small_indexing) {
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(Mesh::SmallTriangle) * source_mesh->small_triangles.size(),
        source_mesh->small_triangles.data(), GL_STATIC_DRAW);
  } else {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(Mesh::Triangle) * source_mesh->triangles.size(),
                 source_mesh->triangles.data(), GL_STATIC_DRAW);
  }
  return new_mesh;
}

RenderableMesh::~RenderableMesh() {
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(buffers.size(), buffers.data());
}

void RenderableMesh::Draw() {
  for (unsigned int i = 0; i < vertex_attribute_count; i++) {
    glEnableVertexAttribArray(i);
  }
  glDrawElements(GL_TRIANGLES, elements,
                 use_small_indexing ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                 (void*)0);
  for (unsigned int i = 0; i < vertex_attribute_count; i++) {
    glDisableVertexAttribArray(i);
  }
}
