
#include "resources/mesh_formats/gltf_mesh.h"

#include <glog/logging.h>

#include <fstream>
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
  ASSIGN_OR_RETURN(const nlohmann::json* element, (GetElementByKey(json, key)));
  if (!element->is_object()) {
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Invalid JSON data: element with key " << key << " is not an object"));
  }
  return element;
}

absl::StatusOr<const nlohmann::json*> GetArray(const nlohmann::json& json,
                                               const std::string& key) {
  ASSIGN_OR_RETURN(const nlohmann::json* element, (GetElementByKey(json, key)));
  if (!element->is_array()) {
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Invalid JSON data: element with key " << key << " is not an array"));
  }
  return element;
}

absl::StatusOr<int> GetInt(const nlohmann::json& json, const std::string& key) {
  ASSIGN_OR_RETURN(const nlohmann::json* element, (GetElementByKey(json, key)));
  if (!element->is_number_integer()) {
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Invalid JSON data: element with key " << key << " is not an integer"));
  }
  return element->get<int>();
}

absl::StatusOr<unsigned int> GetUnsignedInt(const nlohmann::json& json,
                                            const std::string& key) {
  ASSIGN_OR_RETURN(const nlohmann::json* element, (GetElementByKey(json, key)));
  if (!element->is_number_unsigned()) {
    return absl::InvalidArgumentError(
        STATUS_MESSAGE("Invalid JSON data: element with key "
                       << key << " is not an unsigned integer"));
  }
  return element->get<unsigned int>();
}

absl::StatusOr<std::string> GetString(const nlohmann::json& json,
                                      const std::string& key) {
  ASSIGN_OR_RETURN(const nlohmann::json* element, (GetElementByKey(json, key)));
  if (!element->is_string()) {
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Invalid JSON data: element with key " << key << " is not a string"));
  }
  return element->get<std::string>();
}

absl::StatusOr<bool> GetBool(const nlohmann::json& json,
                             const std::string& key) {
  ASSIGN_OR_RETURN(const nlohmann::json* element, (GetElementByKey(json, key)));
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

absl::StatusOr<std::vector<unsigned short>> ReadByteIndexAccessor(
    const std::vector<std::vector<unsigned char>>& buffers,
    const std::vector<BufferView>& buffer_views, const Accessor& accessor) {
  CHECK_EQ(accessor.type, "SCALAR");
  CHECK_EQ((uint32_t)accessor.component_type,
           (uint32_t)ComponentType::UnsignedByte);

  std::vector<unsigned short> indices;
  indices.resize(accessor.count);
  if (!accessor.buffer_view.has_value()) {
    // TODO: Handle sparse accessors.
    return indices;
  }

  const BufferView& buffer_view = buffer_views[*accessor.buffer_view];
  const std::vector<unsigned char>& buffer = buffers[buffer_view.buffer];
  unsigned int byte_start = buffer_view.offset + accessor.byte_offset;
  unsigned int byte_end = std::min((unsigned int)buffer.size(),
                                   buffer_view.offset + buffer_view.size);
  unsigned int byte_stride = buffer_view.stride == 0 ? 1 : buffer_view.stride;
  if (byte_start + byte_stride * accessor.count > byte_end) {
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Accessor requested bytes that are out of range of buffer (view). "
        "Requested range "
        << byte_start << "-" << (byte_start + byte_stride * accessor.count)
        << " but buffer ends at " << byte_end));
  }
  unsigned int byte_index = byte_start;
  for (unsigned int i = 0, byte_index = byte_start; i < accessor.count;
       ++i, byte_index += byte_stride) {
    indices[i] = (unsigned short)buffer[byte_index];
  }
  return indices;
}

