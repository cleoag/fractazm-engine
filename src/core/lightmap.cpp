#include "core/lightmap.h"
#include "core/math3d.h"
#include "core/colorizer.h"
#include "core/external_formulas.h"
#include "core/render_context.h"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <mutex>
#include <unordered_set>
#include <stdexcept>

// stb_image for loading BMP/PNG files
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace mb3d {

namespace fs = std::filesystem;

// ============================================================================
// Constants from Pascal
// ============================================================================

static constexpr double MPi05d = -0.5 / 3.14159265358979323846;
static constexpr double Pi1d   = 1.0 / 3.14159265358979323846;
static constexpr float  s1d255 = 1.0f / 255.0f;
static constexpr float  s255   = 255.0f;

// ============================================================================
// Internal helpers
// ============================================================================

static void CalcLMavrgCol(TPLightMap lm);
static void SetupLightMapFromRGBA(unsigned char* data, int w, int h, TPLightMap lm);

// ColToSVecFlipRB -- extract BGRA pixel to float SVec [R, G, B, 0]
// Pascal stores bitmap pixels in BGRA order and ColToSVecFlipRBc4 flips them
// back to logical RGB before spline/direct map interpolation.
static void LM_ColToSVec(TSVec& result, uint32_t col) {
    result[0] = static_cast<float>((col >> 0) & 0xFF);   // B -> [0]
    result[1] = static_cast<float>((col >> 8) & 0xFF);   // G -> [1]
    result[2] = static_cast<float>((col >> 16) & 0xFF);  // R -> [2]
    result[3] = 0.0f;
}

static void LM_ColToSVecFlipRB(TSVec& result, uint32_t col) {
    result[0] = static_cast<float>((col >> 16) & 0xFF);  // R -> swap to B pos
    result[1] = static_cast<float>((col >> 8) & 0xFF);   // G
    result[2] = static_cast<float>((col >> 0) & 0xFF);   // B -> swap to R pos
    result[3] = 0.0f;
}

// For our implementation, bitmap pixel data is stored as BGRA uint32_t
// (matching Windows DIB order used by the Pascal code).
// Access a pixel from the LMa array.
static inline uint32_t GetMapPixel(const TLightMap& lm, int x, int y) {
    int stride = lm.LMWidth + 1; // Pascal uses (LMWidth+1) entries per row
    return lm.LMa[y * stride + x];
}

// Read 4 neighboring pixels from map data at the given offsets, convert to SVec
static void Read4Pixels(T4SVec& sv, const uint8_t* base, int off0, int off1, int off2, int off3) {
    LM_ColToSVec(sv[0], *reinterpret_cast<const uint32_t*>(base + off0));
    LM_ColToSVec(sv[1], *reinterpret_cast<const uint32_t*>(base + off1));
    LM_ColToSVec(sv[2], *reinterpret_cast<const uint32_t*>(base + off2));
    LM_ColToSVec(sv[3], *reinterpret_cast<const uint32_t*>(base + off3));
}

// SplineWeightedRow: compute weighted sum of 4 SVecs with xv weights
static void SplineWeightedRow(TSVec& result, const T4SVec& sv, const TSVec& xv) {
    result[0] = sv[0][0] * xv[0] + sv[1][0] * xv[1] + sv[2][0] * xv[2] + sv[3][0] * xv[3];
    result[1] = sv[0][1] * xv[0] + sv[1][1] * xv[1] + sv[2][1] * xv[2] + sv[3][1] * xv[3];
    result[2] = sv[0][2] * xv[0] + sv[1][2] * xv[1] + sv[2][2] * xv[2] + sv[3][2] * xv[3];
    result[3] = 0.0f;
}

// Find or load a map by number. Returns index in CalcMaps[0..2], or -1 on fail.
static int FindOrLoadMap(int mapNr) {
    auto& resources = CurrentRenderContext().maps;
    auto& maps = resources.calc_maps;
    std::lock_guard<std::mutex> lock(resources.load_mutex);
    if (resources.banned_numbers.find(mapNr) != resources.banned_numbers.end()) {
        return -1;
    }
    for (int n = 0; n < 3; n++) {
        if (maps[n].LMnumber == mapNr) return n;
        if (maps[n].LMnumber == 0) {
            if (LoadLightMapNr(mapNr, &maps[n])) {
                return n;
            }
            resources.banned_numbers.insert(mapNr);
            return -1;
        }
    }
    return -1;
}

static fs::path ResolveMapResourcePath(const fs::path& path) {
    if (path.is_absolute()) return path;
    return fs::path(CurrentRenderContext().maps.resource_root) / path;
}

static constexpr const char* kMapSearchDirectories[] = {
    "M3Maps", "../M3Maps", "mb3d/M3Maps", "../mb3d/M3Maps",
    "mb3d/maps", "../mb3d/maps", "maps", "../maps"
};

static bool HasSupportedMapExtension(const fs::path& path) {
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return ext == ".JPG" || ext == ".JPEG" || ext == ".PNG" || ext == ".BMP" || ext == ".PGM";
}

static bool ReadPGMToken(std::istream& in, std::string& token) {
    token.clear();
    while (true) {
        int c = in.peek();
        if (c == EOF) return false;
        if (std::isspace(c)) {
            in.get();
            continue;
        }
        if (c == '#') {
            std::string skip;
            std::getline(in, skip);
            continue;
        }
        break;
    }
    while (true) {
        int c = in.peek();
        if (c == EOF || std::isspace(c) || c == '#') break;
        token.push_back(static_cast<char>(in.get()));
    }
    return !token.empty();
}

static bool LoadPGMLightMapFile(const char* filename, TPLightMap lm) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) return false;

    std::string magic, sw, sh, smax;
    if (!ReadPGMToken(in, magic) || (magic != "P5" && magic != "P2")) return false;
    if (!ReadPGMToken(in, sw) || !ReadPGMToken(in, sh) || !ReadPGMToken(in, smax)) return false;

    int w = std::stoi(sw);
    int h = std::stoi(sh);
    int maxv = std::stoi(smax);
    if (w <= 4 || h <= 4 || maxv <= 0 || (magic == "P5" && maxv > 65535)) return false;

    if (magic == "P2") {
        // PNMReader's ordinary P2 format always stores words, even MaxVal<=255.
        // Preserve its contiguous pixel write: unlike BMP, the original PGM
        // loader does not populate the allocated extra column or bottom row.
        const size_t words = ((size_t(w) + 1) * (size_t(h) + 1) * 6 + 6) / 4;
        std::vector<uint32_t> pixels(words, 0);
        auto* dst = reinterpret_cast<uint16_t*>(pixels.data());
        for (size_t i = 0; i < size_t(w) * size_t(h); ++i) {
            std::string token;
            if (!ReadPGMToken(in, token)) return false;
            const uint16_t sample = static_cast<uint16_t>(std::stoi(token));
            dst[3*i] = dst[3*i+1] = dst[3*i+2] = sample;
        }
        FreeLightMap(lm);
        lm->LMa = new uint32_t[words];
        std::memcpy(lm->LMa, pixels.data(), words * 4);
        lm->LMWidth = w;
        lm->LMHeight = h;
        lm->sLMXfactor = static_cast<float>(w);
        lm->sLMYfactor = static_cast<float>(h);
        lm->sIntensity = 1;
        lm->iMapType = 1;
        CalcLMavrgCol(lm);
        std::memset(lm->PicRotMatrix, 0, sizeof(TSMatrix3));
        for (int i = 0; i < 3; ++i) lm->PicRotMatrix[i][i] = 1;
        return true;
    }

    // P5 is a retained C++ extension, not an original-Pascal PGM format.
    // Skip the single whitespace byte after MaxVal.
    int sep = in.get();
    if (sep == EOF) return false;

    lm->LMWidth = w;
    lm->LMHeight = h;
    lm->sLMXfactor = static_cast<float>(w);
    lm->sLMYfactor = static_cast<float>(h);
    lm->sIntensity = 1.0f;
    lm->LMnumber = 0;
    lm->LMframe = 0;

    if (maxv > 255) {
        lm->iMapType = 1;
        const size_t totalBytes = ((static_cast<size_t>(w) + 1u) * (static_cast<size_t>(h) + 1u) * 6u + 6u);
        const size_t totalWords = (totalBytes + 3u) / 4u;
        lm->LMa = new uint32_t[totalWords];
        std::memset(lm->LMa, 0, totalWords * sizeof(uint32_t));
        uint8_t* base = reinterpret_cast<uint8_t*>(lm->LMa);
        auto pixelPtr = [&](int x, int y) -> uint16_t* {
            size_t offset = (static_cast<size_t>(y) * static_cast<size_t>(w + 1) +
                             static_cast<size_t>(x)) * 6u;
            return reinterpret_cast<uint16_t*>(base + offset);
        };

        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                unsigned char hi = 0, lo = 0;
                in.read(reinterpret_cast<char*>(&hi), 1);
                in.read(reinterpret_cast<char*>(&lo), 1);
                if (!in) return false;
                uint16_t sample = static_cast<uint16_t>((static_cast<uint16_t>(hi) << 8) | lo);
                uint16_t* dst = pixelPtr(x, y);
                dst[0] = sample;
                dst[1] = sample;
                dst[2] = sample;
            }
            uint16_t* wrapDst = pixelPtr(w, y);
            uint16_t* first = pixelPtr(0, y);
            wrapDst[0] = first[0];
            wrapDst[1] = first[1];
            wrapDst[2] = first[2];
        }
        for (int x = 0; x <= w; x++) {
            uint16_t* dst = pixelPtr(x, h);
            uint16_t* src = pixelPtr(x, 0);
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
        }
    } else {
        std::vector<unsigned char> rgba(static_cast<size_t>(w) * static_cast<size_t>(h) * 4u);
        for (int i = 0; i < w * h; i++) {
            unsigned char g = 0;
            in.read(reinterpret_cast<char*>(&g), 1);
            if (!in) return false;
            rgba[static_cast<size_t>(i) * 4u + 0u] = g;
            rgba[static_cast<size_t>(i) * 4u + 1u] = g;
            rgba[static_cast<size_t>(i) * 4u + 2u] = g;
            rgba[static_cast<size_t>(i) * 4u + 3u] = 255;
        }
        SetupLightMapFromRGBA(rgba.data(), w, h, lm);
        return true;
    }

    lm->iLMstart = 0;
    CalcLMavrgCol(lm);
    lm->PicRotMatrix[0][0] = 1.0f;
    lm->PicRotMatrix[0][1] = 0.0f;
    lm->PicRotMatrix[0][2] = 0.0f;
    lm->PicRotMatrix[1][0] = 0.0f;
    lm->PicRotMatrix[1][1] = 1.0f;
    lm->PicRotMatrix[1][2] = 0.0f;
    lm->PicRotMatrix[2][0] = 0.0f;
    lm->PicRotMatrix[2][1] = 0.0f;
    lm->PicRotMatrix[2][2] = 1.0f;
    return true;
}

