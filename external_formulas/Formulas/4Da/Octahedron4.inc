// Octahedron4.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Octahedron4
// Hand-ported from ASM: 343 bytes, DEoption=6
// 4D IFS formula: uses de parameter as w coordinate input/output
// 4D Octahedron KIFS with abs-sort folding (Menger-style for x,y,z + mixed w folds)

constexpr unsigned int FORMULA_OCTAHEDRON4 = 10347;

// ASM trace:
//   Fold pattern from description and ASM:
//   1. Take absolute values: x=|x|, y=|y|, z=|z|, w=|w|
//   2. Sort x,y,z descending (octahedral fold like Menger):
//      if(x<y) swap(x,y)
//      if(-x<y) swap... no, after abs and sort, the extra checks involve signs.
//
//   Actually, from the Octahedron4.m3 description:
//     x=abs(x);y=abs(y);z=abs(z);w=abs(w);
//     if(x-y<0){swap(x,y)}
//     if(x-z<0){swap(x,z)}
//     if(y-z<0){swap(y,z)}
//     // fold w like in Menger4 (sort w among sorted x,y,z)
//
//   Looking at ASM more carefully:
//   0x000-0x001E: fld w=|w|, fld z=|z|, fld y=|y|, fld x=|x|
//   0x0020-0x002A: if x < y: swap → sort (x >= y)
//     But then: 0x002A: fchs, 0x002C: fcom st1 (jb 0x36) → if -x < y → skip swap
//     Then fchs back. This is: if(x+y<0) but after abs, x+y>=0 always! So this never fires.
//     Wait, but the ASM has abs first. So x,y,z,w are all non-negative.
//     Then why check -x vs y? After abs, -x <= 0 and y >= 0, so -x < y always (except both zero).
//     So the "half-oct" checks after abs never fire — they become no-ops.
//
//   Hmm, looking again: the fold in Octahedron4 is:
//     abs all 4 → sort x,y descending → half-oct(x,y) [no-op after abs]
//     → sort x,z → half-oct(x,z) [no-op]
//     Then for w: sort(x,w), sort(y,w), sort(z,w) using descending sort only
//     So effectively: abs + full descending sort of all 4 values.
//
//   This is the SAME fold as Menger4! The abs + sort is the octahedral fold.
//   The key difference from Menger4 is subtle — looking at the sort network,
//   Menger4 does 6 comparisons on abs values with jae conditions,
//   Octahedron4 does the same but with mixed jae/jb and extra fchs operations.
//
//   After tracing: the fold patterns differ in edge cases and the specific
//   sort ordering. But for abs values, the result is the same: descending sort.
//
//   Looking at the ASM, the compare conditions are:
//   0x0020: fcom st1 → x vs y, jae → if x >= y, no swap → sort descending ✓
//   0x002A: fchs → -x, fcom st1 → -x vs y, jb → if -x < y, no swap ✓ (always after abs)
//   0x0038: fcom st2 → x vs z, jae → sort ✓
//   0x0044: fchs → -x, fcom st2 → jb ✓ (always after abs)
//   0x0050: fxch st1 → bring y to st0, fcom st3 → y vs w, jae → sort ✓
//   0x0062: fcom st3 → next pair, jae → sort ✓
//   0x0074: fcom st3 → last pair, jae → sort ✓
//
//   So: abs → full 4-way descending sort. Same as Menger4.
//   The only potential difference is the fchs/jb checks which are no-ops after abs.
//
//   After fold: same rotation + Sierpinski-style linear scaling (like Sierpinski4, not Menger).
//   Wait, let me check: does Octahedron4 use Menger z-fold or linear scaling?
//
//   Looking at 0x00F4-0x014E: this is the SAME Sierpinski-style linear scaling as Sierpinski4.
//   So: abs-sort fold + rotation + linear scale (no Menger cross fold on z).

template<>
cpu_double3 execute_formula<FORMULA_OCTAHEDRON4>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    const double CX = params[1];
    const double CY = params[2];
    const double CZ = params[3];
    const double CW = params[4];

    // 4D rotation matrix: 6 raw angles (degrees) → buildRotMatrix4d
    // 4D rotation matrix: 16 pre-computed floats at params[5..20]

    double x = p.x, y = p.y, z = p.z, w = de;

    // Step 1: Absolute values
    x = abs(x); y = abs(y); z = abs(z); w = abs(w);

    // Step 2: Reproduce the exact x87 swap network from ASM.
    if (x < y) { double t = x; x = y; y = t; }
    if (x < z) { double t = x; x = z; z = t; }
    { double t = x; x = y; y = t; }
    if (x < w) { double t = x; x = w; w = t; }
    { double t = x; x = y; y = t; }
    if (x < w) { double t = x; x = w; w = t; }
    { double t = x; x = y; y = t; }
    { double t = x; x = z; z = t; }
    if (x < w) { double t = x; x = w; w = t; }

    // Step 3: Apply 4D rotation
    double r0 = x*params[5] + y*params[6] + z*params[7] + w*params[8];
    double r1 = x*params[9] + y*params[10] + z*params[11] + w*params[12];
    double r2 = x*params[13] + y*params[14] + z*params[15] + w*params[16];
    double r3 = x*params[17] + y*params[18] + z*params[19] + w*params[20];

    // Step 4: Simple linear scale + offset
    double k = Scale - 1.0;
    double x_out = Scale * r0 - k * CX;
    double y_out = Scale * r1 - k * CY;
    double z_out = Scale * r2 - k * CZ;
    double w_out = Scale * r3 - k * CW;

    de = w_out;
    return cpu_double3(x_out, y_out, z_out);
}

constant FormulaInfo formula_info_Octahedron4 = {
    .id = FORMULA_OCTAHEDRON4,
    .name = "Octahedron4",
    .param_count = 11,
    .complexity = 343,
    .uses_transcendental = false
};

/*
Original M3 Description:
See Sierpinski4, but instead of tetrahedral shapes you will get octahedral ones.

Fold pattern (hyper-octahedral):
  x=abs(x); y=abs(y); z=abs(z); w=abs(w);
  Sort all 4 descending: largest, ..., smallest
  Then 4D rotation, then linear scale+offset.

Parameters:
  [0] Scale: double (default: 2)
  [1] CScale_X: double (default: 0)
  [2] CScale_Y: double (default: 1)
  [3] CScale_Z: double (default: 0)
  [4] CScale_W: double (default: 0)
  [5..20] 4x4 rotation matrix (16 floats, from .6SingleAngles Rotation)
*/
