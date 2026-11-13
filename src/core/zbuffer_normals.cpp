#include "core/render_control.h"
#include "core/zbuffer_normals.h"
#include "core/colorizer.h"
#include "core/header_trafos.h"
#include "core/math3d.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace mb3d {
namespace {
using Vec = std::array<float, 4>;
constexpr float missing = -1e20f;

Vec subtract(const Vec& a, const Vec& b) {
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2], 0};
}

float squaredLength(const Vec& v) {
    // The Pascal helper sums in x87 before returning/storing a Single.
    const double x = v[0], y = v[1], z = v[2];
    return static_cast<float>(x*x + y*y + z*z);
}

Vec combine(const Vec& a, const Vec& b, float distance) {
    distance *= distance;
    float wa = 1.0f / (squaredLength(a) + distance);
    float wb = 1.0f / (squaredLength(b) + distance);
    const float sumInverse = 1.0f / (wa + wb);
    wa *= sumInverse;
    wb *= sumInverse;
    // Preserve both ScaleSVector Single stores before AddSVectors.
    return {a[0]*wa + b[0]*wb, a[1]*wa + b[1]*wb,
            a[2]*wa + b[2]*wb, 0};
}

float decodeDepth(const TsiLight5& pixel, double inverseZMul, double correction) {
    const int depth = (static_cast<int>(pixel.Zpos) << 8) |
                      (pixel.RoughZposFine >> 8);
    const double z = (8388352 - depth) * inverseZMul + 1.0;
    return static_cast<float>((z*z - 1.0) * correction);
}
} // namespace

void RecalculateNormalsFromZBuffer(const TMandHeader10& header,
                                  std::span<TsiLight5> surface) {
    int width, height, left, top;
    GetPaintTileSizes(&header, width, height, left, top);
    if (header.Width <= 0 || header.Height <= 0 || width <= 0 || height <= 0 ||
        static_cast<size_t>(width) * height > surface.size()) {
        throw std::invalid_argument("Normals-on-Z requires a complete calculation rectangle");
    }
    const float step = static_cast<float>(header.dStepWidth);
    const float aspect = header.bPlanarOptic == 2 ? 2.0f :
        static_cast<float>(header.Width) / static_cast<float>(header.Height);
    TPaintLightVals view{};
    view.iPlanarOptic = header.bPlanarOptic & 3;
    view.sFOVy = static_cast<float>(header.bPlanarOptic == 2 ? Pi : header.dFOVy * Pid180);
    view.xOff = 0.5f;
    const double halfFov = std::clamp(static_cast<double>(view.sFOVy * 0.5f), 0.01, 1.5);
    view.PlOpticZ = static_cast<float>(std::cos(halfFov) * halfFov / std::sin(halfFov));
    double zCorrection, zMultiplier, unused;
    TMandHeader10 projectionHeader = header;
    CalcPPZvals(projectionHeader, zCorrection, zMultiplier, unused);
    const double correction = step / zCorrection;
    const double inverseZMul = 1.0 / zMultiplier;
    const float stopFactor = static_cast<float>(GetDEstopFactor(&projectionHeader));
    const int stride = width + 1;
    std::vector<Vec> rows(static_cast<size_t>(stride) * 3, Vec{missing, 0, 0, 0});

    auto reconstruct = [&](int sourceY, int x, float xposY) -> Vec {
        RenderWork();
        if (sourceY >= height) return {missing, 0, 0, 0};
        const auto& pixel = surface[static_cast<size_t>(sourceY) * width + x];
        if (pixel.Zpos >= 32768 || pixel.SIgradient >= 32768) return {missing, 0, 0, 0};
        view.xPos = static_cast<float>(x + 1 + left) / static_cast<float>(header.Width);
        view.yPos = static_cast<float>(sourceY + top) / static_cast<float>(header.Height);
        CalcViewVec(&view, aspect);
        const float depth = decodeDepth(pixel, inverseZMul, correction);
        const float xposX = view.iPlanarOptic == 2 ? 0.0f : (x + left) * step;
        const float ypos = view.iPlanarOptic == 2 ? 0.0f : xposY;
        return {view.ViewVec[0]*depth + xposX, view.ViewVec[1]*depth + ypos,
                view.ViewVec[2]*depth, 0};
    };

    // The reference's initial rows deliberately omit CalcR.Top in Xpos.Y;
    // later rows include it. Keep this distinction for tiled calculations.
    for (int y = 1; y <= 2; ++y)
        for (int x = 0; x < width; ++x)
            rows[x + y*stride] = reconstruct(y - 1, x, y*step);

    for (int y = 1; y <= height; ++y) {
        RenderPoll();
        for (int x = 0; x < width; ++x) {
            RenderWork();
            const Vec& center = rows[x + stride];
            if (!(center[0] > -1e19f)) continue;
            auto& pixel = surface[static_cast<size_t>(y - 1) * width + x];
            const float distance = step + decodeDepth(pixel, inverseZMul, correction) * stopFactor;
            auto tangent = [&](const Vec& before, const Vec& after, Vec fallback) {
                if (before[0] < -1e19f) {
                    return after[0] > -1e19f ? subtract(center, after) : fallback;
                }
                if (after[0] > -1e19f)
                    return combine(subtract(before, center), subtract(center, after), distance);
                return subtract(before, center);
            };
            const Vec ty = tangent(rows[x], rows[x + 2*stride], {0, 1, 0, 0});
            const Vec tx = tangent(rows[x + width], rows[x + 1 + stride], {1, 0, 0, 0});
            // Pascal SVectorCross returns Single components, then SVecToNormals
            // normalizes and packs them with x87 FISTP rounding.
            const float nx = ty[1]*tx[2] - ty[2]*tx[1];
            const float ny = ty[2]*tx[0] - ty[0]*tx[2];
            const float nz = ty[0]*tx[1] - ty[1]*tx[0];
            const TVec3D normal{nx, ny, nz};
            int16_t packed[3];
            MakeWNormalsFromDVec(packed, normal);
            pixel.NormalX = packed[0];
            pixel.NormalY = packed[1];
            pixel.NormalZ = packed[2];
        }
        for (int x = 0; x < width; ++x) {
            RenderWork();
            rows[x] = rows[x + stride];
            rows[x + stride] = rows[x + 2*stride];
            rows[x + 2*stride] = reconstruct(y + 1, x, (y + 2 + top)*step);
        }
    }
}
} // namespace mb3d
