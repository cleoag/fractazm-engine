// Original _SinApprox x86: transform one of XYZ/W, with no Julia addition.
constexpr unsigned int FORMULA__SINAPPROX = 4611;
template<> struct ExternalFormulaWritesWState<FORMULA__SINAPPROX> { static constexpr bool value = true; };

template<>
cpu_double3 execute_formula<FORMULA__SINAPPROX>(cpu_double3 p, constant double* params, double &de) {
    const int index = int(params[0]) & 3;
    double coords[4] = {p.x, p.y, p.z, params[FORMULA_STATE_W_SLOT]};
    coords[index] = external_sine_transform(coords[index], params + 1, params + 5, 0.0, false);
    params[FORMULA_STATE_W_SLOT] = coords[3];
    return cpu_double3(coords[0], coords[1], coords[2]);
}

constant FormulaInfo formula_info__SinApprox = {
    .id = FORMULA__SINAPPROX, .name = "_SinApprox", .param_count = 8,
    .complexity = 126, .uses_transcendental = false
};
