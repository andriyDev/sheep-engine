
#include "resources/mesh_formats/obj_mesh.h"

#include <fstream>
#include <sstream>
#include <variant>
#include <vector>

#include "resources/resource.h"

namespace std {

template <>
struct hash<std::tuple<int, int, int>> {
  std::size_t operator()(std::tuple<int, int, int> const& v) const noexcept {
    std::size_t h = std::hash<int>{}(std::get<0>(v));
    h = h ^ (std::hash<int>{}(std::get<1>(v)) << 1);
    h = h ^ (std::hash<int>{}(std::get<2>(v)) << 1);
    return h;
  }
};

}  // namespace std

absl::StatusOr<std::shared_ptr<ObjModel>> ObjModel::Load(
    const Details& details) {
  std::ifstream file;
  file.open(details.file);
  if (!file.is_open()) {
    return absl::NotFoundError(
        STATUS_MESSAGE("Failed to read OBJ file \"" << details.file << "\""));
  }

  std::shared_ptr<ObjModel> model(new ObjModel());

  std::vector<glm::vec3> position_pool;
  std::vector<glm::vec2> tex_coord_pool;
  std::vector<glm::vec3> normal_pool;

  std::shared_ptr<Mesh> current_mesh;
  absl::flat_hash_map<std::tuple<int, int, int>, unsigned int> index_map;
  absl::flat_hash_map<unsigned int, unsigned int> point_number_of_faces;

  int line_number = 0;

  struct Span {
    std::vector<std::string>::const_iterator it;
    std::vector<std::string>::const_iterator end;
  };

  const auto split_string = [](const std::string& text, const char delimiter) {
    std::vector<std::string> split;
    unsigned int last_split = 0;
    for (unsigned int i = 0; i < text.size(); i++) {
      if (text[i] == delimiter) {
        split.push_back(text.substr(last_split, i - last_split));
        last_split = i + 1;
      }
    }
    split.push_back(text.substr(last_split, text.size()));
    return split;
  };
  const auto finalize_current_mesh = [&current_mesh, &point_number_of_faces]() {
    for (const auto& [index, face_count] : point_number_of_faces) {
      current_mesh->vertices[index].normal /= face_count;
    }
    if (current_mesh->vertices.size() <= (uint16_t)-1) {
      current_mesh->small_triangles.reserve(current_mesh->triangles.size());
      for (const Mesh::Triangle& triangle : current_mesh->triangles) {
        current_mesh->small_triangles.push_back(
            {{(uint16_t)triangle.points[0], (uint16_t)triangle.points[1],
              (uint16_t)triangle.points[2]}});
      }
      current_mesh->triangles.clear();
    }
  };
  const auto new_current_mesh =
      [&current_mesh, &model, &point_number_of_faces, &index_map,
       &finalize_current_mesh](const std::string& name) {
        if (current_mesh) {
          finalize_current_mesh();
        }
        current_mesh = std::shared_ptr<Mesh>(new Mesh());
        model->meshes.insert_or_assign(name, current_mesh);
        point_number_of_faces.clear();
        index_map.clear();
      };
  const auto get_word = [](Span& words_span, std::string& word) {
    if (words_span.it == words_span.end) {
      return false;
    }
    word = *words_span.it++;
    return true;
  };
  const auto parse_float = [](const std::string& word, float& value) {
    size_t idx;
    value = std::stof(word, &idx);
    return idx == word.length();
  };
  const auto parse_int = [](const std::string& word, int& value) {
    size_t idx;
    value = std::stoi(word, &idx);
    return idx == word.length();
  };
  const auto get_vec3 = [get_word, parse_float](Span& words, glm::vec3& value) {
    std::string x, y, z;
    if (!get_word(words, x) || !get_word(words, y) || !get_word(words, z)) {
      return false;
    }
    return parse_float(x, value.x) && parse_float(y, value.y) &&
           parse_float(z, value.z);
  };
  const auto get_vec2 = [get_word, parse_float](Span& words, glm::vec2& value) {
    std::string x, y;
    if (!get_word(words, x) || !get_word(words, y)) {
      return false;
    }
    return parse_float(x, value.x) && parse_float(y, value.y);
  };
  const auto make_index_safe = [](int& value, int max_value) {
    if (value == 0) {
      return false;
    } else if (value > 0) {
      value--;
      return value < max_value;
    } else {
      value += max_value;
      return value >= 0;
    }
  };
  const auto parse_index = [&position_pool, &tex_coord_pool, &normal_pool,
                            &line_number, split_string, parse_int,
                            make_index_safe](const std::string& word,
                                             std::tuple<int, int, int>& value) {
    const std::vector<std::string> elements = split_string(word, '/');
    if (elements.size() > 3) {
      return absl::FailedPreconditionError(STATUS_MESSAGE(
          "Too many slashes in element on line " << line_number));
    }
    value = std::make_tuple(-1, -1, -1);

    Span elements_span = {elements.cbegin(), elements.cend()};
    if (!parse_int(*elements_span.it, std::get<0>(value))) {
      return absl::FailedPreconditionError(STATUS_MESSAGE(
          "Failed to parse first index \"" << elements_span.it->c_str()
                                           << "\" on line " << line_number));
    }
    if (!make_index_safe(std::get<0>(value), position_pool.size())) {
      return absl::FailedPreconditionError(STATUS_MESSAGE(
          "Invalid first index \"" << elements_span.it->c_str() << "\" on line"
                                   << line_number));
    }
    ++elements_span.it;

    if (elements_span.it == elements_span.end) {
      return absl::OkStatus();
    }

    if (*elements_span.it == "") {
      if (elements_span.it == elements_span.end) {
        return absl::FailedPreconditionError(STATUS_MESSAGE(
            "Element index cannot end with slash. Line " << line_number));
      }
      std::get<1>(value) = -1;
    } else {
      if (!parse_int(*elements_span.it, std::get<1>(value))) {
        return absl::FailedPreconditionError(STATUS_MESSAGE(
            "Failed to parse second index \"" << elements_span.it->c_str()
                                              << "\" on line" << line_number));
      }
      if (!make_index_safe(std::get<1>(value), tex_coord_pool.size())) {
        return absl::FailedPreconditionError(STATUS_MESSAGE(
            "Invalid second index \"" << elements_span.it->c_str()
                                      << "\" on line" << line_number));
      }
    }
    ++elements_span.it;
    if (elements_span.it == elements_span.end) {
      return absl::OkStatus();
    }

    if (*elements_span.it == "") {
      return absl::FailedPreconditionError(STATUS_MESSAGE(
          "Element index cannot end with slash. Line " << line_number));
    } else {
      if (!parse_int(*elements_span.it, std::get<2>(value))) {
        return absl::FailedPreconditionError(STATUS_MESSAGE(
            "Failed to parse third index \"" << elements_span.it->c_str()
                                             << "\" on line" << line_number));
      }
      if (!make_index_safe(std::get<2>(value), normal_pool.size())) {
        return absl::FailedPreconditionError(STATUS_MESSAGE(
            "Invalid third index \"" << elements_span.it->c_str()
                                     << "\" on line" << line_number));
      }
    }
    return absl::OkStatus();
  };

  std::string line;
  while (getline(file, line)) {
    line_number++;
    const std::vector<std::string> words = split_string(line, ' ');
    Span words_span = {++words.cbegin(), words.cend()};
    std::string word = words[0];
    if (word == "#") {
      // Ignore comment lines.
      continue;
    } else if (word == "o") {
      // Get the name of the new object.
      if (!get_word(words_span, word)) {
        return absl::FailedPreconditionError(STATUS_MESSAGE(
            "Missing name for \"o\" command on line " << line_number));
      }
      // Create the new object.
      new_current_mesh(word);
    } else if (word == "v") {
      glm::vec3 position;
      // Get the vec3 to store.
      if (!get_vec3(words_span, position)) {
        file.close();
        return absl::FailedPreconditionError(
            STATUS_MESSAGE("Failed to read vec3 on line " << line_number));
      }
      // Add the vector to the pool.
      position_pool.push_back(position);
    } else if (word == "vt") {
      glm::vec2 tex_coord;
      // Get the vec2 to store.
      if (!get_vec2(words_span, tex_coord)) {
        file.close();
        return absl::FailedPreconditionError(
            STATUS_MESSAGE("Failed to read vec2 on line " << line_number));
      }
      // Add the vector to the pool.
      tex_coord_pool.push_back(tex_coord);
    } else if (word == "vn") {
      glm::vec3 normal;
      // Get the vec3 to store.
      if (!get_vec3(words_span, normal)) {
        file.close();
        return absl::FailedPreconditionError(
            STATUS_MESSAGE("Failed to read vec3 on line " << line_number));
      }
      // Add the vector to the pool.
      normal_pool.push_back(normal);
    } else if (word == "f") {
      // Ensure a mesh actually exists.
      if (!current_mesh) {
        new_current_mesh("default");
      }
      std::vector<std::tuple<int, int, int>> face_elements;
      // Keep consuming words until all are consumed.
      while (words_span.it != words_span.end) {
        face_elements.push_back(std::make_tuple(0, 0, 0));
        RETURN_IF_ERROR(parse_index(*words_span.it++, face_elements.back()));
      }
      // Ensure at least one triangle is present.
      if (face_elements.size() < 3) {
        return absl::FailedPreconditionError(
            STATUS_MESSAGE("Not enough face elements on line " << line_number));
      }
      // Maps element index to vertex index.
      std::vector<unsigned int> points;
      for (int i = 0; i < face_elements.size(); i++) {
        const auto& face_element = face_elements[i];
        const auto index_it = index_map.find(face_element);
        // If an index already exists for this vertex, just use that.
        if (index_it != index_map.end()) {
          points.push_back(index_it->second);
        } else {
          // If none was found, create a new vertex.
          points.push_back(current_mesh->vertices.size());
          index_map.insert(std::make_pair(
              face_element, (unsigned int)current_mesh->vertices.size()));
          // Actually create the point with 0 for its normal (if it needs to be
          // summed up).
          current_mesh->vertices.push_back(
              {position_pool[std::get<0>(face_element)],
               std::get<1>(face_element) == -1
                   ? glm::vec2(0, 0)
                   : tex_coord_pool[std::get<1>(face_element)],
               glm::vec3(0, 0, 0)});
          if (std::get<2>(face_element) == -1) {
            // Store that this vertex needs to be averaged.
            point_number_of_faces.insert(std::make_pair(
                (unsigned int)current_mesh->vertices.size() - 1, 0u));
          } else {
            // Get the point's normal.
            current_mesh->vertices.back().normal =
                normal_pool[std::get<2>(face_element)];
          }
        }
      }
      // Go through each new face.
      for (int i = 2; i < face_elements.size(); i++) {
        const std::tuple<int, int, int>&element_1 = face_elements[0],
                                   element_2 = face_elements[i - 1],
                                   element_3 = face_elements[i];
        const unsigned int point_1 = points[0], point_2 = points[i - 1],
                           point_3 = points[i];

        // Compute the normal for this face.
        const glm::vec3 normal = glm::normalize(
            glm::cross(current_mesh->vertices[point_3].position -
                           current_mesh->vertices[point_1].position,
                       current_mesh->vertices[point_2].position -
                           current_mesh->vertices[point_1].position));
        // For each element, if the normal was unspecified calculate sum to its
        // normal and add another face it must average.
        if (std::get<2>(element_1) == -1) {
          current_mesh->vertices[point_1].normal += normal;
          point_number_of_faces[point_1] += 1;
        }
        if (std::get<2>(element_2) == -1) {
          current_mesh->vertices[point_2].normal += normal;
          point_number_of_faces[point_2] += 1;
        }
        if (std::get<2>(element_3) == -1) {
          current_mesh->vertices[point_3].normal += normal;
          point_number_of_faces[point_3] += 1;
        }

        // Create the triangles.
        current_mesh->triangles.push_back({point_1, point_2, point_3});
      }
    }
  }
  if (current_mesh) {
    finalize_current_mesh();
  }
  file.close();
  return model;
}

absl::StatusOr<std::shared_ptr<Mesh>> ObjModel::LoadMesh(
    const MeshDetails& details) {
  ASSIGN_OR_RETURN((const std::shared_ptr<ObjModel> model),
                   details.obj_model.Get());
  const auto mesh_it = model->meshes.find(details.name);
  if (mesh_it == model->meshes.end()) {
    return absl::NotFoundError(
        STATUS_MESSAGE("No mesh named \"" << details.name << "\""));
  }
  return mesh_it->second;
}
