// JCube3.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: JCube3
// Hand-ported from ASM: 294 bytes, DEoption=2
// Jerusalem Cube fractal approximation via brute-force IFS

constexpr unsigned int FORMULA_JCUBE3 = 10174;

// ASM trace summary:
//   0x0D-0x18: Clear sign bits on z, y, x -> absolute values (AND 0x7FFFFFFF on high dword)
//   0x1C-0x4: Pre-compute scaling factors from Alpha (DReci2) and GScale:
//     ScaleA = GScale - 1 + Alpha
//     ScaleB = ScaleA / Alpha = ScaleA * (1/Alpha)
//     InvScaleB = 1 / ScaleB = Alpha / ScaleA
//     threshold = 1 - GScale * InvScaleB = 1 - GScale / ScaleB
//   0x50-0x76: Sort |x|,|y|,|z| ascending: ST0=min, ST1=mid, ST2=max
//   0x77-0x83: Test: if InvScaleB > min (i.e. min < InvScaleB):
//   0x86-0x94:   Test: if mid > min + threshold:
//   0x96-0xD4:     INNER case: subtract Cx1/Cy1/Cz1, scale by ScaleB, DE*=ScaleB, add Cx1/Cy1/Cz1
//   0xD7-0x113:  OUTER case: subtract Cx2/Cy2/Cz2, scale by ScaleA, DE*=ScaleA, add Cx2/Cy2/Cz2
//   0x115-0x119: Store [ebx]=x, [edx]=y, [ecx]=z

template<>
cpu_double3 execute_formula<FORMULA_JCUBE3>(cpu_double3 p, constant double* params, double &de) {
    // DReci2 Alpha: value and reciprocal
    const double Alpha    = params[0];   // [PVar-0x10] default: 0.41421356 (sqrt(2)-1)
    const double InvAlpha = params[1];   // [PVar-0x18] 1/Alpha (precomputed by DReci2)
    const double Cx1      = params[2];   // [PVar-0x20] default: 0
    const double Cy1      = params[3];   // [PVar-0x28] default: 1
    const double Cz1      = params[4];   // [PVar-0x30] default: 1
    const double Cx2      = params[5];   // [PVar-0x38] default: 1
    const double Cy2      = params[6];   // [PVar-0x40] default: 1
    const double Cz2      = params[7];   // [PVar-0x48] default: 1
    const double GScale   = params[8];   // [PVar-0x50] default: 3

    // Pre-compute scaling factors (matching ASM 0x1-0x4)
    // ASM trace:
    //   fld GScale; fld1; fsubr st1 -> GScale-1; fxch; fchs -> -GScale on top
    //   fstp [esp+0x18] = -GScale; fadd Alpha -> (GScale-1)+Alpha = ScaleA
    //   fst [esp] = ScaleA; fmul InvAlpha -> ScaleA*InvAlpha = ScaleB
    //   fst [esp+8] = ScaleB; fld1; fdivrp -> 1/ScaleB = InvScaleB
    //   fst [esp+0x10] = InvScaleB; fmul (-GScale) -> -GScale/ScaleB
    //   fld1; faddp -> 1 - GScale/ScaleB = threshold
    //   fstp [esp+0x18] = threshold
    double ScaleA = GScale - 1.0 + Alpha;
    double ScaleB = ScaleA * InvAlpha;
    double InvScaleB = 1.0 / ScaleB;
    double threshold = 1.0 + (-GScale) * InvScaleB;  // = 1 - GScale/ScaleB

    // Absolute values (sign bits cleared in ASM)
    double x = abs(p.x);
    double y = abs(p.y);
    double z = abs(p.z);

    // Sort ascending: min, mid, max
    // ASM loads z, y, x -> ST0=x, ST1=y, ST2=z then sorts
    double a = x, b = y, c = z;
    // Sort a,b: if a >= b, swap -> a=min(x,y), b=max(x,y)
    if (a >= b) { double t = a; a = b; b = t; }
    // Sort a,c: if a >= c, swap -> a=min of all
    if (a < c) { double t = a; a = c; c = t; }
    // After fxch st(1): swap a,b
    // Sort b,c: if b >= c, swap
    { double t = a; a = b; b = t; }  // fxch
    if (a >= c) { double t = a; a = c; c = t; }

    // Result: a=min, b=mid, c=max
    double min_v = a;
    double mid_v = b;
    double max_v = c;

    // Branch decision
    if (InvScaleB > min_v && mid_v > min_v + threshold) {
        // INNER case (0x96-0xD4): point is inside the small cube zone
        // Subtract Cx1/Cy1/Cz1, scale by ScaleB, DE *= ScaleB, add back
        min_v -= Cx1;
        mid_v -= Cy1;
        max_v -= Cz1;

        de = de * ScaleB;
        min_v *= ScaleB;
        mid_v *= ScaleB;
        max_v *= ScaleB;

        min_v += Cx1;
        mid_v += Cy1;
        max_v += Cz1;
    } else {
        // OUTER case (0xD7-0x113): point is outside
        // Subtract Cx2/Cy2/Cz2, scale by ScaleA, DE *= ScaleA, add back
        min_v -= Cx2;
        mid_v -= Cy2;
        max_v -= Cz2;

        de = de * ScaleA;
        min_v *= ScaleA;
        mid_v *= ScaleA;
        max_v *= ScaleA;

        min_v += Cx2;
        mid_v += Cy2;
        max_v += Cz2;
    }

    // Store: [ebx]=x (ST0=min_v), [edx]=y (ST1=mid_v), [ecx]=z (ST2=max_v)
    return cpu_double3(min_v, mid_v, max_v);
}

// Formula metadata
constant FormulaInfo formula_info_JCube3 = {
    .id = FORMULA_JCUBE3,
    .name = "JCube3",
    .param_count = 9,
    .complexity = 750,
    .uses_transcendental = false
};

/*
Original M3 Description:
A brute-force IFS formula, that tries to replicate the "Jerusalem Cube" fractal.
It is discontinue, but it's the best known approximation of the formula.
Warning; GScale parameter may help to obtain more effects but discontinuities
become more evident!

Mathematical Formula:
  1. x=|x|, y=|y|, z=|z|
  2. Pre-compute:
     ScaleA = GScale - 1 + Alpha
     ScaleB = ScaleA / Alpha
     InvScaleB = 1 / ScaleB
     threshold = 1 - GScale / ScaleB
  3. Sort ascending: min, mid, max
  4. If min < InvScaleB AND mid > min + threshold:
     - Subtract C1 offset, scale by ScaleB, add C1 back
  5. Else:
     - Subtract C2 offset, scale by ScaleA, add C2 back
  6. DE *= scale factor used

Parameters:
  [0] Alpha: double (default: 0.41421356, sqrt(2)-1) via DReci2
  [1] 1/Alpha: double (precomputed by DReci2)
  [2] Cx1: double (default: 0)
  [3] Cy1: double (default: 1)
  [4] Cz1: double (default: 1)
  [5] Cx2: double (default: 1)
  [6] Cy2: double (default: 1)
  [7] Cz2: double (default: 1)
  [8] GScale: double (default: 3)

PVar layout:
  [PVar-0x10] = Alpha
  [PVar-0x18] = 1/Alpha (DReci2 precomputed)
  [PVar-0x20] = Cx1
  [PVar-0x28] = Cy1
  [PVar-0x30] = Cz1
  [PVar-0x38] = Cx2
  [PVar-0x40] = Cy2
  [PVar-0x48] = Cz2
  [PVar-0x50] = GScale
*/