static bool TryLoadHeightMapFromDir(const fs::path& dir, int mapNr, TPLightMap lm) {
    std::error_code ec;
    if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
        return false;
    }

    const std::string prefix = std::to_string(mapNr);
    std::vector<fs::path> exactMatches;
    std::vector<fs::path> prefixMatches;

    for (fs::directory_iterator it(dir, ec); !ec && it != fs::directory_iterator(); it.increment(ec)) {
        if (!it->is_regular_file()) continue;
        const fs::path entryPath = it->path();
        if (!HasSupportedMapExtension(entryPath)) continue;

        const std::string stem = entryPath.stem().string();
        if (stem == prefix) {
            exactMatches.push_back(entryPath);
            continue;
        }
        if (stem.rfind(prefix, 0) == 0 && stem.size() > prefix.size()) {
            unsigned char next = static_cast<unsigned char>(stem[prefix.size()]);
            if (!std::isdigit(next)) {
                prefixMatches.push_back(entryPath);
            }
        }
    }

    auto pathLess = [](const fs::path& a, const fs::path& b) {
        return a.filename().string() < b.filename().string();
    };
    std::sort(exactMatches.begin(), exactMatches.end(), pathLess);
    std::sort(prefixMatches.begin(), prefixMatches.end(), pathLess);

    auto tryLoadList = [&](const std::vector<fs::path>& paths) -> bool {
        for (const fs::path& path : paths) {
            if (LoadLightMapFile(path.string().c_str(), lm)) {
                lm->LMnumber = mapNr;
                lm->LMframe = 0;
                return true;
            }
        }
        return false;
    };

    return tryLoadList(exactMatches) || tryLoadList(prefixMatches);
}

// ============================================================================
// Map loading
// ============================================================================

bool LoadLightMapNr(int mapNr, TPLightMap lm) {
    // Check map sequence first (from Pascal LoadLightMapNr in MB3DMaps.pas).
    // If a sequence exists for this map channel, resolve the frame-specific filename.
    const MapSequence* seq = GetMapSequenceList().GetSequence(mapNr);
    if (seq != nullptr) {
        std::string seqFile = seq->GetFilename(GetCurrMapFrameNumber());
        bool loaded = false;
        if (!seqFile.empty()) {
            loaded = LoadLightMapFile(seqFile.c_str(), lm);
        }
        if (!loaded) {
            // Sequence file not found -- load empty map (matching Pascal fallback)
            FreeLightMap(lm);
        }
        lm->LMnumber = mapNr;
        lm->LMframe = GetCurrMapFrameNumber();
        return true; // Pascal returns True even on load failure when sequence exists
    }

    // No sequence -- load from M3Maps directory by number.
    // Mirror Pascal FindHeightMap:
    // 1. exact "<nr>.*" with supported image extension
    // 2. fallback "<nr>*.*" where the first char after the number is not a digit
    for (const char* directory : kMapSearchDirectories) {
        if (TryLoadHeightMapFromDir(ResolveMapResourcePath(directory), mapNr, lm)) return true;
    }

    GetMapMissingState() = 1;
    return false;
}

