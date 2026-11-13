#include "core/header_trafos.h"
#include "core/math3d.h"
#include "core/renderer.h"
#include "io/m3p_parser.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <stdexcept>

#include "fixtures/direct_header_camera_tile_setup.h"

namespace {
using namespace mb3d;
void check(bool pass, const char* message) {
    if (!pass) throw std::runtime_error(message);
}

TMandHeader10 header() {
    TMandHeader10 h{};
    h.Width = 97; h.Height = 73; h.dZoom = 1.25;
    h.dXmid = 0.125; h.dYmid = -0.25; h.dZmid = 0.75;
    h.hVGrads[0][0] = 0.3; h.hVGrads[0][1] = -0.4; h.hVGrads[0][2] = 0.5;
    h.hVGrads[1][1] = h.hVGrads[2][2] = 1.0;
    h.Iterations = h.iMaxItsF2 = 20; h.MinimumIterations = 1;
    h.RStop = 16; h.dFOVy = 45; h.bCalc3D = 1; h.sDEstop = 0.001f;
    h.mZstepDiv = h.sRaystepLimiter = 1.0f;
    return h;
}

int testTileRendering() {
    const auto scene = std::filesystem::path(__FILE__).parent_path().parent_path() /
        "comparison/scenes_phase2/phase2_intpow2_small.m3p";
    const auto loaded = LoadM3PFile(scene.string());
    check(loaded.success, "tile control scene did not load");
    RenderOptions opts;
    opts.thread_count = 1;
    opts.no_ao = opts.no_dof = opts.no_reflections = opts.no_shadows = true;
    opts.deterministic_mode = true;
    constexpr int tileOptions = 2 | (2 << 7) | (1 << 14) | (1 << 21) | (1 << 28);
    int scenarios = 0;
    for (int mode = 0; mode < 4; ++mode) {
        auto h = loaded.header;
        auto addon = loaded.addon;
        h.Width = 32; h.Height = 24; h.TilingOptions = 0;
        h.byCalcNsOnZBufAuto = 0;
        h.bPlanarOptic = mode < 3 ? mode : 0;
        h.bCalc3D = mode < 3 ? 1 : 0;
        const auto full = Render(h, addon, opts);
        check(full.success, "full-frame tile control failed");
        h.TilingOptions = tileOptions;
        const auto tile = Render(h, addon, opts);
        check(tile.success && tile.width == 18 && tile.height == 14 &&
                  h.Width == 32 && h.Height == 24, "tile dimensions or full-frame header changed");
        for (int y = 0; y < 13; ++y) {
            for (int x = 0; x < 17; ++x) {
                const size_t ti = y * tile.width + x;
                const size_t fi = (y + 11) * full.width + x + 15;
                check(std::memcmp(&tile.silight[ti], &full.silight[fi], sizeof(TsiLight5)) == 0,
                      "tile primary state differs from corresponding full-frame pixel");
                check(std::memcmp(&tile.image[ti * 3], &full.image[fi * 3], 3) == 0,
                      "tile paint differs from corresponding full-frame pixel");
            }
        }
        const auto replay = PaintFromSiLight(h, addon, tile.silight, opts);
        check(replay.success && replay.width == tile.width && replay.height == tile.height &&
                  replay.image == tile.image, "tile paint replay differs from render");
        ++scenarios;
    }
    auto h = loaded.header;
    auto addon = loaded.addon;
    h.Width = 32; h.Height = 24; h.TilingOptions = tileOptions;
    h.bCalc3D = 1; h.byCalcNsOnZBufAuto = 0;
    h.bCalcDOFtype = 1;
    auto enabled = opts; enabled.no_dof = false;
    auto rejected = Render(h, addon, enabled);
    check(!rejected.success && rejected.image.empty() && rejected.silight.empty() &&
              rejected.error.find("Unsupported DOF with tiling") != std::string::npos,
          "undefined original tile DOF must fail explicitly without partial output");
    for (int ao : {1, 5, 9}) {
        h.bCalcAmbShadowAutomatic = ao;
        enabled = opts; enabled.no_ao = false;
        rejected = Render(h, addon, enabled);
        check(!rejected.success && rejected.image.empty() && rejected.silight.empty() &&
                  rejected.error.find("Unsupported screen-space AO with tiling") != std::string::npos,
              "undefined original tile screen-space AO must fail explicitly");
        ++scenarios;
    }
    const auto recovered = Render(h, addon, opts);
    check(recovered.success, "tile render did not recover with undefined postprocessors disabled");
    enabled = opts; enabled.no_dof = false;
    rejected = PaintFromSiLight(h, addon, recovered.silight, enabled);
    check(!rejected.success && rejected.error.find("Unsupported DOF with tiling") != std::string::npos,
          "paint replay must reject undefined original tile DOF");
    // A 1x1 tile plus overlap has enough storage for Pascal's full-frame
    // postprocessor prefix. Its remaining suffix must survive DOF unchanged.
    h.TilingOptions = 1 | (1 << 7) | (1 << 28);
    const auto safeControl = Render(h, addon, opts);
    check(safeControl.success && safeControl.width == 34 && safeControl.height == 26,
          "safe-capacity tile control failed");
    for (int dof : {1, 9}) {
        h.bCalcDOFtype = dof;
        enabled = opts; enabled.no_dof = false;
        const auto safe = Render(h, addon, enabled);
        const auto replay = PaintFromSiLight(h, addon, safeControl.silight, enabled);
        check(safe.success && replay.success && safe.image == replay.image,
              "safe-capacity tile DOF render/replay failed");
        check(std::equal(safe.image.begin() + 32 * 24 * 3, safe.image.end(),
                         safeControl.image.begin() + 32 * 24 * 3),
              "tile DOF modified storage after Pascal's full-frame prefix");
        ++scenarios;
    }
    for (int ao : {1, 5, 9}) {
        h.bCalcAmbShadowAutomatic = ao;
        enabled = opts; enabled.no_ao = false;
        const auto safe = Render(h, addon, enabled);
        check(safe.success && safe.silight.size() == safeControl.silight.size(),
              "safe-capacity tile screen-space AO failed");
        for (size_t i = 32 * 24; i < safe.silight.size(); ++i) {
            auto expected = safeControl.silight[i];
            if (ao == 9) expected.AmbShadow = 0; // Original HiQ clears all allocation.
            check(std::memcmp(&safe.silight[i], &expected, sizeof(expected)) == 0,
                  "tile AO suffix differs from Pascal's prefix/clear contract");
        }
        ++scenarios;
    }
    h.TilingOptions = 1 << 28;
    rejected = Render(h, addon, opts);
    check(!rejected.success && rejected.error.find("Invalid tile count") != std::string::npos,
          "zero tile counts must fail explicitly");
    return scenarios + 4;
}
}

