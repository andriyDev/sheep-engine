
#include "resources/mesh_formats/gltf_mesh.h"

#include <GL/glew.h>
#include <absl/strings/string_view.h>
#include <glog/logging.h>

#include <fstream>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

#include "utility/hton.h"

absl::StatusOr<const nlohmann::json*> GetElementByKey(
    const nlohmann::json& json, const std::string& key) {
  const auto it = json.find(key);
  if (it == json.end()) {
    return absl::InvalidArgumentError(
        STATUS_MESSAGE("Invalid JSON data: missing " << key << " key"));
  } else {
    return &it.value();
  }
}

absl::StatusOr<const nlohmann::json*> GetObject(const nlohmann::json& json,
                                                const std::string& key) {
  ASSIGN_OR_RETURN((const nlohmann::json* element),
                   (GetElementByKey(json, key)));
  if (!element->is_object()) {
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Invalid JSON data: element with key " << key << " is not an object"));
  }
  return element;
}

absl::StatusOr<const nlohmann::json*> GetArray(const nlohmann::json& json,
                                               const std::string& key) {
  ASSIGN_OR_RETURN((const nlohmann::json* element),
                   (GetElementByKey(json, key)));
  if (!element->is_array()) {
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Invalid JSON data: element with key " << key << " is not an array"));
  }
  return element;
}

absl::StatusOr<int> GetInt(const nlohmann::json& json, const std::string& key) {
  ASSIGN_OR_RETURN((const nlohmann::json* element),
                   (GetElementByKey(json, key)));
  if (!element->is_number_integer()) {
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Invalid JSON data: element with key " << key << " is not an integer"));
  }
  return element->get<int>();
}

absl::StatusOr<unsigned int> GetUnsignedInt(const nlohmann::json& json,
                                            const std::string& key) {
  ASSIGN_OR_RETURN((const nlohmann::json* element),
                   (GetElementByKey(json, key)));
  if (!element->is_number_unsigned()) {
    return absl::InvalidArgumentError(
        STATUS_MESSAGE("Invalid JSON data: element with key "
                       << key << " is not an unsigned integer"));
  }
  return element->get<unsigned int>();
}

absl::StatusOr<std::string> GetString(const nlohmann::json& json,
                                      const std::string& key) {
  ASSIGN_OR_RETURN((const nlohmann::json* element),
                   (GetElementByKey(json, key)));
  if (!element->is_string()) {
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Invalid JSON data: element with key " << key << " is not a string"));
  }
  return element->get<std::string>();
}

absl::StatusOr<bool> GetBool(const nlohmann::json& json,
                             const std::string& key) {
  ASSIGN_OR_RETURN((const nlohmann::json* element),
                   (GetElementByKey(json, key)));
  if (!element->is_boolean()) {
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Invalid JSON data: element with key " << key << " is not a bool"));
  }
  return element->get<bool>();
}

struct BufferView {
  unsigned int buffer;
  unsigned int offset;
  unsigned int size;
  unsigned int stride;
};

enum class ComponentType : unsigned int {
  Byte = 5120,
  UnsignedByte = 5121,
  Short = 5122,
  UnsignedShort = 5123,
  UnsignedInt = 5125,
  Float = 5126,
};

struct Accessor {
  std::optional<unsigned int> buffer_view;
  unsigned int byte_offset;
  ComponentType component_type;
  bool normalize_ints;
  unsigned int count;
  std::string type;
};

template <typename ComponentType>
inline ComponentType ltoh(ComponentType value);

template <>
inline float ltoh(float value) {
  return ltohf(value);
}

template <>
inline uint32_t ltoh(uint32_t value) {
  return ltohl(value);
}

template <>
inline uint16_t ltoh(uint16_t value) {
  return ltohs(value);
}

template <>
inline uint8_t ltoh(uint8_t value) {
  return value;
}

template <>
inline int16_t ltoh(int16_t value) {
  return ltohs(value);
}

template <>
inline int8_t ltoh(int8_t value) {
  return value;
}

template <typename ComponentType, int components>
absl::StatusOr<std::vector<glm::vec<components, ComponentType>>> ReadAccessor(
    const std::vector<std::vector<uint8_t>>& buffers,
    const std::vector<BufferView>& buffer_views, const Accessor& accessor) {
  std::vector<glm::vec<components, ComponentType>> result;
  result.resize(accessor.count);
  if (!accessor.buffer_view.has_value()) {
    // TODO: Handle sparse accessors.
    return result;
  }

  const BufferView& buffer_view = buffer_views[*accessor.buffer_view];
  const std::vector<unsigned char>& buffer = buffers[buffer_view.buffer];
  unsigned int byte_start = buffer_view.offset + accessor.byte_offset;
  unsigned int byte_end = std::min((unsigned int)buffer.size(),
                                   buffer_view.offset + buffer_view.size);
  unsigned int byte_stride = buffer_view.stride == 0
                                 ? sizeof(glm::vec<components, ComponentType>)
                                 : buffer_view.stride;
  if (byte_start + byte_stride * accessor.count > byte_end) {
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Accessor requested bytes that are out of range of buffer (view). "
        "Requested range "
        << byte_start << "-" << (byte_start + byte_stride * accessor.count)
        << " but buffer ends at " << byte_end));
  }
  ComponentType* result_i = (ComponentType*)result.data();
  for (unsigned int i = 0, byte_index = byte_start; i < accessor.count;
       ++i, byte_index += byte_stride, result_i += components) {
    memcpy(result_i, buffer.data() + byte_index,
           sizeof(ComponentType) * components);
    for (unsigned int component = 0; component < components; ++component) {
      result_i[component] = ltoh(result_i[component]);
    }
  }
  return result;
}

