// Original _PlusSinApp x86: add the approximation of Index2 to Index1.
// Both selectors are masked with 3 and include W. No implicit Julia addition.
constexpr unsigned int FORMULA__PLUSSINAPP = 5485;
template<> struct ExternalFormulaWritesWState<FORMULA__PLUSSINAPP> { static constexpr bool value = true; };

template<>
cpu_double3 execute_formula<FORMULA__PLUSSINAPP>(cpu_double3 p, constant double* params, double &de) {
    const int destination = int(params[0]) & 3;
    const int source = int(params[1]) & 3;
    double coords[4] = {p.x, p.y, p.z, params[FORMULA_STATE_W_SLOT]};
    coords[destination] = external_sine_transform(coords[source], params + 2, params + 6, coords[destination], true);
    params[FORMULA_STATE_W_SLOT] = coords[3];
    return cpu_double3(coords[0], coords[1], coords[2]);
}

constant FormulaInfo formula_info__PlusSinApp = {
    .id = FORMULA__PLUSSINAPP, .name = "_PlusSinApp", .param_count = 9,
    .complexity = 135, .uses_transcendental = false
};