absl::StatusOr<std::vector<unsigned short>> ReadShortIndexAccessor(
    const std::vector<std::vector<unsigned char>>& buffers,
    const std::vector<BufferView>& buffer_views, const Accessor& accessor) {
  CHECK_EQ(accessor.type, "SCALAR");
  CHECK_EQ((uint32_t)accessor.component_type,
           (uint32_t)ComponentType::UnsignedShort);

  std::vector<unsigned short> indices;
  indices.resize(accessor.count);
  if (!accessor.buffer_view.has_value()) {
    // TODO: Handle sparse accessors.
    return indices;
  }

  const BufferView& buffer_view = buffer_views[*accessor.buffer_view];
  const std::vector<unsigned char>& buffer = buffers[buffer_view.buffer];
  unsigned int byte_start = buffer_view.offset + accessor.byte_offset;
  unsigned int byte_end = std::min((unsigned int)buffer.size(),
                                   buffer_view.offset + buffer_view.size);
  unsigned int byte_stride = buffer_view.stride == 0 ? 2 : buffer_view.stride;
  if (byte_start + byte_stride * accessor.count > byte_end) {
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Accessor requested bytes that are out of range of buffer (view). "
        "Requested range "
        << byte_start << "-" << (byte_start + byte_stride * accessor.count)
        << " but buffer ends at " << byte_end));
  }
  unsigned int byte_index = byte_start;
  for (unsigned int i = 0, byte_index = byte_start; i < accessor.count;
       ++i, byte_index += byte_stride) {
    indices[i] = ltohs(((unsigned short)buffer[byte_index]) |
                       ((unsigned short)buffer[byte_index + 1] << 8));
  }
  return indices;
}

absl::StatusOr<std::vector<unsigned int>> ReadIntIndexAccessor(
    const std::vector<std::vector<unsigned char>>& buffers,
    const std::vector<BufferView>& buffer_views, const Accessor& accessor) {
  CHECK_EQ(accessor.type, "SCALAR");
  CHECK_EQ((uint32_t)accessor.component_type,
           (uint32_t)ComponentType::UnsignedInt);

  std::vector<unsigned int> indices;
  indices.resize(accessor.count);
  if (!accessor.buffer_view.has_value()) {
    // TODO: Handle sparse accessors.
    return indices;
  }

  const BufferView& buffer_view = buffer_views[*accessor.buffer_view];
  const std::vector<unsigned char>& buffer = buffers[buffer_view.buffer];
  unsigned int byte_start = buffer_view.offset + accessor.byte_offset;
  unsigned int byte_end = std::min((unsigned int)buffer.size(),
                                   buffer_view.offset + buffer_view.size);
  unsigned int byte_stride = buffer_view.stride == 0 ? 4 : buffer_view.stride;
  if (byte_start + byte_stride * accessor.count > byte_end) {
    return absl::InvalidArgumentError(STATUS_MESSAGE(
        "Accessor requested bytes that are out of range of buffer (view). "
        "Requested range "
        << byte_start << "-" << (byte_start + byte_stride * accessor.count)
        << " but buffer ends at " << byte_end));
  }
  unsigned int byte_index = byte_start;
  for (unsigned int i = 0, byte_index = byte_start; i < accessor.count;
       ++i, byte_index += byte_stride) {
    indices[i] = ltohl(((uint32_t)buffer[byte_index]) |
                       ((uint32_t)buffer[byte_index + 1] << 8) |
                       ((uint32_t)buffer[byte_index + 2] << 16) |
                       ((uint32_t)buffer[byte_index + 3] << 24));
  }
  return indices;
}

