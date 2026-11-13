// ABoxMod4d.inc - Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: ABoxMod4d
// Hand-ported from ASM: 143 bytes, DEoption=5

constexpr unsigned int FORMULA_ABOXMOD4D = 3857;
template<> struct ExternalFormulaUsesXYZState<FORMULA_ABOXMOD4D> { static constexpr bool value = true; };
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ABoxMod4dOriginal_x87(double* state, const double* params);
#endif

// ASM trace:
//   Box fold each axis: v' = |v + Fold| - Fold
//   rr = x'^2 + y'^2 + z'^2 + w'^2 + epsilon
//   if rr < 1: m = Scale / rr; else: m = Scale
//   x_out = x' * m + Cx
//   y_out = y' * m + Cy
//   z_out = z' * m + Cz
//   w_out = w' * m + Cw + W_add
//
// AmazingBox Modified 4D (Trafassel variant).

template<>
cpu_double3 execute_formula<FORMULA_ABOXMOD4D>(cpu_double3 p, double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    // The original retains folded coordinates, squared length and multiplier in
    // x87 registers. Rounding (x + Fold) before subtraction changes small inputs.
    double state[5] = {p.x, p.y, p.z, de, params[FORMULA_STATE_X_SLOT]};
    ABoxMod4dOriginal_x87(state, params);
    params[FORMULA_STATE_X_SLOT] = state[4];
    de = state[3];
    return cpu_double3(state[0], state[1], state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E = numeric::Extended80;
    const E fold(params[1]);
    const E x = abs(E(p.x) + fold) - fold;
    const E y = abs(E(p.y) + fold) - fold;
    const E z = abs(E(p.z) + fold) - fold;
    const E w = abs(E(de) + fold) - fold;
    // Original x87 stack accumulates w,z,y,x in that order.
    const E rr = w*w + z*z + y*y + x*x + E(params[3]);
    const E scale(params[0]);
    const E m = (rr < E(1) || rr == E(1)) ? scale / rr : scale;
    params[FORMULA_STATE_X_SLOT] = static_cast<double>(E(params[FORMULA_STATE_X_SLOT]) * m);
    de = static_cast<double>(w*m + E(params[7]) + E(params[2]));
    return cpu_double3(static_cast<double>(x*m + E(params[4])),
                       static_cast<double>(y*m + E(params[5])),
                       static_cast<double>(z*m + E(params[6])));
#else
    const double Scale = params[0];
    const double Fold = params[1];
    const double W_add = params[2];
    const double epsilon = params[3];
    const double Cx = params[4];
    const double Cy = params[5];
    const double Cz = params[6];
    const double Cw = params[7];

    double x = abs(p.x + Fold) - Fold;
    double y = abs(p.y + Fold) - Fold;
    double z = abs(p.z + Fold) - Fold;
    double w = abs(de + Fold) - Fold;

    double rr = x * x + y * y + z * z + w * w + epsilon;
    double m = (rr < 1.0) ? (Scale / rr) : Scale;

    // Original x86 0x63..0x69 updates the signed derivative multiplier.
    params[FORMULA_STATE_X_SLOT] *= m;
    de = w * m + Cw + W_add;
    return cpu_double3(x * m + Cx, y * m + Cy, z * m + Cz);
#endif
}

// Alternative direct function implementation
cpu_double3 ABoxMod4d_formula(cpu_double3 p, double* params, double &de) {
    return execute_formula<FORMULA_ABOXMOD4D>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_ABoxMod4d = {
    .id = FORMULA_ABOXMOD4D,
    .name = "ABoxMod4d",
    .param_count = 3,
    .complexity = 143,
    .uses_transcendental = false
};

/*
Original M3 Description:
A modification of the amazing box as suggested by Trafassel.

Mathematical Formula:
x = abs(x+Fold) - Fold
y = abs(y+Fold) - Fold
z = abs(z+Fold) - Fold
w = abs(w+Fold) - Fold
rr = x*x + y*y + z*z + w*w + epsilon
if rr < 1 then m = Scale/rr else m = Scale
x = x * m + Cx
y = y * m + Cy
z = z * m + Cz
w = w * m + W_add + Cw

Parameters:
  [0] Scale: double (default: 2)
  [1] Fold: double (default: 1)
  [2] W_add: double (default: 0)
  [3] Double: double (default: 1e-60)
  [4..7] Cx, Cy, Cz, Cw: Julia constants (from iteration context)
*/
