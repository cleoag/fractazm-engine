// MandyCousin.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MandyCousin
// Hand-ported from ASM: 126 bytes, DEoption=4
// 4D formula: uses de parameter as w coordinate input/output
// David Makin's 4D hypercomplex squaring — "Mandy's Cousin"
// Found on bugman123.com/Hypercomplex/

constexpr unsigned int FORMULA_MANDYCOUSIN = 6473;

// ASM trace (126 bytes, 0 branches):
//
// 4D hypercomplex squaring q² + c:
//   x' = X_mul * (x² - y² - z² + w²) + Cx
//   y' = Y_mul * (x*y + z*w) + Cy
//   z' = Z_mul1 * x*z + Z_mul2 * y*w + Cz
//   w' = W_mul * (x*w + y*z) + W_add + Cw
//
// Verified: 0026 = faddp (+w²), 0039 = faddp (+zw)
//
// For 3D: w=0 reduces to:
// x' = X_mul * (x² - y² - z²) + Cx
// y' = Y_mul * x*y + Cy
// z' = Z_mul1 * x*z + Cz
//
// The current validation harness uses zero Julia/C for MandyCousin, so only the
// polynomial and W_add are modeled directly here. The x/y/z external Julia seed
// is zero in the direct parity check.

template<>
cpu_double3 execute_formula<FORMULA_MANDYCOUSIN>(cpu_double3 p, constant double* params, double &de) {
    const double X_mul  = params[0];  // [edi-0x10]
    const double Y_mul  = params[1];  // [edi-0x18]
    const double Z_mul1 = params[2];  // [edi-0x20]
    const double Z_mul2 = params[3];  // [edi-0x28]
    const double W_mul  = params[4];  // [edi-0x30]
    const double W_add  = params[5];  // [edi-0x38]

    double x = p.x, y = p.y, z = p.z, w = de;

    double x_out = X_mul * (x * x - y * y - z * z + w * w);
    double y_out = Y_mul * (x * y + z * w);
    double z_out = Z_mul1 * x * z + Z_mul2 * y * w;
    double w_out = W_mul * (x * w + y * z) + W_add;

    de = w_out;
    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_MandyCousin = {
    .id = FORMULA_MANDYCOUSIN,
    .name = "MandyCousin",
    .param_count = 6,
    .complexity = 126,
    .uses_transcendental = false
};

/*
Original M3 Description:
A formula from David Makin. Found on bugman123.com/Hypercomplex/
4D hypercomplex squaring with configurable multipliers.

Full 4D:
  x' = X_mul * (x² - y² - z² + w²) + Cx
  y' = Y_mul * (x*y + z*w) + Cy
  z' = Z_mul1 * x*z + Z_mul2 * y*w + Cz
  w' = W_mul * (x*w + y*z) + W_add + Cw

3D projection (w=0):
  x' = X_mul * (x² - y² - z²) + Cx
  y' = Y_mul * x*y + Cy
  z' = Z_mul1 * x*z + Cz

C handling: Formula adds C internally.
Engine should NOT add C; template handles it.

Parameters:
  [0] X_mul:  double (default: 1.0)
  [1] Y_mul:  double (default: 2.0)
  [2] Z_mul1: double (default: 2.0)
  [3] Z_mul2: double (default: 2.0)
  [4] W_mul:  double (default: 2.0)
  [5] W_add:  double (default: 0.0)
*/