// CalcLMavrgCol -- compute average color of a lightmap.
// Matches Pascal CalcLMavrgCol in MB3DMaps.pas lines 579-614.
// Branches on iMapType: 0 = 32-bit BGRA (4-byte stride), 1 = 16-bit word (6-byte stride).
static void CalcLMavrgCol(TPLightMap lm) {
    int w = lm->LMWidth;
    int h = lm->LMHeight;
    if (w <= 0 || h <= 0) return;

    double sumR = 0.0, sumG = 0.0, sumB = 0.0;
    const uint8_t* base = reinterpret_cast<const uint8_t*>(lm->LMa);

    if (lm->iMapType == 1) {
        // 16-bit word precision path: TP3word = 3x uint16_t per pixel, 6-byte stride per pixel
        // Row stride = (LMWidth + 1) * 6 bytes
        int rowStride = (w + 1) * 6;
        for (int y = 0; y < h; y++) {
            const uint16_t* pw = reinterpret_cast<const uint16_t*>(base + y * rowStride);
            for (int x = 0; x < w; x++) {
                // ColToSVecFlipRB_word: Result[0]=pw[2], Result[1]=pw[1], Result[2]=pw[0]
                // (RB flip: word[0]=B, word[1]=G, word[2]=R)
                sumR += static_cast<double>(pw[2]);
                sumG += static_cast<double>(pw[1]);
                sumB += static_cast<double>(pw[0]);
                pw += 3; // advance to next pixel (3 uint16_t = 6 bytes)
            }
        }
        // Pascal: scale = 256 for iMapType=1
        double scale = 256.0;
        double denom = static_cast<double>(h) * static_cast<double>(w) * scale;
        lm->LMavrgCol[0] = static_cast<float>(sumR / denom);
        lm->LMavrgCol[1] = static_cast<float>(sumG / denom);
        lm->LMavrgCol[2] = static_cast<float>(sumB / denom);
        lm->LMavrgCol[3] = 0.0f;
    } else {
        // 32-bit BGRA path: 4 bytes per pixel
        // Row stride = (LMWidth + 1) * 4 bytes
        int rowStride = (w + 1) * 4;
        for (int y = 0; y < h; y++) {
            const uint8_t* row = base + y * rowStride;
            for (int x = 0; x < w; x++) {
                // LMa stores BGRA. ColToSVecFlipRBc flips R and B:
                // Result[0] = (col>>16)&0xFF = R, Result[1] = (col>>8)&0xFF = G, Result[2] = col&0xFF = B
                uint32_t col = *reinterpret_cast<const uint32_t*>(row + x * 4);
                sumR += static_cast<double>((col >> 16) & 0xFF);
                sumG += static_cast<double>((col >> 8) & 0xFF);
                sumB += static_cast<double>(col & 0xFF);
            }
        }
        // Pascal: scale = 1 for iMapType=0
        double denom = static_cast<double>(h) * static_cast<double>(w);
        lm->LMavrgCol[0] = static_cast<float>(sumR / denom);
        lm->LMavrgCol[1] = static_cast<float>(sumG / denom);
        lm->LMavrgCol[2] = static_cast<float>(sumB / denom);
        lm->LMavrgCol[3] = 0.0f;
    }

    // Pascal consumers expect average color in 0..255, unlike sampled colors.
    lm->LMavrgColSqr[0] = lm->LMavrgCol[0] * lm->LMavrgCol[0] * s1d255;
    lm->LMavrgColSqr[1] = lm->LMavrgCol[1] * lm->LMavrgCol[1] * s1d255;
    lm->LMavrgColSqr[2] = lm->LMavrgCol[2] * lm->LMavrgCol[2] * s1d255;
    lm->LMavrgColSqr[3] = 0.0f;
}

// SetupLightMapFromRGBA -- shared helper that takes decoded RGBA pixel data and
// populates the TLightMap struct: RGBA->BGRA conversion, wrap rows, average color.
// The caller is responsible for freeing `data` after this returns.
static void SetupLightMapFromRGBA(unsigned char* data, int w, int h, TPLightMap lm) {
    lm->LMWidth = w;
    lm->LMHeight = h;
    lm->sLMXfactor = static_cast<float>(w);
    lm->sLMYfactor = static_cast<float>(h);
    lm->iMapType = 0; // 0 = 32-bit BGRA, 1 = 48-bit

    // Allocate (LMWidth+1) * (LMHeight+1) -- Pascal convention with extra column
    int stride = w + 1;
    int totalPixels = stride * (h + 1);
    lm->LMa = new uint32_t[totalPixels];
    std::memset(lm->LMa, 0, totalPixels * sizeof(uint32_t));

    // Convert RGBA -> BGRA and store
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            const unsigned char* px = data + (y * w + x) * 4;
            uint32_t bgra = px[2] | (px[1] << 8) | (px[0] << 16) | (px[3] << 24);
            lm->LMa[y * stride + x] = bgra;
        }
        // Wrap: copy first pixel to the extra column
        lm->LMa[y * stride + w] = lm->LMa[y * stride + 0];
    }
    // Wrap: copy first row to the extra row
    std::memcpy(&lm->LMa[h * stride], &lm->LMa[0], stride * sizeof(uint32_t));

    lm->iLMstart = 0; // We use array indexing, not raw pointers

    // Compute average color (matches Pascal CalcLMavrgCol in MB3DMaps.pas)
    CalcLMavrgCol(lm);

    lm->sIntensity = 1.0f;

    // Initialize PicRotMatrix to identity (matching Pascal where BuildRotMatrixS(0,0,0,...)
    // is called for lightmap structs, producing the identity rotation matrix).
    std::memset(lm->PicRotMatrix, 0, sizeof(TSMatrix3));
    lm->PicRotMatrix[0][0] = 1.0f;
    lm->PicRotMatrix[1][1] = 1.0f;
    lm->PicRotMatrix[2][2] = 1.0f;
}

bool LoadLightMapFile(const char* filename, TPLightMap lm) {
    const fs::path path = ResolveMapResourcePath(filename);
    const std::string resolvedFilename = path.string();
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (ext == ".pgm") {
        return LoadPGMLightMapFile(resolvedFilename.c_str(), lm);
    }

    int w, h, channels;
    unsigned char* data = stbi_load(resolvedFilename.c_str(), &w, &h, &channels, 4); // Force RGBA
    if (!data) return false;

    SetupLightMapFromRGBA(data, w, h, lm);

    stbi_image_free(data);
    return true;
}

bool LoadBackgroundMapFile(const char* filename, TPLightMap lm) {
    if (!filename || !*filename) return false;
    if (LoadLightMapFile(filename, lm)) return true;
    const fs::path name(filename);
    if (name.has_parent_path()) return false;
    // FileHandling.pas::LoadBackgroundPicT searches configured image/map
    // directories for the stored basename. The CLI uses its per-job roots,
    // shared by CPU and Metal preparation, without changing process cwd.
    for (const char* directory : kMapSearchDirectories) {
        const auto path = ResolveMapResourcePath(fs::path(directory) / name);
        if (LoadLightMapFile(path.string().c_str(), lm)) return true;
    }
    return false;
}

