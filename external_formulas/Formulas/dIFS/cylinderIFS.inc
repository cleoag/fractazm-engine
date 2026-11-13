// cylinderIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: cylinderIFS
// Hand-ported from ASM: 191 bytes, Capstone-verified
// DEoption: 20 (dIFS — formula computes shape DE directly)
// Cylinder shape dIFS with barrel/tip modifiers and OTrap writeback

constexpr unsigned int FORMULA_CYLINDERIFS = 5521;

template<>
cpu_double3 execute_formula<FORMULA_CYLINDERIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Radius = params[0];
    const double Halfwidth = params[1];  // X_halfwidth (half-length along x axis)
    const double Scale = params[2];
    const double Z_add = params[3];
    const double Y_add = params[4];
    const double X_add = params[5];
    const double Barrel = params[6];
    const double Tip = params[7];
    const int apply_scale_add = int(params[8]);
    // params[9] = OTrap_option (0..3), params[10] = OTrap_offset, params[11] = OTrap_scale
    const double Inside_radius = params[12];

    // Step 1: IFS transformation — scale + translate
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    // Step 2: Conditionally store transformed position and scale w
    cpu_double3 pos;
    if (apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;  // w tracking
        pos = cpu_double3(x, y, z);
    } else {
        pos = p;  // position unchanged
    }

    // Step 3: Cylinder DE computation (on transformed coordinates)
    // Cylinder aligned along x-axis
    double ax = abs(x);                              // |x| for cap distance
    double radial = sqrt(y * y + z * z);             // radial distance from x-axis
    double cap_dist = ax - Halfwidth;                // distance from end cap plane
    double clamped_x = min(ax, Halfwidth);           // x clamped within half-length
    double barrel_mod = clamped_x * clamped_x * Barrel;  // barrel shape deformation
    double tip_mod = x * Tip;                        // conical taper (signed, uses original x)

    double outer_dist = (radial - Radius) + barrel_mod + tip_mod;
    double inner_dist = (Inside_radius - radial) - barrel_mod - tip_mod;

    double shape_de = max(max(outer_dist, inner_dist), cap_dist);

    // Step 4: When not applying scale+add, compensate DE
    if (apply_scale_add == 0) {
        shape_de = shape_de / Scale;
    }

    // Store shape DE — ASM writes to [esi-0x20] (dIFS DE slot)
    de = shape_de;

    // OTrap writeback
    // Original code reads live iteration state, not the temporary coordinates.
    // Selector zero addresses DEstop at ESI-0x80 (PIt3D.RStopD).
    double source = params[MAX_FORMULA_PARAMS - 2];
    int otrap_opt = int(params[9]) & 3;
    if (otrap_opt == 1) source = pos.x;
    else if (otrap_opt == 2) source = pos.y;
    else if (otrap_opt == 3) source = pos.z;
    params[MAX_FORMULA_PARAMS - 1] = source * params[11] + params[10];

    return pos;
}

// Cylinder DE computation (standalone, for use by dIFS engine)
double cylinderIFS_shape_de(cpu_double3 p, constant double* params) {
    const double Radius = params[0];
    const double Halfwidth = params[1];
    const double Scale = params[2];
    const double Z_add = params[3];
    const double Y_add = params[4];
    const double X_add = params[5];
    const double Barrel = params[6];
    const double Tip = params[7];
    const int apply_scale_add = int(params[8]);
    const double Inside_radius = params[12];

    // Transform position
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    // Cylinder DE
    double ax = abs(x);
    double radial = sqrt(y * y + z * z);
    double cap_dist = ax - Halfwidth;
    double clamped_x = min(ax, Halfwidth);
    double barrel_mod = clamped_x * clamped_x * Barrel;
    double tip_mod = x * Tip;

    double outer_dist = (radial - Radius) + barrel_mod + tip_mod;
    double inner_dist = (Inside_radius - radial) - barrel_mod - tip_mod;

    double shape_de = max(max(outer_dist, inner_dist), cap_dist);

    if (apply_scale_add == 0) {
        shape_de = shape_de / Scale;
    }

    return shape_de;
}

// Alternative direct function implementation (combined transform + DE)
cpu_double3 cylinderIFS_formula(cpu_double3 p, constant double* params, double &de) {
    const double Radius = params[0];
    const double Halfwidth = params[1];
    const double Scale = params[2];
    const double Z_add = params[3];
    const double Y_add = params[4];
    const double X_add = params[5];
    const double Barrel = params[6];
    const double Tip = params[7];
    const int apply_scale_add = int(params[8]);
    const double Inside_radius = params[12];

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    cpu_double3 pos;
    if (apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;          pos = cpu_double3(x, y, z);
    } else {
        pos = p;
    }

    double ax = abs(x);
    double radial = sqrt(y * y + z * z);
    double cap_dist = ax - Halfwidth;
    double clamped_x = min(ax, Halfwidth);
    double barrel_mod = clamped_x * clamped_x * Barrel;
    double tip_mod = x * Tip;

    double outer_dist = (radial - Radius) + barrel_mod + tip_mod;
    double inner_dist = (Inside_radius - radial) - barrel_mod - tip_mod;
    double shape_de = max(max(outer_dist, inner_dist), cap_dist);

    if (apply_scale_add == 0) {
        shape_de /= Scale;
    }

    // Store shape DE — ASM writes to [esi-0x20] (dIFS DE slot)
    de = shape_de;

    return pos;
}

