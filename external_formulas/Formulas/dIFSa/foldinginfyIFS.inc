// foldinginfyIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: foldinginfyIFS
// Hand-ported from ASM: 293 bytes, DEoption=21
// Continuous tiling of 3D space using "infinity" folding with rotation
// Formula by Luca GN

constexpr unsigned int FORMULA_FOLDINGINFYIFS = 10315;

// ASM trace summary:
// dIFS transform (DEoption=21): no SSE2 preamble
// Coordinates: esi-0x78=x, esi-0x70=y, esi-0x68=z
//
// Step 1: Add offsets (Add X/Y/Z) to coordinates
// Step 2: Apply 3x3 rotation matrix (from .3SingleAngles, pre-computed at edi offsets)
// Step 3: For each axis, if its Fold parameter is nonzero, apply foldinginfy:
//         period = 4 * Fold
//         v' = v - round(v/period) * period  (reduce to [-period/2, period/2])
//         v' = -v'
//         v_final = v' + |v' + Fold| - |v' - Fold|
//         This maps v into [-Fold, Fold] with smooth triangle-wave folding
//
// Parameters at negative offsets from PVar:
//   [edi-0x10] = Add X (default 0)
//   [edi-0x18] = Add Y (default 0)
//   [edi-0x20] = Add Z (default 0)
//   [edi-0x24..edi-0x44] = 3x3 rotation matrix (9 floats from .3SingleAngles)
//   [edi-0x4c] = Fold X (default 2)
//   [edi-0x54] = Fold Y (default 2)
//   [edi-0x5c] = Fold Z (default 0)

template<>
cpu_double3 execute_formula<FORMULA_FOLDINGINFYIFS>(cpu_double3 p, constant double* params, double &de) {
    const double AddX  = params[0];
    const double AddY  = params[1];
    const double AddZ  = params[2];
    // Build rotation matrix from 3 Euler angles (raw .3SingleAngles input)
    RotMatrix3 R = rotMatrix3FromFloats(params, 3);
    const double FoldX = params[12];
    const double FoldY = params[13];
    const double FoldZ = params[14];

    // Step 1: Add offsets
    double x = p.z + AddZ;   // ASM loads z first (esi-0x68)
    double y = p.y + AddY;   // then y (esi-0x70)
    double z = p.x + AddX;   // then x (esi-0x78) — but order matters for rotation

    // Actually, re-reading the ASM more carefully:
    // 0x00: fld [esi-0x68] = z, fadd [edi-0x20] = z + AddZ
    // 0x09: fld [esi-0x70] = y, fadd [edi-0x18] = y + AddY
    // 0x0C: fld [esi-0x78] = x, fadd [edi-0x10] = x + AddX
    // Stack after: st0=x+AddX, st1=y+AddY, st2=z+AddZ
    // Let's call these a=x+AddX, b=y+AddY, c=z+AddZ

    double a = p.x + AddX;
    double b = p.y + AddY;
    double c = p.z + AddZ;

    // Step 2: 3x3 rotation (same pattern as other formulas with .3SingleAngles)
    // The rotation matrix is stored as 9 single-precision floats
    // at edi-0x3c (m00) through edi-0x24 (m22) — but mapped via params
    // This is the standard rotation block seen in many formulas
    // For dIFS transforms, rotation params need proper handling
    // Rotation: x' = a*m[0] + b*m[1] + c*m[2]
    //           y' = a*m[3] + b*m[4] + c*m[5]
    //           z' = a*m[6] + b*m[7] + c*m[8]
    // The exact mapping from edi offsets is:
    // [edi-0x3c]=m00, [edi-0x40]=m01, [edi-0x44]=m02 (row for x')
    // [edi-0x24]=m10, [edi-0x28]=m11, [edi-0x2c]=m12 (row for y')
    // [edi-0x30]=m20, [edi-0x34]=m21, [edi-0x38]=m22 (row for z')

    // ASM rotation block at 0x12-0x4B:
    // x' = a * [edi-0x3c] + b * [edi-0x40] + c * [edi-0x44]
    // y' = a * [edi-0x24] + b * [edi-0x28] + c * [edi-0x2c]
    // z' = a * [edi-0x30] + b * [edi-0x34] + c * [edi-0x38]
    // (using fmul dword ptr — single precision rotation matrix)

    cpu_double3 rv = R * cpu_double3(a, b, c);
    double rx = rv.x;
    double ry = rv.y;
    double rz = rv.z;

    // Step 3: foldinginfy on each axis independently
    // ASM stores rotation results to esi-0x78, esi-0x70, esi-0x68

    // Helper: foldinginfy function
    // If fold_param != 0:
    //   period = 4 * fold_param
    //   v = v - round(v / period) * period   (modular reduction)
    //   v = -v
    //   v = v + |v + fold_param| - |fold_param - v| -- wait, let me retrace
    //
    // ASM pattern (at 0x55-0x96 for X fold, repeated for Y and Z):
    //   fld [edi-0x4c] (FoldX); ftst; if zero, skip
    //   fld FoldX; fadd st(0); fadd st(0) => period = 4*FoldX
    //   fld val; fld st(0); fdiv st(2); frndint; fmul st(2); fsubp => val - round(val/period)*period
    //   fstp st(1) => discard period
    //   fchs => v = -reduced
    //   fld st(0); fchs => -v; fadd FoldX => FoldX - v; fabs => |FoldX - v|
    //   fld st(1); fchs => -v again... wait:
    //   fld st(1) pushes copy of original v; fchs => -v
    //   fsub FoldX => -v - FoldX; fabs => |-v - FoldX| = |v + FoldX|... wait
    //
    // Let me trace step by step from 0x63:
    //   fld [edi-0x4c]; fadd st(0); fadd st(0)  => st0 = 4*FoldX
    //   fld [esi-0x78]  => st0=x, st1=4
    //   fld st(0)        => st0=x, st1=x, st2=4
    //   fdiv st(2)       => st0=x/(4), st1=x, st2=4
    //   frndint           => st0=round(x/(4)), st1=x, st2=4
    //   fmul st(2)       => st0=round(x/(4))*4, st1=x, st2=4  WAIT: fmul st(2) means st0 *= st2
    //                     Actually no — D8 CA = fmul st(0), st(2) => st0 = st0 * st2
    //                     So st0 = round(x/(4)) * 4
    //   fsubp st(1)      => st0 = x - round(x/(4))*4, st1=4
    //   fstp st(1)        => pop st1, so st0 = reduced value (4 discarded)
    //   fchs              => v = -(reduced)
    //
    //   fld st(0)         => dup v: st0=v, st1=v
    //   fchs              => st0=-v, st1=v
    //   fadd [edi-0x4c]  => st0=FoldX - v, st1=v  (since -v + FoldX)...
    //   Wait: DC 47 B4 = fadd qword ptr [edi-0x4c] => st0 = -v + FoldX
    //   fabs              => |FoldX - v|
    //   fld st(1)         => st0=v, st1=|FoldX-v|, st2=v
    //   fchs              => st0=-v
    //   fsub [edi-0x4c]  => st0 = -v - FoldX
    //   DC 67 B4 = fsub qword ptr [edi-0x4c] => st0 = -v - FoldX
    //   fabs              => |-v - FoldX| = |v + FoldX|
    //   fchs              => -|v + FoldX|
    //   faddp st(1)       => st0 = |FoldX - v| - |v + FoldX|
    //   faddp st(1)       => st0 = v + |FoldX - v| - |v + FoldX|
    //
    // Result: v + |FoldX - v| - |v + FoldX|
    // This is equivalent to: clamp(v, -FoldX, FoldX) * 2 - v
    // which is the "foldinginfy" function: smooth triangle wave

    // foldinginfy inline implementation:
    auto foldinginfy = [](double v, double fold) -> double {
        double period = 4.0 * fold;
        double reduced = v - round(v / period) * period;
        reduced = -reduced;
        return reduced + fabs(fold - reduced) - fabs(reduced + fold);
    };

    // Apply FoldX to rx (x axis)
    if (FoldX != 0.0) {
        rx = foldinginfy(rx, FoldX);
    }

    // Apply FoldY to ry (y axis)
    if (FoldY != 0.0) {
        ry = foldinginfy(ry, FoldY);
    }

    // Apply FoldZ to rz (z axis)
    if (FoldZ != 0.0) {
        rz = foldinginfy(rz, FoldZ);
    }

    return cpu_double3(rx, ry, rz);
}