bool LoadLightMapFromMemory(const uint8_t* data, size_t len, TPLightMap lm) {
    int w, h, channels;
    unsigned char* pixels = stbi_load_from_memory(data, static_cast<int>(len),
                                                   &w, &h, &channels, 4); // Force RGBA
    if (!pixels) return false;

    SetupLightMapFromRGBA(pixels, w, h, lm);

    stbi_image_free(pixels);
    return true;
}

void PrepareLightMap(TPLightMap lm, bool smooth, bool spherical, bool fitBorder) {
    if (!lm || !lm->LMa || lm->LMWidth < 4 || lm->LMHeight < 4) return;
    if (fitBorder && lm->LMWidth < 6)
        throw std::invalid_argument("background border requires map width >=6; Pascal accesses before the buffer for smaller loaded maps");
    const int w = lm->LMWidth, h = lm->LMHeight, stride = w + 1;
    const int pixelBytes = lm->iMapType == 1 ? 6 : 4;
    const int rowBytes = stride * pixelBytes;
    auto* bytes = reinterpret_cast<uint8_t*>(lm->LMa);
    auto clip = [](float value, int maximum) -> uint32_t {
        return static_cast<uint32_t>(std::nearbyint(std::max(0.0f, std::min(float(maximum), value))));
    };
    auto filter = [&](uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e) {
        uint32_t out = 0;
        for (int channel = 0; channel < 3; ++channel) {
            const int shift = channel * 8;
            const double value = (((a >> shift) & 255) + ((e >> shift) & 255)) * -0.05 +
                (((b >> shift) & 255) + ((d >> shift) & 255)) * 0.25 + ((c >> shift) & 255) * 0.6;
            out |= clip(static_cast<float>(value), 255) << shift;
        }
        return out;
    };
    if (smooth && lm->iMapType == 0) {
        // Keep the original in-place traversal: the wrap samples are not an
        // immutable convolution source after the first pass.
        for (int x = 0; x < w; ++x) {
            int p3=x, p4=x+stride, p5=x+2*stride;
            uint32_t a=lm->LMa[x+(h-2)*stride], b=lm->LMa[x+(h-1)*stride], c=lm->LMa[p3];
            for (int y=1; y<=h; ++y) {
                lm->LMa[p3]=filter(a,b,c,lm->LMa[p4],lm->LMa[p5]);
                a=b; b=c; c=lm->LMa[p4]; p3=p4; p4=p5;
                p5=(y==h-1)?x:p5+stride;
            }
            lm->LMa[p3]=lm->LMa[x];
        }
        for (int y=0; y<h; ++y) {
            const int start=y*stride;
            int p3=start,p4=start+1,p5=start+2;
            uint32_t a=lm->LMa[start+w-2],b=lm->LMa[start+w-1],c=lm->LMa[start];
            for (int x=1; x<=w; ++x) {
                lm->LMa[p3]=filter(a,b,c,lm->LMa[p4],lm->LMa[p5]);
                a=b; b=c; c=lm->LMa[p4]; p3=p4; p4=p5;
                p5=(x==w-1)?start:p5+1;
            }
            lm->LMa[p3]=lm->LMa[start];
        }
    }
    if (spherical) {
        std::vector<uint8_t> row(rowBytes+pixelBytes);
        const float center=(w-1)*0.5f, right=static_cast<float>(w-1.99);
        for (int y=0; y<h; ++y) {
            auto* dst=bytes+y*rowBytes;
            const float latitude=2.0f*(y+0.5f)/h-1.0f;
            const float scale=static_cast<float>(std::cos(ArcSinSafe(latitude)));
            std::memcpy(row.data(),dst,rowBytes);
            std::memcpy(row.data()+rowBytes,row.data()+pixelBytes,pixelBytes);
            uint8_t originalSecond[6] = {};
            for (int x=0; x<w; ++x) {
                const float at=std::min((x-center)*scale+center,right);
                const int index=static_cast<int>(at);
                const float f=at-index, ff=f*f, fff=ff*f;
                const float weights[4]={3*ff-2*f-fff,3*(fff-2*ff-f+2),3*(ff+2*f-fff),fff-f};
                const auto* src=row.data()+index*pixelBytes;
                if (pixelBytes==6) {
                    auto* out=reinterpret_cast<uint16_t*>(dst);
                    for(int c=0;c<3;++c) {
                        const auto* p=reinterpret_cast<const uint16_t*>(src);
                        const float v=((p[c]*weights[0]+p[c+3]*weights[1])+p[c+6]*weights[2])+p[c+9]*weights[3];
                        out[c]=static_cast<uint16_t>(clip(v*(1.0f/6.0f),65535));
                    }
                } else {
                    uint32_t out=0;
                    const auto* p=reinterpret_cast<const uint32_t*>(src);
                    for (int c=0;c<3;++c) {
                        const uint32_t mask=255u<<(8*c);
                        const float v=(((p[0]&mask)*weights[0]+(p[1]&mask)*weights[1])+(p[2]&mask)*weights[2])+(p[3]&mask)*weights[3];
                        out|=clip(v*(1.0f/(6.0f*float(1u<<(8*c)))),255)<<(8*c);
                    }
                    std::memcpy(dst,&out,4);
                }
                dst+=pixelBytes;
                // Original copies after advancing pc3: the original second
                // pixel becomes the wrap pixel, not the new first pixel.
                if(x==0) std::memcpy(originalSecond,dst,pixelBytes);
            }
            std::memcpy(dst,originalSecond,pixelBytes);
        }
        std::memcpy(bytes+h*rowBytes,bytes,rowBytes);
    }
    if (fitBorder) {
        for(int x=0;x<=5;++x) {
            const float s2=static_cast<float>(0.5-0.5*std::sin(3.14159265358979323846*0.5*(x+0.5)/6));
            const float s1=1-s2;
            for(int y=0;y<h;++y) {
                auto* a=bytes+y*rowBytes+x*pixelBytes;
                auto* b=bytes+y*rowBytes+(w-x-1)*pixelBytes;
                if(pixelBytes==6) {
                    auto* pa=reinterpret_cast<uint16_t*>(a); auto* pb=reinterpret_cast<uint16_t*>(b);
                    uint16_t ra[3],rb[3];
                    for(int c=0;c<3;++c) {
                        ra[c]=static_cast<uint16_t>(std::nearbyint(pa[c]*s1+pb[c]*s2));
                        rb[c]=static_cast<uint16_t>(std::nearbyint(pa[c]*s2+pb[c]*s1));
                    }
                    std::memcpy(b,rb,6); std::memcpy(a,ra,6);
                } else {
                    uint32_t ra=0,rb=0;
                    for(int c=0;c<3;++c) {
                        ra|=clip(a[c]*s1+b[c]*s2,255)<<(c*8);
                        rb|=clip(a[c]*s2+b[c]*s1,255)<<(c*8);
                    }
                    std::memcpy(b,&rb,4); std::memcpy(a,&ra,4);
                }
            }
        }
        for(int y=0;y<h;++y) std::memcpy(bytes+y*rowBytes+w*pixelBytes,bytes+y*rowBytes,pixelBytes);
    }
    CalcLMavrgCol(lm);
}

