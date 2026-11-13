// Thread-safe Halton precomputation for the native Monte Carlo renderer.
// Current scope: docs/PARITY_TRACE.md.

#include "core/calc_monte_carlo.h"
#include "core/math3d.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <mutex>
#include <stdexcept>

namespace mb3d {

double MCNextRandom(uint32_t& state) {
    state = state * 0x343fdu + 0x269ec3u;
    // TypeDefinitions.dSeedMul divides by 2^31 - 1 (not 2^31).
    return (state & 0x7fffffffu) * (1.0 / 2147483647.0);
}

std::array<float, 2> GetMCHaltonShifts(uint32_t x, uint32_t y) {
    const uint32_t a = x * 0x343fdu + 0x269ec3u;
    const uint32_t b = y * 0x343fdu + 0x269ec3u;
    uint32_t state = (x << 15) ^ y ^ (a << 9) ^ (b << 5);
    MCNextRandom(state);
    const float u = float(MCNextRandom(state));
    return {u, float(MCNextRandom(state))};
}

float ShiftMCHalton(uint16_t word, float shift) {
    // d1d65535 is Double; FracSingle receives one final Single conversion.
    const float value = float(word * (1.0 / 65535.0) + double(shift));
    return value - std::trunc(value);
}

std::array<double, 2> CalcMCCameraAngles(float x, float y, int width, int height,
                                       float fovXmul, double fovY) {
    const float horizontal = (.5f * float(width) - x) - 1.f;
    const float vertical = y / float(height) - .5f;
    return {double(horizontal * fovXmul), double(vertical) * fovY};
}

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" int MCAdaptiveRandomSkip_x87(const double* inputs);
#endif

double DecodeMC24(const uint8_t* bytes) {
    const uint32_t packed = uint32_t(bytes[0]) | (uint32_t(bytes[1]) << 8) |
                            (uint32_t(bytes[2]) << 16);
    double value = packed * 4.7683718662483612447000291764754e-7 - 1;
    constexpr double e = 2.7182818284590452353602874713527;
    if (value > 2) value = std::exp(value - 1) - e + 2;
    return value;
}

void EncodeMC24(double value, uint8_t* bytes) {
    if (!std::isfinite(value))
        throw std::invalid_argument("MC accumulator requires finite values");
    constexpr double e = 2.7182818284590452353602874713527;
    const double stretched = value > 2 ? std::log(value - 2 + e) + 2 : value + 1;
    const uint32_t packed = uint32_t(std::nearbyint(std::clamp(stretched, 0.0, 8.0) * 2097151.875));
    bytes[0] = uint8_t(packed);
    bytes[1] = uint8_t(packed >> 8);
    bytes[2] = uint8_t(packed >> 16);
}

double NoiseOfMCRecord(const MCPackedRecord& record) {
    if (record.rayCount == 0) return 0;
    const double mean = DecodeMC24(record.ysum);
    return (DecodeMC24(record.ysqr) - mean * mean) /
           (std::max(0.01, mean) * record.rayCount);
}

MCAdaptiveDecision CalcMCAdaptiveSamples(const uint8_t* records, size_t bytes,
    int width, int height, int x, int y, double averageVariance,
    float averageRayCount, double randomValue) {
    if (!records || width < 1 || height < 1 || width > 16384 || height > 16384 ||
        x < 0 || x >= width || y < 0 || y >= height ||
        bytes < size_t(width) * size_t(height) * 18 ||
        !(averageVariance > 0) || !std::isfinite(averageVariance) ||
        !(averageRayCount >= 0 && averageRayCount <= 65535) ||
        !(randomValue >= 0 && randomValue <= 1))
        throw std::invalid_argument("Invalid MC adaptive inputs");
    const auto offset = [=](int xx, int yy) { return (size_t(yy) * width + xx) * 18; };
    const auto word = [&](size_t at) {
        if (at >= bytes || bytes-at < 2)
            throw std::out_of_range("Pascal CalcN neighbor counter lies outside the supplied buffer");
        return int(records[at]) | (int(records[at+1]) << 8);
    };
    const size_t current = offset(x,y);
    const double center = DecodeMC24(records+current+9);
    const int count = word(current+15);
    double contrast = 0, neighborContrast = 0, noise = 0;
    int countSum = 0, n = 0, n2 = 0;
    const auto square = [](double d) { return d*d; };
    const auto visit = [&](int xx, int yy, bool firstUpper = false) {
        const size_t at = offset(xx,yy);
        const double mean = DecodeMC24(records+at+9);
        const double variance = DecodeMC24(records+at+12)-square(mean);
        contrast += square(mean-center);
        noise = firstUpper ? variance : std::max(noise,variance);
        countSum += word(at+24); // Preserve the source's displaced pointer.
        ++n;
        return mean;
    };
    if (y > 0) {
        const double upper = visit(x,y-1,true);
        if (x > 0) { neighborContrast = square(upper-visit(x-1,y-1)); ++n2; }
        if (x+1 < width) { neighborContrast += square(upper-visit(x+1,y-1)); ++n2; }
    }
    double left = 0;
    if (x > 0) left = visit(x-1,y);
    if (x+1 < width) {
        const double right = visit(x+1,y);
        if (x > 0) { neighborContrast += square(left-right); ++n2; }
    }
    if (y+1 < height) {
        const double lower = visit(x,y+1);
        if (x > 0) { neighborContrast += square(lower-visit(x-1,y+1)); ++n2; }
        if (x+1 < width) { neighborContrast += square(lower-visit(x+1,y+1)); ++n2; }
    }
    n = std::max(1,n); n2 = std::max(1,n2);
    contrast /= n; neighborContrast /= n2;
    const double variance = (DecodeMC24(records+current+12)-square(center)) /
                            (std::max(0.0,center)+0.01);
    double dm = 0;
    if (records[current+17] & 128)
        dm = std::max(0.0,std::max(contrast,neighborContrast)-20*averageVariance) *
             std::clamp(count-double(countSum)/n,-4.0,4.0);
    double amount = std::max(0.0,std::max(variance,noise) /
        std::max(0.3,(center/std::sqrt(1+square(center*0.9)))*count)-dm);
    amount = amount*4/averageVariance;
    if (amount >= 1)
        return {int(std::nearbyint(std::min(double(averageRayCount)*0.5+8,amount))),false};
#if defined(_MSC_VER) && defined(_M_X64)
    // Pascal's untyped 1.1 and 0.1 literals keep this comparison in Extended.
    // At u=1/11 the Double expression cancels to zero, but Pascal is positive.
    const double inputs[] = {randomValue,std::max(0.0,amount)};
    const bool skip = MCAdaptiveRandomSkip_x87(inputs) != 0;
#else
    const bool skip = static_cast<long double>(randomValue)*1.1L-0.1L > std::max(0.0,amount);
#endif
    return {skip ? 0 : 1,true};
}

// -----------------------------------------------------------------------------
// Halton sequence precompute
//   Pascal: CalcMonteCarlo.pas `PreComputeHaltonSequence` around line 130.
//   For each index i in 0..65535:
//     - rect: (HRx, HRy) = (halton(i, 2), halton(i, 3)) * 65535
//     - lens: (HDx, HDy) = (halton(i, 5), halton(i, 7)) * 65535;
//       the renderer applies the concentric disc mapping after pixel shifts.
// -----------------------------------------------------------------------------

namespace {
    constexpr int kHaltonTableSize = 65536;
    std::array<HaltonRec, kHaltonTableSize> g_haltonTable{};

