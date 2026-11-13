#include "io/buffer_export.h"
#include "core/image_process.h"
#include "stb/stb_image_write.h"
#include <zlib.h>
#include <vector>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <limits>

namespace mb3d {

static uint8_t ZBufferGray8FromPixel(const TsiLight5& px) {
    if (px.Zpos > 32767) return 0;
    return static_cast<uint8_t>(px.Zpos / 137 + 16);
}

static bool BuildFinalZBufferGray8(const TsiLight5* siLight, int width, int height,
                                   int imageScale, int tilingCrop,
                                   std::vector<uint8_t>& out, int& outWidth, int& outHeight) {
    out.clear();
    outWidth = 0;
    outHeight = 0;
    if (!siLight || width <= 0 || height <= 0) return false;

    const int scale = std::max(1, std::min(10, imageScale));
    outWidth = width / scale;
    outHeight = height / scale;
    if (outWidth <= 0 || outHeight <= 0) return false;
    out.assign(static_cast<size_t>(outWidth) * outHeight, 0);

    if (scale == 1) {
        for (int i = 0; i < width * height; ++i) out[static_cast<size_t>(i)] = ZBufferGray8FromPixel(siLight[i]);
    } else {
        const int add = 16 * scale * scale;
        const int denom = scale * scale * 137;
        for (int y = 0; y < outHeight; ++y) {
            for (int x = 0; x < outWidth; ++x) {
                int sum = 0;
                for (int yy = 0; yy < scale; ++yy) {
                    int srcY = y * scale + yy;
                    const TsiLight5* row = siLight + static_cast<size_t>(srcY) * width;
                    for (int xx = 0; xx < scale; ++xx) {
                        const TsiLight5& px = row[x * scale + xx];
                        if (px.Zpos < 32768) sum += static_cast<int>(px.Zpos) + add;
                    }
                }
                out[static_cast<size_t>(y) * outWidth + x] =
                    static_cast<uint8_t>(std::max(0, std::min(255, sum / denom)));
            }
        }
    }

    int scaledCrop = tilingCrop;
    if (scale >= 2) scaledCrop /= scale;
    if (scaledCrop > 0 && outWidth > 2 * scaledCrop && outHeight > 2 * scaledCrop) {
        const int croppedW = outWidth - 2 * scaledCrop;
        const int croppedH = outHeight - 2 * scaledCrop;
        std::vector<uint8_t> cropped(static_cast<size_t>(croppedW) * croppedH);
        for (int row = 0; row < croppedH; ++row) {
            const uint8_t* src = out.data() + static_cast<size_t>(row + scaledCrop) * outWidth + scaledCrop;
            uint8_t* dst = cropped.data() + static_cast<size_t>(row) * croppedW;
            std::memcpy(dst, src, static_cast<size_t>(croppedW));
        }
        out.swap(cropped);
        outWidth = croppedW;
        outHeight = croppedH;
    }

    return true;
}

static void AppendU32BE(std::vector<uint8_t>& out, uint32_t value) {
    out.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(value & 0xFF));
}