absl::StatusOr<std::vector<glm::vec2>> ReadVec2Accessor(
    const std::vector<std::vector<unsigned char>>& buffers,
    const std::vector<BufferView>& buffer_views, const Accessor& accessor) {
  CHECK_EQ(accessor.type, "VEC2");

  std::vector<glm::vec2> result;
  result.resize(accessor.count);
  if (!accessor.buffer_view.has_value()) {
    memset(result.data(), 0, result.size() * sizeof(glm::vec2));
    // TODO: Handle sparse accessors.
    return result;
  }

  const BufferView& buffer_view = buffer_views[*accessor.buffer_view];
  const std::vector<unsigned char>& buffer = buffers[buffer_view.buffer];
  unsigned int byte_start = buffer_view.offset + accessor.byte_offset;
  unsigned int byte_end = std::min((unsigned int)buffer.size(),
                                   buffer_view.offset + buffer_view.size);
  unsigned int byte_index = byte_start;

  switch ((uint32_t)accessor.component_type) {
    case GL_UNSIGNED_BYTE: {
      unsigned int byte_stride =
          buffer_view.stride == 0 ? 2 : buffer_view.stride;
      if (byte_start + byte_stride * accessor.count > byte_end) {
        return absl::InvalidArgumentError(STATUS_MESSAGE(
            "Accessor requested bytes that are out of range of buffer (view). "
            "Requested range "
            << byte_start << "-" << (byte_start + byte_stride * accessor.count)
            << " but buffer ends at " << byte_end));
      }
      for (unsigned int i = 0, byte_index = byte_start; i < accessor.count;
           ++i, byte_index += byte_stride) {
        uint8_t x_int = buffer[byte_index], y_int = buffer[byte_index + 1];
        if (accessor.normalize_ints) {
          float norm = (uint8_t)(-1);
          result[i] = glm::vec2(x_int / norm, y_int / norm);
        } else {
          result[i] = glm::vec2(x_int, y_int);
        }
      }
    } break;
    case GL_UNSIGNED_SHORT: {
      unsigned int byte_stride =
          buffer_view.stride == 0 ? 4 : buffer_view.stride;
      if (byte_start + byte_stride * accessor.count > byte_end) {
        return absl::InvalidArgumentError(STATUS_MESSAGE(
            "Accessor requested bytes that are out of range of buffer (view). "
            "Requested range "
            << byte_start << "-" << (byte_start + byte_stride * accessor.count)
            << " but buffer ends at " << byte_end));
      }
      for (unsigned int i = 0, byte_index = byte_start; i < accessor.count;
           ++i, byte_index += byte_stride) {
        uint16_t x_int = ltohs(((uint16_t)buffer[byte_index]) |
                               ((uint16_t)buffer[byte_index + 1] << 8)),
                 y_int = ltohs(((uint16_t)buffer[byte_index + 2]) |
                               ((uint16_t)buffer[byte_index + 3] << 8));
        if (accessor.normalize_ints) {
          float norm = (uint16_t)(-1);
          result[i] = glm::vec2(x_int / norm, y_int / norm);
        } else {
          result[i] = glm::vec2(x_int, y_int);
        }
      }
    } break;
    case GL_FLOAT: {
      unsigned int byte_stride =
          buffer_view.stride == 0 ? 8 : buffer_view.stride;
      if (byte_start + byte_stride * accessor.count > byte_end) {
        return absl::InvalidArgumentError(STATUS_MESSAGE(
            "Accessor requested bytes that are out of range of buffer (view). "
            "Requested range "
            << byte_start << "-" << (byte_start + byte_stride * accessor.count)
            << " but buffer ends at " << byte_end));
      }
      for (unsigned int i = 0, byte_index = byte_start; i < accessor.count;
           ++i, byte_index += byte_stride) {
        union {
          float fval;
          uint32_t ival;
        } x, y;
        x.ival = ltohl(((uint32_t)buffer[byte_index]) |
                       ((uint32_t)buffer[byte_index + 1] << 8) |
                       ((uint32_t)buffer[byte_index + 2] << 16) |
                       ((uint32_t)buffer[byte_index + 3] << 24));
        y.ival = ltohl(((uint32_t)buffer[byte_index + 4]) |
                       ((uint32_t)buffer[byte_index + 5] << 8) |
                       ((uint32_t)buffer[byte_index + 6] << 16) |
                       ((uint32_t)buffer[byte_index + 7] << 24));
        result[i] = glm::vec2(x.fval, y.fval);
      }
    } break;
    default:
      CHECK(false) << "Invalid accessor component type for ";
  }
  return result;
}

