// Logarithmic spiral transform from the shipped x87 implementation.
constexpr unsigned int FORMULA_TRANSLOGSPIFS = 10253;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void TransLogSpiral_x87(double* state, const double* params);
#endif

template<>
cpu_double3 execute_formula<FORMULA_TRANSLOGSPIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x, p.y, p.z, de};
    TransLogSpiral_x87(state, params);
    de = state[3];
    return cpu_double3(state[0], state[1], state[2]);
#else
    long double input[] = {static_cast<long double>(p.x) + params[0],
                           static_cast<long double>(p.y) + params[1],
                           static_cast<long double>(p.z) + params[2]};
    double xyz[3];
    for (int row = 0; row < 3; ++row) {
        const double* m = params + 3 + row * 3;
        xyz[row] = double((input[0] * m[0] + input[1] * m[1]) + input[2] * m[2]);
    }
    long double radius = sqrt(static_cast<long double>(xyz[0]) * xyz[0]
                            + static_cast<long double>(xyz[1]) * xyz[1]) * params[15];
    const double stored_radius = double(radius);
    const double logarithm = double(params[14] * log2(radius) * params[16]);
    const double angle = double(atan2(static_cast<long double>(xyz[1]),
                                     static_cast<long double>(xyz[0])) * params[16]);
    const long double bend = params[12], arms = params[13];
    const long double a = bend * logarithm + arms * angle;
    const long double b = arms * logarithm - bend * angle;
    // FSUBRP is round(value)-value. FDIVRP divides the radius term by VaryScale.
    xyz[0] = double((rint(a) - a) * params[17]);
    xyz[1] = double((rint(b) - b) * params[17]);
    de = double(((sqrt(static_cast<long double>(stored_radius)) + params[18]) / de)
        * sqrt(arms * arms + bend * bend + static_cast<long double>(params[14]) * params[14])
        * params[19]);
    return cpu_double3(xyz[0], xyz[1], xyz[2]);
#endif
}

constant FormulaInfo formula_info_transLogSpIFS = {
    .id = FORMULA_TRANSLOGSPIFS,
    .name = "transLogSpIFS",
    .param_count = 10,
    .complexity = 240,
    .uses_transcendental = true
};
