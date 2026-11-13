// CubicQuat.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: CubicQuat
// Hand-ported from ASM: 132 bytes, DEoption=4
// Gilles Nadau (2003) cubic quaternion with twist parameters.

constexpr unsigned int FORMULA_CUBICQUAT = 3630;

// ASM trace (132 bytes, 0 branches):
//
// Quaternion squaring with per-component multipliers and twist:
// x' = X_mul*x² - (Y_mul*y² + Z_mul*z² + W_mul*w²) + Cx + Cy*X_twist
// y' = 2*Y_mul*x*y + Cy + Cx*Y_twist
// z' = 2*Z_mul*x*z + Cz
// w' = 2*W_mul*x*w + W_add + Cw
//
// The twist params cross-couple Cx and Cy:
//   x' gets +Cy*X_twist from [edi-0x38]
//   y' gets +Cx*Y_twist from [edi-0x40]
//
// For 3D: w=0 reduces to:
// x' = X_mul*x² - Y_mul*y² - Z_mul*z² + Cx + Cy*X_twist
// y' = 2*Y_mul*x*y + Cy + Cx*Y_twist
// z' = 2*Z_mul*x*z + Cz
//
// C handling: Formula handles C internally (DEoption=4).

template<>
cpu_double3 execute_formula<FORMULA_CUBICQUAT>(cpu_double3 p, constant double* params, double &de) {
    const double X_mul    = params[0];  // [edi-0x10] (default: 1.0)
    const double Y_mul    = params[1];  // [edi-0x18] (default: 1.0)
    const double Z_mul    = params[2];  // [edi-0x20] (default: 1.0)
    const double W_mul    = params[3];  // [edi-0x28] (default: 1.0)
    const double W_add    = params[4];  // [edi-0x30] (default: 0.0)
    const double X_twist  = params[5];  // [edi-0x38] (default: 0.0)
    const double Y_twist  = params[6];  // [edi-0x40] (default: 0.0)
    const double Cx       = params[7];
    const double Cy       = params[8];
    const double Cz       = params[9];
    const double Cw       = params[10];

    double x = p.x, y = p.y, z = p.z, w = de;

    double x_out = X_mul * x * x - (Y_mul * y * y + Z_mul * z * z + W_mul * w * w)
                 + Cx + Cy * X_twist;
    double y_out = 2.0 * Y_mul * x * y + Cy + Cx * Y_twist;
    double z_out = 2.0 * Z_mul * x * z + Cz;
    double w_out = 2.0 * W_mul * x * w + W_add + Cw;

    de = w_out;
    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_CubicQuat = {
    .id = FORMULA_CUBICQUAT,
    .name = "CubicQuat",
    .param_count = 11,
    .complexity = 132,
    .uses_transcendental = false
};

/*
Original M3 Description:
Cubic quaternion from Gilles Nadau (2003) with twist parameters.

Full 4D:
  x' = X_mul*x² - (Y_mul*y² + Z_mul*z² + W_mul*w²) + Cx + Cy*X_twist
  y' = 2*Y_mul*x*y + Cy + Cx*Y_twist
  z' = 2*Z_mul*x*z + Cz
  w' = 2*W_mul*x*w + W_add + Cw

3D projection (w=0):
  x' = X_mul*x² - Y_mul*y² - Z_mul*z² + Cx + Cy*X_twist
  y' = 2*Y_mul*x*y + Cy + Cx*Y_twist
  z' = 2*Z_mul*x*z + Cz

C handling: Formula adds C internally.
Engine should NOT add C; template handles it.

Parameters:
  [0] X_mul:   double (default: 1.0) - x² coefficient
  [1] Y_mul:   double (default: 1.0) - y component multiplier
  [2] Z_mul:   double (default: 1.0) - z component multiplier
  [3] W_mul:   double (default: 1.0) - w component multiplier
  [4] W_add:   double (default: 0.0) - additive term for w
  [5] X_twist: double (default: 0.0) - Cy cross-coupling into x
  [6] Y_twist: double (default: 0.0) - Cx cross-coupling into y
  [7..10] Cx, Cy, Cz, Cw: passed by engine from context
*/
