#include "ifs_original_state.h"
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void DecoCubeIFSOriginal_x87(double*, const void*);
#endif

// DecoCubeIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: DecoCubeIFS
// Hand-ported from ASM (227 bytes, Capstone-verified)
// DEoption: 20 (dIFS shape — SSE2 IFS transform + x87 "blobby cube" DE)
// Surface made of six toruses ("DecoCube"), with sphere fallback

constexpr unsigned int FORMULA_DECOCUBEIFS = 10192;

template<>
cpu_double3 execute_formula<FORMULA_DECOCUBEIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    OriginalApplyIFSState native(p,params);
    native.real(-84,params[7]);
    native.real(0,1);
    DecoCubeIFSOriginal_x87(native.state,native.var());
    return native.finish(params,de);
#else
    // Portable numerical parity remains unverified.

    const double Param_1 = params[0];    // [edi-0x10] — torus sum offset
    const double Scale = params[1];      // [edi-0x20] (.2Doubles)
    const double Z_add = params[2];      // [edi-0x30] (.2Doubles)
    const double Y_add = params[3];      // [edi-0x38]
    const double X_add = params[4];      // [edi-0x40]
    const double Param_2 = params[5];    // [edi-0x48] — individual axis offset
    const int Apply_scale_add = int(params[6]); // [edi-0x4c]
    const double Surface_closeness = params[7];  // [edi-0x54]
    // Constants: [edi+0x00] = 1.0

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

    // Step 2: Compute x^2, y^2, z^2
    // ASM 0x4B-0x66:
    //   fld x; fmul st(0) -> x^2; fstp [esp]
    //   fld y; fmul st(0) -> y^2; fstp [esp+8]
    //   fld z; fmul st(0) -> z^2; fstp [esp+0x10]
    double x2 = x * x;
    double y2 = y * y;
    double z2 = z * z;

    // Step 3: Compute three "torus pair" terms and multiply them
    // This formula computes the product of three terms, each being
    // a pair of squared components. Geometrically, it's the implicit
    // surface of six toruses along the cube axes.
    //
    // ASM 0x67-0xAC:
    //   Term 1: (x^2 + y^2 - Param_1)^2 + (z^2 - Param_2)^2
    //   Term 2: (x^2 + z^2 - Param_1)^2 + (y^2 - Param_2)^2
    //   Term 3: (z^2 + y^2 - Param_1)^2 + (x^2 - Param_2)^2
    //   product = Term1 * Term2 * Term3
    //
    // ASM trace:
    // 0x67: fld x2; fadd y2; fsub Param_1 -> (x2+y2-P1)
    // 0x71: fmul st(0) -> (x2+y2-P1)^2
    // 0x73: fld z2; fsub Param_2 -> (z2-P2)
    // 0x7A: fmul st(0) -> (z2-P2)^2
    // 0x7C: faddp -> T1 = (x2+y2-P1)^2 + (z2-P2)^2
    //
    // 0x7E: fld x2; fadd z2; fsub Param_1 -> (x2+z2-P1)
    // 0x88: fmul st(0) -> (x2+z2-P1)^2
    // 0x8A: fld y2; fsub Param_2 -> (y2-P2)
    // 0x91: fmul st(0) -> (y2-P2)^2
    // 0x93: faddp -> T2 = (x2+z2-P1)^2 + (y2-P2)^2
    //
    // 0x95: fld z2; fadd y2; fsub Param_1 -> (z2+y2-P1)
    // 0xA0: fmul st(0) -> (z2+y2-P1)^2
    // 0xA2: fld x2; fsub Param_2 -> (x2-P2)
    // 0xA8: fmul st(0) -> (x2-P2)^2
    // 0xAA: faddp -> T3 = (z2+y2-P1)^2 + (x2-P2)^2
    //
    // 0xAC: fmulp -> T2 * T3
    // 0xAE: fmulp -> T1 * T2 * T3 = product

    double a1 = x2 + y2 - Param_1;
    double a2 = z2 - Param_2;
    double T1 = a1 * a1 + a2 * a2;

    double b1 = x2 + z2 - Param_1;
    double b2 = y2 - Param_2;
    double T2 = b1 * b1 + b2 * b2;

    double c1 = z2 + y2 - Param_1;
    double c2 = x2 - Param_2;
    double T3 = c1 * c1 + c2 * c2;

    double product = T1 * T2 * T3;

    // Step 4: Double square root and subtract Surface_closeness
    // ASM 0xB0-0xB4:
    //   fsqrt -> sqrt(product)
    //   fsqrt -> product^(1/4)
    //   fsub Surface_closeness([edi-0x54])
    double shape_de = sqrt(sqrt(product)) - Surface_closeness;

    // Step 5: Compare with sphere fallback
    // ASM 0xB8-0xD0:
    //   fld [edi+0x00]=1.0; fcomp st(1) -> compare 1.0 vs shape_de
    //   shr ah,1; jae 0xD2 (skip fallback if 1.0 >= shape_de, i.e. shape_de <= 1.0)
    //   If shape_de > 1.0 (C0=1): use sphere distance instead
    //     fstp st(0) -> discard shape_de
    //     fld x2; fadd y2; fadd z2; fsqrt -> r = sqrt(x2+y2+z2)
    //     This replaces shape_de with r (sphere distance)
    if (shape_de > 1.0) {
        shape_de = sqrt(x2 + y2 + z2);
    }

    // Step 6: Scale correction
    // ASM 0xD3-0xD9: if Apply_scale_add == 0: fdiv Scale
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    // Original 0025 skips XYZ stores when Apply is zero; distance still uses local XYZ.
    return Apply_scale_add != 0 ? cpu_double3(x, y, z) : p;