template <typename ComponentType>
float NormalizeInt(ComponentType value);

template <>
float NormalizeInt(uint8_t value) {
  return float(value) / ((uint8_t)-1);
}

template <>
float NormalizeInt(uint16_t value) {
  return float(value) / ((uint16_t)-1);
}

template <>
float NormalizeInt(uint32_t value) {
  return float(value) / ((uint32_t)-1);
}

template <>
float NormalizeInt(int8_t value) {
  return std::max(value / 127.0f, -1.0f);
}

template <>
float NormalizeInt(int16_t value) {
  return std::max(value / 32767.0f, -1.0f);
}

template <typename ComponentType, int components>
std::vector<glm::vec<components, float>> NormalizeAccessorData(
    const std::vector<glm::vec<components, ComponentType>>& accessor_data) {
  std::vector<glm::vec<components, float>> normalized_data;
  normalized_data.resize(accessor_data.size());
  float* normalized_data_ptr = (float*)normalized_data.data();
  ComponentType* accessor_data_ptr = (ComponentType*)accessor_data.data();
  for (unsigned int i = 0; i < accessor_data.size() * components; ++i) {
    for (unsigned int component = 0; component < components; ++component) {
      *(normalized_data_ptr++) = NormalizeInt(*(accessor_data_ptr++));
    }
  }
  return normalized_data;
}

template <int components>
absl::StatusOr<std::vector<glm::vec<components, float>>> ReadFloatAccessor(
    const std::vector<std::vector<uint8_t>>& buffers,
    const std::vector<BufferView>& buffer_views, const Accessor& accessor) {
  switch (accessor.component_type) {
    case ComponentType::Float: {
      return ReadAccessor<float, components>(buffers, buffer_views, accessor);
    } break;
    case ComponentType::UnsignedByte: {
      if (!accessor.normalize_ints) {
        return absl::InvalidArgumentError(
            "Float accessor using integer components must normalize ints.");
      }
      ASSIGN_OR_RETURN(
          (const std::vector<glm::vec<components, uint8_t>>&
               unnormalized_floats),
          (ReadAccessor<uint8_t, components>(buffers, buffer_views, accessor)));
      return NormalizeAccessorData(unnormalized_floats);
    }
    case ComponentType::UnsignedShort: {
      if (!accessor.normalize_ints) {
        return absl::InvalidArgumentError(
            "Float accessor using integer components must normalize ints.");
      }
      ASSIGN_OR_RETURN((const std::vector<glm::vec<components, uint16_t>>&
                            unnormalized_floats),
                       (ReadAccessor<uint16_t, components>(
                           buffers, buffer_views, accessor)));
      return NormalizeAccessorData(unnormalized_floats);
    }
    default:
      return absl::InvalidArgumentError(
          "Accessor has bad component type - cannot be float accessor.");
  }
}

absl::StatusOr<BufferView> ParseBufferView(
    const nlohmann::json& buffer_view_json) {
  BufferView buffer_view;
  ASSIGN_OR_RETURN((buffer_view.buffer),
                   GetUnsignedInt(buffer_view_json, "buffer"));
  ASSIGN_OR_RETURN((buffer_view.size),
                   GetUnsignedInt(buffer_view_json, "byteLength"));
  buffer_view.offset =
      GetUnsignedInt(buffer_view_json, "byteOffset").value_or(0);
  buffer_view.stride =
      GetUnsignedInt(buffer_view_json, "byteStride").value_or(0);
  return buffer_view;
}

absl::StatusOr<Accessor> ParseAccessor(const nlohmann::json& accessor_json) {
  Accessor accessor;
  const absl::StatusOr<unsigned int> buffer_view =
      GetUnsignedInt(accessor_json, "bufferView");
  if (buffer_view.ok()) {
    accessor.buffer_view = *buffer_view;
  }
  accessor.byte_offset =
      GetUnsignedInt(accessor_json, "byteOffset").value_or(0);
  const absl::StatusOr<unsigned int> component_type =
      GetUnsignedInt(accessor_json, "componentType");
  if (!component_type.ok()) {
    return component_type.status();
  }
  accessor.component_type = (ComponentType)*component_type;

  if ((unsigned int)accessor.component_type < 5120 ||
      (unsigned int)accessor.component_type > 5126 ||
      (unsigned int)accessor.component_type == 5124) {
    return absl::InvalidArgumentError(
        STATUS_MESSAGE("Invalid JSON data: Invalid component type "
                       << (unsigned int)accessor.component_type));
  }

  accessor.normalize_ints =
      GetBool(accessor_json, "normalized").value_or(false);
  ASSIGN_OR_RETURN((accessor.count), GetUnsignedInt(accessor_json, "count"));
  ASSIGN_OR_RETURN((accessor.type), GetString(accessor_json, "type"));
  return accessor;
}