// ============================================================================
// IniCalcMaps
// ============================================================================

void IniCalcMaps(TMandHeader10* header, THeaderCustomAddon* addon) {
    // Scan formulas for MAP references and load required maps.
    // Mirror Pascal Calc.pas IniCalcMaps(): collect up to three referenced map
    // numbers, keep already-loaded matching frames, free stale slots, and preload
    // the remaining maps while still allowing on-demand lookups later.

    int mcount = 0;
    int ma[3] = {0, 0, 0};

    if (!addon) return;

    auto& maps = GetCalcMaps();

    bool bIsIpol = (addon->bOptions1 & 3) == 1;
    int i2 = bIsIpol ? 2 : 5;

    for (int i = 0; i <= i2; i++) {
        if (!bIsIpol && addon->Formulas[i].iItCount == 0) continue;
        const ExternalFormulaInfo* extInfo = nullptr;
        if (addon->Formulas[i].iFnr >= 20 && addon->Formulas[i].CustomFname[0] != 0) {
            extInfo = FindExternalFormula(reinterpret_cast<const char*>(addon->Formulas[i].CustomFname));
        }
        int optCount = std::min(16, static_cast<int>(addon->Formulas[i].iOptionCount));
        for (int j = 0; j < optCount; j++) {
            bool isMapSlot = false;
            if (extInfo != nullptr) {
                isMapSlot = (j < 16) && ((extInfo->mapValueMask & (uint16_t(1) << j)) != 0);
            } else {
                isMapSlot = addon->Formulas[i].byOptionType[j] == 2;
            }
            if (!isMapSlot) continue;

            int n = static_cast<int>(std::round(
                std::max(0.0, std::min(33000.0, addon->Formulas[i].dOptionValue[j]))));
            if (n > 0 && n < 32001 && mcount < 3) {
                bool alreadyInList = false;
                for (int c = 0; c < mcount; c++) {
                    if (ma[c] == n) { alreadyInList = true; break; }
                }
                if (!alreadyInList) {
                    ma[mcount] = n;
                    mcount++;
                }
            }
        }
    }

    // Remove already-loaded maps from list, free unneeded maps.
    // Pascal: if the map is already loaded AND either there's no sequence for it
    // or the frame number matches, keep it. Otherwise reload is needed.
    for (int i = 0; i < 3; i++) {
        int c = 0;
        int j = 0;
        while (j < mcount) {
            if (maps[i].LMnumber == ma[j]) {
                const MapSequence* seq = GetMapSequenceList().GetSequence(ma[j]);
                if (seq == nullptr ||
                    maps[i].LMframe == GetCurrMapFrameNumber()) {
                    // Already loaded with correct frame, remove from need list
                    mcount--;
                    for (int n = j; n < mcount; n++) ma[n] = ma[n + 1];
                    c++;
                }
            }
            j++;
        }
        if (c == 0) FreeLightMap(&maps[i]);
    }

    // Load missing maps into free slots
    for (int i = 0; i < mcount; i++) {
        for (int j = 0; j < 3; j++) {
            if (maps[j].LMnumber == 0) {
                LoadLightMapNr(ma[i], &maps[j]);
                break;
            }
        }
    }
}

// ============================================================================
// MakeSplineCoeff
// ============================================================================

static void MakeSplineCoeffDouble(double xs, TSVec& result) {
    // From Math3D.pas:
    //   Result[3] := d1d6 * xs * xs * xs;
    //   Result[0] := d1d6 + 0.5 * xs * (xs - 1.0) - Result[3];
    //   Result[2] := xs + Result[0] - 2.0 * Result[3];
    //   Result[1] := 1.0 - Result[0] - Result[2] - Result[3];
    // Pascal keeps all four intermediate coefficients on x87 and only rounds
    // at each final Single store; do not feed rounded coefficients forward.
    const double c3 = (xs * xs * xs) * (1.0 / 6.0);
    const double c0 = (1.0 / 6.0) - ((1.0 - xs) * xs) * 0.5 - c3;
    const double c2 = ((xs + c0) - c3) - c3;
    result[3] = static_cast<float>(c3);
    result[0] = static_cast<float>(c0);
    result[2] = static_cast<float>(c2);
    result[1] = static_cast<float>(((1.0 - c2) - c3) - c0);
}

void MakeSplineCoeff(float xs, TSVec& result) {
    MakeSplineCoeffDouble(xs, result);
}

static void SampleLightMapRow(TSVec& result, TPLightMap lm, int row,
                             const int (&columns)[4], const TSVec& weights, bool squared) {
    const auto* words = reinterpret_cast<const uint16_t*>(lm->LMa);
    const bool wordMap = lm->iMapType == 1;
    const float normalize = wordMap ? (squared ? static_cast<float>(1.0/(65535.0*65535.0)) : 1.0f/65535.0f)
        : (squared ? static_cast<float>(1.0/(255.0*255.0)) : s1d255);
    for(int channel=0; channel<3; ++channel) {
        float samples[4];
        for(int k=0; k<4; ++k) {
            const size_t pixel=size_t(row)*(lm->LMWidth+1)+columns[k];
            const float raw=wordMap ? float(words[pixel*3+2-channel])
                : float((lm->LMa[pixel]>>(8*(2-channel)))&255u);
            samples[k]=(squared?raw*raw:raw)*(weights[k]*normalize);
        }
        result[channel]=(samples[0]+samples[2])+(samples[1]+samples[3]);
    }
    result[3]=0;
}

// ============================================================================
// FracA
// ============================================================================

double FracA(double d) {
    if (d < 0.0) {
        double frac = d - std::trunc(d);
        return 1.0 + frac;
    }
    return d - std::trunc(d);
}

// ============================================================================
// SplineIpolMap -- bicubic spline interpolation at direct map coordinates
// ============================================================================

void SplineIpolMap(TVec3D& result, double x, double y, TPLightMap pCalcMap) {
    if (!pCalcMap || !pCalcMap->LMa || pCalcMap->LMWidth <= 0) {
        ClearDVec(result);
        return;
    }

    int xf = static_cast<int>(x);
    int yf = static_cast<int>(y);

    TSVec xv, yv;
    MakeSplineCoeffDouble(x - xf, xv);
    MakeSplineCoeffDouble(y - yf, yv);

    int lmW = pCalcMap->LMWidth;
    int lmH = pCalcMap->LMHeight;

    // Wrap yf
    if (yf > 0) yf--; else yf += lmH - 1;

    // Pascal uses a (LMWidth+1)-wide buffer with a duplicated seam column.
    // The x-neighbor selection must therefore follow the original nx1/nx2/nx3
    // arithmetic instead of modulo-by-LMWidth sampling.
    int nx1 = (xf > 0) ? -1 : (lmW - 1);
    int nx2 = (xf < lmW - 1) ? 2 : (2 - lmW);
    int nx3 = (xf < lmW) ? 1 : (1 - lmW);

    TSVec svy[4];
    const int columns[4] = {xf + nx1, xf, xf + nx3, xf + nx2};

    // Process 4 rows
    for (int row = 0; row < 4; row++) {
        int wy = yf;
        if (wy < 0) wy = 0;
        if (wy >= lmH) wy = 0; // wrap

        SampleLightMapRow(svy[row], pCalcMap, wy, columns, xv, false);

        // Advance to next row with wrapping
        yf++;
        if (yf >= lmH) yf = 0;
    }

    // Combine rows with Y spline weights
    result[0] = svy[0][0] * yv[0] + svy[1][0] * yv[1] + svy[2][0] * yv[2] + svy[3][0] * yv[3];
    result[1] = svy[0][1] * yv[0] + svy[1][1] * yv[1] + svy[2][1] * yv[2] + svy[3][1] * yv[3];
    result[2] = svy[0][2] * yv[0] + svy[1][2] * yv[1] + svy[2][2] * yv[2] + svy[3][2] * yv[3];
}