static bool WritePNGGray(const std::string& path, const uint8_t* gray8, const uint16_t* gray16,
                         int width, int height, int bitDepth) {
    if (width <= 0 || height <= 0) return false;
    if (bitDepth != 8 && bitDepth != 16) return false;
    if ((bitDepth == 8 && !gray8) || (bitDepth == 16 && !gray16)) return false;

    const int bytesPerPixel = bitDepth / 8;
    std::vector<uint8_t> raw;
    raw.resize(static_cast<size_t>(height) * (1 + static_cast<size_t>(width) * bytesPerPixel));

    for (int y = 0; y < height; y++) {
        size_t rowBase = static_cast<size_t>(y) * (1 + static_cast<size_t>(width) * bytesPerPixel);
        raw[rowBase] = 0; // filter type 0
        if (bitDepth == 8) {
            std::memcpy(raw.data() + rowBase + 1, gray8 + static_cast<size_t>(y) * width, static_cast<size_t>(width));
        } else {
            for (int x = 0; x < width; x++) {
                uint16_t v = gray16[static_cast<size_t>(y) * width + x];
                raw[rowBase + 1 + static_cast<size_t>(x) * 2 + 0] = static_cast<uint8_t>((v >> 8) & 0xFF);
                raw[rowBase + 1 + static_cast<size_t>(x) * 2 + 1] = static_cast<uint8_t>(v & 0xFF);
            }
        }
    }

    uLongf compBound = compressBound(static_cast<uLong>(raw.size()));
    std::vector<uint8_t> compressed(compBound);
    int zres = compress2(compressed.data(), &compBound, raw.data(), static_cast<uLong>(raw.size()), Z_BEST_COMPRESSION);
    if (zres != Z_OK) return false;
    compressed.resize(compBound);

    std::vector<uint8_t> png;
    png.reserve(8 + 25 + 12 + compressed.size() + 12);

    static const uint8_t sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    png.insert(png.end(), std::begin(sig), std::end(sig));

    std::vector<uint8_t> ihdr;
    ihdr.reserve(13);
    AppendU32BE(ihdr, static_cast<uint32_t>(width));
    AppendU32BE(ihdr, static_cast<uint32_t>(height));
    ihdr.push_back(static_cast<uint8_t>(bitDepth));
    ihdr.push_back(0); // grayscale
    ihdr.push_back(0); // compression
    ihdr.push_back(0); // filter
    ihdr.push_back(0); // interlace

    auto appendChunk = [&](const char type[4], const std::vector<uint8_t>& data) {
        AppendU32BE(png, static_cast<uint32_t>(data.size()));
        size_t typePos = png.size();
        png.insert(png.end(), type, type + 4);
        png.insert(png.end(), data.begin(), data.end());
        uLong crc = crc32(0L, Z_NULL, 0);
        crc = crc32(crc, reinterpret_cast<const Bytef*>(png.data() + typePos), static_cast<uInt>(4 + data.size()));
        AppendU32BE(png, static_cast<uint32_t>(crc));
    };

    appendChunk("IHDR", ihdr);
    appendChunk("IDAT", compressed);
    appendChunk("IEND", {});

    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) return false;
    bool ok = std::fwrite(png.data(), 1, png.size(), f) == png.size();
    std::fclose(f);
    return ok;
}

bool WriteZBufferBMP_8bit(const std::string& path, const TsiLight5* siLight, int width, int height) {
    if (!siLight || width <= 0 || height <= 0) return false;

    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) return false;

    // 8-bit BMP: file header (14) + DIB header (40) + palette (256*4=1024) + pixel data
    int rowStride = (width + 3) & ~3; // 1 byte per pixel, padded to 4-byte boundary
    int imageSize = rowStride * height;
    int paletteSize = 256 * 4;
    int dataOffset = 14 + 40 + paletteSize;
    int fileSize = dataOffset + imageSize;

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
    fileHeader[10] = dataOffset & 0xFF;
    fileHeader[11] = (dataOffset >> 8) & 0xFF;
    fileHeader[12] = (dataOffset >> 16) & 0xFF;
    fileHeader[13] = (dataOffset >> 24) & 0xFF;

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
    dibHeader[12] = 1;  // Planes
    dibHeader[14] = 8;  // Bits per pixel
    // Compression = 0 (BI_RGB)
    dibHeader[20] = imageSize & 0xFF;
    dibHeader[21] = (imageSize >> 8) & 0xFF;
    dibHeader[22] = (imageSize >> 16) & 0xFF;
    dibHeader[23] = (imageSize >> 24) & 0xFF;
    // Resolution: 2835 pixels/meter (72 DPI)
    dibHeader[24] = 0x13; dibHeader[25] = 0x0B;
    dibHeader[28] = 0x13; dibHeader[29] = 0x0B;
    // Colors used = 256
    dibHeader[32] = 0; dibHeader[33] = 1; // 256
    // Important colors = 256
    dibHeader[36] = 0; dibHeader[37] = 1; // 256

    std::fwrite(fileHeader, 1, 14, f);
    std::fwrite(dibHeader, 1, 40, f);

    // 256-entry grayscale palette: index i -> (i, i, i, 0)
    uint8_t palette[1024];
    for (int i = 0; i < 256; i++) {
        palette[i * 4 + 0] = static_cast<uint8_t>(i); // B
        palette[i * 4 + 1] = static_cast<uint8_t>(i); // G
        palette[i * 4 + 2] = static_cast<uint8_t>(i); // R
        palette[i * 4 + 3] = 0;                        // Reserved
    }
    std::fwrite(palette, 1, 1024, f);

    // Pixel data: bottom-up row order, matching original MB3D MakeZbufBMP
    std::vector<uint8_t> rowBuf(rowStride, 0);
    for (int y = height - 1; y >= 0; y--) {
        std::memset(rowBuf.data(), 0, rowStride);
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            rowBuf[x] = ZBufferGray8FromPixel(siLight[idx]);
        }
        std::fwrite(rowBuf.data(), 1, rowStride, f);
    }

    std::fclose(f);
    return true;
}