char ConvertBase64Character(char c) {
  if ('A' <= c && c <= 'Z') {
    return c - 'A';
  } else if ('a' <= c && c <= 'z') {
    return c - 'a' + 26;
  } else if ('0' <= c && c <= '9') {
    return c - '0' + 52;
  } else if (c == '+') {
    return 62;
  } else if (c == '/') {
    return 63;
  } else {
    // We use 64 as a cheap sentinel value.
    return 64;
  }
}

absl::Status FillBufferWithBase64(absl::string_view base64_data,
                                  std::vector<uint8_t>& out_data) {
  unsigned int bits = base64_data.length() * 6;
  out_data.resize(bits / 8 + (bits % 8 != 0));
  unsigned int i;
  unsigned int j;
  for (i = 0, j = 0; i < base64_data.length() - 3; i += 4, j += 3) {
    char a = ConvertBase64Character(base64_data[i]),
         b = ConvertBase64Character(base64_data[i + 1]),
         c = ConvertBase64Character(base64_data[i + 2]),
         d = ConvertBase64Character(base64_data[i + 3]);
    if (a >= 64 || b >= 64 || c >= 64 || d >= 64) {
      return absl::InvalidArgumentError(STATUS_MESSAGE(
          "Unable to convert characters \""
          << base64_data[i] << base64_data[i + 1] << base64_data[i + 2]
          << base64_data[i + 3] << "\" as base64"));
    }
    union {
      uint32_t merged;
      uint8_t split[4];
    };
    merged = ((uint32_t)a << 18) | ((uint32_t)b << 12) | ((uint32_t)c << 6) |
             ((uint32_t)d);
    if constexpr (endian::native == endian::little) {
      out_data[j] = split[2];
      out_data[j + 1] = split[1];
      out_data[j + 2] = split[0];
    } else {
      out_data[j] = split[1];
      out_data[j + 1] = split[2];
      out_data[j + 2] = split[3];
    }
  }
  if (i != base64_data.length()) {
    unsigned int remaining_chars = base64_data.length() - i;
    char a = ConvertBase64Character(base64_data[i]),
         b = i + 1 < base64_data.length()
                 ? ConvertBase64Character(base64_data[i + 1])
                 : 0,
         c = i + 2 < base64_data.length()
                 ? ConvertBase64Character(base64_data[i + 2])
                 : 0;
    if (a >= 64 || b >= 64 || c >= 64) {
      return absl::InvalidArgumentError(STATUS_MESSAGE(
          "Unable to convert characters \""
          << base64_data[i]
          << (i + 1 < base64_data.length() ? base64_data[i + 1] : ' ')
          << (i + 2 < base64_data.length() ? base64_data[i + 2] : ' ')
          << " \" as base64"));
    }
    union {
      uint32_t merged;
      uint8_t split[4];
    };
    merged = ((uint32_t)a << 26) | ((uint32_t)b << 20) | ((uint32_t)c << 14);
    if constexpr (endian::native == endian::little) {
      out_data[j] = split[3];
      out_data[j + 1] = split[2];
      out_data[j + 2] = split[1];
    } else {
      out_data[j] = split[0];
      out_data[j + 1] = split[1];
      out_data[j + 2] = split[2];
    }
  }
  return absl::OkStatus();
}

absl::Status ParseBuffer(const nlohmann::json& buffer_json,
                         std::vector<uint8_t>& out_data) {
  ASSIGN_OR_RETURN((unsigned int byte_len),
                   GetUnsignedInt(buffer_json, "byteLength"));
  ASSIGN_OR_RETURN((const std::string& uri), GetString(buffer_json, "uri"));
  if (uri.rfind("data:", 0) == 0) {
    // Handle data URI.
    size_t comma_index = uri.find(',');
    absl::string_view view = uri;
    RETURN_IF_ERROR(FillBufferWithBase64(
        absl::ClippedSubstr(uri, comma_index + 1), out_data));
  } else {
    // If it is not a data URI, assume it is a relative file.
    std::ifstream buffer_file(uri, std::ios_base::in | std::ios_base::binary);
    if (!buffer_file.is_open()) {
      return absl::InvalidArgumentError(
          STATUS_MESSAGE("Unable to read buffer file \"" << uri << "\""));
    }

    out_data.resize(byte_len);
    buffer_file.read((char*)out_data.data(), byte_len);
    unsigned int read_bytes = buffer_file.gcount();
    if (read_bytes != byte_len) {
      return absl::InvalidArgumentError(STATUS_MESSAGE(
          "Unable to read requested byte count from Buffer file \""
          << uri << "\". Requested " << byte_len << ", but got "
          << read_bytes));
    }
  }
  return absl::OkStatus();
}

struct GltfNode {
  std::string name;
  glm::vec3 position;
  glm::quat rotation;
  glm::vec3 scale;

  std::vector<unsigned int> children;
};

