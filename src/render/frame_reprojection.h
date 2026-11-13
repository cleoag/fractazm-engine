#pragma once
#include "core/header_trafos.h"
#include "core/types.h"
#include "render/navigation_camera.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

// Internal: warp a delivered frame to another camera of the same scene using
// its packed depth. The MB3D camera starts rays on a plane (one StepWidth per
// pixel) and fans them out by FOVy / Height per pixel, so a point is projected
// back by a short Newton iteration instead of a pinhole division.
namespace mb3d::reprojection {

struct View {
    navigation::Vec right, down, forward, center;
    double offset = 0, step = 0, fan = 0; // fan: radians of ray angle per scene unit on the start plane
    double planarZ = 0;                   // z of the unnormalised planar ray; 0 for the angular optic
    double zCorrection = 0, zMultiplier = 0;
    int width = 0, height = 0;
    bool valid = false;

    View(TMandHeader10 header, int w, int h) : width(w), height(h) {
        if (!navigation::Camera::supports(header) || header.bPlanarOptic == 2 || w < 1 || h < 1) return;
        header.Width = w;
        header.Height = h;
        double unused;
        CalcPPZvals(header, zCorrection, zMultiplier, unused); // also sets dStepWidth
        const navigation::Camera camera(header);
        right = camera.right; down = camera.up; forward = camera.forward; center = camera.center;
        offset = camera.offset;
        step = header.dStepWidth;
        const double fov = header.dFOVy * Pid180;
        fan = fov / (h * step);
        if (header.bPlanarOptic == 1) {
            const double half = std::clamp(fov * 0.5, 0.01, 1.5);
            planarZ = std::cos(half) * half / std::sin(half);
        }
        valid = std::isfinite(step) && step > 0 && std::isfinite(fan) && zCorrection > 0 && zMultiplier > 0;
    }

    // One axis of the ray of a start-plane position: its sideways part and the
    // factor it contributes to the forward part.
    void axis(double position, double& side, double& along) const {
        if (planarZ > 0) { side = position * fan; along = 1; }
        else { side = std::sin(position * fan); along = std::cos(position * fan); }
    }
    // Unit ray in camera axes from the two axis parts.
    void ray(double sideU, double alongU, double sideV, double alongV, double out[3]) const {
        out[0] = sideU; out[1] = sideV; out[2] = planarZ > 0 ? planarZ : alongU * alongV;
        const double n = 1.0 / std::sqrt(out[0] * out[0] + out[1] * out[1] + out[2] * out[2]);
        out[0] *= n; out[1] *= n; out[2] *= n;
    }