absl::StatusOr<std::vector<glm::vec3>> ReadVec3Accessor(
    const std::vector<std::vector<unsigned char>>& buffers,
    const std::vector<BufferView>& buffer_views, const Accessor& accessor) {
  CHECK_EQ(accessor.type, "VEC3");

  std::vector<glm::vec3> result;
  result.resize(accessor.count);
  if (!accessor.buffer_view.has_value()) {
    memset(result.data(), 0, result.size() * sizeof(glm::vec3));
    // TODO: Handle sparse accessors.
    return result;
  }

  const BufferView& buffer_view = buffer_views[*accessor.buffer_view];
  const std::vector<unsigned char>& buffer = buffers[buffer_view.buffer];
  unsigned int byte_start = buffer_view.offset + accessor.byte_offset;
  unsigned int byte_end = std::min((unsigned int)buffer.size(),
                                   buffer_view.offset + buffer_view.size);
  unsigned int byte_index = byte_start;

  switch ((uint32_t)accessor.component_type) {
    case GL_UNSIGNED_BYTE: {
      // TODO: Ensure this doesn't need to be aligned to word-boundaries.
      unsigned int byte_stride =
          buffer_view.stride == 0 ? 3 : buffer_view.stride;
      if (byte_start + byte_stride * accessor.count > byte_end) {
        return absl::InvalidArgumentError(STATUS_MESSAGE(
            "Accessor requested bytes that are out of range of buffer (view). "
            "Requested range "
            << byte_start << "-" << (byte_start + byte_stride * accessor.count)
            << " but buffer ends at " << byte_end));
      }
      for (unsigned int i = 0, byte_index = byte_start; i < accessor.count;
           ++i, byte_index += byte_stride) {
        uint8_t x_int = buffer[byte_index], y_int = buffer[byte_index + 1],
                z_int = buffer[byte_index + 2];
        if (accessor.normalize_ints) {
          float norm = (uint8_t)(-1);
          result[i] = glm::vec3(x_int / norm, y_int / norm, z_int / norm);
        } else {
          result[i] = glm::vec3(x_int, y_int, z_int);
        }
      }
    } break;
    case GL_UNSIGNED_SHORT: {
      // TODO: Ensure this doesn't need to be aligned to word-boundaries.
      unsigned int byte_stride =
          buffer_view.stride == 0 ? 6 : buffer_view.stride;
      if (byte_start + byte_stride * accessor.count > byte_end) {
        return absl::InvalidArgumentError(STATUS_MESSAGE(
            "Accessor requested bytes that are out of range of buffer (view). "
            "Requested range "
            << byte_start << "-" << (byte_start + byte_stride * accessor.count)
            << " but buffer ends at " << byte_end));
      }
      for (unsigned int i = 0, byte_index = byte_start; i < accessor.count;
           ++i, byte_index += byte_stride) {
        uint16_t x_int = ltohs(((uint16_t)buffer[byte_index]) |
                               ((uint16_t)buffer[byte_index + 1] << 8)),
                 y_int = ltohs(((uint16_t)buffer[byte_index + 2]) |
                               ((uint16_t)buffer[byte_index + 3] << 8)),
                 z_int = ltohs(((uint16_t)buffer[byte_index + 4]) |
                               ((uint16_t)buffer[byte_index + 5] << 8));
        if (accessor.normalize_ints) {
          float norm = (uint16_t)(-1);
          result[i] = glm::vec3(x_int / norm, y_int / norm, z_int / norm);
        } else {
          result[i] = glm::vec3(x_int, y_int, z_int);
        }
      }
    } break;
    case GL_FLOAT: {
      unsigned int byte_stride =
          buffer_view.stride == 0 ? 12 : buffer_view.stride;
      if (byte_start + byte_stride * accessor.count > byte_end) {
        return absl::InvalidArgumentError(STATUS_MESSAGE(
            "Accessor requested bytes that are out of range of buffer (view). "
            "Requested range "
            << byte_start << "-" << (byte_start + byte_stride * accessor.count)
            << " but buffer ends at " << byte_end));
      }
      for (unsigned int i = 0, byte_index = byte_start; i < accessor.count;
           ++i, byte_index += byte_stride) {
        union {
          float fval;
          uint32_t ival;
        } x, y, z;
        x.ival = ltohl(((uint32_t)buffer[byte_index]) |
                       ((uint32_t)buffer[byte_index + 1] << 8) |
                       ((uint32_t)buffer[byte_index + 2] << 16) |
                       ((uint32_t)buffer[byte_index + 3] << 24));
        y.ival = ltohl(((uint32_t)buffer[byte_index + 4]) |
                       ((uint32_t)buffer[byte_index + 5] << 8) |
                       ((uint32_t)buffer[byte_index + 6] << 16) |
                       ((uint32_t)buffer[byte_index + 7] << 24));
        z.ival = ltohl(((uint32_t)buffer[byte_index + 8]) |
                       ((uint32_t)buffer[byte_index + 9] << 8) |
                       ((uint32_t)buffer[byte_index + 10] << 16) |
                       ((uint32_t)buffer[byte_index + 11] << 24));
        result[i] = glm::vec3(x.fval, y.fval, z.fval);
      }
    } break;
    default:
      CHECK(false) << "Invalid accessor component type for ";
  }
  return result;
}