absl::StatusOr<std::vector<GltfNode>> ParseNodes(const nlohmann::json& root) {
  std::vector<GltfNode> result;
  absl::StatusOr<const nlohmann::json*> nodes = GetArray(root, "nodes");
  if (!nodes.ok()) {
    return result;
  }
  for (const nlohmann::json& node_json : **nodes) {
    if (!node_json.is_object()) {
      return absl::InvalidArgumentError(
          "Element in node array is not an object.");
    }
    GltfNode& new_node = result.emplace_back();
    const absl::StatusOr<std::string> name = GetString(node_json, "name");
    if (name.ok()) {
      new_node.name = *name;
    }
    const absl::StatusOr<const nlohmann::json*> translation =
        GetArray(node_json, "translation");
    if (translation.ok()) {
      if ((*translation)->size() != 3 || !(**translation)[0].is_number() ||
          !(**translation)[1].is_number() || !(**translation)[2].is_number()) {
        return absl::InvalidArgumentError(
            "Translation element did not contain 3 numbers");
      }
      new_node.position.x = (**translation)[0].get<float>();
      new_node.position.y = (**translation)[1].get<float>();
      new_node.position.z = (**translation)[2].get<float>();
    } else {
      new_node.position = glm::vec3(0, 0, 0);
    }
    const absl::StatusOr<const nlohmann::json*> rotation =
        GetArray(node_json, "rotation");
    if (rotation.ok()) {
      if ((*rotation)->size() != 4 || !(**rotation)[0].is_number() ||
          !(**rotation)[1].is_number() || !(**rotation)[2].is_number() ||
          !(**rotation)[3].is_number()) {
        return absl::InvalidArgumentError(
            "Rotation element did not contain 4 numbers");
      }
      new_node.rotation.x = (**rotation)[0].get<float>();
      new_node.rotation.y = (**rotation)[1].get<float>();
      new_node.rotation.z = (**rotation)[2].get<float>();
      new_node.rotation.w = (**rotation)[3].get<float>();
    } else {
      new_node.rotation = glm::quat(1, 0, 0, 0);
    }
    const absl::StatusOr<const nlohmann::json*> scale =
        GetArray(node_json, "scale");
    if (scale.ok()) {
      if ((*scale)->size() != 3 || !(**scale)[0].is_number() ||
          !(**scale)[1].is_number() || !(**scale)[2].is_number()) {
        return absl::InvalidArgumentError(
            "Scale element did not contain 3 numbers");
      }
      new_node.scale.x = (**scale)[0].get<float>();
      new_node.scale.y = (**scale)[1].get<float>();
      new_node.scale.z = (**scale)[2].get<float>();
    } else {
      new_node.scale = glm::vec3(1, 1, 1);
    }
    const absl::StatusOr<const nlohmann::json*> children =
        GetArray(node_json, "children");
    if (children.ok()) {
      for (const nlohmann::json& child_index : **children) {
        if (!child_index.is_number_unsigned()) {
          return absl::InvalidArgumentError(
              "Child index is not an unsigned integer.");
        }
        unsigned int index = child_index.get<unsigned int>();
        if (index >= (*nodes)->size()) {
          return absl::InvalidArgumentError(
              "Child index refers to non-existent node.");
        }
        if (index == result.size() - 1) {
          return absl::InvalidArgumentError("Child index refers to self.");
        }
        new_node.children.push_back(index);
      }
    }
  }
  return result;
}

