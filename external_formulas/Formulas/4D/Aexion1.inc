// Quadray initialization occurs once, when the shared bFirstIt is zero.
// It updates both coordinates and persistent Julia constants, including J4.
constexpr unsigned int FORMULA_AEXION1 = 10323;
template<> struct ExternalFormulaAlwaysWritesJuliaXYZ<FORMULA_AEXION1> { static constexpr bool value = true; };
template<> struct ExternalFormulaAlwaysWritesJuliaW<FORMULA_AEXION1> { static constexpr bool value = true; };
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Aexion1Original_x87(double* state, int first);
#endif

template<>
cpu_double3 execute_formula<FORMULA_AEXION1>(cpu_double3 p, constant double* params, double &de) {
    const int first = int(params[BFIRSTIT_SLOT]);
    double state[] = {p.x, p.y, p.z, de, params[1], params[2], params[3], params[4], params[0]};
#if defined(_MSC_VER) && defined(_M_X64)
    Aexion1Original_x87(state, first);
#else
    const numeric::X87Scope extendedScope;
    if (first == 0) {
        for (int base = 0; base <= 4; base += 4) {
            const numeric::X87Real x = state[base], y = state[base+1], z = state[base+2];
            state[base]   = double(numeric::x87Abs(x + y + z) + state[8]);
            state[base+1] = double(numeric::x87Abs(z - x - y) + state[8]);
            state[base+2] = double(numeric::x87Abs(y - x - z) + state[8]);
            state[base+3] = double(numeric::x87Abs(x - y - z) + state[8]);
        }
    }
    const numeric::X87Real x = state[0], y = state[1], z = state[2], w = state[3];
    const numeric::X87Real xy = x*y, zw = z*w, a = x*x-y*y, b = z*z-w*w;
    state[0] = double(a + (zw+zw) + state[4]);
    state[1] = double((zw+zw) - a + state[5]);
    state[2] = double(b + (xy+xy) + state[6]);
    state[3] = double((xy+xy) - b + state[7]);
#endif
    if (first == 0) params[BFIRSTIT_SLOT] = 1.;
    // Unchanged constants are written too, so legitimate NaNs are preserved.
    params[WRITE_J1_SLOT] = state[4];
    params[WRITE_J2_SLOT] = state[5];
    params[WRITE_J3_SLOT] = state[6];
    params[WRITE_J4_SLOT] = state[7];
    de = state[3];
    return cpu_double3(state[0], state[1], state[2]);
}

constant FormulaInfo formula_info_Aexion1 = {
    .id = FORMULA_AEXION1,
    .name = "Aexion1",
    .param_count = 5,
    .complexity = 225,
    .uses_transcendental = false
};
