
#include "resources/skinned_mesh.h"

absl::StatusOr<std::shared_ptr<SkinnedMesh>> SkinnedMesh::Load(
    const Details& details) {
  ASSIGN_OR_RETURN((const std::shared_ptr<Mesh> source_mesh),
                   details.mesh.Get());
  ASSIGN_OR_RETURN((const std::shared_ptr<Skin> source_skin),
                   details.skin.Get());

  if (source_mesh->vertices.size() != source_skin->vertices.size()) {
    return absl::FailedPreconditionError(STATUS_MESSAGE(
        "Source mesh and source skin contain differing number of vertices: "
        << source_mesh->vertices.size()
        << "(mesh) != " << source_skin->vertices.size() << "(skin)"));
  }

  if (source_mesh->triangles.size() > 0 &&
      source_mesh->small_triangles.size() > 0) {
    return absl::FailedPreconditionError(
        "Source mesh contains both large- and small-indexed triangles.");
  }
  std::shared_ptr<SkinnedMesh> new_mesh(new SkinnedMesh());
  new_mesh->vertex_attribute_count = 8;
  if (source_mesh->triangles.size() == 0 &&
      source_mesh->small_triangles.size() == 0) {
    new_mesh->indexing = Indexing::None;
    new_mesh->elements = source_mesh->vertices.size();
    new_mesh->buffers.resize(2);
  } else {
    new_mesh->indexing =
        source_mesh->triangles.size() > 0 ? Indexing::Large : Indexing::Small;
    new_mesh->elements = (new_mesh->indexing == Indexing::Large
                              ? source_mesh->triangles.size()
                              : source_mesh->small_triangles.size()) *
                         3;
    new_mesh->buffers.resize(3);
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

  glBindBuffer(GL_ARRAY_BUFFER, new_mesh->buffers[1]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(Skin::Vertex) * source_skin->vertices.size(),
               source_skin->vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Skin::Vertex),
                        (void*)offsetof(Skin::Vertex, weights));
  glVertexAttribIPointer(7, 4, GL_UNSIGNED_SHORT, sizeof(Skin::Vertex),
                         (void*)offsetof(Skin::Vertex, bone_indices));

  if (new_mesh->indexing != Indexing::None) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_mesh->buffers[2]);
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

void SkinnedMesh::DrawSkinned() { Draw(); }
