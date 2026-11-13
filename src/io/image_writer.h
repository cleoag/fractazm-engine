#ifndef MB3D_IO_IMAGE_WRITER_H
#define MB3D_IO_IMAGE_WRITER_H

#include <cstdint>
#include <string>

namespace mb3d {

// Write RGBA or RGB image as PNG using stb_image_write
bool WritePNG(const std::string& path, const uint8_t* pixels, int width, int height, int channels);

// Write RGB image as BMP (24-bit, bottom-up, padded rows)
bool WriteBMP(const std::string& path, const uint8_t* pixels, int width, int height);

} // namespace mb3d

#endif // MB3D_IO_IMAGE_WRITER_H
