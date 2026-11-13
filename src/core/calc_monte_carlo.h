#ifndef MB3D_CORE_CALC_MONTE_CARLO_H
#define MB3D_CORE_CALC_MONTE_CARLO_H

#include "core/types.h"
#include <array>
#include <cstdint>
#include <cmath>
#include <cstddef>

// Monte Carlo sampling and native headless rendering.
// Pascal reference: mb3d/CalcMonteCarlo.pas.
// MCoptions is a settings field, not an enable bit. Scope: docs/PARITY_TRACE.md.

namespace mb3d {

// Preserve the names AND types from TypeDefinitions.pas. The historical names
// have one more zero than their values; do not infer values from the names.
namespace mc_reference {
inline constexpr float s0001 = 0.001f;
inline constexpr float s001 = 0.01f;
}
inline bool MCDERefinementRequired(double distance, float stop) {
    return std::abs(distance - stop) > mc_reference::s0001;
}

// Halton sequence record â€” 65536 entries, precomputed once at startup.
// Matches Pascal HaltonRec (CalcMonteCarlo.pas:12-15).
#pragma pack(push, 1)
struct HaltonRec {
    uint16_t HRx;    // 0..65535, rect X
    uint16_t HRy;    // 0..65535, rect Y
    uint16_t HDx;    // 0..65535, base-5 lens sample before disc mapping
    uint16_t HDy;    // 0..65535, base-7 lens sample before disc mapping
};
#pragma pack(pop)

static_assert(sizeof(HaltonRec) == 8, "HaltonRec must be 8 bytes to match Pascal");

// Precompute the global Halton tables (idempotent; safe to call multiple
// times). Must be called before any Monte Carlo render invocation.
void PreComputeHaltonSequence();

// Access the immutable table, initializing it on first use (thread safe).
const HaltonRec* GetHaltonSequence();

// Pascal GetSubPixelShift, including rejection and random-stream consumption.
template<class Random>
void GetMCSubPixelShift(bool gaussian, Random&& next, float& x, float& y) {
    if (!gaussian) {
        x = next() - .5f;
        y = next() - .5f;
        return;
    }
    float r;
    do {
        x = next() * 2 - 1;
        y = next() * 2 - 1;
        r = x * x + y * y;
    } while (r > 1);
    // s001 is a typed Single in Pascal; 0.99 is an untyped real literal.
    r = r * .99 + .01f;
    r = std::sqrt(-2 * std::log(double(r)) / r) * .5;
    x *= r;
    y *= r;
}

// Low-level Halton helper â€” Van der Corput radical inverse of `index`
// in the given `base`. Used by PreComputeHaltonSequence and directly
// by CalcRay for deeper path stratification. Pascal: CalcMonteCarlo.pas
// `halton(index, base)` near line 120.
double HaltonRadicalInverse(int32_t index, int32_t base);

// Lens sampling preserves Pascal Single intermediates and sector boundaries.
void MakeDiscFromHalton(float& x, float& y);
float CalcBokeh(float x, float y, int mode);
struct MCLensSample {
    float x, y, focus, aperture, scale;
};
MCLensSample CalcMCLensSample(float u, float v, int mode, float aperture,
                             float focus, int width, double stepWidth);

// Horizontal arithmetic and FOVXmul are Single; vertical FOVy scaling is Double.
std::array<double, 2> CalcMCCameraAngles(float x, float y, int width, int height,
                                       float fovXmul, double fovY);
double MCNextRandom(uint32_t& state);
std::array<float, 2> GetMCHaltonShifts(uint32_t x, uint32_t y);
float ShiftMCHalton(uint16_t word, float shift);

// DivUtils.pas progressive accumulator encoding. Exactly three bytes are
// accessed, including for unaligned fields and the last field of a buffer.
double DecodeMC24(const uint8_t* bytes);
void EncodeMC24(double value, uint8_t* bytes);

#pragma pack(push, 1)
struct MCPackedRecord {
    uint8_t red[3], green[3], blue[3], ysum[3], ysqr[3];
    uint16_t rayCount;
    uint8_t zbyte;
};
#pragma pack(pop)
static_assert(sizeof(MCPackedRecord) == 18, "Pascal TMCrecord layout");
static_assert(offsetof(MCPackedRecord, ysum) == 9 &&
              offsetof(MCPackedRecord, rayCount) == 15 &&
              offsetof(MCPackedRecord, zbyte) == 17, "Pascal accumulator field offsets");
double NoiseOfMCRecord(const MCPackedRecord& record);

struct MCAdaptiveDecision { int samples; bool consumedRandom; };
// Literal CalcN reference semantics, with checked byte access. Pascal casts
// @neighbor.Ysum to TPMCrecord before reading RayCount: the resulting word is
// at neighbor base + 24, NOT + 15. It can lie beyond the image. Callers must
// explicitly provide those bytes; absent bytes throw, never invent a count.
// Coordinates are zero-based. This is the decision kernel, not a scheduler.
MCAdaptiveDecision CalcMCAdaptiveSamples(const uint8_t* records, size_t bytes,
    int width, int height, int x, int y, double averageVariance,
    float averageRayCount, double randomValue);

struct RenderOptions;
struct RenderResult;
// Bounded native path tracing; enable explicitly with --mc-samples.
RenderResult RenderMonteCarlo(TMandHeader10& header, THeaderCustomAddon& addon,
                              const RenderOptions& options);
} // namespace mb3d
#endif