absl::StatusOr<std::shared_ptr<GltfModel>> GltfModel::Load(
    const Details& details) {
  std::ifstream file(details.file, std::ios_base::in | std::ios_base::binary);
  if (!file.is_open()) {
    return absl::NotFoundError(
        STATUS_MESSAGE("Unable to read file " << details.file));
  }

  std::vector<std::vector<unsigned char>> buffers;
  nlohmann::json root;

  union {
    uint8_t chars[12];
    uint32_t ints[3];
  } header;
  file.read((char*)header.chars, 12);
  if (file.gcount() == 12 &&
      absl::string_view((char*)header.chars, 4) == "glTF") {
    header.ints[1] = ltohl(header.ints[1]);
    header.ints[2] = ltohl(header.ints[2]);
    if (header.ints[1] != 2) {
      return absl::InvalidArgumentError(STATUS_MESSAGE(
          "Bad GLB version. Expected 2, but got " << header.ints[1]));
    }

    bool parsed_json = false, parsed_buffer = false;
    uint32_t data_read = 12;
    while (data_read < header.ints[2]) {
      union {
        uint8_t chars[8];
        uint32_t ints[2];
      } chunk;
      file.read((char*)chunk.chars, 8);
      size_t read_bytes = file.gcount();
      if (read_bytes != 8) {
        return absl::InvalidArgumentError(STATUS_MESSAGE(
            "Inconsistent GLB file. Expected 8 more bytes, but only got "
            << read_bytes));
      }
      data_read += 8;
      chunk.ints[0] = ltohl(chunk.ints[0]);
      chunk.ints[1] = ltohl(chunk.ints[1]);
      if (chunk.ints[1] == 0x4e4f534a) {
        if (parsed_json) {
          return absl::InvalidArgumentError(
              "Inconsistent GLB file. Expected exactly one JSON chunk, but got "
              "more than one.");
        }
        parsed_json = true;
        std::vector<uint8_t> json_data;
        json_data.resize(chunk.ints[0]);
        file.read((char*)json_data.data(), chunk.ints[0]);
        read_bytes = file.gcount();
        if (read_bytes != chunk.ints[0]) {
          return absl::InvalidArgumentError(STATUS_MESSAGE(
              "Inconsistent GLB file. Expected "
              << chunk.ints[0] << " more bytes, but only got " << read_bytes));
        }
        data_read += read_bytes;
        try {
          root = nlohmann::json::parse(json_data);
        } catch (int err) {
          return absl::InvalidArgumentError(
              "Inconsistent GLB file. Failed to parse JSON chunk.");
        }
      } else if (chunk.ints[1] == 0x004e4942) {
        if (parsed_buffer) {
          return absl::InvalidArgumentError(
              "Inconsistent GLB file. Expected at most one binary chunk, but "
              "got more than one.");
        }
        parsed_buffer = true;
        buffers.push_back(std::vector<uint8_t>());
        buffers.back().resize(chunk.ints[0]);
        file.read((char*)buffers.back().data(), chunk.ints[0]);
        read_bytes = file.gcount();
        if (read_bytes != chunk.ints[0]) {
          return absl::InvalidArgumentError(STATUS_MESSAGE(
              "Inconsistent GLB file. Expected "
              << chunk.ints[0] << " more bytes, but only got " << read_bytes));
        }
        data_read += read_bytes;
      }
    }
    if (!parsed_json) {
      return absl::InvalidArgumentError(
          "Inconsistent GCB file. Expected exactly one JSON chunk, but got "
          "none.");
    }
    file.close();
  } else {
    file.seekg(0, std::ios::beg);
    file >> root;
    file.close();
  }

  if (!root.is_object()) {
    return absl::InvalidArgumentError("Root of glTF file is not an object");
  }

  ASSIGN_OR_RETURN((const std::vector<GltfNode> nodes), ParseNodes(root));

  ASSIGN_OR_RETURN((const nlohmann::json* buffers_json_array),
                   GetArray(root, "buffers"));
  buffers.reserve(buffers.size() + buffers_json_array->size());
  for (const auto& buffer_json : *buffers_json_array) {
    if (!buffer_json.is_object()) {
      return absl::InvalidArgumentError(
          "Invalid JSON data: buffer is not an object.");
    }
    ParseBuffer(buffer_json, buffers.emplace_back());
  }

  std::vector<BufferView> buffer_views;
  ASSIGN_OR_RETURN((const nlohmann::json* buffer_view_json_array),
                   GetArray(root, "bufferViews"));
  buffer_views.reserve(buffer_view_json_array->size());
  for (const nlohmann::json& buffer_view_json : *buffer_view_json_array) {
    if (!buffer_view_json.is_object()) {
      return absl::InvalidArgumentError(
          "Invalid JSON data: buffer view is not an object.");
    }
    ASSIGN_OR_RETURN((buffer_views.emplace_back()),
                     ParseBufferView(buffer_view_json));
  }

  std::vector<Accessor> accessors;
  ASSIGN_OR_RETURN((const nlohmann::json* accessor_json_array),
                   GetArray(root, "accessors"));
  accessors.reserve(accessor_json_array->size());
  for (const nlohmann::json& accessor_json : *accessor_json_array) {
    if (!accessor_json.is_object()) {
      return absl::InvalidArgumentError(
          "Invalid JSON data: accessor is not an object.");
    }
    ASSIGN_OR_RETURN((accessors.emplace_back()), ParseAccessor(accessor_json));
  }

  std::shared_ptr<GltfModel> model(new GltfModel());

  ASSIGN_OR_RETURN((const nlohmann::json* meshes_array),
                   GetArray(root, "meshes"));

  for (const nlohmann::json& mesh : *meshes_array) {
    if (!mesh.is_object()) {
      return absl::InvalidArgumentError(
          "Invalid JSON data: mesh is not an object.");
    }

    const absl::StatusOr<std::string> mesh_name = GetString(mesh, "name");
    // Failing to find a name is fine - just skip the mesh.
    if (!mesh_name.ok()) {
      continue;
    }

    ASSIGN_OR_RETURN((const nlohmann::json* primitives_array),
                     GetArray(mesh, "primitives"));
    if (primitives_array->size() == 0) {
      continue;
    }

    for (const nlohmann::json& primitive_json : *primitives_array) {
      if (!primitive_json.is_object()) {
        return absl::InvalidArgumentError(
            "Invalid JSON data: primitive is not an object.");
      }

      std::shared_ptr<Mesh> mesh(new Mesh());
      Primitive& primitive = model->primitives[*mesh_name].emplace_back();
      primitive.mesh = mesh;

      ASSIGN_OR_RETURN((const nlohmann::json* attributes),
                       GetObject(primitive_json, "attributes"));
      {
        ASSIGN_OR_RETURN((const unsigned int position_accessor_id),
                         GetUnsignedInt(*attributes, "POSITION"));
        if (position_accessor_id >= accessors.size()) {
          return absl::InvalidArgumentError(STATUS_MESSAGE(
              "Missing position accessor: " << position_accessor_id));
        }
        const Accessor& position_accessor = accessors[position_accessor_id];
        if (position_accessor.type != "VEC3" ||
            position_accessor.component_type != ComponentType::Float) {
          return absl::InvalidArgumentError(
              STATUS_MESSAGE("Accessor " << position_accessor_id
                                         << " cannot be a position accessor "
                                            "(wrong type or component type"));
        }
        ASSIGN_OR_RETURN(
            (const std::vector<glm::vec3>& positions),
            (ReadAccessor<float, 3>(buffers, buffer_views, position_accessor)));
        mesh->vertices.resize(positions.size());
        for (unsigned int i = 0; i < positions.size(); i++) {
          mesh->vertices[i].position = positions[i];
        }
      }
      {
        const absl::StatusOr<unsigned int> texcoord_accessor_id =
            GetUnsignedInt(*attributes, "TEXCOORD_0");
        if (!texcoord_accessor_id.ok()) {
          for (Mesh::Vertex& vertex : mesh->vertices) {
            vertex.texCoord = glm::vec2(0, 0);
          }
        } else {
          if (*texcoord_accessor_id >= accessors.size()) {
            return absl::InvalidArgumentError(STATUS_MESSAGE(
                "Missing texcoord accessor: " << (*texcoord_accessor_id)));
          }
          const Accessor& texcoord_accessor = accessors[*texcoord_accessor_id];
          if (texcoord_accessor.type != "VEC2") {
            return absl::InvalidArgumentError(STATUS_MESSAGE(
                "Accessor "
                << (*texcoord_accessor_id)
                << " is not a VEC2 - cannot be texcoord accessor."));
          }
          ASSIGN_OR_RETURN(
              (const std::vector<glm::vec2>& texcoords),
              (ReadFloatAccessor<2>(buffers, buffer_views, texcoord_accessor)));
          if (texcoords.size() != mesh->vertices.size()) {
            return absl::InvalidArgumentError(STATUS_MESSAGE(
                "Texture coordinates do not have same size as vertices. Have "
                << mesh->vertices.size() << " vertices, but "
                << texcoords.size() << " texture coordinates"));
          }
          for (unsigned int i = 0; i < mesh->vertices.size(); i++) {
            mesh->vertices[i].texCoord = texcoords[i];
          }
        }
      }
      {
        const absl::StatusOr<unsigned int> colour_accessor_id =
            GetUnsignedInt(*attributes, "COLOR_0");
        if (!colour_accessor_id.ok()) {
          for (Mesh::Vertex& vertex : mesh->vertices) {
            vertex.colour = glm::vec4(0, 0, 0, 0);
          }
        } else {
          if (*colour_accessor_id >= accessors.size()) {
            return absl::InvalidArgumentError(STATUS_MESSAGE(
                "Missing colour accessor: " << (*colour_accessor_id)));
          }
          const Accessor& colour_accessor = accessors[*colour_accessor_id];
          if (colour_accessor.type == "VEC3") {
            ASSIGN_OR_RETURN(
                (const std::vector<glm::vec3>& colours),
                (ReadFloatAccessor<3>(buffers, buffer_views, colour_accessor)));
            if (colours.size() != mesh->vertices.size()) {
              return absl::InvalidArgumentError(STATUS_MESSAGE(
                  "Colours do not have same size as vertices. Have "
                  << mesh->vertices.size() << " vertices, but "
                  << colours.size() << " colours"));
            }
            for (unsigned int i = 0; i < mesh->vertices.size(); i++) {
              mesh->vertices[i].colour = glm::vec4(colours[i], 1.0f);
            }
          } else if (colour_accessor.type == "VEC4") {
            ASSIGN_OR_RETURN(
                (const std::vector<glm::vec4>& colours),
                (ReadFloatAccessor<4>(buffers, buffer_views, colour_accessor)));
            if (colours.size() != mesh->vertices.size()) {
              return absl::InvalidArgumentError(STATUS_MESSAGE(
                  "Colours do not have same size as vertices. Have "
                  << mesh->vertices.size() << " vertices, but "
                  << colours.size() << " colours"));
            }
            for (unsigned int i = 0; i < mesh->vertices.size(); i++) {
              mesh->vertices[i].colour = colours[i];
            }
          } else {
            return absl::InvalidArgumentError(
                "Accessor cannot be a colour accessor: wrong type");
          }
        }
      }
      {
        const absl::StatusOr<unsigned int> normal_accessor_id =
            GetUnsignedInt(*attributes, "NORMAL");
        if (!normal_accessor_id.ok()) {
          for (Mesh::Vertex& vertex : mesh->vertices) {
            vertex.normal = glm::vec3(0, 0, 0);
          }
        } else {
          if (*normal_accessor_id >= accessors.size()) {
            return absl::InvalidArgumentError(STATUS_MESSAGE(
                "Missing normal accessor: " << (*normal_accessor_id)));
          }
          const Accessor& normal_accessor = accessors[*normal_accessor_id];
          if (normal_accessor.type != "VEC3" ||
              normal_accessor.component_type != ComponentType::Float) {
            return absl::InvalidArgumentError(
                STATUS_MESSAGE("Accessor " << (*normal_accessor_id)
                                           << " cannot be a normal accessor "
                                              "(wrong type or component type"));
          }
          ASSIGN_OR_RETURN(
              (const std::vector<glm::vec3>& normals),
              (ReadAccessor<float, 3>(buffers, buffer_views, normal_accessor)));
          if (normals.size() != mesh->vertices.size()) {
            return absl::InvalidArgumentError(STATUS_MESSAGE(
                "Normals do not have same size as vertices. Have "
                << mesh->vertices.size() << " vertices, but " << normals.size()
                << " normals"));
          }
          for (unsigned int i = 0; i < mesh->vertices.size(); i++) {
            mesh->vertices[i].normal = glm::normalize(normals[i]);
          }
        }
      }
      {
        const absl::StatusOr<unsigned int> tangent_accessor_id =
            GetUnsignedInt(*attributes, "TANGENT");
        if (!tangent_accessor_id.ok()) {
          for (Mesh::Vertex& vertex : mesh->vertices) {
            vertex.tangent = glm::vec3(0, 0, 0);
            vertex.bitangent = glm::vec3(0, 0, 0);
          }
        } else {
          if (*tangent_accessor_id >= accessors.size()) {
            return absl::InvalidArgumentError(STATUS_MESSAGE(
                "Missing tangent accessor: " << (*tangent_accessor_id)));
          }
          const Accessor& tangent_accessor = accessors[*tangent_accessor_id];
          if (tangent_accessor.type != "VEC4" ||
              tangent_accessor.component_type != ComponentType::Float) {
            return absl::InvalidArgumentError(
                STATUS_MESSAGE("Accessor " << (*tangent_accessor_id)
                                           << " cannot be a tangent accessor "
                                              "(wrong type or component type"));
          }
          ASSIGN_OR_RETURN((const std::vector<glm::vec4>& tangents),
                           (ReadAccessor<float, 4>(buffers, buffer_views,
                                                   tangent_accessor)));
          if (tangents.size() != mesh->vertices.size()) {
            return absl::InvalidArgumentError(STATUS_MESSAGE(
                "Tangents do not have same size as vertices. Have "
                << mesh->vertices.size() << " vertices, but " << tangents.size()
                << " tangents"));
          }
          for (unsigned int i = 0; i < mesh->vertices.size(); i++) {
            mesh->vertices[i].tangent = tangents[i];
            mesh->vertices[i].bitangent =
                glm::normalize(glm::cross(mesh->vertices[i].normal,
                                          mesh->vertices[i].tangent) *
                               tangents[i].w);
          }
        }
      }
      {
        const absl::StatusOr<unsigned int> indices_accessor_id =
            GetUnsignedInt(primitive_json, "indices");
        if (indices_accessor_id.ok()) {
          if (*indices_accessor_id >= accessors.size()) {
            return absl::InvalidArgumentError(STATUS_MESSAGE(
                "Missing indices accessor: " << (*indices_accessor_id)));
          }
          const Accessor& indices_accessor = accessors[*indices_accessor_id];
          if (indices_accessor.type != "SCALAR" ||
              (indices_accessor.component_type != ComponentType::UnsignedByte &&
               indices_accessor.component_type !=
                   ComponentType::UnsignedShort &&
               indices_accessor.component_type != ComponentType::UnsignedInt)) {
            return absl::InvalidArgumentError(
                STATUS_MESSAGE("Accessor " << (*indices_accessor_id)
                                           << " cannot be a indices accessor "
                                              "(wrong type or component type"));
          }
          if (indices_accessor.component_type == ComponentType::UnsignedInt) {
            ASSIGN_OR_RETURN(
                (const std::vector<glm::vec<1, unsigned int>>& indices),
                (ReadAccessor<unsigned int, 1>(buffers, buffer_views,
                                               indices_accessor)));
            mesh->triangles.resize(indices.size() / 3);
            for (unsigned int i = 0; i < indices.size(); i += 3) {
              mesh->triangles.push_back(
                  {indices[i].x, indices[i + 1].x, indices[i + 2].x});
            }
          } else if (indices_accessor.component_type ==
                     ComponentType::UnsignedShort) {
            ASSIGN_OR_RETURN(
                (const std::vector<glm::vec<1, unsigned short>>& indices),
                (ReadAccessor<unsigned short, 1>(buffers, buffer_views,
                                                 indices_accessor)));
            mesh->small_triangles.resize(indices.size() / 3);
            for (unsigned int i = 0; i < indices.size(); i += 3) {
              mesh->small_triangles.push_back(
                  {indices[i].x, indices[i + 1].x, indices[i + 2].x});
            }
          } else {
            ASSIGN_OR_RETURN(
                (const std::vector<glm::vec<1, unsigned char>>& indices),
                (ReadAccessor<unsigned char, 1>(buffers, buffer_views,
                                                indices_accessor)));
            mesh->small_triangles.resize(indices.size() / 3);
            for (unsigned int i = 0; i < indices.size(); i += 3) {
              mesh->small_triangles.push_back(
                  {indices[i].x, indices[i + 1].x, indices[i + 2].x});
            }
          }
        }
      }
      {
        const absl::StatusOr<unsigned int> bones_accessor_id =
            GetUnsignedInt(*attributes, "JOINTS_0");
        const absl::StatusOr<unsigned int> weights_accessor_id =
            GetUnsignedInt(*attributes, "WEIGHTS_0");
        if (bones_accessor_id.ok() != weights_accessor_id.ok()) {
          if (!weights_accessor_id.ok()) {
            return weights_accessor_id.status();
          } else {
            return bones_accessor_id.status();
          }
        } else if (bones_accessor_id.ok()) {
          // If one is ok, then both are ok.
          if (*bones_accessor_id >= accessors.size()) {
            return absl::InvalidArgumentError(STATUS_MESSAGE(
                "Missing bones accessor: " << (*bones_accessor_id)));
          }
          if (*weights_accessor_id >= accessors.size()) {
            return absl::InvalidArgumentError(STATUS_MESSAGE(
                "Missing weights accessor: " << (*weights_accessor_id)));
          }
          const Accessor& bones_accessor = accessors[*bones_accessor_id];
          const Accessor& weights_accessor = accessors[*weights_accessor_id];
          if (bones_accessor.type != "VEC4") {
            return absl::InvalidArgumentError(STATUS_MESSAGE(
                "Accessor " << (*bones_accessor_id)
                            << " cannot be a bone accessor: wrong type"));
          }
          if (weights_accessor.type != "VEC4") {
            return absl::InvalidArgumentError(STATUS_MESSAGE(
                "Accessor " << (*weights_accessor_id)
                            << " cannot be a weight accessor: wrong type"));
          }
          primitive.skin = std::shared_ptr<Skin>(new Skin());
          if (bones_accessor.count != mesh->vertices.size()) {
            return absl::InvalidArgumentError(
                STATUS_MESSAGE("Wrong bone count. Expected "
                               << mesh->vertices.size() << ", but got "
                               << bones_accessor.count));
          }
          if (weights_accessor.count != mesh->vertices.size()) {
            return absl::InvalidArgumentError(
                STATUS_MESSAGE("Wrong weight count. Expected "
                               << mesh->vertices.size() << ", but got "
                               << weights_accessor.count));
          }
          primitive.skin->vertices.resize(mesh->vertices.size());
          if (bones_accessor.component_type == ComponentType::UnsignedByte) {
            ASSIGN_OR_RETURN(
                (const std::vector<glm::vec<4, unsigned char>>& bones),
                (ReadAccessor<unsigned char, 4>(buffers, buffer_views,
                                                bones_accessor)));
            for (unsigned int i = 0; i < mesh->vertices.size(); i++) {
              primitive.skin->vertices[i].bone_indices = {
                  bones[i].x, bones[i].y, bones[i].z, bones[i].w};
            }
          } else if (bones_accessor.component_type ==
                     ComponentType::UnsignedShort) {
            ASSIGN_OR_RETURN(
                (const std::vector<glm::vec<4, unsigned short>>& bones),
                (ReadAccessor<unsigned short, 4>(buffers, buffer_views,
                                                 bones_accessor)));
            for (unsigned int i = 0; i < mesh->vertices.size(); i++) {
              primitive.skin->vertices[i].bone_indices = bones[i];
            }
          } else {
            return absl::InvalidArgumentError(STATUS_MESSAGE(
                "Accessor "
                << (*bones_accessor_id)
                << " cannot be a bones accessor: bad component type"));
          }
          ASSIGN_OR_RETURN(
              (const std::vector<glm::vec4>& weights),
              (ReadFloatAccessor<4>(buffers, buffer_views, weights_accessor)));
          for (unsigned int i = 0; i < mesh->vertices.size(); i++) {
            float sum =
                weights[i].x + weights[i].y + weights[i].z + weights[i].w;
            if (sum == 0) {
              sum = 1;
            }
            // Store normalized weights.
            primitive.skin->vertices[i].weights = weights[i] / sum;
          }
        }
      }
    }
  }
  return model;
}

