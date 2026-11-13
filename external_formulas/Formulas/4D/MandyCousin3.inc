// MandyCousin3.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MandyCousin3
// Hand-ported from ASM: 122 bytes, DEoption=4
// Complexified quaternions from T. Gintz's site.
// MandyCousin variant with +z² and +w² in x component.

constexpr unsigned int FORMULA_MANDYCOUSIN3 = 7864;

// ASM trace (122 bytes, 0 branches):
//
// Differs from MandyCousin:
//   - x component: no fchs on z² → +z², and faddp on w² → +w²
//   - w component: fsubp at 005E → x*w - y*z (not x*w + y*z)
//   - 4 bytes shorter (122 vs 126) due to fewer fchs ops
//
// 4D complexified quaternion squaring:
// x' = X_mul * (x² - y² + z² + w²) + Cx
// y' = Y_mul * (x*y + z*w) + Cy
// z' = Z_mul1 * x*z + Z_mul2 * y*w + Cz
// w' = W_mul * (x*w - y*z) + W_add + Cw
//
// w is carried in `de` for CPU parity, matching the other 4D formula ports.
// C handling: formula adds C internally.

template<>
cpu_double3 execute_formula<FORMULA_MANDYCOUSIN3>(cpu_double3 p, constant double* params, double &de) {
    const double X_mul  = params[0];  // [edi-0x10]
    const double Y_mul  = params[1];  // [edi-0x18]
    const double Z_mul1 = params[2];  // [edi-0x20]
    const double Z_mul2 = params[3];  // [edi-0x28]
    const double W_mul  = params[4];  // [edi-0x30]
    const double W_add  = params[5];  // [edi-0x38]

    const double Cx = params[6];      // [esi+0x18]
    const double Cy = params[7];      // [esi+0x20]
    const double Cz = params[8];      // [esi+0x28]
    const double Cw = params[9];      // [esi-0x38]

    const double x = p.x;
    const double y = p.y;
    const double z = p.z;
    const double w = de;

    const double x_out = X_mul * (x * x - y * y + z * z + w * w) + Cx;
    const double y_out = Y_mul * (x * y + z * w) + Cy;
    const double z_out = Z_mul1 * x * z + Z_mul2 * y * w + Cz;
    const double w_out = W_mul * (x * w - y * z) + W_add + Cw;

    de = w_out;
    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_MandyCousin3 = {
    .id = FORMULA_MANDYCOUSIN3,
    .name = "MandyCousin3",
    .param_count = 10,
    .complexity = 122,
    .uses_transcendental = false
};

/*
Original M3 Description:
Complexified quaternions; found in T. Gintz's site.
Almost identical to MandyCousin but different results are possible.

Full 4D:
  x' = X_mul * (x² - y² + z² + w²) + Cx    ← +z², +w²
  y' = Y_mul * (x*y + z*w) + Cy
  z' = Z_mul1 * x*z + Z_mul2 * y*w + Cz
  w' = W_mul * (x*w - y*z) + W_add + Cw      ← -y*z

3D projection (w=0):
  x' = X_mul * (x² - y² + z²) + Cx
  y' = Y_mul * x*y + Cy
  z' = Z_mul1 * x*z + Cz

Note: In 3D the +z² term makes this distinct from MandyCousin/MandyCousin2.

Parameters:
  [0] X_mul:  double (default: 1.0)
  [1] Y_mul:  double (default: 2.0)
  [2] Z_mul1: double (default: 2.0)
  [3..5] Cx, Cy, Cz: passed by engine from context
  4D-only: Z_mul2 [edi-0x28]=-2.0, W_mul [edi-0x30]=2.0, W_add [edi-0x38]=0.0
*/