// Formula metadata
constant FormulaInfo formula_info_cylinderIFS = {
    .id = FORMULA_CYLINDERIFS,
    .name = "cylinderIFS",
    .param_count = 13,
    .complexity = 420,
    .uses_transcendental = true
};

/*
Original M3 Description:
Cylinder shape dIFS with OTrap coloring options:
  0: Change with DEstop
  1: Change with X
  2: Change with Y
  3: Change with Z
DEstop might vary on distance, choose OTrap_scale = 0 to switch off.

Mathematical Formula:
  dIFS execution model (DEoption=20):
    - Formula computes shape DE directly (not via iteration w tracking)
    - IFS engine accumulates: min_de = min(shape_de_i / w_i)
    - w tracks accumulated scale factor across iterations

  Step 1: IFS transformation (if apply_scale_add != 0):
    x' = x * Scale + X_add
    y' = y * Scale + Y_add
    z' = z * Scale + Z_add
    w *= Scale

  Step 2: Cylinder DE (computed on transformed position):
    Cylinder aligned along x-axis.
    radial = sqrt(y'^2 + z'^2)
    cap_dist = |x'| - Halfwidth
    clamped_x = min(|x'|, Halfwidth)
    barrel_mod = clamped_x^2 * Barrel
    tip_mod = x' * Tip  (signed, creates conical taper)

    outer_dist = (radial - Radius) + barrel_mod + tip_mod
    inner_dist = (Inside_radius - radial) - barrel_mod - tip_mod

    shape_de = max(outer_dist, inner_dist, cap_dist)

  Step 3: DE compensation (if apply_scale_add == 0):
    shape_de /= Scale

  Step 4: OTrap coloring:
    source = [DEstop, x', y', z'][OTrap_option & 3]
    color = source * OTrap_scale + OTrap_offset

Parameters:
  [0] Radius: double (default: 1)
  [1] Halfwidth: double (default: 1) — from .2Doubles X_halfwidth
  [2] Scale: double (default: 2) — from .2Doubles Scale (replicated for SIMD)
  [3] X_add: double (default: 3)
  [4] Y_add: double (default: 0)
  [5] Z_add: double (default: 0)
  [6] Barrel: double (default: 0)
  [7] Tip: double (default: 0)
  [8] apply_scale_add: int (default: 1)
  [9] OTrap_option: int (default: 0) — 0..3
  [10] OTrap_offset: single (default: 1)
  [11] OTrap_scale: single (default: 1)
  [12] Inside_radius: double (default: -1) — negative = solid cylinder

PVar layout (verified against SSE2 ASM offsets):
  [PVar + 0x00..0x0] = CONSTANTS: abs mask ($7FFFFFFFFFFFFFFF x2, for andpd)
  [PVar - 0x10] = Radius (double)
  [PVar - 0x18] = Halfwidth high (unused in code)
  [PVar - 0x20] = Halfwidth low (double, from .2Doubles X_halfwidth)
  [PVar - 0x28] = Scale high (double, from .2Doubles Scale)
  [PVar - 0x30] = Scale low (double, from .2Doubles Scale — both = Scale)
  [PVar - 0x38] = Z_add (double)
  [PVar - 0x40] = Y_add (double)
  [PVar - 0x48] = X_add (double)
  [PVar - 0x50] = Barrel (double)
  [PVar - 0x58] = Tip (double)
  [PVar - 0x5C] = apply_scale_add (integer)
  [PVar - 0x60] = OTrap_option (integer, 0..3)
  [PVar - 0x64] = OTrap_offset (single)
  [PVar - 0x68] = OTrap_scale (single)
  [PVar - 0x70] = Inside_radius (double)

dIFS state structure (via esi):
  [esi - 0x78] = x position (double)
  [esi - 0x70] = y position (double, packed low with z)
  [esi - 0x68] = z position (double, packed high)
  [esi + 0x70] = w (DE scale tracking, double)
  [esi - 0x20] = shape DE output (double)
  [esi + 0x80] = OTrap coloring output (double)
  [esi - 0x80] = DEstop reference (double, for OTrap option 0)

SSE2 register usage:
  xmm0 = packed (y, z) position, then (y², z²), then y²+z², then radial
  xmm1 = x position, then |x|, then cap_dist
  xmm2 = shuffled (z², y²), then Inside_radius, then inner_dist
  xmm3 = |x|, then clamped_x, then barrel_mod
  xmm4 = x (saved), then tip_mod
  xmm5 = w (for scaling)

Key notes:
  - SSE2 formula (not x87 FPU) — uses packed double operations for (y,z) pair
  - .2Doubles Scale: stores Scale twice for SIMD mulpd on (y,z) pair
  - .2Doubles X_halfwidth: only low double used, high double unused
  - When Inside_radius < 0 (default): inner_dist always negative, gives solid cylinder
  - When Barrel/Tip = 0 (default): simple cylinder with no deformation
  - Barrel creates barrel-shaped bulge: proportional to min(|x|, hw)²
  - Tip creates conical taper: proportional to x (signed)
*/