absl::StatusOr<std::vector<glm::vec4>> ReadVec4Accessor(
    const std::vector<std::vector<unsigned char>>& buffers,
    const std::vector<BufferView>& buffer_views, const Accessor& accessor) {
  CHECK_EQ(accessor.type, "VEC4");

  std::vector<glm::vec4> result;
  result.resize(accessor.count);
  if (!accessor.buffer_view.has_value()) {
    memset(result.data(), 0, result.size() * sizeof(glm::vec4));
    // TODO: Handle sparse accessors.
    return result;
  }

  const BufferView& buffer_view = buffer_views[*accessor.buffer_view];
  const std::vector<unsigned char>& buffer = buffers[buffer_view.buffer];
  unsigned int byte_start = buffer_view.offset + accessor.byte_offset;
  unsigned int byte_end = std::min((unsigned int)buffer.size(),
                                   buffer_view.offset + buffer_view.size);
  unsigned int byte_index = byte_start;

  switch ((uint32_t)accessor.component_type) {
    case GL_UNSIGNED_BYTE: {
      unsigned int byte_stride =
          buffer_view.stride == 0 ? 4 : buffer_view.stride;
      if (byte_start + byte_stride * accessor.count > byte_end) {
        return absl::InvalidArgumentError(STATUS_MESSAGE(
            "Accessor requested bytes that are out of range of buffer (view). "
            "Requested range "
            << byte_start << "-" << (byte_start + byte_stride * accessor.count)
            << " but buffer ends at " << byte_end));
      }
      for (unsigned int i = 0, byte_index = byte_start; i < accessor.count;
           ++i, byte_index += byte_stride) {
        uint8_t x_int = buffer[byte_index], y_int = buffer[byte_index + 1],
                z_int = buffer[byte_index + 2], w_int = buffer[byte_index + 3];
        if (accessor.normalize_ints) {
          float norm = (uint8_t)(-1);
          result[i] =
              glm::vec4(x_int / norm, y_int / norm, z_int / norm, z_int / norm);
        } else {
          result[i] = glm::vec4(x_int, y_int, z_int, z_int);
        }
      }
    } break;
    case GL_UNSIGNED_SHORT: {
      unsigned int byte_stride =
          buffer_view.stride == 0 ? 8 : buffer_view.stride;
      if (byte_start + byte_stride * accessor.count > byte_end) {
        return absl::InvalidArgumentError(STATUS_MESSAGE(
            "Accessor requested bytes that are out of range of buffer (view). "
            "Requested range "
            << byte_start << "-" << (byte_start + byte_stride * accessor.count)
            << " but buffer ends at " << byte_end));
      }
      for (unsigned int i = 0, byte_index = byte_start; i < accessor.count;
           ++i, byte_index += byte_stride) {
        uint16_t x_int = ltohs(((uint16_t)buffer[byte_index]) |
                               ((uint16_t)buffer[byte_index + 1] << 8)),
                 y_int = ltohs(((uint16_t)buffer[byte_index + 2]) |
                               ((uint16_t)buffer[byte_index + 3] << 8)),
                 z_int = ltohs(((uint16_t)buffer[byte_index + 4]) |
                               ((uint16_t)buffer[byte_index + 5] << 8)),
                 w_int = ltohs(((uint16_t)buffer[byte_index + 6]) |
                               ((uint16_t)buffer[byte_index + 7] << 8));
        if (accessor.normalize_ints) {
          float norm = (uint16_t)(-1);
          result[i] =
              glm::vec4(x_int / norm, y_int / norm, z_int / norm, w_int / norm);
        } else {
          result[i] = glm::vec4(x_int, y_int, z_int, w_int);
        }
      }
    } break;
    case GL_FLOAT: {
      unsigned int byte_stride =
          buffer_view.stride == 0 ? 16 : buffer_view.stride;
      if (byte_start + byte_stride * accessor.count > byte_end) {
        return absl::InvalidArgumentError(STATUS_MESSAGE(
            "Accessor requested bytes that are out of range of buffer (view). "
            "Requested range "
            << byte_start << "-" << (byte_start + byte_stride * accessor.count)
            << " but buffer ends at " << byte_end));
      }
      for (unsigned int i = 0, byte_index = byte_start; i < accessor.count;
           ++i, byte_index += byte_stride) {
        union {
          float fval;
          uint32_t ival;
        } x, y, z, w;
        x.ival = ltohl(((uint32_t)buffer[byte_index]) |
                       ((uint32_t)buffer[byte_index + 1] << 8) |
                       ((uint32_t)buffer[byte_index + 2] << 16) |
                       ((uint32_t)buffer[byte_index + 3] << 24));
        y.ival = ltohl(((uint32_t)buffer[byte_index + 4]) |
                       ((uint32_t)buffer[byte_index + 5] << 8) |
                       ((uint32_t)buffer[byte_index + 6] << 16) |
                       ((uint32_t)buffer[byte_index + 7] << 24));
        z.ival = ltohl(((uint32_t)buffer[byte_index + 8]) |
                       ((uint32_t)buffer[byte_index + 9] << 8) |
                       ((uint32_t)buffer[byte_index + 10] << 16) |
                       ((uint32_t)buffer[byte_index + 11] << 24));
        w.ival = ltohl(((uint32_t)buffer[byte_index + 12]) |
                       ((uint32_t)buffer[byte_index + 13] << 8) |
                       ((uint32_t)buffer[byte_index + 14] << 16) |
                       ((uint32_t)buffer[byte_index + 15] << 24));
        result[i] = glm::vec4(x.fval, y.fval, z.fval, w.fval);
      }
    } break;
    default:
      CHECK(false) << "Invalid accessor component type for ";
  }
  return result;
}

