// _JuliaSets.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _JuliaSets
// Hand-ported from ASM: 89 bytes, DEoption=-1

constexpr unsigned int FORMULA__JULIASETS = 10003;

template<>
struct ExternalFormulaAlwaysWritesJuliaXYZ<FORMULA__JULIASETS> { static constexpr bool value = true; };

template<>
struct ExternalFormulaConsumesPersistentCLanes<FORMULA__JULIASETS> { static constexpr bool value = true; };

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void JuliaSetsOriginal_x87(double* state, const double* params);
#endif

// Round the scaled saved point before the first Double store, as FRNDINT does.
template<>
cpu_double3 execute_formula<FORMULA__JULIASETS>(cpu_double3 p, constant double* params, double &de) {
    double state[] = {params[2], params[3], params[4], 0., 0., 0.};
#if defined(_MSC_VER) && defined(_M_X64)
    JuliaSetsOriginal_x87(state, params);
#else
    const numeric::X87Scope extendedScope;
    const numeric::X87Real inverse = numeric::X87Real(1.) / params[0];
    for (int i = 0; i < 3; ++i) {
        const numeric::X87Real scaled = state[i] * inverse, tile = numeric::x87Round(scaled);
        state[i] = double((scaled - tile)*params[0]);
        state[i+3] = double(tile*params[1]);
    }
#endif
    params[WRITE_J1_SLOT] = state[3];
    params[WRITE_J2_SLOT] = state[4];
    params[WRITE_J3_SLOT] = state[5];
    return cpu_double3(state[0], state[1], state[2]);
}

// Formula metadata
constant FormulaInfo formula_info__juliasets = {
    .id = FORMULA__JULIASETS,
    .name = "_JuliaSets",
    .param_count = 2,
    .complexity = 50,
    .uses_transcendental = false
};

/*
Original M3 Description:
A pretransform to show juliasets from formulas.

rs = 1 / Separation
Cx = Round(px * rs)
Cy = Round(py * rs)
Cz = Round(pz * rs)
x = (px * rs - Cx) * Separation
y = (py * rs - Cy) * Separation
z = (pz * rs - Cz) * Separation
Cx = Cx * Julia_factor
Cy = Cy * Julia_factor
Cz = Cz * Julia_factor

Parameters:
  [0] Separation: double (default: 4)
  [1] Julia_factor: double (default: 0.2)
*/
