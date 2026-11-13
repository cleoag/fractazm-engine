#include "ifs_original_state.h"
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void UmbrellaIFSOriginal_x87(double*, const void*);
#endif

// UmbrellaIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: UmbrellaIFS
// Hand-ported from ASM: 259 bytes, Capstone-verified
// DEoption: 20 (dIFS shape — SSE2 IFS transform + x87 Whitney umbrella DE)
// Whitney's umbrella cubic surface: y*z*z - x*x = 0, intersected with cut box

constexpr unsigned int FORMULA_UMBRELLAIFS = 6261;

template<>
cpu_double3 execute_formula<FORMULA_UMBRELLAIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    OriginalApplyIFSState native(p,params);
    native.integer(-80,static_cast<int32_t>(params[7]));
    native.real(-88,params[8]);
    native.real(-96,params[9]);
    native.real(-104,params[10]);
    native.real(0,0.10000000000000001);
    UmbrellaIFSOriginal_x87(native.state,native.var());
    return native.finish(params,de);
#elif defined(MB3D_SOFTWARE_EXT80)
    Arm64ApplyIFSState native(p,params);
    using E=Arm64ApplyIFSState::E;
    const bool flip=int(params[7])!=0;
    const E a=flip ? native.transformed.z : native.transformed.x,b=native.transformed.y,
            c=flip ? native.transformed.x : native.transformed.z;
    E distance=(abs((a*a-(c*b)*b)+params[5])-params[5])*E(params[11]);
    if(!(E(1)>=distance)) distance=sqrt(sqrt(distance));
    E clip=abs(a)-params[0],candidate=abs(b)-params[0];
    if(candidate>=clip) clip=candidate;
    candidate=abs(c)-params[0];
    if(candidate>=clip) clip=candidate;
    if(clip>=distance) distance=clip;
    params[MAX_FORMULA_PARAMS-1]=double(abs(((b*a-c)*params[8]+E(int(params[ITER_COUNT_SLOT]))*params[9])+params[10]));
    return native.finish(distance,de);
#else
    // Portable numerical parity remains unverified.

    const double CutBoxSz = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Thickness = params[5];
    const int Apply_scale_add = int(params[6]);
    const int Flip_XZ = int(params[7]);
    // params[8] = OTrapOnDist, params[9] = OTrapOnIts, params[10] = OTrapOffset (coloring)

    // Constant (from M3 [CONSTANTS] section)
    const double K_SCALE = 0.1;

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;    // w *= Scale
    }

    // Step 2: Optional XZ flip
    // ASM: if Flip_XZ != 0: swap x and z
    double a = x, b = y, c = z;
    if (Flip_XZ != 0) {
        a = z;
        c = x;
    }

    // Step 3: Whitney's umbrella surface equation
    // Surface: x*x - z*y*y = 0 (equivalently y*z*z - x*x after variable relabeling)
    // ASM: c*b*b → fld a → fmul st(0) → fsubrp → f = a*a - c*b*b
    // Then offset shell: |f + Thickness| - Thickness
    double f = a * a - c * b * b;
    double surface_dist = (abs(f + Thickness) - Thickness) * K_SCALE;

    // Step 4: Double sqrt clamp for large values
    // ASM: fld1; fcomp → if surface_dist > 1.0: fsqrt; fsqrt (= 4th root)
    if (surface_dist > 1.0) {
        surface_dist = sqrt(sqrt(surface_dist));
    }

    // Step 5: Cut box distance
    // ASM: max(|a| - CutBoxSz, |b| - CutBoxSz, |c| - CutBoxSz)
    double box_dist = max(abs(a) - CutBoxSz, max(abs(b) - CutBoxSz, abs(c) - CutBoxSz));

    // Step 6: Shape DE = max(box_dist, surface_dist)
    // ASM: fcom/jae → max
    double shape_de = max(box_dist, surface_dist);

    // Step 7: Scale correction
    // ASM: if Apply_scale_add == 0: fdiv Scale
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    // Step 8: OTrap writeback
    // ASM: OTrap = |(b*a - c) * OTrapOnDist + iter * OTrapOnIts + OTrapOffset| → [esi+0x80]
    const double OTrapOnDist = params[8];
    const double OTrapOnIts  = params[9];
    const double OTrapOffset = params[10];
    params[63] = fabs((b * a - c) * OTrapOnDist + params[ITER_COUNT_SLOT] * OTrapOnIts + OTrapOffset);

    // Original 0025 skips XYZ stores when Apply is zero; distance still uses local XYZ.
    return Apply_scale_add != 0 ? cpu_double3(x, y, z) : p;
#endif
}

