// CommQuat.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: CommQuat
// Hand-ported from ASM: 118 bytes, DEoption=4
// Commutative quaternion squaring by Fracmonk.
// Unlike Hamilton quaternions (ij=-ji), here ij=ji.

constexpr unsigned int FORMULA_COMMQUAT = 8800;

// ASM trace (118 bytes, 0 branches):
//
// Commutative quaternion squaring: q² + c
// where q = x + yi + zj + wk with ij = ji (commutative)
//
// x' = X_mul * (x² - y² - 2zw) + Cx
// y' = Y_mul * (2xy + z² - w²) + Cy
// z' = Z_mul * 2(xz - yw) + Cz
// w' = W_mul * 2(xw + yz) + W_add + Cw
//
// For 3D: w=0 reduces to:
// x' = X_mul * (x² - y²) + Cx
// y' = Y_mul * (2xy + z²) + Cy
// z' = Z_mul * 2xz + Cz
//
// C handling: Formula handles C internally (DEoption=4).

template<>
cpu_double3 execute_formula<FORMULA_COMMQUAT>(cpu_double3 p, double* params, double &de) {
    const double X_mul = params[0];  // [edi-0x10] (default: 1.0)
    const double Y_mul = params[1];  // [edi-0x18] (default: 1.0)
    const double Z_mul = params[2];  // [edi-0x20] (default: 1.0)
    const double W_mul = params[3];  // [edi-0x28] (default: 1.0)
    const double W_add = params[4];  // [edi-0x30] (default: 0.0)

    // Engine/harness-provided C values. For current validation cases these are zero,
    // but the slots are present in the fixed-size params buffer.
    const double Cx = params[5];     // [esi+0x18]
    const double Cy = params[6];     // [esi+0x20]
    const double Cz = params[7];     // [esi+0x28]
    const double Cw = params[8];     // [esi-0x38]

    double x = p.x, y = p.y, z = p.z, w = de;

    double x_out = X_mul * (x * x - y * y - 2.0 * z * w) + Cx;
    double y_out = Y_mul * (2.0 * x * y + z * z - w * w) + Cy;
    double z_out = Z_mul * 2.0 * (x * z - y * w) + Cz;
    double w_out = W_mul * 2.0 * (x * w + y * z) + W_add + Cw;

    de = w_out;
    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_CommQuat = {
    .id = FORMULA_COMMQUAT,
    .name = "CommQuat",
    .param_count = 9,
    .complexity = 118,
    .uses_transcendental = false
};

/*
Original M3 Description:
A formula from Fracmonk on fractalforums.com.
Commutative quaternion squaring q² + c.

Mathematical Formula (3D, w=0):
  x' = X_mul * (x² - y²) + Cx
  y' = Y_mul * (2xy + z²) + Cy
  z' = Z_mul * 2xz + Cz

Full 4D:
  x' = X_mul * (x² - y² - 2zw) + Cx
  y' = Y_mul * (2xy + z² - w²) + Cy
  z' = Z_mul * 2(xz - yw) + Cz
  w' = W_mul * 2(xw + yz) + W_add + Cw

C handling: Formula adds C internally.
Engine should NOT add C; template handles it.

Parameters:
  [0] X_mul: double (default: 1.0) - x component multiplier
  [1] Y_mul: double (default: 1.0) - y component multiplier
  [2] Z_mul: double (default: 1.0) - z component multiplier
  [3] W_mul: double (default: 1.0) - w component multiplier
  [4] W_add: double (default: 0.0) - additive offset for w
  [5..8] Cx, Cy, Cz, Cw: passed by engine/harness from context
*/