bool WriteZBufferPNG_8bit(const std::string& path, const TsiLight5* siLight, int width, int height) {
    if (!siLight || width <= 0 || height <= 0) return false;
    std::vector<uint8_t> gray(static_cast<size_t>(width) * height);
    for (int i = 0; i < width * height; i++) {
        gray[i] = ZBufferGray8FromPixel(siLight[i]);
    }
    return WritePNGGray(path, gray.data(), nullptr, width, height, 8);
}

bool WriteZBufferBMP_8bit_Final(const std::string& path, const TsiLight5* siLight, int width, int height,
                                int imageScale, int tilingCrop) {
    std::vector<uint8_t> gray;
    int finalWidth = 0;
    int finalHeight = 0;
    if (!BuildFinalZBufferGray8(siLight, width, height, imageScale, tilingCrop, gray, finalWidth, finalHeight)) {
        return false;
    }

    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) return false;

    int rowStride = (finalWidth + 3) & ~3;
    int imageSize = rowStride * finalHeight;
    int paletteSize = 256 * 4;
    int dataOffset = 14 + 40 + paletteSize;
    int fileSize = dataOffset + imageSize;

    uint8_t fileHeader[14];
    std::memset(fileHeader, 0, 14);
    fileHeader[0] = 'B';
    fileHeader[1] = 'M';
    fileHeader[2] = fileSize & 0xFF;
    fileHeader[3] = (fileSize >> 8) & 0xFF;
    fileHeader[4] = (fileSize >> 16) & 0xFF;
    fileHeader[5] = (fileSize >> 24) & 0xFF;
    fileHeader[10] = dataOffset & 0xFF;
    fileHeader[11] = (dataOffset >> 8) & 0xFF;
    fileHeader[12] = (dataOffset >> 16) & 0xFF;
    fileHeader[13] = (dataOffset >> 24) & 0xFF;

    uint8_t dibHeader[40];
    std::memset(dibHeader, 0, 40);
    dibHeader[0] = 40;
    dibHeader[4] = finalWidth & 0xFF;
    dibHeader[5] = (finalWidth >> 8) & 0xFF;
    dibHeader[6] = (finalWidth >> 16) & 0xFF;
    dibHeader[7] = (finalWidth >> 24) & 0xFF;
    dibHeader[8] = finalHeight & 0xFF;
    dibHeader[9] = (finalHeight >> 8) & 0xFF;
    dibHeader[10] = (finalHeight >> 16) & 0xFF;
    dibHeader[11] = (finalHeight >> 24) & 0xFF;
    dibHeader[12] = 1;
    dibHeader[14] = 8;
    dibHeader[20] = imageSize & 0xFF;
    dibHeader[21] = (imageSize >> 8) & 0xFF;
    dibHeader[22] = (imageSize >> 16) & 0xFF;
    dibHeader[23] = (imageSize >> 24) & 0xFF;
    dibHeader[24] = 0x13; dibHeader[25] = 0x0B;
    dibHeader[28] = 0x13; dibHeader[29] = 0x0B;
    dibHeader[32] = 0; dibHeader[33] = 1;
    dibHeader[36] = 0; dibHeader[37] = 1;

    std::fwrite(fileHeader, 1, 14, f);
    std::fwrite(dibHeader, 1, 40, f);

    uint8_t palette[1024];
    for (int i = 0; i < 256; i++) {
        palette[i * 4 + 0] = static_cast<uint8_t>(i);
        palette[i * 4 + 1] = static_cast<uint8_t>(i);
        palette[i * 4 + 2] = static_cast<uint8_t>(i);
        palette[i * 4 + 3] = 0;
    }
    std::fwrite(palette, 1, 1024, f);

    std::vector<uint8_t> rowBuf(rowStride, 0);
    for (int y = finalHeight - 1; y >= 0; --y) {
        std::memset(rowBuf.data(), 0, rowStride);
        std::memcpy(rowBuf.data(), gray.data() + static_cast<size_t>(y) * finalWidth, static_cast<size_t>(finalWidth));
        std::fwrite(rowBuf.data(), 1, rowStride, f);
    }
    std::fclose(f);
    return true;
}

