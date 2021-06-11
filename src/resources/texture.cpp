
#include "resources/texture.h"

#include <glog/logging.h>

Texture::Texture(Texture::PixelType type_, unsigned int bit_depth_,
                 uint32_t width_, uint32_t height_)
    : pixel_type(type_), bit_depth(bit_depth_), width(width_), height(height_) {
  switch (bit_depth) {
    case 8:
      switch (pixel_type) {
        case PixelType::RGBA:
          data_rgba_8 = new pixel_rgba_8[width * height];
          break;
        case PixelType::RGB:
          data_rgb_8 = new pixel_rgb_8[width * height];
          break;
        case PixelType::Grey:
          data_grey_8 = new pixel_grey_8[width * height];
          break;
        default:
          CHECK(false);
      }
      break;
    case 16:
      switch (pixel_type) {
        case PixelType::RGBA:
          data_rgba_16 = new pixel_rgba_16[width * height];
          break;
        case PixelType::RGB:
          data_rgb_16 = new pixel_rgb_16[width * height];
          break;
        case PixelType::Grey:
          data_grey_16 = new pixel_grey_16[width * height];
          break;
        default:
          CHECK(false);
      }
      break;
    case 32:
      switch (pixel_type) {
        case PixelType::RGBA:
          data_rgba_32 = new pixel_rgba_32[width * height];
          break;
        case PixelType::RGB:
          data_rgb_32 = new pixel_rgb_32[width * height];
          break;
        case PixelType::Grey:
          data_grey_32 = new pixel_grey_32[width * height];
          break;
        default:
          CHECK(false);
      }
      break;
    default:
      CHECK(false) << "Invalid bit depth: " << bit_depth;
  }
}

Texture::~Texture() {
  switch (bit_depth | (uint8_t)pixel_type) {
    case 8 | (uint8_t)PixelType::RGBA:
      delete[] data_rgba_8;
      break;
    case 8 | (uint8_t)PixelType::RGB:
      delete[] data_rgb_8;
      break;
    case 8 | (uint8_t)PixelType::Grey:
      delete[] data_grey_8;
      break;
    case 16 | (uint8_t)PixelType::RGBA:
      delete[] data_rgba_16;
      break;
    case 16 | (uint8_t)PixelType::RGB:
      delete[] data_rgb_16;
      break;
    case 16 | (uint8_t)PixelType::Grey:
      delete[] data_grey_16;
      break;
    case 32 | (uint8_t)PixelType::RGBA:
      delete[] data_rgba_32;
      break;
    case 32 | (uint8_t)PixelType::RGB:
      delete[] data_rgb_32;
      break;
    case 32 | (uint8_t)PixelType::Grey:
      delete[] data_grey_32;
      break;
    default:
      CHECK(false);
  }
}

Texture::pixel_rgba_8* Texture::GetDataAsRGBA8() const { return data_rgba_8; }
Texture::pixel_rgb_8* Texture::GetDataAsRGB8() const { return data_rgb_8; }
Texture::pixel_grey_8* Texture::GetDataAsGrey8() const { return data_grey_8; }
Texture::pixel_rgba_16* Texture::GetDataAsRGBA16() const {
  return data_rgba_16;
}
Texture::pixel_rgb_16* Texture::GetDataAsRGB16() const { return data_rgb_16; }
Texture::pixel_grey_16* Texture::GetDataAsGrey16() const {
  return data_grey_16;
}
Texture::pixel_rgba_32* Texture::GetDataAsRGBA32() const {
  return data_rgba_32;
}
Texture::pixel_rgb_32* Texture::GetDataAsRGB32() const { return data_rgb_32; }
Texture::pixel_grey_32* Texture::GetDataAsGrey32() const {
  return data_grey_32;
}

uint32_t Texture::GetWidth() const { return width; }

uint32_t Texture::GetHeight() const { return height; }

Texture::PixelType Texture::GetPixelType() const { return pixel_type; }

unsigned int Texture::GetBitDepth() const { return bit_depth; }

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

  switch (source_data->GetBitDepth() | (uint8_t)source_data->GetPixelType()) {
    case 8 | (uint8_t)Texture::PixelType::RGBA:
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->width, texture->height,
                   0, GL_RGBA, GL_UNSIGNED_BYTE, source_data->GetDataAsRGBA8());
      break;
    case 8 | (uint8_t)Texture::PixelType::RGB:
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture->width, texture->height,
                   0, GL_RGB, GL_UNSIGNED_BYTE, source_data->GetDataAsRGB8());
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      break;
    case 8 | (uint8_t)Texture::PixelType::Grey:
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, texture->width, texture->height, 0,
                   GL_RED, GL_UNSIGNED_BYTE, source_data->GetDataAsGrey8());
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      break;
    case 16 | (uint8_t)Texture::PixelType::RGBA:
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, texture->width, texture->height,
                   0, GL_RGBA, GL_UNSIGNED_SHORT,
                   source_data->GetDataAsRGBA16());
      break;
    case 16 | (uint8_t)Texture::PixelType::RGB:
      glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, texture->width, texture->height,
                   0, GL_RGB, GL_UNSIGNED_SHORT, source_data->GetDataAsRGB16());
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      break;
    case 16 | (uint8_t)Texture::PixelType::Grey:
      glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, texture->width, texture->height, 0,
                   GL_RED, GL_UNSIGNED_SHORT, source_data->GetDataAsGrey16());
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      break;
    case 32 | (uint8_t)Texture::PixelType::RGBA:
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, texture->width,
                   texture->height, 0, GL_RGBA, GL_UNSIGNED_INT,
                   source_data->GetDataAsRGBA32());
      break;
    case 32 | (uint8_t)Texture::PixelType::RGB:
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, texture->width, texture->height,
                   0, GL_RGB, GL_UNSIGNED_INT, source_data->GetDataAsRGB32());
      break;
    case 32 | (uint8_t)Texture::PixelType::Grey:
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, texture->width, texture->height,
                   0, GL_RED, GL_UNSIGNED_INT, source_data->GetDataAsGrey32());
      break;
    default:
      CHECK(false);
  }
  CHECK_EQ(glGetError(), 0);

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