// Alternative direct function
cpu_double3 foldinginfyIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_FOLDINGINFYIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_foldinginfyIFS = {
    .id = FORMULA_FOLDINGINFYIFS,
    .name = "foldinginfyIFS",
    .param_count = 9,
    .complexity = 293,
    .uses_transcendental = true
};

/*
Original M3 Description:
Continuous tiling of 3D space. Similar to plain "tiling" but without cuts.
Very interesting effects when combined with rotations.
When a fold is 0 it is never executed.
Do not enable both folds in iterate transforms or the entire space will be
densely filled (just two at once is fine)

Mathematical Formula (DEoption=21, dIFS transform):
  Step 1: Add X/Y/Z offsets to input coordinates
  Step 2: Apply 3x3 rotation matrix (from .3SingleAngles parameter)
  Step 3: For each axis with nonzero Fold:
    period = 4 * Fold
    v = -(v - round(v/period) * period)
    result = v + |Fold - v| - |v + Fold|
    This is a continuous triangle-wave folding function with period 4*Fold

Parameters:
  [0] Add X: double (default: 0)
  [1] Add Y: double (default: 0)
  [2] Add Z: double (default: 0)
  [3..11] Rotation matrix: 9 floats (from .3SingleAngles Rotation)
  [12] Fold X: double (default: 2)
  [13] Fold Y: double (default: 2)
  [14] Fold Z: double (default: 0)

Note: Rotation is specified by .3SingleAngles which is pre-computed into a
3x3 rotation matrix by the Mandelbulb3D engine. The Metal port must handle
this pre-computation in the parameter setup code.
*/