bool WriteZBufferPNG_8bit_Final(const std::string& path, const TsiLight5* siLight, int width, int height,
                                int imageScale, int tilingCrop) {
    std::vector<uint8_t> gray;
    int finalWidth = 0;
    int finalHeight = 0;
    if (!BuildFinalZBufferGray8(siLight, width, height, imageScale, tilingCrop, gray, finalWidth, finalHeight)) {
        return false;
    }
    return WritePNGGray(path, gray.data(), nullptr, finalWidth, finalHeight, 8);
}

bool WriteZBuffer(const std::string& path, const uint16_t* zbuf, int width, int height) {
    if (!zbuf || width <= 0 || height <= 0) return false;

    // Convert 16-bit Z values to 8-bit grayscale by scaling
    std::vector<uint8_t> gray(width * height);
    for (int i = 0; i < width * height; i++) {
        // Map 0..65535 to 0..255
        gray[i] = static_cast<uint8_t>(zbuf[i] >> 8);
    }

    int result = stbi_write_png(path.c_str(), width, height, 1, gray.data(), width);
    return result != 0;
}

bool WriteZBufferPNG_16bit(const std::string& path, const TsiLight5* siLight, int width, int height,
                           double ZcMul, double Zcorr) {
    if (!siLight || width <= 0 || height <= 0) return false;

    // Pascal ref: ZBuf16BitGen.pas — GetZPosFromSI + GuessParamsFromInfo + GetGrayValueFromZZ
    // Two-pass approach matching Pascal's per-scene dynamic min/max normalization.

    // Helper: decode packed Z from siLight5 to real Z distance
    // Pascal: GetZPosFromSI = (Sqr((8388351.5 - iZ) / ZcMul + 1) - 1) / Zcorr
    auto getZPos = [&](int i) -> double {
        int32_t packedZ = (static_cast<int32_t>(siLight[i].Zpos) << 8) |
                          ((siLight[i].RoughZposFine >> 8) & 0xFF);
        double v = (8388351.5 - static_cast<double>(packedZ)) / ZcMul + 1.0;
        return (v * v - 1.0) / Zcorr;
    };

    // First pass: find min/max real Z across all non-background pixels
    // Pascal: InitInfo sets MinRawZValue := MaxDouble, MaxRawZValue := MinDouble
    double minRawZ = std::numeric_limits<double>::max();
    double maxRawZ = std::numeric_limits<double>::lowest();
    bool hasHit = false;
    for (int i = 0; i < width * height; i++) {
        if (siLight[i].Zpos > 32767) continue; // background
        double zz = getZPos(i);
        if (zz < minRawZ) minRawZ = zz;
        if (zz > maxRawZ) maxRawZ = zz;
        hasHit = true;
    }

    if (!hasHit) {
        // All background — write all zeros
        std::vector<uint16_t> gray(static_cast<size_t>(width) * height, 0);
        return WritePNGGray(path, nullptr, gray.data(), width, height, 16);
    }

    // Pascal: GuessParamsFromInfo — maps raw Z range to gray values [1000, 64535]
    // MinZValue = 1000, MaxZValue = 65535 - 1000 = 64535
    // ZOffset = (MaxRawZ * MinZValue - MaxZValue * MinRawZ) / (MaxRawZ - MinRawZ) / 65535
    // ZScale  = 200 / 13107 * (MaxZValue - MinZValue) / (MaxRawZ - MinRawZ)
    constexpr int kMinZValue = 1000;
    constexpr int kMaxZValue = 65535 - 1000;
    double rawRange = maxRawZ - minRawZ;
    double ZOffset, ZScale;
    if (rawRange < 1e-30) {
        // Degenerate case: all hits at same Z — place at midpoint
        ZOffset = 0.5;
        ZScale = 0.0;
    } else {
        ZOffset = (maxRawZ * kMinZValue - static_cast<double>(kMaxZValue) * minRawZ) /
                  rawRange / 65535.0;
        ZScale = 200.0 / 13107.0 * (kMaxZValue - kMinZValue) / rawRange;
    }

    // Second pass: compute gray value per pixel
    // Pascal: GetGrayValueFromZZ = Round((ZZ * ZScale / 1000.0 + ZOffset) * 65535)
    // Clamped to [0, 65535]; background = 0
    std::vector<uint16_t> gray(static_cast<size_t>(width) * height);
    for (int i = 0; i < width * height; i++) {
        if (siLight[i].Zpos > 32767) {
            gray[i] = 0; // background
        } else {
            double zz = getZPos(i);
            int grayValue = static_cast<int>(std::lround(
                (zz * ZScale / 1000.0 + ZOffset) * 65535.0));
            gray[i] = static_cast<uint16_t>(std::max(0, std::min(65535, grayValue)));
        }
    }

    return WritePNGGray(path, nullptr, gray.data(), width, height, 16);
}

