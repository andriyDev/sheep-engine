#pragma once

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <assert.h>

#include <functional>
#include <memory>
#include <string>
#include <typeindex>

// Manages resource loading.
class ResourceLoader {
 public:
  // Loads a resource by `resourceName` given its type.
  template <typename ResourceType>
  std::shared_ptr<ResourceType> Load(const std::string& resourceName);

  // Adds a new resource with its `resourceName` and the `details` required to
  // load it.
  template <typename ResourceType>
  void Add(const std::string& resourceName,
           const typename ResourceType::detail_type& details);

  // Adds a new resource with its `resourceName` using `loader` and the
  // `details` required to load it.
  template <typename ResourceType, typename DetailType>
  void Add(const std::string& resourceName,
           std::shared_ptr<ResourceType> (*loader)(const DetailType&),
           const DetailType& details);

  // Adds a new resource with its `resourceName` and the `loader` function to
  // load it.
  template <typename ResourceType>
  void Add(const std::string& resourceName,
           std::function<std::shared_ptr<ResourceType>()> loader);

  // Adds 1 to depth of the loader. While loading depth is greater than 0
  // (default value), newly loaded resources are not allowed to unload.
  void IncrementLoadingDepth() { loadingDepth++; }

  // Subtracts 1 from depth of the loader. While loading depth is greater than 0
  // (default value), newly loaded resource are not allowed to unload. Crashes
  // if loading depth becomes negative.
  void DecrementLoadingDepth() { loadingDepth--; }

  // Releases any resources that are currently being held. If these resources
  // have no other references, the resource will be unloaded.
  void ManualRelease() { heldResources.clear(); }

  static ResourceLoader& Get() { return instance; }

 private:
  static ResourceLoader instance;

  template <typename Resource>
  struct Loader {
    virtual std::shared_ptr<Resource> Load() = 0;
  };

  template <typename ResourceType, typename Details>
  struct LoaderContainer : public Loader<ResourceType> {
    LoaderContainer(std::shared_ptr<ResourceType> (*loader_)(const Details&),
                    const Details& value)
        : loader(loader_), contents(value) {}

    std::shared_ptr<ResourceType> (*loader)(const Details&);
    const Details contents;

    std::shared_ptr<ResourceType> Load() override { return loader(contents); }
  };

  template <typename ResourceType>
  struct RawLoaderContainer : public Loader<ResourceType> {
    RawLoaderContainer(std::function<std::shared_ptr<ResourceType>()> loader_)
        : loader(loader_) {}

    std::function<std::shared_ptr<ResourceType>()> loader;

    std::shared_ptr<ResourceType> Load() override { return loader(); }
  };

  struct ResourceInfo {
    std::shared_ptr<void> loaderContainer;
    std::weak_ptr<void> ref;
    const std::type_index type;
  };

  absl::flat_hash_map<std::string, ResourceInfo> resourceInfo;
  absl::flat_hash_set<std::string> loadingResources;
  std::vector<std::shared_ptr<void>> heldResources;
  int loadingDepth = 0;
};

// ===== Template Implementations ===== //

template <typename ResourceType>
std::shared_ptr<ResourceType> ResourceLoader::Load(
    const std::string& resourceName) {
  auto info_it = resourceInfo.find(resourceName);
  if (info_it == resourceInfo.end()) {
    fprintf(stderr, "Failed to load resource: %s\n", resourceName.c_str());
    return nullptr;
  }
  assert(info_it->second.type == typeid(ResourceType));
  std::shared_ptr<ResourceType> ptr =
      std::static_pointer_cast<ResourceType>(info_it->second.ref.lock());
  if (ptr) {
    return ptr;
  }

  // Crash if dependencies form a cycle.
  assert(loadingResources.insert(resourceName).second);

  IncrementLoadingDepth();

  const auto loader_ptr =
      static_cast<Loader<ResourceType>*>(info_it->second.loaderContainer.get());
  ptr = loader_ptr->Load();
  info_it->second.ref = ptr;

  DecrementLoadingDepth();

  // No longer loading the resource.
  loadingResources.erase(resourceName);
  // If loading depth is non-zero, hold the resource.
  if (loadingDepth > 0) {
    heldResources.push_back(ptr);
  }
  return ptr;
}

template <typename ResourceType>
void ResourceLoader::Add(const std::string& resourceName,
                         const typename ResourceType::detail_type& details) {
  Add<ResourceType, typename ResourceType::detail_type>(
      resourceName, ResourceType::Load, details);
}

template <typename ResourceType, typename DetailType>
void ResourceLoader::Add(
    const std::string& resourceName,
    std::shared_ptr<ResourceType> (*loader)(const DetailType&),
    const DetailType& details) {
  auto info_pair = std::make_pair(
      resourceName,
      ResourceInfo{std::make_shared<LoaderContainer<ResourceType, DetailType>>(
                       loader, details),
                   std::weak_ptr<ResourceType>(), typeid(ResourceType)});
  assert(resourceInfo.insert(std::move(info_pair)).second);
}

template <typename ResourceType>
void ResourceLoader::Add(
    const std::string& resourceName,
    std::function<std::shared_ptr<ResourceType>()> loader) {
  auto info_pair = std::make_pair(
      resourceName,
      ResourceInfo{std::make_shared<RawLoaderContainer<ResourceType>>(loader),
                   std::weak_ptr<ResourceType>(), typeid(ResourceType)});
  assert(resourceInfo.insert(std::move(info_pair)).second);
}
