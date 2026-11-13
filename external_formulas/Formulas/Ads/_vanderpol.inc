// _vanderpol.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _vanderpol
// Hand-ported from ASM: 114 bytes, DEoption=-1
// Van der Pol modified chaotic 4D system.
// By Luca GN 2011.

constexpr unsigned int FORMULA__VANDERPOL = 6285;
template<> struct ExternalFormulaWritesWState<FORMULA__VANDERPOL> { static constexpr bool value = true; };

// ASM trace (114 bytes, 0 branches):
//
// esi = PVar
// x' = (z + Beta*x) * Strength
// y' = (w + Beta*y) * Strength
// z' = Gamma*y - Delta*x + (Alpha - r1*x²)*z
// w' = Gamma*x - Delta*y + (Phi - r2*y²)*w
//
// Note: z' and w' do NOT have Strength multiplier.
//
// C handling: Transform (DEoption=-1), no C.

template<>
cpu_double3 execute_formula<FORMULA__VANDERPOL>(cpu_double3 p, constant double* params, double &de) {
    const double Strength = params[0];  // [esi-0x10] (default: 1.0)
    const double Phi      = params[1];  // [esi-0x18] (default: 2.0) — w Van der Pol param
    const double Beta     = params[2];  // [esi-0x20] (default: 0.6)
    const double Gamma    = params[3];  // [esi-0x28] (default: 0.1)
    const double Delta    = params[4];  // [esi-0x30] (default: 0.4)
    const double Alpha    = params[5];  // [esi-0x38] (default: 2.0)
    const double r1       = params[6];  // [esi-0x40] (default: 1.4)
    const double r2       = params[7];  // [esi-0x48]

    double x = p.x, y = p.y, z = p.z, w = params[FORMULA_STATE_W_SLOT];

    double x_out = (z + Beta * x) * Strength;
    double y_out = (w + Beta * y) * Strength;
    double z_out = Gamma * y - Delta * x + (Alpha - r1 * x * x) * z;
    double w_out = Gamma * x - Delta * y + (Phi - r2 * y * y) * w;

    params[FORMULA_STATE_W_SLOT] = w_out;
    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info__vanderpol = {
    .id = FORMULA__VANDERPOL,
    .name = "_vanderpol",
    .param_count = 8,
    .complexity = 114,
    .uses_transcendental = false
};

/*
Original M3 Description:
Van der Pol modified chaotic system in 4D.
Formula by Luca GN 2011.

Full 4D system:
  x' = (z + Beta*x) * Strength
  y' = (w + Beta*y) * Strength
  z' = Gamma*y - Delta*x + (Alpha - r1*x²)*z
  w' = Gamma*x - Delta*y + (Phi - r2*y²)*w

Parameters:
  [0] Strength: double (default: 1.0) - linear coupling scale
  [1] Phi: double (default: 2.0) - w Van der Pol param (4D only)
  [2] Beta: double (default: 0.6) - linear coupling
  [3] Gamma: double (default: 0.1) - cross-coupling
  [4] Delta: double (default: 0.4) - cross-coupling
  [5] Alpha: double (default: 2.0) - z Van der Pol param
  [6] r1: double (default: 1.4) - nonlinear damping for x²
  [7] r2: double (default: 1.4) - nonlinear damping for y²
*/