bool WriteZBufferPNG_16bit_Final(const std::string& path, const TsiLight5* siLight, int width, int height,
                                 int imageScale, int tilingCrop, double ZcMul, double Zcorr) {
    if (!siLight || width <= 0 || height <= 0) return false;

    auto getClampedGray16 = [&](const TsiLight5& px) -> uint16_t {
        if (px.Zpos > 32767) return 0;
        int32_t packedZ = (static_cast<int32_t>(px.Zpos) << 8) | ((px.RoughZposFine >> 8) & 0xFF);
        double v = (8388351.5 - static_cast<double>(packedZ)) / ZcMul + 1.0;
        double zz = (v * v - 1.0) / Zcorr;

        // Preserve current Pascal-like dynamic normalization already used by WriteZBufferPNG_16bit.
        // Recompute min/max on the full-resolution source surface first.
        return static_cast<uint16_t>(std::max(0, std::min(65535,
            static_cast<int>(std::lround(zz)))));
    };

    const int scale = std::max(1, std::min(10, imageScale));
    int finalWidth = width / scale;
    int finalHeight = height / scale;
    if (finalWidth <= 0 || finalHeight <= 0) return false;

    // Use existing 16-bit path for scale=1 without crop.
    if (scale == 1 && tilingCrop <= 0) {
        return WriteZBufferPNG_16bit(path, siLight, width, height, ZcMul, Zcorr);
    }

    // Reuse current dynamic normalization as a stable approximation, then average full-resolution gray16.
    std::vector<uint16_t> fullGray(static_cast<size_t>(width) * height, 0);
    {
        std::vector<uint16_t> tmp(static_cast<size_t>(width) * height, 0);
        // Build using the existing function's min/max logic.
        // Duplicate the relevant logic locally to get the full-resolution grayscale surface.
        auto getZPos = [&](int i) -> double {
            int32_t packedZ = (static_cast<int32_t>(siLight[i].Zpos) << 8) |
                              ((siLight[i].RoughZposFine >> 8) & 0xFF);
            double vv = (8388351.5 - static_cast<double>(packedZ)) / ZcMul + 1.0;
            return (vv * vv - 1.0) / Zcorr;
        };
        double minRawZ = std::numeric_limits<double>::max();
        double maxRawZ = std::numeric_limits<double>::lowest();
        bool hasHit = false;
        for (int i = 0; i < width * height; ++i) {
            if (siLight[i].Zpos > 32767) continue;
            double zz = getZPos(i);
            if (zz < minRawZ) minRawZ = zz;
            if (zz > maxRawZ) maxRawZ = zz;
            hasHit = true;
        }
        if (!hasHit) {
            std::vector<uint16_t> zeros(static_cast<size_t>(finalWidth) * finalHeight, 0);
            return WritePNGGray(path, nullptr, zeros.data(), finalWidth, finalHeight, 16);
        }
        constexpr int kMinZValue = 1000;
        constexpr int kMaxZValue = 65535 - 1000;
        double rawRange = maxRawZ - minRawZ;
        double ZOffset, ZScale;
        if (rawRange < 1e-30) {
            ZOffset = 0.5;
            ZScale = 0.0;
        } else {
            ZOffset = (maxRawZ * kMinZValue - static_cast<double>(kMaxZValue) * minRawZ) /
                      rawRange / 65535.0;
            ZScale = 200.0 / 13107.0 * (kMaxZValue - kMinZValue) / rawRange;
        }
        for (int i = 0; i < width * height; ++i) {
            if (siLight[i].Zpos > 32767) {
                tmp[i] = 0;
            } else {
                double zz = getZPos(i);
                int grayValue = static_cast<int>(std::lround(
                    (zz * ZScale / 1000.0 + ZOffset) * 65535.0));
                tmp[i] = static_cast<uint16_t>(std::max(0, std::min(65535, grayValue)));
            }
        }
        fullGray.swap(tmp);
    }

    std::vector<uint16_t> gray(static_cast<size_t>(finalWidth) * finalHeight, 0);
    if (scale == 1) {
        gray = fullGray;
    } else {
        for (int y = 0; y < finalHeight; ++y) {
            for (int x = 0; x < finalWidth; ++x) {
                int sum = 0;
                for (int yy = 0; yy < scale; ++yy) {
                    int srcY = y * scale + yy;
                    const uint16_t* row = fullGray.data() + static_cast<size_t>(srcY) * width;
                    for (int xx = 0; xx < scale; ++xx) {
                        const TsiLight5& px = siLight[static_cast<size_t>(srcY) * width + x * scale + xx];
                        if (px.Zpos < 32768) sum += row[x * scale + xx];
                    }
                }
                gray[static_cast<size_t>(y) * finalWidth + x] =
                    static_cast<uint16_t>(std::max(0, std::min(65535, sum / (scale * scale))));
            }
        }
    }

    int scaledCrop = tilingCrop;
    if (scale >= 2) scaledCrop /= scale;
    if (scaledCrop > 0 && finalWidth > 2 * scaledCrop && finalHeight > 2 * scaledCrop) {
        const int croppedW = finalWidth - 2 * scaledCrop;
        const int croppedH = finalHeight - 2 * scaledCrop;
        std::vector<uint16_t> cropped(static_cast<size_t>(croppedW) * croppedH, 0);
        for (int row = 0; row < croppedH; ++row) {
            const uint16_t* src = gray.data() + static_cast<size_t>(row + scaledCrop) * finalWidth + scaledCrop;
            uint16_t* dst = cropped.data() + static_cast<size_t>(row) * croppedW;
            std::memcpy(dst, src, static_cast<size_t>(croppedW) * sizeof(uint16_t));
        }
        gray.swap(cropped);
        finalWidth = croppedW;
        finalHeight = croppedH;
    }

    return WritePNGGray(path, nullptr, gray.data(), finalWidth, finalHeight, 16);
}