// ============================================================================
// GetMapPixelSphereSpline
// ============================================================================

void GetMapPixelSphereSpline(TVec3D& result, TPVec3D pVec3D, int32_t mapNr) {
    int n = FindOrLoadMap(mapNr);
    if (n < 0) {
        GetMapMissingState() = 1;
        ClearDVec(result);
        return;
    }

    // Normalize direction vector
    TVec3D dv;
    NormalizeVec3D(dv, *pVec3D);

    // Sphere to UV mapping
    // x = ArcTan2(dv[0], dv[1]) * MPi05d + 0.5
    // y = 0.5 - ArcSin(dv[2]) * Pi1d
    double x = std::atan2(dv[0], dv[1]) * MPi05d + 0.5;
    double y = 0.5 - ArcSinSafe(dv[2]) * Pi1d;

    TLightMap& cm = GetCalcMaps()[n];
    if (cm.LMWidth > 0) {
        double xx = Min0MaxCD(x * cm.sLMXfactor, static_cast<double>(cm.sLMXfactor));
        double yy = Min0MaxCD(y * cm.sLMYfactor, static_cast<double>(cm.sLMYfactor));
        SplineIpolMap(result, xx, yy, &cm);
    } else {
        ClearDVec(result);
    }
}

// ============================================================================
// GetMapPixelDirectXYspline
// ============================================================================

void GetMapPixelDirectXYspline(TVec3D& result, TPVec3D pVec3D, int32_t mapNr) {
    int n = FindOrLoadMap(mapNr);
    if (n < 0) {
        GetMapMissingState() = 1;
        ClearDVec(result);
        return;
    }

    TLightMap& cm = GetCalcMaps()[n];
    if (cm.LMWidth > 0) {
        double x = FracA((*pVec3D)[0]) * cm.sLMXfactor;
        double y = FracA((*pVec3D)[1]) * cm.sLMYfactor;
        SplineIpolMap(result, x, y, &cm);
    } else {
        ClearDVec(result);
    }
}

// ============================================================================
// GetMapPixelSphere -- linear interpolation variant
// ============================================================================

void GetMapPixelSphere(TVec3D& result, TPVec3D pVec3D, int32_t mapNr) {
    int n = FindOrLoadMap(mapNr);
    if (n < 0) {
        ClearDVec(result);
        return;
    }

    TVec3D dv;
    NormalizeVec3D(dv, *pVec3D);

    double x = std::atan2(dv[0], dv[1]) * MPi05d + 0.5;
    double y = 0.5 - ArcSinSafe(dv[2]) * Pi1d;

    TLightMap& cm = GetCalcMaps()[n];
    if (cm.LMWidth > 0) {
        double xx = MinMaxCD(0.0, x * cm.sLMXfactor, static_cast<double>(cm.sLMXfactor));
        double yy = MinMaxCD(0.0, y * cm.sLMYfactor, static_cast<double>(cm.sLMYfactor));
        SplineIpolMap(result, xx, yy, &cm);
    } else {
        ClearDVec(result);
    }
}

// ============================================================================
// GetMapPixelSphereBicubic
// ============================================================================

void GetMapPixelSphereBicubic(TVec3D& result, TPVec3D pVec3D, int32_t mapNr) {
    int n = FindOrLoadMap(mapNr);
    if (n < 0) {
        ClearDVec(result);
        return;
    }

    TVec3D dv;
    NormalizeVec3D(dv, *pVec3D);

    double x = std::atan2(dv[0], dv[1]) * MPi05d + 0.5;
    double y = 0.5 - ArcSinSafe(dv[2]) * Pi1d;

    TLightMap& cm = GetCalcMaps()[n];
    if (cm.LMWidth <= 0) {
        ClearDVec(result);
        return;
    }

    double xx = MinMaxCD(0.0, x * cm.sLMXfactor, static_cast<double>(cm.sLMXfactor));
    double yy = MinMaxCD(0.0, y * cm.sLMYfactor, static_cast<double>(cm.sLMYfactor));

    int xf = static_cast<int>(xx);
    float xs = static_cast<float>(xx - xf);
    int yf = static_cast<int>(yy);
    float ys = static_cast<float>(yy - yf);

    // Bicubic Catmull-Rom weights
    float x0 = ((-0.8f * xs + 3.2f) * (xs + 1.0f) - 6.4f) * (xs + 1.0f) + 3.2f;
    float x1 = (1.2f * xs - 2.2f) * xs * xs + 1.0f;
    float x2 = (-1.2f * xs - 1.0f) * (1.0f - xs) * (1.0f - xs) + 1.0f;
    float x3 = ((0.8f * xs + 2.4f) * (2.0f - xs) - 6.4f) * (2.0f - xs) + 3.2f;
    float y0 = ((-0.8f * ys + 3.2f) * (ys + 1.0f) - 6.4f) * (ys + 1.0f) + 3.2f;
    float y1 = (1.2f * ys - 2.2f) * ys * ys + 1.0f;
    float y2 = (-1.2f * ys - 1.0f) * (1.0f - ys) * (1.0f - ys) + 1.0f;
    float y3 = ((0.8f * ys + 2.4f) * (2.0f - ys) - 6.4f) * (2.0f - ys) + 3.2f;

    int lmW = cm.LMWidth;
    int stride = lmW + 1;

    // Wrap coordinates
    auto wrapX = [lmW](int xi) -> int { return ((xi % lmW) + lmW) % lmW; };
    auto wrapY = [&](int yi) -> int {
        if (yi < 0) return 0;
        if (yi >= cm.LMHeight) return cm.LMHeight - 1;
        return yi;
    };

    // Adjust yf to start one row above
    if (yf > 0) yf--; else yf += cm.LMHeight - 1;

    TSVec svy[4];
    T4SVec sv;

    for (int row = 0; row < 4; row++) {
        int cy = wrapY(yf);
        LM_ColToSVec(sv[0], cm.LMa[cy * stride + wrapX(xf - 1)]);
        LM_ColToSVec(sv[1], cm.LMa[cy * stride + xf]);
        LM_ColToSVec(sv[2], cm.LMa[cy * stride + wrapX(xf + 1)]);
        LM_ColToSVec(sv[3], cm.LMa[cy * stride + wrapX(xf + 2)]);

        svy[row][0] = sv[0][0] * x0 + sv[1][0] * x1 + sv[2][0] * x2 + sv[3][0] * x3;
        svy[row][1] = sv[0][1] * x0 + sv[1][1] * x1 + sv[2][1] * x2 + sv[3][1] * x3;
        svy[row][2] = sv[0][2] * x0 + sv[1][2] * x1 + sv[2][2] * x2 + sv[3][2] * x3;
        svy[row][3] = 0.0f;

        yf++;
        if (yf >= cm.LMHeight) yf = 0;
    }

    result[0] = (svy[0][0] * y0 + svy[1][0] * y1 + svy[2][0] * y2 + svy[3][0] * y3) * s1d255;
    result[1] = (svy[0][1] * y0 + svy[1][1] * y1 + svy[2][1] * y2 + svy[3][1] * y3) * s1d255;
    result[2] = (svy[0][2] * y0 + svy[1][2] * y1 + svy[2][2] * y2 + svy[3][2] * y3) * s1d255;
}

