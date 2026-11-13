// Original sphere fold uses the engine's stored Rout, which need not equal |p|^2.
constexpr unsigned int FORMULA__SPHEREFOLDING1 = 6030;
template<> struct ExternalFormulaReadsRout<FORMULA__SPHEREFOLDING1> { static constexpr bool value = true; };
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void SphereFolding1Original_x87(double* state, const double* params);
#endif

template<>
cpu_double3 execute_formula<FORMULA__SPHEREFOLDING1>(cpu_double3 p, constant double* params, double &de) {
    const double radius = params[FORMULA_ROUT_SLOT];
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x, p.y, p.z, radius};
    SphereFolding1Original_x87(state, params);
    return cpu_double3(state[0], state[1], state[2]);
#else
    const numeric::X87Scope extendedScope;
    // Unordered comparisons take the low branch. Both signed folding sides
    // are reachable with negative radii or negative user thresholds.
    numeric::X87Real numerator;
    if (radius >= params[0]) numerator = (numeric::X87Real)params[1] - radius;
    else if (!(radius >= params[2])) numerator = (numeric::X87Real)params[3] - radius;
    else return p;
    const numeric::X87Real scale = numerator / radius;
    return cpu_double3(double((numeric::X87Real)p.x * scale),
                       double((numeric::X87Real)p.y * scale),
                       double((numeric::X87Real)p.z * scale));
#endif
}

constant FormulaInfo formula_info__SphereFolding1 = {
    .id = FORMULA__SPHEREFOLDING1,
    .name = "_SphereFolding1",
    .param_count = 1,
    .complexity = 75,
    .uses_transcendental = false
};
