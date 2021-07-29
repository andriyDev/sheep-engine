
#pragma once

#include <nlohmann/json.hpp>
#include <optional>

#include "utility/status.h"

namespace json {

using json = nlohmann::json;

std::optional<const json*> GetElementByKey(const json& object,
                                           const std::string& key);
std::optional<const json*> GetElementByIndex(const json& array,
                                             unsigned int index);

absl::StatusOr<unsigned int> GetRequiredUint(const json& object,
                                             const std::string& key);
absl::StatusOr<int> GetRequiredInt(const json& object, const std::string& key);
absl::StatusOr<float> GetRequiredFloat(const json& object,
                                       const std::string& key);
absl::StatusOr<std::string> GetRequiredString(const json& object,
                                              const std::string& key);
absl::StatusOr<bool> GetRequiredBool(const json& object,
                                     const std::string& key);
absl::StatusOr<const json*> GetRequiredObject(const json& object,
                                              const std::string& key);
absl::StatusOr<const json*> GetRequiredArray(const json& object,
                                             const std::string& key);

absl::StatusOr<unsigned int> GetRequiredUint(const json& array,
                                             unsigned int index);
absl::StatusOr<int> GetRequiredInt(const json& array, unsigned int index);
absl::StatusOr<float> GetRequiredFloat(const json& array, unsigned int index);
absl::StatusOr<std::string> GetRequiredString(const json& array,
                                              unsigned int index);
absl::StatusOr<bool> GetRequiredBool(const json& array, unsigned int index);
absl::StatusOr<const json*> GetRequiredObject(const json& array,
                                              unsigned int index);
absl::StatusOr<const json*> GetRequiredArray(const json& array,
                                             unsigned int index);

std::optional<unsigned int> GetOptionalUint(const json& object,
                                            const std::string& key);
std::optional<int> GetOptionalInt(const json& object, const std::string& key);
std::optional<float> GetOptionalFloat(const json& object,
                                      const std::string& key);
std::optional<std::string> GetOptionalString(const json& object,
                                             const std::string& key);
std::optional<bool> GetOptionalBool(const json& object, const std::string& key);
std::optional<const json*> GetOptionalObject(const json& object,
                                             const std::string& key);
std::optional<const json*> GetOptionalArray(const json& object,
                                            const std::string& key);

std::optional<unsigned int> GetOptionalUint(const json& array,
                                            unsigned int index);
std::optional<int> GetOptionalInt(const json& array, unsigned int index);
std::optional<float> GetOptionalFloat(const json& array, unsigned int index);
std::optional<std::string> GetOptionalString(const json& array,
                                             unsigned int index);
std::optional<bool> GetOptionalBool(const json& array, unsigned int index);
std::optional<const json*> GetOptionalObject(const json& array,
                                             unsigned int index);
std::optional<const json*> GetOptionalArray(const json& array,
                                            unsigned int index);

}  // namespace json