// ============================================================================
// GetMapPixelDirectXY -- direct XY with cos-approximation interpolation
// ============================================================================

void GetMapPixelDirectXY(TVec3D& result, TPVec3D pVec3D, int32_t mapNr) {
    int n = FindOrLoadMap(mapNr);
    if (n < 0) {
        ClearDVec(result);
        return;
    }

    TLightMap& cm = GetCalcMaps()[n];
    if (cm.LMWidth <= 0) {
        ClearDVec(result);
        return;
    }

    double xx = FracA((*pVec3D)[0]) * cm.sLMXfactor;
    double yy = FracA((*pVec3D)[1]) * cm.sLMYfactor;

    int xf = static_cast<int>(xx);
    double xs = xx - xf;
    if (xf >= cm.LMWidth) xf = 0;

    int yf = static_cast<int>(yy);
    double ys = yy - yf;
    if (yf >= cm.LMHeight) yf = 0;

    // Cos-approximation interpolation
    if (xs > 0.5) xs = 1.0 - (1.0 - xs) * (1.0 - xs) * 2.0;
    else xs = xs * xs * 2.0;
    if (ys > 0.5) ys = (1.0 - (1.0 - ys) * (1.0 - ys) * 2.0) * s1d255;
    else ys = ys * ys * 2.0 * s1d255;
    double yyW = s1d255 - ys;

    int stride = cm.LMWidth + 1;

    T4SVec sv;
    int xf1 = (xf + 1 >= cm.LMWidth) ? 0 : xf + 1;
    int yf1 = (yf + 1 >= cm.LMHeight) ? 0 : yf + 1;

    LM_ColToSVec(sv[0], cm.LMa[yf * stride + xf]);
    LM_ColToSVec(sv[1], cm.LMa[yf * stride + xf1]);
    LM_ColToSVec(sv[2], cm.LMa[yf1 * stride + xf]);
    LM_ColToSVec(sv[3], cm.LMa[yf1 * stride + xf1]);

    result[0] = (sv[0][0] + (sv[1][0] - sv[0][0]) * xs) * yyW + (sv[2][0] + (sv[3][0] - sv[2][0]) * xs) * ys;
    result[1] = (sv[0][1] + (sv[1][1] - sv[0][1]) * xs) * yyW + (sv[2][1] + (sv[3][1] - sv[2][1]) * xs) * ys;
    result[2] = (sv[0][2] + (sv[1][2] - sv[0][2]) * xs) * yyW + (sv[2][2] + (sv[3][2] - sv[2][2]) * xs) * ys;
}

// ============================================================================
// GetLightMapPixel -- spline-interpolated lightmap pixel at (x,y) in [0,1]
// ============================================================================

void GetLightMapPixel(TSVec& result, float x, float y, TPLightMap lm,
                      LongBool bSqr, int wrapAround) {
    if (!lm || !lm->LMa || lm->LMWidth <= 4) {
        ClearSVec(result);
        return;
    }

    float xs = Clamp01S(x) * lm->sLMXfactor;
    float ys = Clamp01S(y) * lm->sLMYfactor;

    int lmW = lm->LMWidth;
    int lmH = lm->LMHeight;

    int xf = std::min(lmW - 1, static_cast<int>(xs));
    int yf = std::min(lmH - 1, static_cast<int>(ys));

    TSVec xv, yv;
    MakeSplineCoeff(xs - xf, xv);
    MakeSplineCoeff(ys - yf, yv);

    // Horizontal neighbor offsets with wrapping
    int nx1, nx2, nx3;
    if (wrapAround > 0) {
        nx1 = (xf > 0) ? -1 : (lmW - 1);
        nx2 = (xf < lmW - 1) ? 2 : (2 - lmW);
        nx3 = (xf < lmW) ? 1 : (1 - lmW);
    } else {
        nx1 = (xf > 0) ? -1 : 0;
        nx3 = (xf < lmW - 1) ? 1 : 0;
        nx2 = (xf < lmW - 2) ? 2 : (nx3);
    }

    // Adjust yf for vertical centering of 4-tap filter
    if (wrapAround == 1) {
        if (yf > 0) yf--; else yf += lmH - 1;
    } else {
        yf--;
    }

    TSVec svy[4];
    const int columns[4] = {xf + nx1, xf, xf + nx3, xf + nx2};

    for (int row = 0; row < 4; row++) {
        int cy;
        if (yf < 0) {
            cy = (wrapAround == 1) ? (lmH + yf) : 0;
        } else if (yf >= lmH) {
            cy = (wrapAround == 1) ? (yf % lmH) : (lmH - 1);
        } else {
            cy = yf;
        }

        // Include the real extra seam column, including P2's contiguous layout.
        SampleLightMapRow(svy[row], lm, cy, columns, xv, bSqr != 0);

        yf++;
    }

    // Combine rows
    result[0] = (svy[0][0] * yv[0] + svy[1][0] * yv[1] + svy[2][0] * yv[2] + svy[3][0] * yv[3]);
    result[1] = (svy[0][1] * yv[0] + svy[1][1] * yv[1] + svy[2][1] * yv[2] + svy[3][1] * yv[3]);
    result[2] = (svy[0][2] * yv[0] + svy[1][2] * yv[1] + svy[2][2] * yv[2] + svy[3][2] * yv[3]);
    result[3] = 0.0f;

    // Apply intensity scaling
    result[0] *= lm->sIntensity;
    result[1] *= lm->sIntensity;
    result[2] *= lm->sIntensity;
}

// ============================================================================
// GetLightMapPixelSphere
// ============================================================================

void GetLightMapPixelSphere(TSVec& result, const TSVec& svec,
                            const TSMatrix3* SM, TLightMap& lm, LongBool bSqr) {
    TSVec sv;
    NormalizeSVec(sv, svec);
    if (SM) {
        RotateSVectorS(&sv, const_cast<TSMatrix3*>(SM));
    }

    float u = static_cast<float>(std::atan2(sv[0], sv[2]) * MPi05d + 0.5);
    float v = static_cast<float>(0.5 - ArcSinSafe(static_cast<double>(sv[1])) * Pi1d);

    GetLightMapPixel(result, u, v, &lm, bSqr, 2);
}

