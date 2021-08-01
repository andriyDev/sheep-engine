
#include <string>

#include "resources/skeleton.h"
#include "resources/skin.h"
#include "resources/transit/transit.h"
#include "utility/resource_handle.h"

namespace transit {

struct TransitSkinDetails : TransitDetails {
  ResourceHandle<Skeleton> skeleton;
};

absl::StatusOr<std::shared_ptr<Skin>> LoadSkin(
    const TransitSkinDetails& details);

}  // namespace transit