absl::StatusOr<BufferView> ParseBufferView(
    const nlohmann::json& buffer_view_json) {
  BufferView buffer_view;
  ASSIGN_OR_RETURN(buffer_view.buffer,
                   GetUnsignedInt(buffer_view_json, "buffer"));
  ASSIGN_OR_RETURN(buffer_view.size,
                   GetUnsignedInt(buffer_view_json, "byteLength"));
  ASSIGN_OR_RETURN(buffer_view.offset,
                   GetUnsignedInt(buffer_view_json, "byteOffset"));
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
  ASSIGN_OR_RETURN(accessor.count, GetUnsignedInt(accessor_json, "count"));
  ASSIGN_OR_RETURN(accessor.type, GetString(accessor_json, "type"));
  return accessor;
}

absl::StatusOr<std::shared_ptr<GltfModel>> GltfModel::Load(
    const Details& details) {
  std::ifstream file(details.file);
  if (!file.is_open()) {
    return absl::NotFoundError(
        STATUS_MESSAGE("Unable to read file " << details.file));
  }

  // TODO: Check if file is glB and handle.

  nlohmann::json root;
  file >> root;
  file.close();

  if (!root.is_object()) {
    return absl::InvalidArgumentError("Root of glTF file is not an object");
  }

  std::vector<std::vector<unsigned char>> buffers;
  ASSIGN_OR_RETURN(const nlohmann::json* buffers_json_array,
                   GetArray(root, "buffers"));
  buffers.reserve(buffers_json_array->size());
  for (const auto& buffer_json : *buffers_json_array) {
    if (!buffer_json.is_object()) {
      return absl::InvalidArgumentError(
          "Invalid JSON data: buffer is not an object.");
    }
    buffers.push_back(std::vector<unsigned char>());

    ASSIGN_OR_RETURN(unsigned int byte_len,
                     GetUnsignedInt(buffer_json, "byteLength"));
    ASSIGN_OR_RETURN(const std::string& uri, GetString(buffer_json, "uri"));
    if (uri.rfind("data:", 0) == 0) {
      // TODO: Handle data URIs.
      return absl::UnimplementedError("TODO: Handle data URIs.");
    } else {
      // If it is not a data URI, assume it is a relative file.
      std::ifstream buffer_file(uri, std::ios_base::in | std::ios_base::binary);
      if (!buffer_file.is_open()) {
        return absl::InvalidArgumentError(
            STATUS_MESSAGE("Unable to read buffer file \"" << uri << "\""));
      }

      std::vector<unsigned char>& buffer_data = buffers.back();
      buffer_data.resize(byte_len);
      buffer_file.read((char*)buffer_data.data(), byte_len);
      unsigned int read_bytes = buffer_file.gcount();
      if (read_bytes != byte_len) {
        return absl::InvalidArgumentError(STATUS_MESSAGE(
            "Unable to read requested byte count from Buffer file \""
            << uri << "\". Requested " << byte_len << ", but got "
            << read_bytes));
      }
    }
  }

  std::vector<BufferView> buffer_views;
  ASSIGN_OR_RETURN(const nlohmann::json* buffer_view_json_array,
                   GetArray(root, "bufferViews"));
  buffer_views.reserve(buffer_view_json_array->size());
  for (const nlohmann::json& buffer_view_json : *buffer_view_json_array) {
    if (!buffer_view_json.is_object()) {
      return absl::InvalidArgumentError(
          "Invalid JSON data: buffer view is not an object.");
    }
    ASSIGN_OR_RETURN(buffer_views.emplace_back(),
                     ParseBufferView(buffer_view_json));
  }

  std::vector<Accessor> accessors;
  ASSIGN_OR_RETURN(const nlohmann::json* accessor_json_array,
                   GetArray(root, "accessors"));
  accessors.reserve(accessor_json_array->size());
  for (const nlohmann::json& accessor_json : *accessor_json_array) {
    if (!accessor_json.is_object()) {
      return absl::InvalidArgumentError(
          "Invalid JSON data: accessor is not an object.");
    }
    ASSIGN_OR_RETURN(accessors.emplace_back(), ParseAccessor(accessor_json));
  }

  std::shared_ptr<GltfModel> model(new GltfModel());

  ASSIGN_OR_RETURN(const nlohmann::json* meshes_array,
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

    ASSIGN_OR_RETURN(const nlohmann::json* primitives_array,
                     GetArray(mesh, "primitives"));
    if (primitives_array->size() == 0) {
      continue;
    }

    for (const nlohmann::json& primitive : *primitives_array) {
      if (!primitive.is_object()) {
        return absl::InvalidArgumentError(
            "Invalid JSON data: primitive is not an object.");
      }

      std::shared_ptr<Mesh> mesh(new Mesh());
      model->meshes[*mesh_name].push_back(mesh);

      ASSIGN_OR_RETURN(const nlohmann::json* attributes,
                       GetObject(primitive, "attributes"));
      {
        ASSIGN_OR_RETURN(const unsigned int position_accessor_id,
                         GetUnsignedInt(*attributes, "POSITION"));
        if (position_accessor_id >= accessors.size()) {
          return absl::InvalidArgumentError(
              STATUS_MESSAGE("Missing accessor: " << position_accessor_id));
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
            const std::vector<glm::vec3>& positions,
            ReadVec3Accessor(buffers, buffer_views, position_accessor));
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
                "Missing accessor: " << (*texcoord_accessor_id)));
          }
          const Accessor& texcoord_accessor = accessors[*texcoord_accessor_id];
          if (texcoord_accessor.type != "VEC2" ||
              (texcoord_accessor.component_type != ComponentType::Float &&
               (texcoord_accessor.component_type !=
                    ComponentType::UnsignedByte ||
                !texcoord_accessor.normalize_ints) &&
               (texcoord_accessor.component_type !=
                    ComponentType::UnsignedShort ||
                !texcoord_accessor.normalize_ints))) {
            return absl::InvalidArgumentError(
                STATUS_MESSAGE("Accessor " << (*texcoord_accessor_id)
                                           << " cannot be a texcoord accessor "
                                              "(wrong type or component type"));
          }
          ASSIGN_OR_RETURN(
              const std::vector<glm::vec2>& texcoords,
              ReadVec2Accessor(buffers, buffer_views, texcoord_accessor));
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
            vertex.colour = glm::vec3(0, 0, 0);
          }
        } else {
          if (*colour_accessor_id >= accessors.size()) {
            return absl::InvalidArgumentError(
                STATUS_MESSAGE("Missing accessor: " << (*colour_accessor_id)));
          }
          const Accessor& colour_accessor = accessors[*colour_accessor_id];
          if (colour_accessor.type != "VEC3" ||
              (colour_accessor.component_type != ComponentType::Float &&
               (colour_accessor.component_type != ComponentType::UnsignedByte ||
                !colour_accessor.normalize_ints) &&
               (colour_accessor.component_type !=
                    ComponentType::UnsignedShort ||
                !colour_accessor.normalize_ints))) {
            return absl::InvalidArgumentError(
                STATUS_MESSAGE("Accessor " << (*colour_accessor_id)
                                           << " cannot be a colour accessor "
                                              "(wrong type or component type"));
          }
          ASSIGN_OR_RETURN(
              const std::vector<glm::vec3>& colours,
              ReadVec3Accessor(buffers, buffer_views, colour_accessor));
          if (colours.size() != mesh->vertices.size()) {
            return absl::InvalidArgumentError(STATUS_MESSAGE(
                "Colours do not have same size as vertices. Have "
                << mesh->vertices.size() << " vertices, but " << colours.size()
                << " colours"));
          }
          for (unsigned int i = 0; i < mesh->vertices.size(); i++) {
            mesh->vertices[i].colour = colours[i];
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
            return absl::InvalidArgumentError(
                STATUS_MESSAGE("Missing accessor: " << (*normal_accessor_id)));
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
              const std::vector<glm::vec3>& normals,
              ReadVec3Accessor(buffers, buffer_views, normal_accessor));
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
            return absl::InvalidArgumentError(
                STATUS_MESSAGE("Missing accessor: " << (*tangent_accessor_id)));
          }
          const Accessor& tangent_accessor = accessors[*tangent_accessor_id];
          if (tangent_accessor.type != "VEC4" ||
              tangent_accessor.component_type != ComponentType::Float) {
            return absl::InvalidArgumentError(
                STATUS_MESSAGE("Accessor " << (*tangent_accessor_id)
                                           << " cannot be a tangent accessor "
                                              "(wrong type or component type"));
          }
          ASSIGN_OR_RETURN(
              const std::vector<glm::vec4>& tangents,
              ReadVec4Accessor(buffers, buffer_views, tangent_accessor));
          if (tangents.size() != mesh->vertices.size()) {
            return absl::InvalidArgumentError(STATUS_MESSAGE(
                "Tangents do not have same size as vertices. Have "
                << mesh->vertices.size() << " vertices, but " << tangents.size()
                << " tangents"));
          }
          for (unsigned int i = 0; i < mesh->vertices.size(); i++) {
            // TODO: Handle tangents with negative w component.
            mesh->vertices[i].tangent = tangents[i];
            mesh->vertices[i].bitangent = glm::normalize(glm::cross(
                mesh->vertices[i].normal, mesh->vertices[i].tangent));
          }
        }
      }
      {
        const absl::StatusOr<unsigned int> indices_accessor_id =
            GetUnsignedInt(primitive, "indices");
        if (indices_accessor_id.ok()) {
          if (*indices_accessor_id >= accessors.size()) {
            return absl::InvalidArgumentError(
                STATUS_MESSAGE("Missing accessor: " << (*indices_accessor_id)));
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
                const std::vector<unsigned int>& indices,
                ReadIntIndexAccessor(buffers, buffer_views, indices_accessor));
            mesh->triangles.resize(indices.size() / 3);
            for (unsigned int i = 0; i < indices.size(); i += 3) {
              mesh->triangles.push_back(
                  {indices[i], indices[i + 1], indices[i + 2]});
            }
          } else if (indices_accessor.component_type ==
                     ComponentType::UnsignedShort) {
            ASSIGN_OR_RETURN(const std::vector<unsigned short>& indices,
                             ReadShortIndexAccessor(buffers, buffer_views,
                                                    indices_accessor));
            mesh->small_triangles.resize(indices.size() / 3);
            for (unsigned int i = 0; i < indices.size(); i += 3) {
              mesh->small_triangles.push_back(
                  {indices[i], indices[i + 1], indices[i + 2]});
            }
          } else {
            ASSIGN_OR_RETURN(
                const std::vector<unsigned short>& indices,
                ReadByteIndexAccessor(buffers, buffer_views, indices_accessor));
            mesh->small_triangles.resize(indices.size() / 3);
            for (unsigned int i = 0; i < indices.size(); i += 3) {
              mesh->small_triangles.push_back(
                  {indices[i], indices[i + 1], indices[i + 2]});
            }
          }
        }
      }
      // TODO: Handle skinning.
    }
  }
  return model;
}

absl::StatusOr<std::shared_ptr<Mesh>> GltfModel::LoadMesh(
    const MeshDetails& details) {
  ASSIGN_OR_RETURN(const std::shared_ptr<GltfModel> model, details.model.Get());

  const auto it = model->meshes.find(details.mesh_name);
  if (it == model->meshes.end()) {
    return absl::NotFoundError(
        STATUS_MESSAGE("No mesh named \"" << details.mesh_name << "\""));
  }

  if (details.index >= it->second.size()) {
    return absl::NotFoundError(STATUS_MESSAGE(
        "No primitive at index " << details.index << " in mesh named \""
                                 << details.mesh_name << "\""));
  }

  return it->second[details.index];
}
