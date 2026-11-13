// TorusIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: TorusIFS
// Hand-ported from ASM: 217 bytes, Capstone-verified
// DEoption: 20 (dIFS shape — SSE2 IFS transform + x87 torus DE)
// Torus with selectable round/boxy norms for XY and Z cross-sections

constexpr unsigned int FORMULA_TORUSIFS = 7824;

template<>
cpu_double3 execute_formula<FORMULA_TORUSIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Radius_1 = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Radius_2 = params[5];
    const int Apply_scale_add = int(params[6]);
    const int Boxy_XY = int(params[7]);
    const int Boxy_Z = int(params[8]);
    const double Aspect_Ratio = params[9];
    // params[10] = OTrapOnIts, params[11] = OTrapOffset (coloring, framework-managed)

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;    // w *= Scale
    }

    // Step 2: Scale x by Aspect_Ratio
    // ASM: fmul [edi-0x5c] → fstp
    double x_scaled = x * Aspect_Ratio;

    // Step 3: XY distance (round or boxy)
    // ASM: Boxy_XY==0 → sqrt(x_scaled²+y²); Boxy_XY!=0 → max(|x_scaled|,|y|)
    double r_xy;
    if (Boxy_XY == 0) {
        r_xy = sqrt(x_scaled * x_scaled + y * y);
    } else {
        r_xy = max(abs(x_scaled), abs(y));
    }

    // Step 4: Ring distance (subtract major radius)
    // ASM: fsub Radius_1
    double ring_dist = r_xy - Radius_1;

    // Step 5: Tube distance (round or boxy)
    // ASM: Boxy_Z==0 → sqrt(ring²+z²); Boxy_Z!=0 → max(|ring|,|z|)
    double tube_dist;
    if (Boxy_Z == 0) {
        tube_dist = sqrt(ring_dist * ring_dist + z * z);
    } else {
        tube_dist = max(abs(ring_dist), abs(z));
    }

    // Step 6: Shape DE = tube - Radius_2
    // ASM: fsub [edi-0x48]
    double shape_de = tube_dist - Radius_2;

    // Step 7: Scale correction
    // ASM: if Apply_scale_add == 0: fdiv Scale
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    // Step 8: OTrap writeback
    params[MAX_FORMULA_PARAMS - 1] =
        fabs(params[ITER_COUNT_SLOT] * params[10] + params[11]);

    return cpu_double3(x, y, z);
}

// Alternative direct function implementation
cpu_double3 TorusIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_TORUSIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_TorusIFS = {
    .id = FORMULA_TORUSIFS,
    .name = "TorusIFS",
    .param_count = 12,
    .complexity = 390,
    .uses_transcendental = false
};

/*
Original M3 Description:
Cute variations of the torus. Added colors!

Luca GN 2012

Mathematical Formula:
  DEoption=20 (dIFS shape — SSE2 IFS + x87 torus DE):
    - Standard SSE2 IFS preamble
    - Aspect ratio scaling on x coordinate
    - Selectable Euclidean/Chebyshev norms for XY and Z
    - Standard torus SDF with two radii

  Step 1: IFS transform:
    x = p.x * Scale + X_add;  y = p.y * Scale + Y_add;  z = p.z * Scale + Z_add
    if Apply_scale_add: w *= Scale, store position

  Step 2: Aspect ratio:
    x_scaled = x * Aspect_Ratio

  Step 3: XY distance (ring distance computation):
    if Boxy_XY == 0: r_xy = sqrt(x_scaled² + y²)      [round torus]
    if Boxy_XY != 0: r_xy = max(|x_scaled|, |y|)       [square torus]

  Step 4: Ring distance:
    ring_dist = r_xy - Radius_1

  Step 5: Tube distance (cross-section shape):
    if Boxy_Z == 0: tube_dist = sqrt(ring_dist² + z²)   [round cross-section]
    if Boxy_Z != 0: tube_dist = max(|ring_dist|, |z|)   [square cross-section]

  Step 6: Shape DE:
    shape_de = tube_dist - Radius_2

  Step 7: OTrap coloring (framework-managed):
    OTrap = |OTrapOffset + iterations * OTrapOnIts|

Parameters:
  [0] Radius_1: double (default: 0.5) — major radius (ring center)
  [1] Scale: double (default: 1) — IFS scale factor (.2Doubles)
  [2] Z_add: double (default: 0) — Z offset (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] Radius_2: double (default: 0.1) — minor radius (tube radius)
  [6] Apply_scale_add: int (default: 1)
  [7] Boxy_XY: int (default: 0) — 0=round, 1=square XY cross-section
  [8] Boxy_Z: int (default: 0) — 0=round, 1=square Z cross-section
  [9] Aspect_Ratio: double (default: 1.0) — x-axis scaling
  [10] OTrapOnIts: double (default: 0.1) — orbit trap iteration scale
  [11] OTrapOffset: double (default: -1) — orbit trap offset

PVar layout (verified against x87 ASM offsets):
  Options (negative offsets):
  [PVar - 0x10] = Radius_1 (double)
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add (double)
  [PVar - 0x40] = X_add (double)
  [PVar - 0x48] = Radius_2 (double)
  [PVar - 0x4c] = Apply_scale_add (integer)
  [PVar - 0x50] = Boxy_XY (integer)
  [PVar - 0x54] = Boxy_Z (integer)
  [PVar - 0x5c] = Aspect_Ratio (double)
  [PVar - 0x64] = OTrapOnIts (double)
  [PVar - 0x6c] = OTrapOffset (double)

Code structure:
  0x00-0x40: Standard SSE2 IFS preamble
  0x40-0x45: Store to stack for x87
  0x4B-0x53: x_scaled = x * Aspect_Ratio
  0x54-0x69: Round XY: sqrt(x_scaled²+y²)
  0x6B-0x83: Boxy XY: max(|x_scaled|, |y|)
  0x85: ring_dist = r_xy - Radius_1
  0x88-0x9A: Round Z: sqrt(ring²+z²)
  0x9C-0xB1: Boxy Z: max(|ring|, |z|)
  0xB3: tube_dist - Radius_2 → shape_de
  0xB6-0xBF: Scale correction + store DE
  0xC2-0xD5: OTrap = |OTrapOffset + iter * OTrapOnIts|
  0xD5-0xD8: Return

Key notes:
  - 4 torus variants from Boxy_XY × Boxy_Z: round-round, round-square, square-round, square-square
  - Aspect_Ratio stretches the torus in x direction (elliptical torus)
  - OTrap uses iteration-based coloring (no spatial component)
  - Stack allocation 0x1c (28 bytes) — minimal for no trig
  - 217 bytes — one of the simpler dIFS formulas
*/
