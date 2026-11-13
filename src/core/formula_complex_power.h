#pragma once
#include <cmath>
#include <limits>

namespace mb3d {
// Shared SSE complex-rotation arithmetic in GenIntPower (0xE2..0x303)
// and _IntPowRotateC (0xB2..0x2D3). Preserve divisions at singular axes.
struct ComplexPowerPair {
    double x, y;
    ComplexPowerPair(double a, double b) : x(a), y(b) {}
    ComplexPowerPair& operator*=(double scale) { x *= scale; y *= scale; return *this; }
};

inline ComplexPowerPair complex_power_mul(ComplexPowerPair z, ComplexPowerPair w) {
    return ComplexPowerPair(z.x * w.x - z.y * w.y,
                       z.y * w.x + z.x * w.y);
}

// Follow the original multiplication tree, including out-of-range powers:
// <=2 squares; >=10 selects power 10 or 11 by the low bit.
inline ComplexPowerPair complex_power_power(ComplexPowerPair z, int power, bool flip) {
    if (flip) z = ComplexPowerPair(z.y, z.x);
    const ComplexPowerPair original = z;
    z = complex_power_mul(z, z);
    if (power > 2) {
        const ComplexPowerPair squared = z;
        if (power >= 4) {
            z = complex_power_mul(z, z);
            if (power >= 8) {
                z = complex_power_mul(z, z);
                if (power >= 10) z = complex_power_mul(z, squared);
            } else if (power >= 6) z = complex_power_mul(z, squared);
        }
        if (power & 1) z = complex_power_mul(z, original);
    }
    return flip ? ComplexPowerPair(z.y, z.x) : z;
}

inline void complex_power_rotate(double &x, double &y, double &z,
                           int power, int type, double one,
                           bool sequential_type_dispatch = false) {
    // Both original dispatchers use SUB/JNS rather than signed comparisons.
    // INT_MIN - 1 wraps positive: GenIntPower reaches its YZ branch, whereas
    // _IntPowRotateC's sequential chain reaches Ryz|x. Avoid C++ signed overflow.
    if (type == (std::numeric_limits<int>::min)())
        type = sequential_type_dispatch ? 5 : 2;
    const bool flip = type >= 6;
    if (flip) type -= 6;
    // Signed branch chain: negative types take XY; types >=5 take Ryz|x.
    if (type <= 2) {
        double a = type <= 1 ? x : y;
        double b = type <= 0 ? y : z;
        const double r = sqrt(a*a + b*b);
        const double inv = one / r;
        ComplexPowerPair q = complex_power_power(ComplexPowerPair(a*inv, b*inv), power, flip);
        q *= r;
        if (type <= 0) { x = q.x; y = q.y; }
        else if (type == 1) { x = q.x; z = q.y; }
        else { y = q.x; z = q.y; }
    } else {
        const double a = type <= 4 ? x : y;
        const double b = type == 3 ? y : z;
        const double other = type == 3 ? z : type == 4 ? y : x;
        const double r = sqrt(a*a + b*b);
        const ComplexPowerPair q = complex_power_power(ComplexPowerPair(r, other), power, flip);
        const double scale = q.x / r;
        if (type == 3) { x = a*scale; y = b*scale; z = q.y; }
        else if (type == 4) { x = a*scale; z = b*scale; y = q.y; }
        else { y = a*scale; z = b*scale; x = q.y; }
    }
}

} // namespace mb3d
