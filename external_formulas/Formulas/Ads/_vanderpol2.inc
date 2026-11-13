// _vanderpol2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _vanderpol2
// Hand-ported from ASM: 162 bytes, DEoption=-1
// Van der Pol modified chaotic 4D system with 3 oscillators.
// By Luca GN 2011.

constexpr unsigned int FORMULA__VANDERPOL2 = 9222;
template<> struct ExternalFormulaWritesWState<FORMULA__VANDERPOL2> { static constexpr bool value = true; };

// ASM trace (162 bytes, 0 branches):
//
// esi = PVar (overwrite convention: mov esi, [esi+0x30])
// 4D system with observer x and three Van der Pol oscillators y,z,w:
//
// x' = (Rho*x + Alpha*y + Beta*z + Gamma*w) * Strength
// y' = Delta*z*w - Alpha*x + (Alpha - r*x²)*y
// z' = Epsilon*y*w - Beta*x + (Beta - r*x²)*z
// w' = Phi*y*z - Gamma*x + (Gamma - r*x²)*w
//
// Note: y', z', w' do NOT have Strength multiplier.
// Each oscillator has same structure: cross-coupling - linear*x + VdP_damping*self
//
// C handling: Transform (DEoption=-1), no C.

template<>
cpu_double3 execute_formula<FORMULA__VANDERPOL2>(cpu_double3 p, constant double* params, double &de) {
    const double Strength = params[0];  // [esi-0x10] (default: 1.0)
    const double Alpha    = params[1];  // [esi-0x18] (default: 0.7)
    const double Beta     = params[2];  // [esi-0x20] (default: 0.7)
    const double Gamma    = params[3];  // [esi-0x28]
    const double Delta    = params[4];  // [esi-0x30]
    const double Epsilon  = params[5];  // [esi-0x38]
    const double Phi      = params[6];  // [esi-0x40]
    const double Rho      = params[7];  // [esi-0x48] (default: 0.8) - x self-coupling
    const double r        = params[8];  // [esi-0x50] (default: 1.4) - nonlinear damping

    double x = p.x, y = p.y, z = p.z, w = params[FORMULA_STATE_W_SLOT];
    double x2 = x * x;

    double x_out = (Rho * x + Alpha * y + Beta * z + Gamma * w) * Strength;
    double y_out = Delta * z * w - Alpha * x + (Alpha - r * x2) * y;
    double z_out = Epsilon * y * w - Beta * x + (Beta - r * x2) * z;
    double w_out = Phi * y * z - Gamma * x + (Gamma - r * x2) * w;

    params[FORMULA_STATE_W_SLOT] = w_out;
    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info__vanderpol2 = {
    .id = FORMULA__VANDERPOL2,
    .name = "_vanderpol2",
    .param_count = 9,
    .complexity = 162,
    .uses_transcendental = false
};

/*
Original M3 Description:
Van der Pol modified chaotic system in 4D.
Formula by Luca GN 2011. Three oscillators (y,z,w).
Cut at z=0, x=0... to see amazing views.

Full 4D system:
  x' = (Rho*x + Alpha*y + Beta*z + Gamma*w) * Strength
  y' = Delta*z*w - Alpha*x + (Alpha - r*x²)*y
  z' = Epsilon*y*w - Beta*x + (Beta - r*x²)*z
  w' = Phi*y*z - Gamma*x + (Gamma - r*x²)*w

Parameters:
  [0] Strength: double (default: 1.0) - x' scaling
  [1] Alpha:    double (default: 0.7) - y coupling / VdP param
  [2] Beta:     double (default: 0.7) - z coupling / VdP param
  [3] Gamma:    double (default: 0.7) - w to x / x to w coupling
  [4] Delta:    double (default: 0.2) - z*w coupling in y'
  [5] Epsilon:  double (default: 0.2) - y*w coupling in z'
  [6] Phi:      double (default: 0.2) - y*z coupling in w'
  [7] Rho:      double (default: 0.8) - x self-coupling [esi-0x48]
  [8] r:        double (default: 1.4) - nonlinear damping
*/
