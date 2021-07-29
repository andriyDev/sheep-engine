
#include "utility/json.h"

namespace json {

std::optional<const json*> GetElementByKey(const json& object,
                                           const std::string& key) {
  const auto it = object.find(key);
  if (it == object.end()) {
    return std::nullopt;
  } else {
    return &it.value();
  }
}

std::optional<const json*> GetElementByIndex(const json& array,
                                             unsigned int index) {
  if (index >= array.size()) {
    return std::nullopt;
  } else {
    return &array.at(index);
  }
}

absl::StatusOr<unsigned int> GetRequiredUint(const json& object,
                                             const std::string& key) {
  const std::optional<const json*> element = GetElementByKey(object, key);
  if (!element.has_value()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Object does not contain key \"" << key << "\""));
  }
  if (!(*element)->is_number_unsigned()) {
    return absl::FailedPreconditionError(STATUS_MESSAGE(
        "Value for key \"" << key << "\" is not an unsigned integer"));
  }
  return (*element)->get<unsigned int>();
}

absl::StatusOr<int> GetRequiredInt(const json& object, const std::string& key) {
  const std::optional<const json*> element = GetElementByKey(object, key);
  if (!element.has_value()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Object does not contain key \"" << key << "\""));
  }
  if (!(*element)->is_number_integer()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Value for key \"" << key << "\" is not an integer"));
  }
  return (*element)->get<int>();
}

absl::StatusOr<float> GetRequiredFloat(const json& object,
                                       const std::string& key) {
  const std::optional<const json*> element = GetElementByKey(object, key);
  if (!element.has_value()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Object does not contain key \"" << key << "\""));
  }
  if (!(*element)->is_number_float()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Value for key \"" << key << "\" is not a float"));
  }
  return (*element)->get<float>();
}

absl::StatusOr<std::string> GetRequiredString(const json& object,
                                              const std::string& key) {
  const std::optional<const json*> element = GetElementByKey(object, key);
  if (!element.has_value()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Object does not contain key \"" << key << "\""));
  }
  if (!(*element)->is_string()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Value for key \"" << key << "\" is not a string"));
  }
  return (*element)->get<std::string>();
}

absl::StatusOr<bool> GetRequiredBool(const json& object,
                                     const std::string& key) {
  const std::optional<const json*> element = GetElementByKey(object, key);
  if (!element.has_value()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Object does not contain key \"" << key << "\""));
  }
  if (!(*element)->is_boolean()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Value for key \"" << key << "\" is not a bool"));
  }
  return (*element)->get<bool>();
}

absl::StatusOr<const json*> GetRequiredObject(const json& object,
                                              const std::string& key) {
  const std::optional<const json*> element = GetElementByKey(object, key);
  if (!element.has_value()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Object does not contain key \"" << key << "\""));
  }
  if (!(*element)->is_object()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Value for key \"" << key << "\" is not an object"));
  }
  return *element;
}

absl::StatusOr<const json*> GetRequiredArray(const json& object,
                                             const std::string& key) {
  const std::optional<const json*> element = GetElementByKey(object, key);
  if (!element.has_value()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Object does not contain key \"" << key << "\""));
  }
  if (!(*element)->is_array()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Value for key \"" << key << "\" is not an array"));
  }
  return *element;
}

absl::StatusOr<unsigned int> GetRequiredUint(const json& array,
                                             unsigned int index) {
  const std::optional<const json*> element = GetElementByIndex(array, index);
  if (!element.has_value()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Array not large enough for index " << index));
  }
  if (!(*element)->is_number_unsigned()) {
    return absl::FailedPreconditionError(STATUS_MESSAGE(
        "Value at index " << index << " is not an unsigned integer"));
  }
  return (*element)->get<unsigned int>();
}

absl::StatusOr<int> GetRequiredInt(const json& array, unsigned int index) {
  const std::optional<const json*> element = GetElementByIndex(array, index);
  if (!element.has_value()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Array not large enough for index " << index));
  }
  if (!(*element)->is_number_integer()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Value at index " << index << " is not an integer"));
  }
  return (*element)->get<int>();
}

absl::StatusOr<float> GetRequiredFloat(const json& array, unsigned int index) {
  const std::optional<const json*> element = GetElementByIndex(array, index);
  if (!element.has_value()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Array not large enough for index " << index));
  }
  if (!(*element)->is_number_float()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Value at index " << index << " is not a float"));
  }
  return (*element)->get<float>();
}

absl::StatusOr<std::string> GetRequiredString(const json& array,
                                              unsigned int index) {
  const std::optional<const json*> element = GetElementByIndex(array, index);
  if (!element.has_value()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Array not large enough for index " << index));
  }
  if (!(*element)->is_string()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Value at index " << index << " is not a string"));
  }
  return (*element)->get<std::string>();
}

