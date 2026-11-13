// _lorenz.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _lorenz
// Hand-ported from ASM: 96 bytes, DEoption=-1
// Lorenz hyperchaotic 4D system.
// By Luca GN 2011, based on Jiang Xu et al.

constexpr unsigned int FORMULA__LORENZ = 9764;
template<> struct ExternalFormulaWritesWState<FORMULA__LORENZ> { static constexpr bool value = true; };

// ASM trace (96 bytes, 0 branches):
//
// esi = PVar
// x' = (Alpha*(y-x) + w) * Strength
// y' = (Gamma*x - y - x*z) * Strength
// z' = (x*y - Beta*z) * Strength
// w' = (r*w - y*z) * Strength
//
// C handling: Transform (DEoption=-1), no C.

template<>
cpu_double3 execute_formula<FORMULA__LORENZ>(cpu_double3 p, constant double* params, double &de) {
    const double Strength = params[0];  // [esi-0x10] (default: 1.0)
    const double Alpha    = params[1];  // [esi-0x18] (default: 0.5)
    const double Beta     = params[2];  // [esi-0x20] (default: -2.0)
    const double Gamma    = params[3];  // [esi-0x28] (default: 1.0)
    const double R        = params[4];  // [esi-0x30]

    double x = p.x, y = p.y, z = p.z, w = params[FORMULA_STATE_W_SLOT];

    double x_out = (Alpha * (y - x) + w) * Strength;
    double y_out = (Gamma * x - y - x * z) * Strength;
    double z_out = (x * y - Beta * z) * Strength;
    double w_out = (R * w - y * z) * Strength;

    params[FORMULA_STATE_W_SLOT] = w_out;
    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info__lorenz = {
    .id = FORMULA__LORENZ,
    .name = "_lorenz",
    .param_count = 5,
    .complexity = 96,
    .uses_transcendental = false
};

/*
Original M3 Description:
Lorenz hyperchaotic system in 4D.
Adapted from Jiang Xu, Guoliang Cai, Song Zheng.
"International Journal of Nonlinear Science Vol.8(2009) No.1,pp.117-123"
Formula by Luca GN 2011

Full 4D system:
  x' = (Alpha*(y-x) + w) * Strength
  y' = (Gamma*x - y - x*z) * Strength
  z' = (x*y - Beta*z) * Strength
  w' = (r*w - y*z) * Strength

Parameters:
  [0] Strength: double (default: 1.0) - overall scaling
  [1] Alpha: double (default: 0.5) - coupling coefficient
  [2] Beta: double (default: -2.0) - z damping
  [3] Gamma: double (default: 1.0) - x amplification
  [4] r: double (default: 1.0) - w feedback
*/
