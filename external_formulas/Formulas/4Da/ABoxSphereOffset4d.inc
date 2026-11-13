// ABoxSphereOffset4d: 228-byte shipped formula, DEoption 5.
// Fold XYZW, scale around four offsets, update signed Deriv1, then add Julia C.
constexpr unsigned int FORMULA_ABOXSPHEREOFFSET4D = 10324;
template<> struct ExternalFormulaUsesXYZState<FORMULA_ABOXSPHEREOFFSET4D> {
    static constexpr bool value = true;
};
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ABoxSphereOffset4dOriginal_x87(double* state, const double* params);
#endif

template<>
cpu_double3 execute_formula<FORMULA_ABOXSPHEREOFFSET4D>(cpu_double3 p, double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x, p.y, p.z, de, params[FORMULA_STATE_X_SLOT]};
    ABoxSphereOffset4dOriginal_x87(state, params);
    de = state[3];
    params[FORMULA_STATE_X_SLOT] = state[4];
    return cpu_double3(state[0], state[1], state[2]);
#else
    const double fold = params[3];
    auto box = [fold](double v) { return fabs(v + fold) - (fabs(v - fold) + v); };
    const double x = box(p.x) - params[4];
    const double y = box(p.y) - params[5];
    const double z = box(p.z) - params[6];
    const double w = box(de) - params[7];
    const double rr = ((z*z + y*y) + x*x) + w*w;
    // Unordered comparisons take the inner sphere branch in the original.
    const double m = !(rr >= params[2]) ? params[1] :
                     (rr > 1. ? params[0] : params[0] / rr);
    params[FORMULA_STATE_X_SLOT] *= m;
    de = ((w*m + params[8]) + params[7]) + params[12];
    return cpu_double3((x*m + params[9]) + params[4],
                       (y*m + params[10]) + params[5],
                       (z*m + params[11]) + params[6]);
#endif
}

constant FormulaInfo formula_info_ABoxSphereOffset4d = {
    .id = FORMULA_ABOXSPHEREOFFSET4D,
    .name = "ABoxSphereOffset4d",
    .param_count = 9,
    .complexity = 228,
    .uses_transcendental = false
};
