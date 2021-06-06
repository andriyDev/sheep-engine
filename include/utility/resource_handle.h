
#pragma once

#include <absl/status/statusor.h>
#include <absl/types/variant.h>

#include <string>

#include "resources/resource.h"

template <typename ResourceType>
struct ResourceHandle {
 public:
  ResourceHandle(const char* name);
  ResourceHandle(const std::string& name);
  ResourceHandle(const std::shared_ptr<ResourceType>& resource);

  absl::StatusOr<std::shared_ptr<ResourceType>> Get() const;

  ResourceHandle<ResourceType>& operator=(const std::string& name);

  ResourceHandle<ResourceType>& operator=(
      const std::shared_ptr<ResourceType>& resource);

 private:
  absl::variant<std::string, std::shared_ptr<ResourceType>> value;
};

// ===== Template implementation ===== //

template <typename ResourceType>
ResourceHandle<ResourceType>::ResourceHandle(const char* name) {
  value = std::string(name);
}

template <typename ResourceType>
ResourceHandle<ResourceType>::ResourceHandle(const std::string& name) {
  value = name;
}

template <typename ResourceType>
ResourceHandle<ResourceType>::ResourceHandle(
    const std::shared_ptr<ResourceType>& resource) {
  value = resource;
}

template <typename ResourceType>
absl::StatusOr<std::shared_ptr<ResourceType>>
ResourceHandle<ResourceType>::Get() const {
  if (absl::holds_alternative<std::string>(value)) {
    return ResourceLoader::Get().Load<ResourceType>(
        absl::get<std::string>(value));
  } else {
    return absl::get<std::shared_ptr<ResourceType>>(value);
  }
}

template <typename ResourceType>
ResourceHandle<ResourceType>& ResourceHandle<ResourceType>::operator=(
    const std::string& name) {
  value = name;
}

template <typename ResourceType>
ResourceHandle<ResourceType>& ResourceHandle<ResourceType>::operator=(
    const std::shared_ptr<ResourceType>& resource) {
  value = resource;
}
