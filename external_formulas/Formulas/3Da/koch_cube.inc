// koch_cube.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: koch_cube
// Hand-ported from ASM: 370 bytes, DEoption=2
// Cubic Koch fractal inspired by Wikipedia's Quadratic Koch 3D image

constexpr unsigned int FORMULA_KOCH_CUBE = 10175;

// ASM trace summary:
//   0x14-0x2: x=|x|*3, y=|y|*3, z=|z|*3  ([edi+8]=constant 3)
//   0x31-0x3D: DE = DE * 3 / XY_Stretch * Post_scale
//   0x43-0x82: Sort descending: x >= y >= z (in-memory swap pattern)
//   0x84-0xCA: Add offsets (XAdd,YAdd,ZAdd), apply rotation matrix
//   0xD3-0xDF: Z fold: z = ZFold - |ZFold - z|
//   0xE2-0xF4: Pre-compute bounds: a = 3+stretch, b = 3-stretch
//   0xF8-0x109: c = x - b, d = x - a
//   0x10A-0x14A: Three-way branch (Koch cube face logic):
//     if c < y: x=c, y=y-b          (outside small cube: normalize)
//     elif d > y: x=d                (just over small cube)
//     else: swap x=y, y=d           (vertical faces: swap)
//   0x14C-0x167: Final non-conformal scaling:
//     x = x * Post_scale / XY_Stretch
//     y = y * Post_scale / XY_Stretch
//     z = z * Post_scale

template<>
cpu_double3 execute_formula<FORMULA_KOCH_CUBE>(cpu_double3 p, constant double* params, double &de) {
    const double Post_scale  = params[0];   // [edi-0x10] default: 1
    const double XY_Stretch  = params[1];   // [edi-0x18] default: 1
    const double ZFold       = params[2];   // [edi-0x20] default: 1
    const double X_Add       = params[3];   // [edi-0x28] default: 0
    const double Y_Add       = params[4];   // [edi-0x30] default: 0
    const double Z_Add       = params[5];   // [edi-0x38] default: 0

    // Rotation matrix from 9 pre-computed float elements (from .3SingleAngles)
    RotMatrix3 R = rotMatrix3FromFloats(params, 6);

    // Constants from [CONSTANTS] section
    // const double const2 = 2.0;  // [edi+0x00] - not used in main body
    const double const3 = 3.0;     // [edi+0x08] - the cube scale
    // const double const4 = 4.0;  // [edi+0x10] - not used in main body

    // Step 1: Absolute value and scale by 3
    double x = abs(p.x) * const3;
    double y = abs(p.y) * const3;
    double z = abs(p.z) * const3;

    // DE tracking: de = de * 3 / XY_Stretch * Post_scale
    de = de * const3 / XY_Stretch * Post_scale;

    // Step 2: Sort descending (x >= y >= z)
    if (x < y) { double t = x; x = y; y = t; }
    if (x < z) { double t = x; x = z; z = t; }
    if (y < z) { double t = y; y = z; z = t; }

    // Step 3: Add offsets
    z += Z_Add;
    y += Y_Add;
    x += X_Add;

    // Step 4: Apply rotation matrix
    cpu_double3 rv = R * cpu_double3(x, y, z);
    x = rv.x;
    y = rv.y;
    z = rv.z;

    // Step 5: Z fold: z = ZFold - |ZFold - z|
    z = ZFold - abs(ZFold - z);

    // Step 6: Koch cube face logic
    double a = const3 + XY_Stretch;   // 3 + stretch
    double b = const3 - XY_Stretch;   // 3 - stretch
    double c = x - b;                 // x - (3-stretch)
    double d = x - a;                 // x - (3+stretch)

    // Three-way branch:
    // Compare c vs y: if c < y (y is above c), case 1
    // Compare d vs y: if d > y (y is below d), case 2
    // Otherwise: case 3 (swap)
    if (c < y) {
        // Case 1: out of the cube - normalize
        x = c;
        y = y - b;
    } else if (d > y) {
        // Case 2: point falls just over the small cube
        x = d;
        // y unchanged
    } else {
        // Case 3: vertical faces of small cube (swap)
        double t = y;
        y = d;
        x = t;
    }

    // Step 7: Final non-conformal scaling
    // x and y get divided by stretch, then all multiplied by Post_scale
    x = x * Post_scale / XY_Stretch;
    y = y * Post_scale / XY_Stretch;
    z = z * Post_scale;

    return cpu_double3(x, y, z);
}

// Formula metadata
constant FormulaInfo formula_info_koch_cube = {
    .id = FORMULA_KOCH_CUBE,
    .name = "koch_cube",
    .param_count = 9,
    .complexity = 900,
    .uses_transcendental = true
};

/*
Original M3 Description:
Cubic Koch fractal. Inspired from a Wikipedia image
(http://en.wikipedia.org/wiki/File:Quadratic_Koch_3D_(type1_stage2).png)
Added options for scale, stretching and other goodies.

TIP - With default settings you don't see the fractal correctly; decrease
DEstop, and use miniter=10 or something to see a better plot. Be careful
with scale and stretching tweaks, they greatly affect the render!
If you use xy stretch, the fractal will be less uniform but with some
singularities.

Mathematical Formula:
  1. x=|x|*3, y=|y|*3, z=|z|*3
  2. Sort descending: x >= y >= z
  3. Add offsets (XAdd, YAdd, ZAdd)
  4. Apply rotation
  5. Z fold: z = ZFold - |ZFold - z|
  6. a=3+stretch, b=3-stretch, c=x-b, d=x-a
     if c < y: x=c, y=y-b
     elif d > y: x=d
     else: t=x; x=y; y=d
  7. x = x*PostScale/stretch, y = y*PostScale/stretch, z = z*PostScale

Parameters:
  [0]  Post-scale: double (default: 1)
  [1]  XY Stretch: double (default: 1)
  [2]  Z Fold: double (default: 1)
  [3]  X Add: double (default: 0)
  [4]  Y Add: double (default: 0)
  [5]  Z Add: double (default: 0)
  [6-14] Rotation matrix: 9 singles (default: identity)

Constants:
  [edi+0x00] = 2.0
  [edi+0x08] = 3.0 (the cubic scale)
  [edi+0x10] = 4.0

PVar layout:
  [PVar-0x10] = Post-scale
  [PVar-0x18] = XY Stretch
  [PVar-0x20] = Z Fold
  [PVar-0x28] = X Add
  [PVar-0x30] = Y Add
  [PVar-0x38] = Z Add
  [PVar-0x3C..-0x5C] = Rotation matrix (9 singles)

FORMULA AND IMPLEMENTATION; 100% by Luca G.N. 2011.
*/
