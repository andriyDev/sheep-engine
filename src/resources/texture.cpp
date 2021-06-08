
#include "resources/texture.h"

#include <glog/logging.h>

Texture::Texture(Texture::PixelType type_, uint32_t width_, uint32_t height_)
    : type(type_), width(width_), height(height_) {
  switch (type) {
    case PixelType::RGBA:
      data_rgba = new pixel_rgba[width * height];
      break;
    case PixelType::RGB:
      data_rgb = new pixel_rgb[width * height];
      break;
    case PixelType::Grey:
      data_grey = new pixel_grey[width * height];
      break;
    default:
      CHECK(false);
  }
}

Texture::~Texture() {
  switch (type) {
    case PixelType::RGBA:
      delete[] data_rgba;
      break;
    case PixelType::RGB:
      delete[] data_rgb;
      break;
    case PixelType::Grey:
      delete[] data_grey;
      break;
    default:
      CHECK(false);
  }
}

Texture::pixel_rgba* Texture::GetDataAsRGBA() const { return data_rgba; }

Texture::pixel_rgb* Texture::GetDataAsRGB() const { return data_rgb; }

Texture::pixel_grey* Texture::GetDataAsGrey() const { return data_grey; }

uint32_t Texture::GetWidth() const { return width; }

uint32_t Texture::GetHeight() const { return height; }

Texture::PixelType Texture::GetType() const { return type; }
