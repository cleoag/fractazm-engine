#ifndef MB3D_IO_BUFFER_EXPORT_H
#define MB3D_IO_BUFFER_EXPORT_H

#include <cstdint>
#include <string>
#include "core/types.h"

namespace mb3d {

// Write Z-buffer as 8-bit grayscale BMP matching original MB3D format (MakeZbufBMP)
// Formula: Zpos > 32767 -> 0 (background), else Zpos / 137 + 16
bool WriteZBufferBMP_8bit(const std::string& path, const TsiLight5* siLight, int width, int height);
bool WriteZBufferPNG_8bit(const std::string& path, const TsiLight5* siLight, int width, int height);
bool WriteZBufferBMP_8bit_Final(const std::string& path, const TsiLight5* siLight, int width, int height,
                                int imageScale, int tilingCrop);
bool WriteZBufferPNG_8bit_Final(const std::string& path, const TsiLight5* siLight, int width, int height,
                                int imageScale, int tilingCrop);

// Write Z-buffer as 16-bit grayscale PNG (raw Zpos values, legacy)
bool WriteZBuffer(const std::string& path, const uint16_t* zbuf, int width, int height);

// Write Z-buffer as 16-bit grayscale PNG using real Z decode
bool WriteZBufferPNG_16bit(const std::string& path, const TsiLight5* siLight, int width, int height,
                           double ZcMul, double Zcorr);
bool WriteZBufferPNG_16bit_Final(const std::string& path, const TsiLight5* siLight, int width, int height,
                                 int imageScale, int tilingCrop, double ZcMul, double Zcorr);

// Write normal map: int16 normals -> RGB uint8 PNG
bool WriteNormalMap(const std::string& path, const int16_t* normals, int width, int height);
bool WriteNormalMapFinal(const std::string& path, const int16_t* normals, int width, int height,
                         int imageScale, int tilingCrop);

// Write normal map computed from Z-buffer via finite differences (NormalsOnZbuf)
// Decodes real Z from siLight5, computes dZ/dx and dZ/dy gradients,
// normalizes to get surface normal, encodes as RGB PNG.
bool WriteNormalsFromZbuf(const std::string& path, const TsiLight5* siLight,
                          int width, int height, double ZcMul, double Zcorr);

} // namespace mb3d

#endif // MB3D_IO_BUFFER_EXPORT_H