void GetLightMapPixelNN(TSVec& result, float x, float y, TPLightMap lm, LongBool bSqr) {
    ClearSVec(result);
    if (!lm || !lm->LMa || lm->LMWidth <= 0 || lm->LMHeight <= 0) return;
    const int ix = std::clamp(int(std::lrint(Clamp01S(x) * lm->sLMXfactor)), 0, lm->LMWidth);
    const int iy = std::clamp(int(std::lrint(Clamp01S(y) * lm->sLMYfactor)), 0, lm->LMHeight);
    const size_t pixel = size_t(iy) * (lm->LMWidth + 1) + ix;
    if (lm->iMapType == 1) {
        uint16_t words[3];
        std::memcpy(words, reinterpret_cast<const uint8_t*>(lm->LMa) + pixel * 6, 6);
        for (int c = 0; c < 3; ++c) {
            double v = words[2 - c] / 65535.0;
            result[c] = float((bSqr ? v * v : v) * lm->sIntensity);
        }
    } else {
        LM_ColToSVecFlipRB(result, lm->LMa[pixel]);
        for (int c = 0; c < 3; ++c) {
            if (bSqr) result[c] = result[c] * result[c] * s1d255;
            result[c] *= s1d255 * lm->sIntensity;
        }
    }
}

void GetLightMapPixelSphereNN(TSVec& result, const TSVec& direction,
                             const TSMatrix3* rotation, TLightMap& lm, LongBool bSqr) {
    TSVec sv;
    NormalizeSVec(sv, direction);
    if (rotation) RotateSVectorS(&sv, const_cast<TSMatrix3*>(rotation));
    GetLightMapPixelNN(result, float(std::atan2(sv[0], sv[2]) * MPi05d + .5),
                      float(.5 - ArcSinSafe(double(sv[1])) * Pi1d), &lm, bSqr);
}

// ============================================================================
// GetDiffMapWrap3D
// ============================================================================

void GetDiffMapWrap3D(TSVec& result, TLightVals* plVals, TSVec* lns,
                      TPPaintLightVals plv) {
    if (!plVals || !plv) {
        ClearSVec(result);
        return;
    }

    // SV1 = (ObjPos + lvMidPos) * lvMapScale
    TVec3D sv1;
    SVecToDVec(sv1, plv->ObjPos);
    sv1[0] = (sv1[0] + plVals->lvMidPos[0]) * plVals->lvMapScale;
    sv1[1] = (sv1[1] + plVals->lvMidPos[1]) * plVals->lvMapScale;
    sv1[2] = (sv1[2] + plVals->lvMidPos[2]) * plVals->lvMapScale;

    // SV2 = normal rotated and normalized by abs sum
    TSVec sv2;
    std::memcpy(sv2, lns, sizeof(TSVec));
    RotateSVectorS(&sv2, plv->PSmatrix);
    float absSum = std::fabs(sv2[0]) + std::fabs(sv2[1]) + std::fabs(sv2[2]);
    if (absSum > 1e-30f) {
        float invSum = 1.0f / absSum;
        sv2[0] *= invSum;
        sv2[1] *= invSum;
        sv2[2] *= invSum;
    }
    AbsSVecVar(sv2);

    if (plVals->iColOnOT > 7) {
        // 3-projection box mapping
        if (sv1[0] < 0) sv1[0] = sv1[0] + 1.0 - std::round(sv1[0]);
        if (sv1[1] < 0) sv1[1] = sv1[1] + 1.0 - std::round(sv1[1]);
        if (sv1[2] < 0) sv1[2] = sv1[2] + 1.0 - std::round(sv1[2]);

        TSVec lm1, lm2, lm3;
        float f1y = static_cast<float>(sv1[1]) + plVals->DCLMapOffX;
        float f1x = static_cast<float>(sv1[2]) + plVals->DCLMapOffY;
        GetLightMapPixel(lm1, std::fmod(f1y - std::floor(f1y) + 1.0f, 1.0f),
                              std::fmod(f1x - std::floor(f1x) + 1.0f, 1.0f),
                              plVals->DiffColLightMap, plVals->bCalcPixColSqr, 1);
        ScaleSVec_inplace(lm1, sv2[0]);

        float f2y = static_cast<float>(sv1[0]) + plVals->DCLMapOffX;
        float f2x = static_cast<float>(sv1[2]) + plVals->DCLMapOffY;
        GetLightMapPixel(lm2, std::fmod(f2y - std::floor(f2y) + 1.0f, 1.0f),
                              std::fmod(f2x - std::floor(f2x) + 1.0f, 1.0f),
                              plVals->DiffColLightMap, plVals->bCalcPixColSqr, 1);
        ScaleSVec_inplace(lm2, sv2[1]);

        float f3y = static_cast<float>(sv1[0]) + plVals->DCLMapOffX;
        float f3x = static_cast<float>(sv1[1]) + plVals->DCLMapOffY;
        GetLightMapPixel(lm3, std::fmod(f3y - std::floor(f3y) + 1.0f, 1.0f),
                              std::fmod(f3x - std::floor(f3x) + 1.0f, 1.0f),
                              plVals->DiffColLightMap, plVals->bCalcPixColSqr, 1);
        ScaleSVec_inplace(lm3, sv2[2]);

        Add3SVectors(result, lm1, lm2, lm3);
    } else {
        // Sin-weighted projection
        sv1[0] = std::sin(sv1[0]);
        sv1[1] = std::sin(sv1[1]);
        sv1[2] = std::sin(sv1[2]);

        double coordX = sv1[1] * sv2[0] + sv1[0] * (sv2[1] + sv2[2]) + plVals->DCLMapOffX;
        double coordY = sv1[2] * (sv2[0] + sv2[1]) + sv1[1] * sv2[2] + plVals->DCLMapOffY;

        if (coordX < 0) coordX = coordX + 1.0 - std::round(coordX);
        if (coordY < 0) coordY = coordY + 1.0 - std::round(coordY);

        float fx = static_cast<float>(coordX);
        float fy = static_cast<float>(coordY);
        GetLightMapPixel(result,
                         fx - std::floor(fx),
                         fy - std::floor(fy),
                         plVals->DiffColLightMap, plVals->bCalcPixColSqr, 1);
    }
}

// ============================================================================
// TLMSfunction wrappers
// ============================================================================

void MapFuncSphereSpline(TPVec3D resultVec, TPVec3D pVec3D, int32_t mapNr) {
    TVec3D res;
    GetMapPixelSphereSpline(res, pVec3D, mapNr);
    std::memcpy(*resultVec, res, sizeof(TVec3D));
}

void MapFuncDirectXYspline(TPVec3D resultVec, TPVec3D pVec3D, int32_t mapNr) {
    TVec3D res;
    GetMapPixelDirectXYspline(res, pVec3D, mapNr);
    std::memcpy(*resultVec, res, sizeof(TVec3D));
}

} // namespace mb3d
