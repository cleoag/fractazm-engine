#include "io/m3p_parser.h"

#include <array>
#include <barrier>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>
#include <vector>
#include <zlib.h>

namespace fs = std::filesystem;
using Bytes = std::vector<uint8_t>;

static void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

static Bytes read(const fs::path& path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    check(in.is_open(), "missing independent M3I fixture");
    const auto size = in.tellg();
    check(size >= 0, "cannot size M3I fixture");
    Bytes bytes(static_cast<size_t>(size));
    in.seekg(0);
    in.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    check(in.gcount() == static_cast<std::streamsize>(bytes.size()), "short fixture read");
    return bytes;
}

static bool same(const Bytes& expected, const void* actual, size_t size) {
    return expected.size() == size && (size == 0 || std::memcmp(expected.data(), actual, size) == 0);
}

static void verify(const fs::path& fixtures, const char* name, bool success, bool image) {
    const auto result = mb3d::LoadM3IFile((fixtures / (std::string(name) + ".m3i")).string());
    check(result.success == success, "M3I success differs from original raw reader");
    if (!success) {
        check(!result.has_silight && !result.has_image && result.siLight.empty() && result.image.empty(),
              "invalid M3I exposed a partial cache");
        check(!result.error.empty(), "invalid M3I has no diagnostic");
        return;
    }
    check(result.has_silight, "valid M3I has no lighting data");
    check(same(read(fixtures / (std::string(name) + ".silight")), result.siLight.data(),
               result.siLight.size() * sizeof(TsiLight5)), "M3I packed state differs from native Pascal fixture");
    check(result.has_image == image, "optional-image presence differs from Pascal");
    if (image)
        check(same(read(fixtures / (std::string(name) + ".image")), result.image.data(), result.image.size() * 4),
              "M3I image differs from native Pascal fixture");
    else check(result.image.empty(), "absent optional image has partial storage");
}

static void write(const fs::path& path, const Bytes& bytes) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    check(out.good(), "failed to write temporary M3I input");
}

static void reject(const fs::path& path, const Bytes& bytes) {
    write(path, bytes);
    const auto result = mb3d::LoadM3IFile(path.string());
    check(!result.success && !result.error.empty() && !result.has_silight && !result.has_image &&
          result.siLight.empty() && result.image.empty(), "invalid M3I was accepted or allocated a partial result");
}

int main() {
    const fs::path fixtures = fs::path(MB3D_TEST_SOURCE_DIR) / "tests/reference/m3i_codec";
    const fs::path scratch = fs::temp_directory_path() / "mb3d_m3i_parser_test";
    try {
        // First production conversion in this process: all lanes start cold.
        // Golden 89-record fixture contains every mismatching half-tie found by
        // the independent 32768-angle native Pascal scan, plus quadrant edges.
        std::barrier start(12);
        std::array<std::exception_ptr, 12> errors{};
        std::vector<std::jthread> workers;
        for (size_t lane = 0; lane < errors.size(); ++lane) {
            workers.emplace_back([&, lane] {
                start.arrive_and_wait();
                try {
                    for (int repeat = 0; repeat < 3; ++repeat)
                        verify(fixtures, "v18_rounding_boundaries", true, true);
                } catch (...) { errors[lane] = std::current_exception(); }
            });
        }
        for (auto& worker : workers) worker.join();
        for (const auto& error : errors) if (error) std::rethrow_exception(error);

        const char* exact[] = {"v00_records", "v04_records", "v07_records", "v16_records", "v17_records",
            "v18_records", "v19_records", "v20_records", "v35_records", "v44_records",
            "v16_hard_shadow", "v18_hard_shadow", "v19_hard_shadow", "v35_dfog",
            "v44_tile_3x2", "v44_tile_1x1"};
        for (const char* name : exact) verify(fixtures, name, true, true);
        for (const char* name : {"v44_no_image", "v16_old_addon_no_image", "v16_truncated_image",
                                "v18_truncated_image", "v44_truncated_image"})
            verify(fixtures, name, true, false);
        for (const char* name : {"v16_truncated_pixel", "v18_truncated_pixel", "v44_truncated_pixel",
            "v16_parameter_only_as_cache", "v18_parameter_only_as_cache", "v44_parameter_only_as_cache"}) {
            verify(fixtures, name, false, false);
            verify(fixtures, "v18_rounding_boundaries", true, true);
        }

        fs::create_directories(scratch);
        const auto temp = scratch / "input.m3i";
        auto bytes = read(fixtures / "v44_records.m3i");
        // The independent GetTileSize + LoadFullM3I oracle confirms both axes
        // clamp to30000, even untiled. SetImageSize still allocates30001 image
        // pixels, so the optional image must not be loaded when lengths differ.
        for (int axis : {0, 1}) {
            const auto original = bytes;
            Bytes wide(original.begin(), original.begin() + 840);
            const int32_t dimensions[] = {axis == 0 ? 30001 : 1, axis == 0 ? 1 : 30001};
            std::memcpy(wide.data() + 4, dimensions, sizeof(dimensions));
            for (int i = 0; i < 30000; ++i)
                wide.insert(wide.end(), original.begin() + 840, original.begin() + 858);
            wide.insert(wide.end(), original.begin() + 840 + 48 * 18, original.begin() + 840 + 48 * 18 + 1136);
            wide.resize(wide.size() + 30000 * 4, 0xA7);
            write(temp, wide);
            const auto result = mb3d::LoadM3IFile(temp.string());
            check(result.success && result.has_silight && result.siLight.size() == 30000,
                  "untiled legacy dimension clamp mismatch");
            check(!result.has_image && result.image.empty(), "image loaded despite Pascal allocation-length mismatch");
            for (const auto& pixel : result.siLight)
                check(std::memcmp(&pixel, original.data() + 840, 18) == 0, "clamped cache changed a packed record");
        }
        // Short files with huge declared dimensions must fail before allocation.
        const int32_t huge = std::numeric_limits<int32_t>::max();
        std::memcpy(bytes.data() + 4, &huge, 4);
        std::memcpy(bytes.data() + 8, &huge, 4);
        reject(temp, bytes);

        bytes = read(fixtures / "v44_records.m3i");
        const int32_t zeroTileDivisor = 1 << 7;
        std::memcpy(bytes.data() + 428, &zeroTileDivisor, 4);
        reject(temp, bytes);

        // zlib parameters are a LoadM3P extension, not raw pixel records.
        bytes = read(fixtures / "v44_records.m3i");
        uLongf compressedSize = compressBound(static_cast<uLong>(bytes.size()));
        Bytes compressed(compressedSize);
        check(compress2(compressed.data(), &compressedSize, bytes.data(), static_cast<uLong>(bytes.size()),
                        Z_BEST_SPEED) == Z_OK, "compression fixture failed");
        compressed.resize(compressedSize);
        reject(temp, compressed);
        verify(fixtures, "v44_records", true, true);
        fs::remove(temp);
        fs::remove(scratch);
        std::cout << "PASS: 28 independent legacy M3I fixtures, 2 dimension-clamp witnesses, 36 cold/warm concurrent loads, "
                     "3 bounded malformed-input checks and recovery\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
