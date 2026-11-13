// Counter-dependent translation from the original x86 formula.
#include "core/formula_counter.h"
constexpr unsigned int FORMULA__TRANSLATERND = 10320;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" double TranslateRandom_x87(double coordinate, double range, double randomValue,
                                      const double* constants);
#endif
template<>
cpu_double3 execute_formula<FORMULA__TRANSLATERND>(cpu_double3 p, constant double* params, double &de) {
    const uint32_t word = formulaCounterRandomWord(incrementFormulaCounter(params));
    double xyz[] = {p.x, p.y, p.z};
    for (int axis = 0; axis < 3; ++axis) {
        const uint32_t seed = static_cast<uint32_t>(static_cast<int32_t>(params[3 + axis]));
        const double randomValue = ((seed & 0xFFFFu) * word) & 0x3FFFu;
#if defined(_MSC_VER) && defined(_M_X64)
        xyz[axis] = TranslateRandom_x87(xyz[axis], params[axis], randomValue, params + 6);
#else
        xyz[axis] = double(xyz[axis] + static_cast<long double>(params[axis])
            * (static_cast<long double>(randomValue) * params[7] - params[6]));
#endif
    }
    return cpu_double3(xyz[0], xyz[1], xyz[2]);
}
constant FormulaInfo formula_info__TranslateRnd = {
    .id = FORMULA__TRANSLATERND, .name = "_TranslateRnd",
    .param_count = 7, .complexity = 184, .uses_transcendental = false
};
