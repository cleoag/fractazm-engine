// MandyCousin2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MandyCousin2
// Hand-ported from ASM: 126 bytes, DEoption=4
// Stig Pettersson's "MQuaternion" — sign-flipped variant of MandyCousin.
// Two opcodes differ from MandyCousin: fsubp vs faddp.

constexpr unsigned int FORMULA_MANDYCOUSIN2 = 3557;

// ASM trace (126 bytes, 0 branches):
//
// Identical to MandyCousin except:
//   0026: fsubp st(1) instead of faddp → -w² instead of +w²
//   0039: fsubp st(1) instead of faddp → -z*w instead of +z*w
//
// 4D hypercomplex squaring variant:
// x' = X_mul * (x² - y² - z² - w²) + Cx    ← -w² (MandyCousin: +w²)
// y' = Y_mul * (x*y - z*w) + Cy              ← -z*w (MandyCousin: +z*w)
// z' = Z_mul1 * x*z + Z_mul2 * y*w + Cz     (same)
// w' = W_mul * (x*w + y*z) + W_add + Cw      (same)
//
// For 3D: w=0 reduces to (identical to MandyCousin 3D):
// x' = X_mul * (x² - y² - z²) + Cx
// y' = Y_mul * x*y + Cy
// z' = Z_mul1 * x*z + Cz
//
// C handling: Formula handles C internally (DEoption=4).

template<>
cpu_double3 execute_formula<FORMULA_MANDYCOUSIN2>(cpu_double3 p, constant double* params, double &de) {
    const double X_mul  = params[0];  // [edi-0x10] (default: 1.0)
    const double Y_mul  = params[1];  // [edi-0x18] (default: 2.0)
    const double Z_mul1 = params[2];  // [edi-0x20] (default: 2.0)
    const double Z_mul2 = params[3];  // [edi-0x28] (default: -2.0)
    const double W_mul  = params[4];  // [edi-0x30] (default: 2.0)
    const double W_add  = params[5];  // [edi-0x38] (default: 0.0)

    // Engine/harness-provided C values. For current validation cases these are zero,
    // but the slots are present in the fixed-size params buffer.
    const double Cx     = params[6];  // [esi+0x18]
    const double Cy     = params[7];  // [esi+0x20]
    const double Cz     = params[8];  // [esi+0x28]
    const double Cw     = params[9];  // [esi-0x38]

    double x = p.x, y = p.y, z = p.z, w = de;

    double x_out = X_mul * (x * x - y * y - z * z - w * w) + Cx;
    double y_out = Y_mul * (x * y - z * w) + Cy;
    double z_out = Z_mul1 * x * z + Z_mul2 * y * w + Cz;
    double w_out = W_mul * (x * w + y * z) + W_add + Cw;

    de = w_out;
    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_MandyCousin2 = {
    .id = FORMULA_MANDYCOUSIN2,
    .name = "MandyCousin2",
    .param_count = 10,
    .complexity = 126,
    .uses_transcendental = false
};

/*
Original M3 Description:
A formula from Stig Pettersson (sp.ufm). Called "MQuaternion".
Very similar to MandyCousin, only two terms with different sign.

Full 4D:
  x' = X_mul * (x² - y² - z² - w²) + Cx    ← note: -w²
  y' = Y_mul * (x*y - z*w) + Cy              ← note: -z*w
  z' = Z_mul1 * x*z + Z_mul2 * y*w + Cz
  w' = W_mul * (x*w + y*z) + W_add + Cw

3D projection (w=0):
  x' = X_mul * (x² - y² - z²) + Cx
  y' = Y_mul * x*y + Cy
  z' = Z_mul1 * x*z + Cz

Note: In 3D (w=0), this produces identical results to MandyCousin.
The sign differences only manifest in 4D rendering.

Parameters:
  [0] X_mul:  double (default: 1.0)
  [1] Y_mul:  double (default: 2.0)
  [2] Z_mul1: double (default: 2.0)
  [3] Z_mul2: double (default: -2.0)
  [4] W_mul:  double (default: 2.0)
  [5] W_add:  double (default: 0.0)
  [6..9] Cx, Cy, Cz, Cw: passed by engine/harness from context
*/
