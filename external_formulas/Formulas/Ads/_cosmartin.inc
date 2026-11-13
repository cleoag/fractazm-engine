// _cosmartin.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _cosmartin
// Hand-ported from ASM: 91 bytes, DEoption=-1
// CosMartin deformation in 4D, retaining the actual W coordinate.
// Inspired by CosMartin formula from Mark Townsend.
// By Luca GN 2011.

constexpr unsigned int FORMULA__COSMARTIN = 8110;
template<> struct ExternalFormulaWritesWState<FORMULA__COSMARTIN> { static constexpr bool value = true; };

// ASM trace (91 bytes, 0 branches):
//
// esi = PVar
// Applied to (x,y) and (z,w) planes independently:
//
// x' = (y - Alpha*cos(Beta*x)) * Strength
// y' = (Gamma - x) * Strength
// z' = (w - Alpha*cos(Beta*z)) * Strength
// w' = (Delta - z) * Strength
//
// For 3D: w=0, so z' = -Alpha*cos(Beta*z) * Strength
// C handling: Transform (DEoption=-1), no C.

template<>
cpu_double3 execute_formula<FORMULA__COSMARTIN>(cpu_double3 p, constant double* params, double &de) {
    const double Strength = params[0];  // [esi-0x10] (default: 1.0)
    const double Alpha    = params[1];  // [esi-0x18] (default: 0.5)
    const double Beta     = params[2];  // [esi-0x20] (default: 0.75)
    const double Gamma    = params[3];  // [esi-0x28] (default: 1.0)
    const double Delta    = params[4];  // [esi-0x30] (default: 1.0)

    double x = p.x, y = p.y, z = p.z, w = params[FORMULA_STATE_W_SLOT];

    // XY plane: match the original ASM/.m3f contract directly.
    double x_out = (y - Alpha * cos(Beta * x)) * Strength;
    double y_out = (Gamma - x) * Strength;

    // The wrapper supplies the actual W coordinate through its dedicated state slot.
    double z_out = (w - Alpha * cos(Beta * z)) * Strength;
    double w_out = (Delta - z) * Strength;

    params[FORMULA_STATE_W_SLOT] = w_out;

    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info__cosmartin = {
    .id = FORMULA__COSMARTIN,
    .name = "_cosmartin",
    .param_count = 5,
    .complexity = 91,
    .uses_transcendental = true
};

/*
Original M3 Description:
CosMartin deformation in 4D.
Inspired by CosMartin formula from Mark Townsend.
Formula by Luca GN 2011.

Full 4D system:
  x' = (y - Alpha*cos(Beta*x)) * Strength
  y' = (Gamma - x) * Strength
  z' = (w - Alpha*cos(Beta*z)) * Strength
  w' = (Delta - z) * Strength

3D projection (w=0):
  x' = (y - Alpha*cos(Beta*x)) * Strength
  y' = (Gamma - x) * Strength
  z' = -Alpha*cos(Beta*z) * Strength

Parameters:
  [0] Strength: double (default: 1.0) - overall scaling
  [1] Alpha: double (default: 0.5) - cosine amplitude
  [2] Beta: double (default: 0.75) - cosine frequency
  [3] Gamma: double (default: 1.0) - y offset
  [4] Delta: double (default: 1.0) - w offset (4D only)
*/