bool WriteNormalMap(const std::string& path, const int16_t* normals, int width, int height) {
    if (!normals || width <= 0 || height <= 0) return false;

    // Each pixel has 3 int16 normals (NX, NY, NZ from TsiLight5)
    // Map from [-32767..32767] to [0..255]
    std::vector<uint8_t> rgb(width * height * 3);
    for (int i = 0; i < width * height; i++) {
        int nx = normals[i * 3 + 0];
        int ny = normals[i * 3 + 1];
        int nz = normals[i * 3 + 2];
        // Map signed int16 to unsigned byte
        rgb[i * 3 + 0] = static_cast<uint8_t>((nx + 32767) >> 8);
        rgb[i * 3 + 1] = static_cast<uint8_t>((ny + 32767) >> 8);
        rgb[i * 3 + 2] = static_cast<uint8_t>((nz + 32767) >> 8);
    }

    int result = stbi_write_png(path.c_str(), width, height, 3, rgb.data(), width * 3);
    return result != 0;
}

bool WriteNormalMapFinal(const std::string& path, const int16_t* normals, int width, int height,
                         int imageScale, int tilingCrop) {
    if (!normals || width <= 0 || height <= 0) return false;

    std::vector<uint8_t> rgb(static_cast<size_t>(width) * height * 3);
    for (int i = 0; i < width * height; i++) {
        int nx = normals[i * 3 + 0];
        int ny = normals[i * 3 + 1];
        int nz = normals[i * 3 + 2];
        rgb[i * 3 + 0] = static_cast<uint8_t>((nx + 32767) >> 8);
        rgb[i * 3 + 1] = static_cast<uint8_t>((ny + 32767) >> 8);
        rgb[i * 3 + 2] = static_cast<uint8_t>((nz + 32767) >> 8);
    }

    FinalRGBSurface finalSurface;
    if (!BuildFinalRGBSurface(rgb.data(), width, height, imageScale, 0, tilingCrop, finalSurface)) {
        return false;
    }

    return stbi_write_png(path.c_str(),
                          finalSurface.width,
                          finalSurface.height,
                          3,
                          finalSurface.pixels.data(),
                          finalSurface.width * 3) != 0;
}