    std::once_flag g_haltonOnce;
}

double HaltonRadicalInverse(int32_t index, int32_t base) {
    // Van der Corput radical inverse â€” mirrors Pascal
    // CalcMonteCarlo.pas:120-134 bit-for-bit:
    //     f := 1 / base; d := f; i := index;
    //     while i > 0 do begin
    //       result := result + f * (i mod base);
    //       i := Trunc(i * d);     // NOT integer division
    //       f := f * d;
    //     end;
    // The `Trunc(i * d)` path matters for bases that are not powers of
    // two (3, 5, 7): 1/base is not representable exactly in Double, so
    // integer `i / base` and `Trunc(i * (1/base))` can disagree by one
    // at specific indices. Match Pascal's float-trunc semantics so the
    // Halton table is byte-identical between ports.
    if (index <= 0 || base < 2) return 0.0;
    const double d = 1.0 / static_cast<double>(base);
    double f = d;
    double result = 0.0;
    int32_t i = index;
    while (i > 0) {
        result += f * static_cast<double>(i % base);
        i = static_cast<int32_t>(std::trunc(static_cast<double>(i) * d));
        f *= d;
    }
    return result;
}

void MakeDiscFromHalton(float& x, float& y) {
    x = x * 2 - 1;
    y = y * 2 - 1;
    constexpr float quarterPi = 0.7853981633974483096f;
    const auto nonzero = [](double v) {
        return std::abs(v) > 1e-16 ? v : v * .5 + (v < 0 ? -.5e-16 : .5e-16);
    };
    double angle;
    if (x > std::abs(y)) {
        angle = (quarterPi * y) / nonzero(x);
        y = x;
    } else if (x > y) {
        angle = quarterPi * (6 - x / nonzero(y));
        y = -y;
    } else if (x > -y) {
        angle = quarterPi * (2 - x / nonzero(y));
    } else {
        angle = quarterPi * (4 + y / nonzero(x));
        y = -x;
    }
    float sine, cosine;
    SinCosS(angle, sine, cosine);
    x = sine * y;
    y = cosine * y;
}

float CalcBokeh(float x, float y, int mode) {
    const float r = std::sqrt(x * x + y * y);
    float value = (mode & 1) ? 1.5 - r * .5f : 1;
    if (mode == 0) {
        if (r < .94) value = 1.03;
        else if (r < 1) value = .9682 + (r - .94) * .5f;
    } else if (mode > 1) {
        const double wave = std::cos(std::atan2(double(x), double(y)) * (mode < 4 ? 5 : 7)) * .5 + .5;
        float u = mode < 4 ? .92 + wave * wave * .12 : .94 + wave * wave * .08;
        const float s = u - .04;
        u = r < s ? 1 : (r - s) / (1 - s) * .05 + s;
        value *= u;
    }
    return value;
}

MCLensSample CalcMCLensSample(float u, float v, int mode, float aperture,
                             float focus, int width, double stepWidth) {
    // CalcMonteCarlo.pas: MaxCS has Single arguments/results; CalcBokehMC
    // multiplies three Singles before the Double StepWidth and Single store.
    const float lens = std::max(1e-7f, aperture * .5f);
    const float zsharp = std::max(.01f, focus * width);
    MakeDiscFromHalton(u, v);
    const float scale = CalcBokeh(u, v, mode) * lens * zsharp * stepWidth;
    return {u * scale, v * scale, zsharp, lens, scale};
}

void PreComputeHaltonSequence() {
    std::call_once(g_haltonOnce, []() {
        // Mirrors Pascal CalcMonteCarlo.pas:136-151 exactly. Pascal uses
        // 1-indexed halton calls (i+1) for all 4 bases. The "disc" pair
        // is NOT remapped through sqrt/cos/sin here â€” that mapping
        // happens at consumption time (CalcBokehMC, DoDOF, VaryVecSphere).
        // Pascal comment: "disc distri done after shifting" â€” the stored
        // HDx/HDy are raw Halton-5 / Halton-7 values * 65535.
        for (int32_t i = 0; i < kHaltonTableSize; ++i) {
            const double hx = HaltonRadicalInverse(i + 1, 2);
            const double hy = HaltonRadicalInverse(i + 1, 3);
            const double hdx = HaltonRadicalInverse(i + 1, 5);
            const double hdy = HaltonRadicalInverse(i + 1, 7);
            g_haltonTable[i].HRx = static_cast<uint16_t>(
                std::lrint(hx  * 65535.0));
            g_haltonTable[i].HRy = static_cast<uint16_t>(
                std::lrint(hy  * 65535.0));
            g_haltonTable[i].HDx = static_cast<uint16_t>(
                std::lrint(hdx * 65535.0));
            g_haltonTable[i].HDy = static_cast<uint16_t>(
                std::lrint(hdy * 65535.0));
        }

    });
}

const HaltonRec* GetHaltonSequence() {
    PreComputeHaltonSequence();
    return g_haltonTable.data();
}

} // namespace mb3d
