// SierpHilbert.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: SierpHilbert
// Hand-ported from ASM: 289 bytes, Capstone-verified
// DEoption: 2 (analytic DE via w tracking)
// Sierpinski-Hilbert hybrid: R1 rotation + Hilbert fold + Edge folds + R2 rotation + scale

constexpr unsigned int FORMULA_SIERPHILBERT = 590;

template<>
cpu_double3 execute_formula<FORMULA_SIERPHILBERT>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    const double CScaleX = params[1];
    const double CScaleY = params[2];
    const double CScaleZ = params[3];
    const double Edge1 = params[22];
    const double Edge2 = params[23];

    // Pascal .3SingleAngles are packed into PVar as precomputed 3x3 matrices.
    RotMatrix3 R1 = rotMatrix3FromFloats(params, 4);
    RotMatrix3 R2 = rotMatrix3FromFloats(params, 13);

    // Step 1: Apply R1 to (x, y, z) — NOTE: NO abs() unlike Menger formulas
    cpu_double3 r1 = R1 * p;
    double rx = r1.x;
    double ry = r1.y;
    double rz = r1.z;

    // Step 2: Sierpinski-Hilbert fold (conditional swaps with negation)
    // This implements the fractal symmetry via sign-aware sorting
    double sa = -rx;
    double sb = ry;
    double sc = rz;

    // Phase 1: ascending compare-swap (put smaller in sa)
    if (sa >= sb) { double t = sa; sa = sb; sb = t; }
    if (sa >= sc) { double t = sa; sa = sc; sc = t; }

    // Negate, exchange, negate
    sa = -sa;
    { double t = sa; sa = sb; sb = t; }
    sa = -sa;

    // Phase 2: one more compare-swap
    if (sa >= sc) { double t = sa; sa = sc; sc = t; }

    // Final negate + exchange
    sa = -sa;
    { double t = sa; sa = sb; sb = t; }

    // Step 3: Edge1 fold on sb (mirror fold at Edge1)
    // sb_new = Edge1 - |Edge1 - sb| when Edge1 > 0
    if (Edge1 > 0.0) {
        sb = Edge1 - abs(Edge1 - sb);
    }

    // Step 4: Edge2 fold on sa (mirror fold at Edge2)
    // sa_new = Edge2 - |Edge2 - sa| when Edge2 > 0
    if (Edge2 > 0.0) {
        sa = Edge2 - abs(Edge2 - sa);
    }

    // Step 5: Apply R2 to (sa, sb, sc)
    cpu_double3 r2 = R2 * cpu_double3(sa, sb, sc);
    double r2x = r2.x;
    double r2y = r2.y;
    double r2z = r2.z;

    // Step 6: Scale + DE tracking
    de = de * Scale;

    // Step 7: Menger offsets (all three uniform, no conditional z-fold)
    double k = Scale - 1.0;

    cpu_double3 result;
    result.x = r2x * Scale - k * CScaleX;
    result.y = r2y * Scale - k * CScaleY;
    result.z = r2z * Scale - k * CScaleZ;
    return result;
}

// Alternative direct function implementation
cpu_double3 SierpHilbert_formula(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    const double CScaleX = params[1];
    const double CScaleY = params[2];
    const double CScaleZ = params[3];
    const double Edge1 = params[22];
    const double Edge2 = params[23];

    RotMatrix3 R1 = rotMatrix3FromFloats(params, 4);
    RotMatrix3 R2 = rotMatrix3FromFloats(params, 13);

    cpu_double3 r1 = R1 * p;
    double rx = r1.x;
    double ry = r1.y;
    double rz = r1.z;

    double sa = -rx; double sb = ry; double sc = rz;
    if (sa >= sb) { double t = sa; sa = sb; sb = t; }
    if (sa >= sc) { double t = sa; sa = sc; sc = t; }
    sa = -sa;
    { double t = sa; sa = sb; sb = t; }
    sa = -sa;
    if (sa >= sc) { double t = sa; sa = sc; sc = t; }
    sa = -sa;
    { double t = sa; sa = sb; sb = t; }

    if (Edge1 > 0.0) { sb = Edge1 - abs(Edge1 - sb); }
    if (Edge2 > 0.0) { sa = Edge2 - abs(Edge2 - sa); }

    cpu_double3 r2 = R2 * cpu_double3(sa, sb, sc);
    double r2x = r2.x;
    double r2y = r2.y;
    double r2z = r2.z;

    de = de * Scale;
    double k = Scale - 1.0;

    cpu_double3 result;
    result.x = r2x * Scale - k * CScaleX;
    result.y = r2y * Scale - k * CScaleY;
    result.z = r2z * Scale - k * CScaleZ;
    return result;
}

