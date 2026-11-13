// Hopf4D.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Hopf4D
// Hand-ported from ASM: 163 bytes, DEoption=4
// Hopf fibration squaring formula from Bugman (fractalforums.com).

constexpr unsigned int FORMULA_HOPF4D = 9306;

// ASM trace (163 bytes, 0 branches):
//
// Based on Hopf map S³→S² applied to quaternion squaring.
// Uses local stack variables [ebp-8] and [ebp-0x10].
//
// A = x² + y²           (xy pair squared norm)
// B = z² + w²           (zw pair squared norm)
// scale1 = 1 - B/A      = (A-B)/A
// scale2 = 2*sqrt(A/B)
//
// x' = (x²-y²) * scale1 + Cx
// y' = 2*x*y * scale1 + Cy
// z' = (z²-w²) * scale2 * Z_mul + Cz
// w' = z*w * scale2 + Cw + W_add
//
// For 3D: w=0, so B=z², scale2=2*sqrt(A/z²):
// x' = (x²-y²) * (1 - z²/(x²+y²)) + Cx
// y' = 2*x*y * (1 - z²/(x²+y²)) + Cy
// z' = z² * 2*sqrt((x²+y²)/z²) * Z_mul + Cz
//    = 2*|z|*sqrt(x²+y²) * Z_mul + Cz
//
// C handling: Formula handles C internally (DEoption=4).

template<>
cpu_double3 execute_formula<FORMULA_HOPF4D>(cpu_double3 p, constant double* params, double &de) {
    const double W_add = params[0];   // [edi-0x10] (default: 0.0)
    const double Z_mul = params[1];   // [edi-0x18] (default: -1.0)
    const double Cx = params[2];      // [esi+0x18]
    const double Cy = params[3];      // [esi+0x20]
    const double Cz = params[4];      // [esi+0x28]
    const double Cw = params[5];      // [esi-0x38]

    const double x = p.x;
    const double y = p.y;
    const double z = p.z;
    const double w = de;

    const double A = x * x + y * y;
    const double B = z * z + w * w;
    const double scale1 = 1.0 - B / A;
    const double scale2 = 2.0 * sqrt(A / B);

    const double x_out = (x * x - y * y) * scale1 + Cx;
    const double y_out = 2.0 * x * y * scale1 + Cy;
    const double z_out = (z * z - w * w) * scale2 * Z_mul + Cz;
    const double w_out = z * w * scale2 + Cw + W_add;

    de = w_out;
    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_Hopf4D = {
    .id = FORMULA_HOPF4D,
    .name = "Hopf4D",
    .param_count = 6,
    .complexity = 163,
    .uses_transcendental = false
};

/*
Original M3 Description:
Hopf fibration squaring formula, from Bugman.
http://www.fractalforums.com/theory/3d-mandelbrot-formula-based-on-the-hopf-map/

Full 4D:
  A = x² + y²
  B = z² + w²
  scale1 = (A - B) / A
  scale2 = 2*sqrt(A/B)
  x' = (x²-y²) * scale1 + Cx
  y' = 2*x*y * scale1 + Cy
  z' = (z²-w²) * scale2 * Z_mul + Cz
  w' = z*w * scale2 + Cw + W_add

3D projection (w=0):
  x' = (x²-y²) * (1 - z²/(x²+y²)) + Cx
  y' = 2*x*y * (1 - z²/(x²+y²)) + Cy
  z' = 2*|z|*sqrt(x²+y²) * Z_mul + Cz

Parameters:
  [0] W_add: double (default: 0.0)
  [1] Z_mul: double (default: -1.0) - z-component scaling
  [2..5] Cx, Cy, Cz, Cw: passed by engine/harness from context
*/
