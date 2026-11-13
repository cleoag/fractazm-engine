#include "core/render_context.h"
#include "core/lightmap.h"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <stdexcept>

static void check(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }

void TestBackgroundSearch() {
    namespace fs = std::filesystem;
    const auto root = fs::temp_directory_path() / ("mb3d-background-" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(root / "M3Maps");
    struct Cleanup { fs::path path; ~Cleanup() { std::error_code ec; fs::remove_all(path, ec); } } cleanup{root};
    auto pgm = [](const fs::path& path, unsigned char value) {
        std::ofstream stream(path, std::ios::binary);
        stream << "P5\n8 6\n255\n";
        for (int i = 0; i < 48; ++i) stream.put(static_cast<char>(value));
    };
    auto context = std::make_shared<mb3d::RenderContext>(root.string());
    mb3d::ScopedRenderContext binding(context);
    TLightMap map{};
    pgm(root / "M3Maps/sky.pgm", 24);
    check(!mb3d::LoadLightMapFile("sky.pgm", &map), "Exact file loader unexpectedly searches directories");
    check(mb3d::LoadBackgroundMapFile("sky.pgm", &map) && map.LMa && map.LMWidth >= 6,
          "Background basename did not resolve in M3Maps");
    const auto fromMaps = map.LMa[0];
    mb3d::FreeLightMap(&map);
    pgm(root / "sky.pgm", 200);
    check(mb3d::LoadBackgroundMapFile("sky.pgm", &map) && map.LMa[0] != fromMaps,
          "Explicit resource-root background lost precedence");
    mb3d::FreeLightMap(&map);
    check(!mb3d::LoadBackgroundMapFile("absent/sky.pgm", &map), "Explicit missing path substituted a basename");
    check(!mb3d::LoadBackgroundMapFile("missing.pgm", &map), "Missing background reported loaded");
    check(mb3d::LoadBackgroundMapFile("M3Maps/sky.pgm", &map) && map.LMa[0] == fromMaps,
          "Explicit relative background path changed");
    mb3d::FreeLightMap(&map);
}

int main() {
    try { TestBackgroundSearch(); std::puts("Background map resolution: PASS"); return 0; }
    catch (const std::exception& error) { std::fprintf(stderr, "%s\n", error.what()); return 1; }
}