// Decode real Z distance from a siLight5 pixel.
// Returns negative value for background pixels.
static double DecodeRealZ(const TsiLight5& px, double ZcMul, double Zcorr) {
    if (px.Zpos > 32767) return -1.0; // background
    int32_t packedZ = (static_cast<int32_t>(px.Zpos) << 8) | ((px.RoughZposFine >> 8) & 0xFF);
    double v = (8388352.0 - static_cast<double>(packedZ)) / ZcMul + 1.0;
    return (v * v - 1.0) / Zcorr;
}

bool WriteNormalsFromZbuf(const std::string& path, const TsiLight5* siLight,
                          int width, int height, double ZcMul, double Zcorr) {
    if (!siLight || width <= 0 || height <= 0) return false;

    // Decode Z-buffer to real Z heights
    std::vector<double> zBuf(static_cast<size_t>(width) * height);
    const double BG = -1e20; // sentinel for background
    for (int i = 0; i < width * height; i++) {
        double rz = DecodeRealZ(siLight[i], ZcMul, Zcorr);
        zBuf[i] = (rz < 0.0) ? BG : rz;
    }

    auto zAt = [&](int x, int y) -> double {
        x = std::max(0, std::min(width - 1, x));
        y = std::max(0, std::min(height - 1, y));
        return zBuf[static_cast<size_t>(y) * width + x];
    };

    std::vector<uint8_t> rgb(static_cast<size_t>(width) * height * 3);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            size_t idx = static_cast<size_t>(y) * width + x;
            double zC = zBuf[idx];

            double nx, ny, nz;
            if (zC <= BG + 1.0) {
                // Background pixel: flat normal pointing toward camera
                nx = 0.0;
                ny = 0.0;
                nz = 1.0;
            } else {
                // Finite difference gradients with boundary clamping
                double zL = zAt(x - 1, y);
                double zR = zAt(x + 1, y);
                double zU = zAt(x, y - 1);
                double zD = zAt(x, y + 1);

                // If neighbor is background, use center value (one-sided difference -> zero gradient)
                if (zL <= BG + 1.0) zL = zC;
                if (zR <= BG + 1.0) zR = zC;
                if (zU <= BG + 1.0) zU = zC;
                if (zD <= BG + 1.0) zD = zC;

                double dZdx = zR - zL;
                double dZdy = zD - zU;

                // Normal = normalize(-dZdx, -dZdy, 1.0)
                double len = std::sqrt(dZdx * dZdx + dZdy * dZdy + 1.0);
                nx = -dZdx / len;
                ny = -dZdy / len;
                nz = 1.0 / len;
            }

            // Encode as RGB: (N + 1.0) * 0.5 * 255
            size_t pixOfs = idx * 3;
            rgb[pixOfs + 0] = static_cast<uint8_t>(std::max(0.0, std::min(255.0, (nx + 1.0) * 0.5 * 255.0)));
            rgb[pixOfs + 1] = static_cast<uint8_t>(std::max(0.0, std::min(255.0, (ny + 1.0) * 0.5 * 255.0)));
            rgb[pixOfs + 2] = static_cast<uint8_t>(std::max(0.0, std::min(255.0, (nz + 1.0) * 0.5 * 255.0)));
        }
    }

    int result = stbi_write_png(path.c_str(), width, height, 3, rgb.data(), width * 3);
    return result != 0;
}

} // namespace mb3d
