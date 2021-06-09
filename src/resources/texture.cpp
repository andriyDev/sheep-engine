
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

GLuint ToGL(RenderableTexture::WrapMode mode) {
  switch (mode) {
    case RenderableTexture::WrapMode::Repeat:
      return GL_REPEAT;
    case RenderableTexture::WrapMode::Clamp:
      return GL_CLAMP_TO_EDGE;
    default:
      CHECK(false);
      return 0;
  }
}

GLuint ToGL(RenderableTexture::FilterMode mode, bool use_mipmaps) {
  switch (mode) {
    case RenderableTexture::FilterMode::Linear:
      return use_mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
    case RenderableTexture::FilterMode::Nearest:
      return use_mipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
    default:
      CHECK(false);
      return 0;
  }
}

absl::StatusOr<std::shared_ptr<RenderableTexture>> RenderableTexture::Load(
    const Details& details) {
  std::shared_ptr<RenderableTexture> texture(new RenderableTexture());
  ASSIGN_OR_RETURN(const std::shared_ptr<Texture> source_data,
                   details.texture.Get());

  texture->width = source_data->GetWidth();
  texture->height = source_data->GetHeight();
  glGenTextures(1, &texture->id);

  glBindTexture(GL_TEXTURE_2D, texture->id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ToGL(details.x_wrap));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ToGL(details.y_wrap));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  ToGL(details.min_filter, details.use_mipmaps));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  ToGL(details.mag_filter, details.use_mipmaps));

  switch (source_data->GetType()) {
    case Texture::PixelType::RGBA:
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height,
                   0, GL_RGBA, GL_UNSIGNED_BYTE, source_data->GetDataAsRGBA());
      break;
    case Texture::PixelType::RGB:
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->width, texture->height, 0,
                   GL_RGB, GL_UNSIGNED_BYTE, source_data->GetDataAsRGB());
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      break;
    case Texture::PixelType::Grey:
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texture->width, texture->height, 0,
                   GL_RED, GL_UNSIGNED_BYTE, source_data->GetDataAsGrey());
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      break;
    default:
      CHECK(false);
  }

  if (details.use_mipmaps) {
    glGenerateMipmap(GL_TEXTURE_2D);
  }

  return texture;
}

RenderableTexture::~RenderableTexture() { glDeleteTextures(1, &id); }

void RenderableTexture::Use(unsigned int texture_unit) {
  glActiveTexture(GL_TEXTURE0 + texture_unit);
  glBindTexture(GL_TEXTURE_2D, id);
}

uint32_t RenderableTexture::GetWidth() const { return width; }
uint32_t RenderableTexture::GetHeight() const { return height; }
