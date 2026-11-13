// foldinghexIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: foldinghexIFS
// Hand-ported from ASM: 255 bytes, DEoption=21
// Hexagonal continuous tiling of x,y plane
// Code by Knighty, modified by Luca GN 2012

constexpr unsigned int FORMULA_FOLDINGHEXIFS = 10314;

// ASM trace summary:
// dIFS transform (DEoption=21): no SSE2 preamble, direct x87 shape computation
// Constants: [PVar+0x00]=0.5, [PVar+0x08]=0.57735, [PVar+0x10]=-0.866025,
//            [PVar+0x18]=-0.5, [PVar+0x20]=0.4330125
// The code implements hexagonal folding in XY using repeated abs/round/fold operations
//
// Coordinates: esi-0x78 = x, esi-0x70 = y, esi-0x68 = z (dIFS convention)
// Parameter: [edi-0x10] = Grid styling (default 1.5)
// Constants used for hex geometry:
//   c0 = 0.5, c1 = 0.57735 (1/sqrt(3)), c2 = -0.866025 (-sqrt(3)/2), c3 = -0.5, c4 = 0.4330125

template<>
cpu_double3 execute_formula<FORMULA_FOLDINGHEXIFS>(cpu_double3 p, constant double* params, double &de) {
    const double GridStyling = params[0];

    // Constants from [CONSTANTS] section
    const double c_half     = 0.5;         // [PVar+0x00]
    const double c_invrt3   = 0.57735;     // [PVar+0x08] ≈ 1/sqrt(3)
    const double c_neg866   = -0.866025;   // [PVar+0x10] = -sqrt(3)/2
    const double c_neghalf  = -0.5;        // [PVar+0x18]
    const double c_hex      = 0.4330125;   // [PVar+0x20] ≈ sqrt(3)/4

    double x = p.x;
    double y = p.y;

    // --- First axis fold (x direction) ---
    // ASM 0x10-0x37: x = |x - GridStyling|, then fold
    x = fabs(x - GridStyling);
    // x / GridStyling * c_half - c_half, round, *2+1, * GridStyling, negate, add x, abs
    double t = round(x / GridStyling * c_half - c_half);
    t = t * 2.0 + 1.0;
    t = t * GridStyling;
    x = fabs(x - t);

    // --- Second axis fold (y direction) ---
    // ASM 0x3A-0x72: similar fold for y with GridStyling * c1 scale
    double yscale = GridStyling * c_invrt3;
    y = fabs(y - yscale);
    double t2 = round(y / yscale * c_half - c_half);
    t2 = t2 * 2.0 + 1.0;
    t2 = t2 * yscale;
    y = fabs(y - t2);

    // --- Hex folding iterations (2 rounds) ---
    // ASM 0x76-0xFE: Two rounds of hexagonal projection and fold
    // Each round: v = x * c_neg866 + y * c_neghalf + c_hex
    //             fold = min(v, 0) (clamp negative part)
    //             x = |x - fold * c_neg866|
    //             y = |y - fold * c_neghalf|
    //             then recompute v and fold again

    for (int iter = 0; iter < 2; iter++) {
        double v = x * c_neg866 + y * c_neghalf + c_hex;
        // ASM: v + |v| - v = v if v>=0, 0 if v<0 ... but code does:
        // fold = v - |v| = min(2v, 0) ... actually tracing more carefully:
        // st0 = v, dup, abs, neg, add => v + (-|v|) = v - |v|
        // if v >= 0: v - v = 0
        // if v < 0: v - (-v) = 2v
        // So fold = min(v, 0) * 2? No...
        // Looking at ASM: fld st(0); fabs; fchs; faddp => v + (-|v|) = v - |v|
        // v >= 0: 0; v < 0: 2v. This is 2*min(v,0).
        double fold = v - fabs(v);  // = 2 * min(v, 0)
        // Store fold (used for both x and y update)
        // x = |x - fold * c_neg866| (but fold*c_neg866... let me re-trace)

        // ASM stores fold to [esp+0x18], then:
        // load fold, mul c_neg866, negate, add x, abs => x' = |x + fold*0.866025|
        x = fabs(x - fold * c_neg866);  // fold is negative when active, c_neg866 is negative
        // Actually: fold*c_neg866 is positive when fold<0, negate makes it negative
        // Let me just trace: fold * c_neg866 = fold * (-0.866025)
        // fchs => -(fold * (-0.866025)) = fold * 0.866025
        // fadd x => x + fold * 0.866025
        // fabs => |x + fold * 0.866025|
        // But fold = 2*min(v,0) which is <= 0
        // So this subtracts from x when v < 0

        y = fabs(y - fold * c_neghalf);
        // Similarly: fold * c_neghalf = fold * (-0.5)
        // fchs => fold * 0.5
        // fadd y => y + fold * 0.5
        // fabs
    }

    // ASM 0xEE-0xFE: store results back
    // fld [esp] = x, fld [esp+8] = y  =>  st0=y, st1=x
    // fstp [esi-0x70] stores y to y slot, fstp [esi-0x78] stores x to x slot
    // No swap — x stays x, y stays y, z unchanged

    return cpu_double3(x, y, p.z);
}

// Alternative direct function
cpu_double3 foldinghexIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_FOLDINGHEXIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_foldinghexIFS = {
    .id = FORMULA_FOLDINGHEXIFS,
    .name = "foldinghexIFS",
    .param_count = 1,
    .complexity = 255,
    .uses_transcendental = false
};

/*
Original M3 Description:
Hexagonal continuous tiling of x,y plane.
Code by Knighty, a bit modified by me.
Luca GN 2012

Mathematical Formula (DEoption=21, dIFS transform):
  No SSE2 preamble — direct x87 computation on dIFS coords.

  Step 1: Fold x to |x - GridStyling|
  Step 2: Round-fold x: x = |x - (2*round(x/G*0.5 - 0.5) + 1)*G|
  Step 3: Same fold for y with scale G * 0.57735
  Step 4: Two rounds of hex projection fold:
    v = x * (-sqrt(3)/2) + y * (-0.5) + sqrt(3)/4
    fold = 2 * min(v, 0)
    x = |x + fold * sqrt(3)/2|
    y = |y + fold * 0.5|

Parameters:
  [0] Grid styling: double (default: 1.5)

Constants:
  [PVar+0x00] = 0.5
  [PVar+0x08] = 0.57735 (1/sqrt(3))
  [PVar+0x10] = -0.866025 (-sqrt(3)/2)
  [PVar+0x18] = -0.5
  [PVar+0x20] = 0.4330125 (sqrt(3)/4)
*/