int main() {
    try {
        THeaderCustomAddon addon{};
        GetHAddOnFromInternFormula(&addon, 0, 0);
        addon.Formulas[0].iItCount = 1;
        int scenarios = 0;
        for (const auto& f : kStereoSetupFixtures) {
            auto h = header();
            h.bStereoMode = static_cast<uint8_t>(f.mode);
            h.StereoScreenWidth = f.config == 1 ? 2.3f : 0.45f;
            h.StereoScreenDistance = f.config == 0 ? 0.65f : 1.25f;
            h.StereoMinDistance = f.config == 0 ? 0.08f : f.config == 1 ? 0.25f : 1.25f;
            float xoff = CalcXoff(&h);
            uint32_t bits; std::memcpy(&bits, &xoff, sizeof(bits));
            check(bits == f.xoff, "direct-header stereo screen offset differs from Pascal");
            const auto mct = GetMCTparasFromHeader(h, &addon, 0);
            check(mct.bMCTisValid, "direct-header stereo setup is invalid");
            const double mid[] = {mct.Xmit, mct.Ymit, mct.Zmit};
            for (int i = 0; i < 3; ++i) {
                double expected; std::memcpy(&expected, &f.mid[i], sizeof(expected));
                check(std::abs(mid[i] - expected) <= 2e-15 * std::max(1.0, std::abs(expected)),
                      "direct-header stereo camera position differs from Pascal");
            }
            ++scenarios;
        }
        for (const auto& f : kTileSetupFixtures) {
            auto h = header();
            h.TilingOptions = f.options;
            int w, hei, x, y;
            GetPaintTileSizes(&h, w, hei, x, y);
            check(w == f.w && hei == f.h && x == f.x && y == f.y,
                  "tile paint size or pixel offsets differ from Pascal");
            const auto mct = GetMCTparasFromHeader(h, &addon, 0);
            check(mct.CalcRect.Left == f.left && mct.CalcRect.Top == f.top &&
                      mct.CalcRect.Right == f.right && mct.CalcRect.Bottom == f.bottom &&
                      mct.SLoffset == f.tileW * static_cast<int>(sizeof(TsiLight5)),
                  "tile calculation rectangle or packed stride differs from Pascal");
            ++scenarios;
        }
        const int renderScenarios = testTileRendering();
        std::printf("Camera/tile setup: %d Pascal fixture cases and %d renderer cases passed.\n",
                    scenarios, renderScenarios);
        return 0;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Camera/tile setup FAILED: %s\n", e.what());
        return 1;
    }
}
