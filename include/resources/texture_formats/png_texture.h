
#pragma once

#include <absl/status/statusor.h>

#include <memory>

#include "resources/texture.h"

namespace PngTexture {

struct Details {
  std::string file;
};

absl::StatusOr<std::shared_ptr<Texture>> Load(const Details& details);

}  // namespace PngTexture
