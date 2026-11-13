// Counter-dependent rotations from the original x86 formula.
#include "core/formula_counter.h"
constexpr unsigned int FORMULA__ROTATERND = 10019;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void RotateRandomPlane_x87(double* xy, const double* params, double randomValue);
#endif
template<>
cpu_double3 execute_formula<FORMULA__ROTATERND>(cpu_double3 p, constant double* params, double &de) {
    const uint32_t word = formulaCounterRandomWord(incrementFormulaCounter(params));
    double xyz[] = {p.x, p.y, p.z};
    const int axes[][2] = {{0,1}, {0,2}, {1,2}};
    for (int plane = 0; plane < 3; ++plane) {
        const uint32_t seed = static_cast<uint32_t>(static_cast<int32_t>(params[3 + plane]));
        const double randomValue = ((seed & 0xFFFFu) * word) & 0x3FFFu;
        const int a = axes[plane][0], b = axes[plane][1];
#if defined(_MSC_VER) && defined(_M_X64)
        const double compact[] = {params[plane], params[6], params[7], params[8]};
        double pair[] = {xyz[a], xyz[b]};
        RotateRandomPlane_x87(pair, compact, randomValue);
        xyz[a] = pair[0]; xyz[b] = pair[1];
#else
        // The original skips |range| > 2 (and unordered), not zero range.
        if (!(fabs(params[plane]) <= params[6])) continue;
        long double angle = (static_cast<long double>(randomValue) * params[7] - params[8]) * params[plane];
        bool flip = false;
        if (!(angle <= 1.)) { angle = params[6] - angle; flip = true; }
        else if (!(angle > -1.)) { angle = -(params[6] + angle); flip = true; }
        long double cosine = sqrt(1.L - angle * angle);
        if (flip) cosine = -cosine;
        const double x = double(xyz[a] * cosine + xyz[b] * angle);
        xyz[b] = double(xyz[b] * cosine - xyz[a] * angle);
        xyz[a] = x;
#endif
    }
    return cpu_double3(xyz[0], xyz[1], xyz[2]);
}
constant FormulaInfo formula_info__RotateRnd = {
    .id = FORMULA__ROTATERND, .name = "_RotateRnd",
    .param_count = 7, .complexity = 482, .uses_transcendental = false
};