    // Start-plane position whose ray passes through the camera-space point q (q[2] > 0).
    bool project(const double q[3], double& u, double& v) const {
        if (planarZ > 0) {
            const double scale = 1.0 / (1.0 + q[2] * fan / planarZ);
            u = q[0] * scale; v = q[1] * scale;
            return true;
        }
        const double limit = 1.5 / fan; // rays stay below 86 degrees off axis
        u = q[0] / (1.0 + q[2] * fan); v = q[1] / (1.0 + q[2] * fan);
        for (int i = 0; i < 4; ++i) { // u + z tan(fan u) / cos(fan v) = x, and the same for v
            u = std::clamp(u, -limit, limit); v = std::clamp(v, -limit, limit);
            const double cu = std::cos(u * fan), cv = std::cos(v * fan);
            const double tu = std::sin(u * fan) / cu, tv = std::sin(v * fan) / cv;
            const double du = (u + q[2] * tu / cv - q[0]) / (1.0 + q[2] * fan / (cu * cu * cv));
            const double dv = (v + q[2] * tv / cu - q[1]) / (1.0 + q[2] * fan / (cv * cv * cu));
            u -= du; v -= dv;
            if (std::abs(du) + std::abs(dv) < step * 0.01) break; // a hundredth of a pixel
        }
        return std::abs(u) < limit && std::abs(v) < limit;
    }
};

// Returns the fraction of `out` pixels a source pixel landed on, or a negative
// value when either camera is not modelled. `out` holds width*height*4 bytes.
inline double warp(const TMandHeader10& from, const TMandHeader10& to, const unsigned char* rgba,
                   const unsigned short* depth, int width, int height, unsigned char* out) {
    const View source(from, width, height), target(to, width, height);
    if (!source.valid || !target.valid) return -1;
    const size_t pixels = size_t(width) * height;
    constexpr float empty = -1, sky = std::numeric_limits<float>::max();
    std::vector<float> nearest(pixels, empty); // distance of the pixel's content along the target view
    std::memset(out, 0, pixels * 4);

    // Camera axes of the source expressed in the target's axes, and the offset between the origins.
    const navigation::Vec* sourceAxes[3] = {&source.right, &source.down, &source.forward};
    const navigation::Vec* targetAxes[3] = {&target.right, &target.down, &target.forward};
    double rotation[3][3], shift[3];
    for (int t = 0; t < 3; ++t) {
        shift[t] = 0;
        for (int i = 0; i < 3; ++i) shift[t] += (source.center[i] - target.center[i]) * (*targetAxes[t])[i];
        for (int s = 0; s < 3; ++s) {
            rotation[t][s] = 0;
            for (int i = 0; i < 3; ++i) rotation[t][s] += (*sourceAxes[s])[i] * (*targetAxes[t])[i];
        }
    }
    shift[2] -= target.offset;

    const double halfWidth = width * 0.5, halfHeight = height * 0.5;
    const double depthScale = source.step / source.zCorrection, inverseZMul = 1.0 / source.zMultiplier;
    std::vector<double> sideU(width), alongU(width);
    for (int x = 0; x < width; ++x) source.axis((x - halfWidth) * source.step, sideU[x], alongU[x]);
    size_t landed = 0;
    for (int y = 0; y < height; ++y) {
        const double v = (y - halfHeight) * source.step;
        double sideV, alongV;
        source.axis(v, sideV, alongV);
        for (int x = 0; x < width; ++x) {
            const size_t index = size_t(y) * width + x;
            size_t slot = index;
            float distance = sky;
            if (depth[index] <= 32767) {
                const double u = (x - halfWidth) * source.step;
                double direction[3];
                source.ray(sideU[x], alongU[x], sideV, alongV, direction);
                const double z = (8388352.0 - (double(depth[index]) * 256.0 + 128.0)) * inverseZMul + 1.0;
                const double along = (z * z - 1.0) * depthScale;
                const double p[3] = {u + direction[0] * along, v + direction[1] * along, source.offset + direction[2] * along};
                double q[3], tu, tv;
                for (int t = 0; t < 3; ++t)
                    q[t] = shift[t] + rotation[t][0] * p[0] + rotation[t][1] * p[1] + rotation[t][2] * p[2];
                if (!(q[2] > 0) || !target.project(q, tu, tv)) continue;
                const double fx = tu / target.step + halfWidth, fy = tv / target.step + halfHeight;
                if (!(fx > -1 && fy > -1 && fx < width + 1 && fy < height + 1)) continue;
                const int tx = int(std::lround(fx)), ty = int(std::lround(fy));
                if (tx < 0 || ty < 0 || tx >= width || ty >= height) continue;
                slot = size_t(ty) * width + tx;
                distance = float(q[2]);
            } // the background stays where it is: MB3D draws it in screen space
            if (nearest[slot] == empty) ++landed;
            else if (nearest[slot] <= distance) continue;
            nearest[slot] = distance;
            std::memcpy(out + slot * 4, rgba + index * 4, 4);
        }
    }
    if (!landed) return 0;

    // Pixels nothing landed on take the farthest filled neighbour, ring by
    // ring: what a moved camera uncovers lies behind the nearer surface.
    std::vector<size_t> holes, remaining;
    for (size_t i = 0; i < pixels; ++i) if (nearest[i] == empty) holes.push_back(i);
    std::vector<std::pair<size_t, size_t>> fills; // hole, donor
    for (int ring = 0; ring < 64 && !holes.empty(); ++ring) {
        fills.clear(); remaining.clear();
        for (const size_t hole : holes) {
            const int x = int(hole % width), y = int(hole / width);
            size_t donor = pixels;
            const auto offer = [&](bool inside, size_t candidate) {
                if (inside && nearest[candidate] != empty && (donor == pixels || nearest[candidate] > nearest[donor]))
                    donor = candidate;
            };
            offer(x > 0, hole - 1); offer(x + 1 < width, hole + 1);
            offer(y > 0, hole - width); offer(y + 1 < height, hole + width);
            if (donor == pixels) remaining.push_back(hole); else fills.push_back({hole, donor});
        }
        if (fills.empty()) break;
        // Applied after the scan, so a ring grows by one pixel from every side.
        for (const auto& fill : fills) std::memcpy(out + fill.first * 4, out + fill.second * 4, 4);
        for (const auto& fill : fills) nearest[fill.first] = nearest[fill.second];
        holes.swap(remaining);
    }
    return double(landed) / double(pixels);
}
} // namespace mb3d::reprojection