// Alternative direct function implementation
cpu_double3 UmbrellaIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_UMBRELLAIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_UmbrellaIFS = {
    .id = FORMULA_UMBRELLAIFS,
    .name = "UmbrellaIFS",
    .param_count = 11,
    .complexity = 500,
    .uses_transcendental = false
};

/*
Original M3 Description:
Whitney's umbrella cubic curve: y*z*z - x*x = 0
The center and the x-axis are singularities.
Reminds me of a modern sculpture.

Luca GN 2012

Mathematical Formula:
  DEoption=20 (dIFS shape — SSE2 IFS + x87 Whitney umbrella DE):
    - Standard SSE2 IFS preamble
    - Optional XZ flip for orientation variants
    - Whitney umbrella surface: f = a*a - c*b*b
    - Offset shell thickening: |f + T| - T
    - Scale by 0.1 constant + double sqrt clamp for DE stability
    - Cut box intersection

  Step 1: IFS transform:
    x = p.x * Scale + X_add;  y = p.y * Scale + Y_add;  z = p.z * Scale + Z_add
    if Apply_scale_add: w *= Scale, store position

  Step 2: Optional XZ flip:
    if Flip_XZ: swap(a, c) where a=x, b=y, c=z

  Step 3: Whitney umbrella surface:
    f = a*a - c*b*b
    surface_dist = |f + Thickness| - Thickness
    surface_dist *= 0.1   [constant scaling for DE conservatism]

  Step 4: DE clamp for algebraic surface:
    if surface_dist > 1.0: surface_dist = surface_dist^0.25   [double fsqrt]

  Step 5: Cut box:
    box_dist = max(|a| - CutBoxSz, |b| - CutBoxSz, |c| - CutBoxSz)

  Step 6: Shape DE:
    shape_de = max(box_dist, surface_dist)

  Step 7: OTrap coloring (framework-managed):
    OTrap = |(b*a - c) * OTrapOnDist + iter * OTrapOnIts + OTrapOffset|

Parameters:
  [0] CutBoxSz: double (default: 2) — bounding box half-size
  [1] Scale: double (default: 2) — IFS scale factor (.2Doubles)
  [2] Z_add: double (default: 0) — Z offset (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] Thickness: double (default: 0.01) — surface shell thickness
  [6] Apply_scale_add: int (default: 1)
  [7] Flip_XZ: int (default: 0) — swap x and z coordinates
  [8] OTrapOnDist: double (default: 0.1) — orbit trap spatial scale
  [9] OTrapOnIts: double (default: 0.1) — orbit trap iteration scale
  [10] OTrapOffset: double (default: 0) — orbit trap offset

PVar layout (verified against x87 ASM offsets):
  Constants (positive offsets):
  [PVar + 0x00] = 0.1 (surface distance scaling)

  Options (negative offsets):
  [PVar - 0x10] = CutBoxSz (double)
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add (double)
  [PVar - 0x40] = X_add (double)
  [PVar - 0x48] = Thickness (double)
  [PVar - 0x4c] = Apply_scale_add (integer)
  [PVar - 0x50] = Flip_XZ (integer)
  [PVar - 0x58] = OTrapOnDist (double)
  [PVar - 0x60] = OTrapOnIts (double)
  [PVar - 0x68] = OTrapOffset (double)

Code structure:
  0x00-0x40: Standard SSE2 IFS preamble
  0x40-0x45: Store to stack for x87
  0x4B-0x5D: Optional XZ flip (swap [esp] and [esp+0x10])
  0x5-0x67: c*b*b (Whitney cubic term)
  0x6B-0x70: a*a - c*b*b (surface value via fsubrp)
  0x72-0x77: Offset shell: |f + Thickness| - Thickness
  0x7A: Multiply by 0.1 constant
  0x7C-0x89: If > 1.0: double fsqrt (4th root clamp)
  0x8B-0xB1: Cut box: max(|a|, |b|, |c|) - CutBoxSz
  0xC3-0xD0: max(box_dist, surface_dist) → shape_de
  0xD2-0xDB: Scale correction + store DE
  0xDE-0xF9: OTrap = |(b*a - c) * OTrapOnDist + iter * OTrapOnIts + OTrapOffset|
  0xFF-0x102: Return

Key notes:
  - Whitney umbrella is a classical algebraic surface with self-intersection
  - Surface equation a*a = c*b*b has singularity at origin and along a-axis
  - 0.1 scaling factor prevents overestimation (algebraic surfaces have unbounded DE)
  - Double sqrt (4th root) clamps distant DE to prevent stepping artifacts
  - fcomp used instead of fcom (compare and pop 1.0) to avoid stack pollution
  - Stack allocation 0x1c (28 bytes) — minimal, no trig needed
  - 259 bytes, straightforward algebraic surface + box intersection
*/
