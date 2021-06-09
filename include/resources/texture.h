
#pragma once

#include <GL/glew.h>
#include <absl/status/statusor.h>

#include <glm/detail/type_vec4.hpp>
#include <glm/glm.hpp>
#include <memory>

#include "utility/resource_handle.h"

class Texture {
 public:
  enum class PixelType { RGBA, RGB, Grey };

  Texture(PixelType type_, uint32_t width_, uint32_t height_);
  ~Texture();

  using pixel_rgba = glm::tvec4<unsigned char>;
  using pixel_rgb = glm::tvec3<unsigned char>;
  using pixel_grey = unsigned char;

  pixel_rgba* GetDataAsRGBA() const;
  pixel_rgb* GetDataAsRGB() const;
  pixel_grey* GetDataAsGrey() const;

  uint32_t GetWidth() const;
  uint32_t GetHeight() const;
  PixelType GetType() const;

 private:
  uint32_t width;
  uint32_t height;
  PixelType type;

  union {
    pixel_rgba* data_rgba;
    pixel_rgb* data_rgb;
    pixel_grey* data_grey;
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
