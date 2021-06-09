
#pragma once

#include <GL/glew.h>
#include <absl/status/statusor.h>

#include <glm/detail/type_vec4.hpp>
#include <glm/glm.hpp>
#include <memory>

#include "utility/resource_handle.h"

class Texture {
 public:
  enum class PixelType : uint8_t { RGBA, RGB, Grey };

  Texture(PixelType type_, unsigned int bit_depth_, uint32_t width_,
          uint32_t height_);
  ~Texture();

  using pixel_rgba_8 = glm::tvec4<uint8_t>;
  using pixel_rgb_8 = glm::tvec3<uint8_t>;
  using pixel_grey_8 = uint8_t;

  using pixel_rgba_16 = glm::tvec4<uint16_t>;
  using pixel_rgb_16 = glm::tvec3<uint16_t>;
  using pixel_grey_16 = uint16_t;

  using pixel_rgba_32 = glm::tvec4<uint32_t>;
  using pixel_rgb_32 = glm::tvec3<uint32_t>;
  using pixel_grey_32 = uint32_t;

  pixel_rgba_8* GetDataAsRGBA8() const;
  pixel_rgb_8* GetDataAsRGB8() const;
  pixel_grey_8* GetDataAsGrey8() const;

  pixel_rgba_16* GetDataAsRGBA16() const;
  pixel_rgb_16* GetDataAsRGB16() const;
  pixel_grey_16* GetDataAsGrey16() const;

  pixel_rgba_32* GetDataAsRGBA32() const;
  pixel_rgb_32* GetDataAsRGB32() const;
  pixel_grey_32* GetDataAsGrey32() const;

  uint32_t GetWidth() const;
  uint32_t GetHeight() const;
  PixelType GetPixelType() const;
  unsigned int GetBitDepth() const;

 private:
  uint32_t width;
  uint32_t height;
  PixelType pixel_type;
  unsigned int bit_depth;

  union {
    pixel_rgba_8* data_rgba_8;
    pixel_rgb_8* data_rgb_8;
    pixel_grey_8* data_grey_8;

    pixel_rgba_16* data_rgba_16;
    pixel_rgb_16* data_rgb_16;
    pixel_grey_16* data_grey_16;

    pixel_rgba_32* data_rgba_32;
    pixel_rgb_32* data_rgb_32;
    pixel_grey_32* data_grey_32;
  };
};

class RenderableTexture {
 public:
  enum class WrapMode { Repeat, Clamp };
  enum class FilterMode { Linear, Nearest };

  struct Details {
    ResourceHandle<Texture> texture;

    WrapMode x_wrap;
    WrapMode y_wrap;

    FilterMode min_filter;
    FilterMode mag_filter;

    bool use_mipmaps;
  };
  using detail_type = Details;

  static absl::StatusOr<std::shared_ptr<RenderableTexture>> Load(
      const Details& details);

  ~RenderableTexture();

  void Use(unsigned int texture_unit);

  uint32_t GetWidth() const;
  uint32_t GetHeight() const;

 private:
  uint32_t width;
  uint32_t height;

  GLuint id = 0;
};
