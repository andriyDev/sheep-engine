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
    // If in manual release mode, store the resource to hold it.
    if (holdResources) {
      heldResources.push_back(ptr);
    }
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

  // Sets loader to hold any resources loaded afterwards. These resources are
  // guaranteed not to unload until `UseManualRelease(false)` or `ManualRelease`
  // is called.
  void UseManualRelease(bool use) {
    if (use == holdResources) {
      return;
    }
    if (!use) {
      ManualRelease();
    }
    holdResources = use;
  }

  // Releases any resources that are currently being held. If these resources
  // have no other references, the resource will be unloaded. If
  // `UseManualRelease(true)` has not been called, does nothing.
  void ManualRelease() {
    if (!holdResources) {
      return;
    }
    heldResources.clear();
  }

  static ResourceLoader& Get() { return instance; }

 private:
  static ResourceLoader instance;

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
  std::vector<std::shared_ptr<void>> heldResources;
  bool holdResources;
};