absl::StatusOr<std::shared_ptr<Mesh>> GltfModel::LoadMesh(
    const PrimitiveDetails& details) {
  ASSIGN_OR_RETURN((const std::shared_ptr<GltfModel> model),
                   details.model.Get());

  const auto it = model->primitives.find(details.mesh_name);
  if (it == model->primitives.end()) {
    return absl::NotFoundError(
        STATUS_MESSAGE("No mesh named \"" << details.mesh_name << "\""));
  }

  if (details.index >= it->second.size()) {
    return absl::NotFoundError(STATUS_MESSAGE(
        "No primitive at index " << details.index << " in mesh named \""
                                 << details.mesh_name << "\""));
  }

  return it->second[details.index].mesh;
}

absl::StatusOr<std::shared_ptr<Skin>> GltfModel::LoadSkin(
    const PrimitiveDetails& details) {
  ASSIGN_OR_RETURN((const std::shared_ptr<GltfModel> model),
                   details.model.Get());

  const auto it = model->primitives.find(details.mesh_name);
  if (it == model->primitives.end()) {
    return absl::NotFoundError(
        STATUS_MESSAGE("No mesh named \"" << details.mesh_name << "\""));
  }

  if (details.index >= it->second.size()) {
    return absl::NotFoundError(STATUS_MESSAGE(
        "No primitive at index " << details.index << " in mesh named \""
                                 << details.mesh_name << "\""));
  }

  if (!it->second[details.index].skin) {
    return absl::NotFoundError(
        STATUS_MESSAGE("Primitive at index "
                       << details.index << " in mesh named \""
                       << details.mesh_name << "\" does not have a skin."));
  }
  return it->second[details.index].skin;
}
