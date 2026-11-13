#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#include "io/image_writer.h"
#include <cstdio>
#include <cstring>
#include <vector>

namespace mb3d {

bool WritePNG(const std::string& path, const uint8_t* pixels, int width, int height, int channels) {
    if (!pixels || width <= 0 || height <= 0 || (channels != 3 && channels != 4)) {
        return false;
    }
    int result = stbi_write_png(path.c_str(), width, height, channels, pixels, width * channels);
    return result != 0;
}

bool WriteBMP(const std::string& path, const uint8_t* pixels, int width, int height) {
    if (!pixels || width <= 0 || height <= 0) {
        return false;
    }

    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) return false;

    // Row stride: 3 bytes per pixel, padded to 4-byte boundary
    int rowStride = (width * 3 + 3) & ~3;
    int imageSize = rowStride * height;
    int fileSize = 14 + 40 + imageSize;

    // BMP File Header (14 bytes)
    uint8_t fileHeader[14];
    std::memset(fileHeader, 0, 14);
    fileHeader[0] = 'B';
    fileHeader[1] = 'M';
    fileHeader[2] = fileSize & 0xFF;
    fileHeader[3] = (fileSize >> 8) & 0xFF;
    fileHeader[4] = (fileSize >> 16) & 0xFF;
    fileHeader[5] = (fileSize >> 24) & 0xFF;
    // Reserved (4 bytes = 0)
    fileHeader[10] = 54; // Offset to pixel data

    // DIB Header (BITMAPINFOHEADER, 40 bytes)
    uint8_t dibHeader[40];
    std::memset(dibHeader, 0, 40);
    dibHeader[0] = 40; // Header size
    dibHeader[4] = width & 0xFF;
    dibHeader[5] = (width >> 8) & 0xFF;
    dibHeader[6] = (width >> 16) & 0xFF;
    dibHeader[7] = (width >> 24) & 0xFF;
    dibHeader[8] = height & 0xFF;
    dibHeader[9] = (height >> 8) & 0xFF;
    dibHeader[10] = (height >> 16) & 0xFF;
    dibHeader[11] = (height >> 24) & 0xFF;
    dibHeader[12] = 1; // Planes
    dibHeader[14] = 24; // Bits per pixel
    // Compression = 0 (BI_RGB)
    dibHeader[20] = imageSize & 0xFF;
    dibHeader[21] = (imageSize >> 8) & 0xFF;
    dibHeader[22] = (imageSize >> 16) & 0xFF;
    dibHeader[23] = (imageSize >> 24) & 0xFF;
    // Resolution: 2835 pixels/meter (72 DPI)
    dibHeader[24] = 0x13; dibHeader[25] = 0x0B;
    dibHeader[28] = 0x13; dibHeader[29] = 0x0B;

    std::fwrite(fileHeader, 1, 14, f);
    std::fwrite(dibHeader, 1, 40, f);

    // Pixel data (bottom-up, BGR)
    std::vector<uint8_t> rowBuf(rowStride, 0);
    for (int y = height - 1; y >= 0; y--) {
        const uint8_t* src = pixels + y * width * 3;
        for (int x = 0; x < width; x++) {
            rowBuf[x * 3 + 0] = src[x * 3 + 2]; // B
            rowBuf[x * 3 + 1] = src[x * 3 + 1]; // G
            rowBuf[x * 3 + 2] = src[x * 3 + 0]; // R
        }
        // Padding bytes already zeroed
        std::fwrite(rowBuf.data(), 1, rowStride, f);
    }

    std::fclose(f);
    return true;
}

} // namespace mb3d