absl::StatusOr<bool> GetRequiredBool(const json& array, unsigned int index) {
  const std::optional<const json*> element = GetElementByIndex(array, index);
  if (!element.has_value()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Array not large enough for index " << index));
  }
  if (!(*element)->is_boolean()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Value at index " << index << " is not a bool"));
  }
  return (*element)->get<bool>();
}

absl::StatusOr<const json*> GetRequiredObject(const json& array,
                                              unsigned int index) {
  const std::optional<const json*> element = GetElementByIndex(array, index);
  if (!element.has_value()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Array not large enough for index " << index));
  }
  if (!(*element)->is_object()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Value at index " << index << " is not an object"));
  }
  return *element;
}

absl::StatusOr<const json*> GetRequiredArray(const json& array,
                                             unsigned int index) {
  const std::optional<const json*> element = GetElementByIndex(array, index);
  if (!element.has_value()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Array not large enough for index " << index));
  }
  if (!(*element)->is_array()) {
    return absl::FailedPreconditionError(
        STATUS_MESSAGE("Value at index " << index << " is not an array"));
  }
  return *element;
}

std::optional<unsigned int> GetOptionalUint(const json& object,
                                            const std::string& key) {
  const std::optional<const json*> element = GetElementByKey(object, key);
  if (!element.has_value() || !(*element)->is_number_unsigned()) {
    return std::nullopt;
  }
  return (*element)->get<unsigned int>();
}

std::optional<int> GetOptionalInt(const json& object, const std::string& key) {
  const std::optional<const json*> element = GetElementByKey(object, key);
  if (!element.has_value() || !(*element)->is_number_integer()) {
    return std::nullopt;
  }
  return (*element)->get<int>();
}

std::optional<float> GetOptionalFloat(const json& object,
                                      const std::string& key) {
  const std::optional<const json*> element = GetElementByKey(object, key);
  if (!element.has_value() || !(*element)->is_number_float()) {
    return std::nullopt;
  }
  return (*element)->get<float>();
}

std::optional<std::string> GetOptionalString(const json& object,
                                             const std::string& key) {
  const std::optional<const json*> element = GetElementByKey(object, key);
  if (!element.has_value() || !(*element)->is_string()) {
    return std::nullopt;
  }
  return (*element)->get<std::string>();
}

std::optional<bool> GetOptionalBool(const json& object,
                                    const std::string& key) {
  const std::optional<const json*> element = GetElementByKey(object, key);
  if (!element.has_value() || !(*element)->is_boolean()) {
    return std::nullopt;
  }
  return (*element)->get<bool>();
}

std::optional<const json*> GetOptionalObject(const json& object,
                                             const std::string& key) {
  const std::optional<const json*> element = GetElementByKey(object, key);
  if (!element.has_value() || !(*element)->is_object()) {
    return std::nullopt;
  }
  return *element;
}

std::optional<const json*> GetOptionalArray(const json& object,
                                            const std::string& key) {
  const std::optional<const json*> element = GetElementByKey(object, key);
  if (!element.has_value() || !(*element)->is_array()) {
    return std::nullopt;
  }
  return *element;
}

std::optional<unsigned int> GetOptionalUint(const json& array,
                                            unsigned int index) {
  const std::optional<const json*> element = GetElementByIndex(array, index);
  if (!element.has_value() || !(*element)->is_number_unsigned()) {
    return std::nullopt;
  }
  return (*element)->get<unsigned int>();
}

std::optional<int> GetOptionalInt(const json& array, unsigned int index) {
  const std::optional<const json*> element = GetElementByIndex(array, index);
  if (!element.has_value() || !(*element)->is_number_integer()) {
    return std::nullopt;
  }
  return (*element)->get<int>();
}

std::optional<float> GetOptionalFloat(const json& array, unsigned int index) {
  const std::optional<const json*> element = GetElementByIndex(array, index);
  if (!element.has_value() || !(*element)->is_number_float()) {
    return std::nullopt;
  }
  return (*element)->get<float>();
}

std::optional<std::string> GetOptionalString(const json& array,
                                             unsigned int index) {
  const std::optional<const json*> element = GetElementByIndex(array, index);
  if (!element.has_value() || !(*element)->is_string()) {
    return std::nullopt;
  }
  return (*element)->get<std::string>();
}

std::optional<bool> GetOptionalBool(const json& array, unsigned int index) {
  const std::optional<const json*> element = GetElementByIndex(array, index);
  if (!element.has_value() || !(*element)->is_boolean()) {
    return std::nullopt;
  }
  return (*element)->get<bool>();
}

std::optional<const json*> GetOptionalObject(const json& array,
                                             unsigned int index) {
  const std::optional<const json*> element = GetElementByIndex(array, index);
  if (!element.has_value() || !(*element)->is_object()) {
    return std::nullopt;
  }
  return *element;
}

std::optional<const json*> GetOptionalArray(const json& array,
                                            unsigned int index) {
  const std::optional<const json*> element = GetElementByIndex(array, index);
  if (!element.has_value() || !(*element)->is_array()) {
    return std::nullopt;
  }
  return *element;
}

}  // namespace json
