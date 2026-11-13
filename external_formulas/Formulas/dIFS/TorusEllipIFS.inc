// TorusEllipIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: TorusEllipIFS
// Hand-ported from ASM: 197 bytes, Capstone-verified
// DEoption: 20 (dIFS shape — SSE2 IFS transform + x87 elliptic torus DE)
// Torus with elliptic offset (moon shape) and boxy norm options

constexpr unsigned int FORMULA_TORUSELLIPIFS = 4952;

template<>
cpu_double3 execute_formula<FORMULA_TORUSELLIPIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Radius_1 = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Radius_2 = params[5];
    const int Apply_scale_add = int(params[6]);
    const int Boxy_XY = int(params[7]);
    const int Boxy_Z = int(params[8]);
    const double Elliptic = params[9];

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;    // w *= Scale
    }

    // Step 2: XY distance (round or boxy)
    // ASM: Boxy_XY==0 → sqrt(x²+y²); Boxy_XY!=0 → max(|x|,|y|) via fcom/jae
    double r_xy;
    if (Boxy_XY == 0) {
        r_xy = sqrt(x * x + y * y);
    } else {
        r_xy = max(abs(x), abs(y));
    }

    // Step 3: Ring distance (subtract major radius)
    // ASM: fsub Radius_1
    double ring_dist = r_xy - Radius_1;

    // Step 4: Tube distance (round or boxy)
    // ASM: Boxy_Z==0 → sqrt(ring²+z²); Boxy_Z!=0 → max(|ring|,|z|) via fcom/jae
    double tube_dist;
    if (Boxy_Z == 0) {
        tube_dist = sqrt(ring_dist * ring_dist + z * z);
    } else {
        tube_dist = max(abs(ring_dist), abs(z));
    }

    // Step 5: Shape DE = tube - Radius_2 + x * Elliptic
    // ASM: fsub Radius_2 → fld x → fmul Elliptic → faddp
    double shape_de = (tube_dist - Radius_2) + x * Elliptic;

    // Step 6: Scale correction
    // ASM: if Apply_scale_add == 0: fdiv Scale
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    return cpu_double3(x, y, z);
}

// Alternative direct function implementation
cpu_double3 TorusEllipIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_TORUSELLIPIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_TorusEllipIFS = {
    .id = FORMULA_TORUSELLIPIFS,
    .name = "TorusEllipIFS",
    .param_count = 10,
    .complexity = 400,
    .uses_transcendental = false
};

/*
Original M3 Description:
Cute variations of the torus. Looks like a moon. Found on italian Wikipedia (thanks!)
Be careful with "Elliptic", you may get an infinitely big object!

Luca GN 2012

Mathematical Formula:
  DEoption=20 (dIFS shape — SSE2 IFS + x87 elliptic torus DE):
    - Standard SSE2 IFS preamble
    - Same as TorusIFS but with additive Elliptic term: x * Elliptic
    - Creates asymmetric torus (one side thicker than the other)

  Step 1: IFS transform
  Step 2: XY ring distance (round or boxy)
  Step 3: Tube distance (round or boxy)
  Step 4: Shape DE = (tube_dist - Radius_2) + x * Elliptic

Parameters:
  [0] Radius_1: double (default: 0.5) — major radius
  [1] Scale: double (default: 1) — IFS scale factor (.2Doubles)
  [2] Z_add: double (default: 0) — Z offset (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] Radius_2: double (default: 0.1) — minor radius (tube)
  [6] Apply_scale_add: int (default: 1)
  [7] Boxy_XY: int (default: 0) — 0=round, 1=square XY
  [8] Boxy_Z: int (default: 0) — 0=round, 1=square Z
  [9] Elliptic: double (default: 0.2) — x-dependent thickness offset

PVar layout (verified against x87 ASM offsets):
  [PVar - 0x10] = Radius_1 (double)
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add (double)
  [PVar - 0x40] = X_add (double)
  [PVar - 0x48] = Radius_2 (double)
  [PVar - 0x4c] = Apply_scale_add (integer)
  [PVar - 0x50] = Boxy_XY (integer)
  [PVar - 0x54] = Boxy_Z (integer)
  [PVar - 0x5c] = Elliptic (double)

Code structure:
  0x00-0x40: Standard SSE2 IFS preamble
  0x40-0x45: Store to stack for x87
  0x4B-0x60: Round XY: sqrt(x²+y²)
  0x62-0x7A: Boxy XY: max(|x|,|y|) via fcom/shr/jae
  0x7C: ring_dist = r_xy - Radius_1
  0x7-0x91: Round Z: sqrt(ring²+z²)
  0x93-0xA8: Boxy Z: max(|ring|,|z|) via fcom/shr/jae
  0xAA: tube_dist - Radius_2
  0xAD-0xB3: + x * Elliptic
  0xB5-0xC4: Scale correction + store + return

Key notes:
  - Identical to TorusIFS except for the x*Elliptic additive term
  - Elliptic > 0 makes torus thinner on +x side, thicker on -x side
  - Creates moon/crescent shapes
  - Same fcom/jae max pattern as TorusIFS for boxy norms
  - Stack allocation 0x1c (28 bytes)
  - 197 bytes
*/
