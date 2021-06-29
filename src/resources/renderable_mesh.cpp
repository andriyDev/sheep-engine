
#include "resources/renderable_mesh.h"

#include <memory>

#include "resources/resource.h"
#include "utility/status.h"

absl::StatusOr<std::shared_ptr<RenderableMesh>> RenderableMesh::Load(
    const Details& details) {
  ASSIGN_OR_RETURN((const std::shared_ptr<Mesh> source_mesh),
                   details.mesh.Get());
  std::shared_ptr<Skin> source_skin;
  if (details.skin.has_value()) {
    ASSIGN_OR_RETURN((source_skin), details.skin->Get());

    if (source_mesh->vertices.size() != source_skin->vertices.size()) {
      return absl::FailedPreconditionError(STATUS_MESSAGE(
          "Source mesh and source skin contain differing number of vertices: "
          << source_mesh->vertices.size()
          << "(mesh) != " << source_skin->vertices.size() << "(skin)"));
    }
  }

  if (source_mesh->triangles.size() > 0 &&
      source_mesh->small_triangles.size() > 0) {
    return absl::FailedPreconditionError(
        "Source mesh contains both large- and small-indexed triangles.");
  }
  std::shared_ptr<RenderableMesh> new_mesh(new RenderableMesh());
  new_mesh->vertex_attribute_count = (source_skin ? 8 : 6);
  if (source_mesh->triangles.size() == 0 &&
      source_mesh->small_triangles.size() == 0) {
    new_mesh->indexing = Indexing::None;
    new_mesh->elements = source_mesh->vertices.size();
    new_mesh->buffers.resize(source_skin ? 2 : 1);
  } else {
    new_mesh->indexing =
        source_mesh->triangles.size() > 0 ? Indexing::Large : Indexing::Small;
    new_mesh->elements = (new_mesh->indexing == Indexing::Large
                              ? source_mesh->triangles.size()
                              : source_mesh->small_triangles.size()) *
                         3;
    new_mesh->buffers.resize(source_skin ? 3 : 2);
  }

  glGenVertexArrays(1, &new_mesh->vao);
  glBindVertexArray(new_mesh->vao);
  glGenBuffers(new_mesh->buffers.size(), new_mesh->buffers.data());
  glBindBuffer(GL_ARRAY_BUFFER, new_mesh->buffers[0]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(Mesh::Vertex) * source_mesh->vertices.size(),
               source_mesh->vertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
                        (void*)offsetof(Mesh::Vertex, position));
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
                        (void*)offsetof(Mesh::Vertex, texCoord));
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
                        (void*)offsetof(Mesh::Vertex, colour));
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
                        (void*)offsetof(Mesh::Vertex, normal));
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
                        (void*)offsetof(Mesh::Vertex, tangent));
  glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
                        (void*)offsetof(Mesh::Vertex, bitangent));

  if (source_skin) {
    glBindBuffer(GL_ARRAY_BUFFER, new_mesh->buffers[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(Skin::Vertex) * source_skin->vertices.size(),
                 source_skin->vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Skin::Vertex),
                          (void*)offsetof(Skin::Vertex, weights));
    glVertexAttribIPointer(7, 4, GL_UNSIGNED_INT, sizeof(Skin::Vertex),
                           (void*)offsetof(Skin::Vertex, bone_indices));
  }

  if (new_mesh->indexing != Indexing::None) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                 new_mesh->buffers[source_skin ? 2 : 1]);
    if (new_mesh->indexing == Indexing::Small) {
      glBufferData(
          GL_ELEMENT_ARRAY_BUFFER,
          sizeof(Mesh::SmallTriangle) * source_mesh->small_triangles.size(),
          source_mesh->small_triangles.data(), GL_STATIC_DRAW);
    } else {
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   sizeof(Mesh::Triangle) * source_mesh->triangles.size(),
                   source_mesh->triangles.data(), GL_STATIC_DRAW);
    }
  }
  return new_mesh;
}

RenderableMesh::~RenderableMesh() {
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(buffers.size(), buffers.data());
}

void RenderableMesh::Draw() {
  glBindVertexArray(vao);
  for (unsigned int i = 0; i < vertex_attribute_count; i++) {
    glEnableVertexAttribArray(i);
  }
  if (indexing == Indexing::None) {
    glDrawArrays(GL_TRIANGLES, 0, elements);
  } else {
    glDrawElements(
        GL_TRIANGLES, elements,
        indexing == Indexing::Small ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
        (void*)0);
  }
  for (unsigned int i = 0; i < vertex_attribute_count; i++) {
    glDisableVertexAttribArray(i);
  }
}
