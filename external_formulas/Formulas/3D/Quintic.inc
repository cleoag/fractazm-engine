// Quintic.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Quintic
// Hand-ported from ASM: 424 bytes, DEoption=0
// Quintic polynomial fractal by Luca GN 2011, based on Paul Bird's work.
// "Many of its 2D sections are the power 5 Mandelbrot."
// Each axis output is a degree-5 polynomial with cyclic symmetry.

constexpr unsigned int FORMULA_QUINTIC = 2861;

// ASM trace (424 bytes, 1 branch for sign):
//
// Precompute:
//   x², y², z²                   (ebp-8, ebp-0x10, ebp-0x18)
//   x³, y³, z³                   (ebp-0x40, ebp-0x48, ebp-0x50)
//   xy, xz, yz                   (ebp-0x20, ebp-0x28, ebp-0x30)
//   xyz                          (ebp-0x38)
//
// Sign: if [edi-0x2c] != 0: sign = -1, else sign = +1
//
// For x output (y,z are cyclic permutations):
//   T1 = x⁵
//   T2 = (y²+z² + yz*A) * x³ * A0
//   T3 = (y⁴+z⁴ + y³z*B + y²z²*C + yz³*B) * x * A1
//   T4 = x² * yz * (y+z) * D
//   x_out = sign * (T1 + T2 + T3 + T4) + Cx
//
// Cyclic permutation for y: replace (x→y, y→z, z→x)
// Cyclic permutation for z: replace (x→z, y→x, z→y)
//
// C handling: Formula handles C internally (DEoption=0).

template<>
cpu_double3 execute_formula<FORMULA_QUINTIC>(cpu_double3 p, constant double* params, double &de) {
    const double A        = params[0];  // [edi-0x10] (default: 0)
    const double B        = params[1];  // [edi-0x18] (default: 0)
    const double C        = params[2];  // [edi-0x20] (default: 0)
    const double D        = params[3];  // [edi-0x28] (default: 0)
    const int   Negative = int(params[4]); // [edi-0x2c] (default: 1)
    // [CONSTANTS] embedded as compile-time values:
    const double A0       = -10.0;    // [edi+0x00]
    const double A1       = 5.0;      // [edi+0x08]
    // C values passed by engine from context
    const double Cx       = params[7];
    const double Cy       = params[8];
    const double Cz       = params[9];

    double x = p.x, y = p.y, z = p.z;

    // Precompute squares
    double x2 = x * x, y2 = y * y, z2 = z * z;
    // Precompute cubes
    double x3 = x2 * x, y3 = y2 * y, z3 = z2 * z;
    // Precompute cross products
    double xy = x * y, xz = x * z, yz = y * z;
    double xyz = xy * z;

    // Sign toggle
    double sign = (Negative != 0) ? -1.0 : 1.0;

    // X output: f(x; y, z)
    double xT1 = x3 * x2;                                                  // x⁵
    double xT2 = (y2 + z2 + yz * A) * x3 * A0;                           // cubic term
    double xT3 = (y2*y2 + z2*z2 + y3*z*B + yz*yz*C + yz*z2*B) * x * A1; // quintic cross
    double xT4 = x2 * yz * (y + z) * D;                                   // mixed term
    double x_out = sign * (xT1 + xT2 + xT3 + xT4) + Cx;

    // Y output: f(y; z, x) — cyclic permutation x→y, y→z, z→x
    double yT1 = y3 * y2;                                                  // y⁵
    double yT2 = (z2 + x2 + xz * A) * y3 * A0;                           // cubic term
    double yT3 = (z2*z2 + x2*x2 + xz*z2*B + xz*xz*C + x2*xz*B) * y * A1; // quintic cross
    double yT4 = y2 * xz * (z + x) * D;                                   // mixed term
    double y_out = sign * (yT1 + yT2 + yT3 + yT4) + Cy;

    // Z output: f(z; x, y) — cyclic permutation x→z, y→x, z→y
    double zT1 = z3 * z2;                                                  // z⁵
    double zT2 = (x2 + y2 + xy * A) * z3 * A0;                           // cubic term
    double zT3 = (x2*x2 + y2*y2 + x3*y*B + xy*xy*C + xy*y2*B) * z * A1; // quintic cross
    double zT4 = z2 * xy * (x + y) * D;                                   // mixed term
    double z_out = sign * (zT1 + zT2 + zT3 + zT4) + Cz;

    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_Quintic = {
    .id = FORMULA_QUINTIC,
    .name = "Quintic",
    .param_count = 8,
    .complexity = 424,
    .uses_transcendental = false
};

/*
Original M3 Description:
Polynomial fractal found on... Wikipedia (!), shows the property that many of its 2D
sections are the power 5 Mandelbrot.

The negative mode is very nice (can be disabled setting 0 to its value).
Interesting Julias too.

Based on Paul Bird's work (date august 2011). — Luca GN 2011

Mathematical Formula:
  For each axis i with cyclic partners (j, k):
    Ti_1 = i⁵
    Ti_2 = (j²+k² + jk*A) * i³ * A0
    Ti_3 = (j⁴+k⁴ + j³k*B + j²k²*C + jk³*B) * i * A1
    Ti_4 = i² * jk * (j+k) * D
    i_out = sign * (Ti_1 + Ti_2 + Ti_3 + Ti_4) + Ci

  Cyclic assignments:
    x: (i,j,k) = (x,y,z)
    y: (i,j,k) = (y,z,x)
    z: (i,j,k) = (z,x,y)

  sign = (Negative != 0) ? -1 : +1

C handling: Standard addition of Ci per axis.
Engine should NOT add C; template handles it.

Parameters:
  [0] A: double (default: 0) - quadratic cross coefficient
  [1] B: double (default: 0) - cubic cross coefficient
  [2] C: double (default: 0) - quartic cross coefficient
  [3] D: double (default: 0) - mixed quintic coefficient
  [4] Negative: int (default: 1) - 0=positive, nonzero=negative
  [5] A0: double (default: -10) - cubic term scale
  [6] A1: double (default: 5) - quintic cross scale
  [7..9] Cx, Cy, Cz: passed by engine from context
*/
