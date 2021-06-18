
#include "resources/texture_formats/png_texture.h"

#include <glog/logging.h>
#include <png.h>

#include <fstream>

#include "utility/hton.h"
#include "utility/scope_cleanup.h"

void ReadPngFile(png_structp png_ptr, png_bytep out_bytes,
                 png_size_t bytes_to_read) {
  png_voidp io_ptr = png_get_io_ptr(png_ptr);
  CHECK(io_ptr);

  std::ifstream& file = *static_cast<std::ifstream*>(io_ptr);
  file.read((char*)out_bytes, bytes_to_read);
  CHECK_EQ(bytes_to_read, file.gcount());
}

absl::StatusOr<std::shared_ptr<Texture>> PngTexture::Load(
    const Details& details) {
  std::ifstream file(details.file, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    return absl::NotFoundError(
        STATUS_MESSAGE("Failed to open file \"" << details.file << "\""));
  }

  png_byte header[8];
  file.read((char*)header, 8);
  if (file.bad()) {
    return absl::InternalError(
        STATUS_MESSAGE("Failed to read file \"" << details.file << "\". "));
  }
  if (png_sig_cmp(header, 0, 8)) {
    return absl::InvalidArgumentError(
        STATUS_MESSAGE("File " << details.file << " is not a valid PNG file."));
  }

  png_structp png_ptr =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    return absl::InternalError("Failed to initialize libpng");
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    return absl::InternalError("Failed to create info struct for libpng");
  }

  const ScopeCleanup cleanup_png([&png_ptr, &info_ptr]() {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  });

  // TODO: Figure out why the default is entirely flipped.
  png_set_bgr(png_ptr);
  png_set_swap_alpha(png_ptr);

  if (setjmp(png_jmpbuf(png_ptr))) {
    return absl::InternalError("libpng failure.");
  }

  png_set_read_fn(png_ptr, &file, ReadPngFile);
  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);

  png_uint_32 width, height;
  int bit_depth, colour_type;
  if (!png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
                    &colour_type, NULL, NULL, NULL)) {
    return absl::InternalError(STATUS_MESSAGE(
        "Failed to read data info of PNG file \"" << details.file << "\""));
  }

  if (bit_depth != 8 && bit_depth != 16) {
    return absl::FailedPreconditionError(STATUS_MESSAGE(
        "Invalid bit depth: " << bit_depth << ". Must be 8 or 16."));
  }

  std::shared_ptr<Texture> texture;
  png_byte* row_data = new png_byte[png_get_rowbytes(png_ptr, info_ptr)];
  const ScopeCleanup cleanup_row_data([row_data]() { delete[] row_data; });

  switch (colour_type) {
    case PNG_COLOR_TYPE_GRAY:
      texture = std::shared_ptr<Texture>(
          new Texture(Texture::PixelType::Grey, bit_depth, width, height));
      if (bit_depth == 8) {
        Texture::pixel_grey_8* data = texture->GetDataAsGrey8();
        for (uint32_t row = 0; row < height; ++row) {
          const uint32_t row_offset = row * width;
          png_read_row(png_ptr, row_data, NULL);

          for (uint32_t col = 0; col < width; ++col) {
            data[col + row_offset] = row_data[col];
          }
        }
      } else {
        Texture::pixel_grey_16* data = texture->GetDataAsGrey16();
        for (uint32_t row = 0; row < height; ++row) {
          const uint32_t row_offset = row * width;
          png_read_row(png_ptr, row_data, NULL);

          uint32_t byte_index = 0;
          for (uint32_t col = 0; col < width; ++col) {
            data[col + row_offset] = ((uint16_t)row_data[byte_index++] << 8) |
                                     (uint16_t)row_data[byte_index++];
          }
        }
      }
      return texture;
    case PNG_COLOR_TYPE_RGB:
      texture = std::shared_ptr<Texture>(
          new Texture(Texture::PixelType::RGB, bit_depth, width, height));
      if (bit_depth == 8) {
        Texture::pixel_rgb_8* data = texture->GetDataAsRGB8();
        for (uint32_t row = 0; row < height; ++row) {
          const uint32_t row_offset = row * width;
          png_read_row(png_ptr, row_data, NULL);

          uint32_t byte_index = 0;
          for (uint32_t col = 0; col < width; ++col) {
            data[col + row_offset] = Texture::pixel_rgb_8(
                row_data[byte_index++], row_data[byte_index++],
                row_data[byte_index++]);
          }
        }
      } else {
        Texture::pixel_rgb_16* data = texture->GetDataAsRGB16();
        for (uint32_t row = 0; row < height; ++row) {
          const uint32_t row_offset = row * width;
          png_read_row(png_ptr, row_data, NULL);

          uint32_t byte_index = 0;
          for (uint32_t col = 0; col < width; ++col) {
            data[col + row_offset] =
                Texture::pixel_rgb_16(((uint16_t)row_data[byte_index++] << 8) |
                                          (uint16_t)row_data[byte_index++],
                                      ((uint16_t)row_data[byte_index++] << 8) |
                                          (uint16_t)row_data[byte_index++],
                                      ((uint16_t)row_data[byte_index++] << 8) |
                                          (uint16_t)row_data[byte_index++]);
          }
        }
      }
      return texture;
    case PNG_COLOR_TYPE_RGBA:
      texture = std::shared_ptr<Texture>(
          new Texture(Texture::PixelType::RGBA, bit_depth, width, height));
      if (bit_depth == 8) {
        Texture::pixel_rgba_8* data = texture->GetDataAsRGBA8();
        for (uint32_t row = 0; row < height; ++row) {
          const uint32_t row_offset = row * width;
          png_read_row(png_ptr, row_data, NULL);

          uint32_t byte_index = 0;
          for (uint32_t col = 0; col < width; ++col) {
            data[col + row_offset] = Texture::pixel_rgba_8(
                row_data[byte_index++], row_data[byte_index++],
                row_data[byte_index++], row_data[byte_index++]);
          }
        }
      } else {
        Texture::pixel_rgba_16* data = texture->GetDataAsRGBA16();
        for (uint32_t row = 0; row < height; ++row) {
          const uint32_t row_offset = row * width;
          png_read_row(png_ptr, row_data, NULL);

          uint32_t byte_index = 0;
          for (uint32_t col = 0; col < width; ++col) {
            data[col + row_offset] =
                Texture::pixel_rgba_16(((uint16_t)row_data[byte_index++] << 8) |
                                           (uint16_t)row_data[byte_index++],
                                       ((uint16_t)row_data[byte_index++] << 8) |
                                           (uint16_t)row_data[byte_index++],
                                       ((uint16_t)row_data[byte_index++] << 8) |
                                           (uint16_t)row_data[byte_index++],
                                       ((uint16_t)row_data[byte_index++] << 8) |
                                           (uint16_t)row_data[byte_index++]);
          }
        }
      }
      return texture;
    default:
      return absl::FailedPreconditionError(
          STATUS_MESSAGE("Unable to process PNG colour type " << colour_type));
  }
}
