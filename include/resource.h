#pragma once

#include <assert.h>

#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>

// Manages resource loading.
class ResourceLoader {
 public:
  // Loads a resource by `resourceName` given its type.
  template <typename T>
  std::shared_ptr<T> Load(const std::string& resourceName) {
    auto info_it = resourceInfo.find(resourceName);
    if (info_it == resourceInfo.end()) {
      return nullptr;
    }
    assert(info_it->second.type == typeid(T));
    std::shared_ptr<T> ptr =
        std::static_pointer_cast<T>(info_it->second.ref.lock());
    if (ptr) {
      return ptr;
    }

    // Crash if dependencies form a cycle.
    assert(loadingResources.insert(resourceName).second);

    const auto details_ptr =
        static_cast<DetailsContainer<typename T::detail_type>*>(
            info_it->second.detailsContainer.get());
    ptr = T::Load(details_ptr->contents);
    info_it->second.ref = ptr;
    // No longer loading the resource.
    loadingResources.erase(resourceName);
    return ptr;
  }

  // Adds a new resource with its `resourceName` and the `details` required to
  // load it.
  template <typename T>
  void Add(const std::string& resourceName,
           const typename T::detail_type& details) {
    auto info_pair = std::make_pair(
        resourceName,
        ResourceInfo{
            std::make_shared<DetailsContainer<typename T::detail_type>>(
                details),
            std::weak_ptr<T>(), typeid(T)});
    assert(resourceInfo.insert(std::move(info_pair)).second);
  }

 private:
  template <typename T>
  struct DetailsContainer {
    DetailsContainer(const T& value) : contents(value) {}

    const T contents;
  };
  struct ResourceInfo {
    std::shared_ptr<void> detailsContainer;
    std::weak_ptr<void> ref;
    const std::type_index type;
  };

  std::unordered_map<std::string, ResourceInfo> resourceInfo;
  std::unordered_set<std::string> loadingResources;
};