#endif
}

// Alternative direct function implementation
cpu_double3 DecoCubeIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_DECOCUBEIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_DecoCubeIFS = {
    .id = FORMULA_DECOCUBEIFS,
    .name = "DecoCubeIFS",
    .param_count = 8,
    .complexity = 380,
    .uses_transcendental = false
};

/*
Original M3 Description:
A "blobby" surface made of six toruses.
Decrease raystep/stepwidth/destop or use a bigger "Closeness" to fix DE,
but closeness tends to take off details.
Uncheck "Vary DEstop on FOV" to make the scene more uniform, also.

edit: DE fixed (well not fully but works a lot better now)
Luca GN 2012

Mathematical Formula:
  DEoption=20 (dIFS shape — SSE2 IFS + x87 "DecoCube" DE):
    - Standard SSE2 IFS preamble
    - Compute x^2, y^2, z^2
    - Three "torus pair" terms along cube axes:
      T1 = (x^2+y^2-P1)^2 + (z^2-P2)^2
      T2 = (x^2+z^2-P1)^2 + (y^2-P2)^2
      T3 = (z^2+y^2-P1)^2 + (x^2-P2)^2
    - product = T1 * T2 * T3
    - shape_de = product^(1/4) - Surface_closeness
    - If shape_de > 1.0: fallback to sphere = sqrt(x^2+y^2+z^2)

Parameters:
  [0] Param_1: double (default: 0.1) — torus sum offset
  [1] Scale: double (default: 2) — IFS scale (.2Doubles)
  [2] Z_add: double (default: 0) — Z offset (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] Param_2: double (default: 0.15) — individual axis offset
  [6] Apply_scale_add: int (default: 1)
  [7] Surface_closeness: double (default: 0.02)

Constants:
  [edi+0x00] = 1.0 (sphere fallback threshold)

ASM trace summary:
  0x00-0x02: allocate stack (add esp, -0x1c)
  0x03-0x3: Standard SSE2 IFS preamble
  0x40-0x4A: Store transformed coords to stack
  0x4B-0x66: Compute x^2, y^2, z^2; store to stack
  0x67-0xAA: Three torus-pair terms T1, T2, T3
  0xAC-0xAE: product = T1 * T2 * T3
  0xB0-0xB4: shape_de = product^(1/4) - Surface_closeness
  0xB8-0xD0: Sphere fallback if shape_de > 1.0
  0xD3-0xDC: Scale correction + store DE
  0xDF-0xE2: Cleanup + return

Key notes:
  - Six implicit toruses placed on the faces of a cube
  - Product formula creates smooth "blobby" intersections
  - Fourth-root normalization for approximate distance field
  - Sphere fallback prevents DE overestimation far from surface
  - 227 bytes, moderate complexity
*/