// Formula metadata
constant FormulaInfo formula_info_SierpHilbert = {
    .id = FORMULA_SIERPHILBERT,
    .name = "SierpHilbert",
    .param_count = 12,
    .complexity = 1040,
    .uses_transcendental = true
};

/*
Original M3 Description:
This fractal is an hybridation of Sierpinski and Hilbert axioms, with these parameters.
In reality, an uncountable number of curves and fractals can be obtained varying the parameters.

Mathematical Formula:
  1. Apply rotation R1 to (x, y, z) — NO abs() step
  2. Sierpinski-Hilbert fold:
     - Negate first component, ascending compare-swap network
     - Negate, exchange, negate, compare-swap, negate, exchange
     This creates specific symmetry reflections for the hybrid fractal
  3. Edge1 fold on second sorted value (sb):
     if Edge1 > 0: sb = Edge1 - |Edge1 - sb|
  4. Edge2 fold on first sorted value (sa):
     if Edge2 > 0: sa = Edge2 - |Edge2 - sa|
  5. Apply rotation R2 to (sa, sb, sc)
  6. Scale + DE tracking: de *= Scale
  7. Uniform offsets (no conditional z-fold):
     x = r2x * Scale - (Scale-1) * CScaleX
     y = r2y * Scale - (Scale-1) * CScaleY
     z = r2z * Scale - (Scale-1) * CScaleZ

Parameters:
  [0] Scale: double (default: 3)
  [1] CScale_X: double (default: 1)
  [2] CScale_Y: double (default: 1)
  [3] CScale_Z: double (default: 1)
  [4] Rotation1_X: double (default: 0) - angle in degrees
  [5] Rotation1_Y: double (default: 0) - angle in degrees
  [6] Rotation1_Z: double (default: 0) - angle in degrees
  [7] Rotation2_X: double (default: 0) - angle in degrees
  [8] Rotation2_Y: double (default: 0) - angle in degrees
  [9] Rotation2_Z: double (default: 0) - angle in degrees
  [10] Edge1: double (default: 2)
  [11] Edge2: double (default: 1)

PVar layout (verified against ASM offsets):
  [PVar - 0x10] = Scale (double)
  [PVar - 0x18] = CScale_X (double)
  [PVar - 0x20] = CScale_Y (double)
  [PVar - 0x28] = CScale_Z (double)
  Rotation1 matrix (9 singles):
  [PVar - 0x2C] = R1[0,0]  [PVar - 0x30] = R1[0,1]  [PVar - 0x34] = R1[0,2]
  [PVar - 0x38] = R1[1,0]  [PVar - 0x3C] = R1[1,1]  [PVar - 0x40] = R1[1,2]
  [PVar - 0x44] = R1[2,0]  [PVar - 0x48] = R1[2,1]  [PVar - 0x4C] = R1[2,2]
  Rotation2 matrix (9 singles):
  [PVar - 0x50] = R2[0,0]  [PVar - 0x54] = R2[0,1]  [PVar - 0x58] = R2[0,2]
  [PVar - 0x5C] = R2[1,0]  [PVar - 0x60] = R2[1,1]  [PVar - 0x64] = R2[1,2]
  [PVar - 0x68] = R2[2,0]  [PVar - 0x6C] = R2[2,1]  [PVar - 0x70] = R2[2,2]
  [PVar - 0x78] = Edge1 (double)
  [PVar - 0x80] = Edge2 (double)

Key differences from Menger3:
  - No abs() at the beginning (works with signed values)
  - Hilbert fold: conditional swaps with negation instead of standard sort
  - Edge folds: mirror fold at Edge1/Edge2 thresholds
  - Uniform offsets (no conditional z-fold)
  - R2 input: (sa, sb, sc) from Hilbert fold + edge folds
*/
