// roomclrIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: roomclrIFS
// Hand-ported from ASM: 218 bytes, DEoption=20
// An empty room (box) with three selectable wall colors and sphere pinch/punch.
// dIFS shape: box interior distance with spherical distortion.

constexpr unsigned int FORMULA_ROOMCLRIFS = 10065;

// ASM trace summary:
//   0x00-0x3: Standard SSE2 IFS preamble (Scale, X/Y/Z_add, Apply_scale_add)
//   0x40-0x4A: Store x,y,z to stack; load z for sign test
//   0x4B-0x5: ftst z → if z < 0: OTrap = OTClr_L; else OTrap = OTClr_R
//   0x65-0x72: |z| - Z_halfwidth → dz
//   0x6A-0x88: |y| - Y_halfwidth → dy; if dy >= dz: OTrap = OTClr, keep dy
//   0x8A-0xA9: |x| - X_halfwidth → dx; if dx >= dy: OTrap = OTClr, keep dx
//   0xA9-0xBE: r = sqrt(x*x + y*y + z*z); shape_de = de_best + Pinch * r
//   0xC5-0xD3: |shape_de| - RoomThick; Scale correction; store DE

template<>
cpu_double3 execute_formula<FORMULA_ROOMCLRIFS>(cpu_double3 p, constant double* params, double &de) {
    // Parameters from M3 [OPTIONS]:
    const double RoomThick  = params[0];     // .Double RoomThick = .1
    const double Scale      = params[1];     // .2Doubles Scale = 2
    const double Z_add      = params[2];     // .2Doubles Z add = 0
    const double Y_add      = params[3];     // .Double Y add = 0
    const double X_add      = params[4];     // .Double X add = 0
    const double Pinch      = params[5];     // .Double Pinch / Punch = 0.1
    const int Apply_scale_add = int(params[6]); // .Integer Apply scale+add = 0
    const double X_halfwidth = params[7];    // .Double X halfwidth = 1
    const double Y_halfwidth = params[8];    // .Double Y halfwidth = 1
    const double Z_halfwidth = params[9];    // .Double Z halfwidth = 1
    const double OTClr_L     = params[10];   // .Double OTClr_L = -1
    const double OTClr_R     = params[11];   // .Double OTClr_R = 1
    const double OTClr       = params[12];   // .Double OTClr = 0

    double state_x = p.x;
    double state_y = p.y;
    double state_z = p.z;

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
        state_x = x;
        state_y = y;
        state_z = z;
    }

    // Step 2: Sign test on z for OTrap selection
    double otrap;
    if (z < 0.0) {
        otrap = OTClr_L;
    } else {
        otrap = OTClr_R;
    }

    // Step 3: Compute box interior distance
    // For each axis: distance from wall = |coord| - halfwidth
    // Keep the maximum (closest wall)
    double dz = fabs(z) - Z_halfwidth;
    double dy = fabs(y) - Y_halfwidth;

    // ASM: fcom st(1) at 0x73 → if dy >= dz: keep dy (it's larger, closer to wall)
    // Also sets OTClr to [edi - 0x7c] if dy >= dz
    double de_best;
    if (dy >= dz) {
        de_best = dy;
        otrap = OTClr;
    } else {
        de_best = dz;
    }

    double dx = fabs(x) - X_halfwidth;

    // ASM: fcom at 0x92 → if dx >= de_best: keep dx
    if (dx >= de_best) {
        de_best = dx;
        otrap = OTClr;
    }

    params[63] = otrap;

    // Step 4: Sphere pinch/punch distortion
    // ASM at 0xA9: x*x + y*y + z*z → sqrt → r
    // Then: shape = de_best + Pinch * r
    double r = sqrt(x * x + y * y + z * z);
    double shape_raw = de_best + Pinch * r;

    // Step 5: Apply abs and subtract RoomThick
    // ASM at 0xC5: fabs → fsub RoomThick
    double shape_de = fabs(shape_raw) - RoomThick;

    // Step 6: Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return cpu_double3(state_x, state_y, state_z);
}

// Alternative direct function implementation
cpu_double3 roomclrIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_ROOMCLRIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_roomclrIFS = {
    .id = FORMULA_ROOMCLRIFS,
    .name = "roomclrIFS",
    .param_count = 13,
    .complexity = 218,
    .uses_transcendental = false
};

/*
Original M3 Description:
An empty room, it's interesting because it has three selectable colors for the walls.
Pinch/Punch makes the room look "spherical" if desired.
Of course it's suitable to be used as a 'scenario', not for iterations.
Enable Color2 on choice to see colored DIFS!

Mathematical Formula:
  DEoption=20 (dIFS shape - SSE2 IFS + x87 box room DE):
    - Standard SSE2 IFS preamble
    - Box interior: max(|x|-Xhw, |y|-Yhw, |z|-Zhw) gives closest wall distance
    - Sphere pinch: add Pinch * sqrt(x^2+y^2+z^2) to distort the box into a sphere
    - Shell: |distorted_de| - RoomThick gives room wall thickness
    - OTrap coloring: selects color based on which wall is closest + z sign

  Step 1: IFS transform:
    x = p.x * Scale + X_add;  y = p.y * Scale + Y_add;  z = p.z * Scale + Z_add
    if Apply_scale_add: w *= Scale

  Step 2: Box interior distance:
    de_best = max(|z| - Z_hw, |y| - Y_hw, |x| - X_hw)

  Step 3: Spherical distortion:
    shape_raw = de_best + Pinch * sqrt(x^2 + y^2 + z^2)

  Step 4: Room shell:
    shape_de = |shape_raw| - RoomThick

Parameters:
  [0] RoomThick: double (default: 0.1) - wall thickness
  [1] Scale: double (default: 2) - IFS scale factor (.2Doubles)
  [2] Z_add: double (default: 0) - Z offset (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] Pinch / Punch: double (default: 0.1) - spherical distortion
  [6] Apply_scale_add: int (default: 0)
  [7] X_halfwidth: double (default: 1)
  [8] Y_halfwidth: double (default: 1)
  [9] Z_halfwidth: double (default: 1)
  (OTClr_L, OTClr_R, OTClr are orbit trap color values - framework-managed)

PVar layout (verified against ASM offsets):
  [PVar - 0x10] = RoomThick (double)
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add (double)
  [PVar - 0x40] = X_add (double)
  [PVar - 0x48] = Pinch/Punch (double)
  [PVar - 0x4c] = Apply_scale_add (integer)
  [PVar - 0x54] = X_halfwidth (double) -- [edi - 0x54] in ASM at 0x8
  [PVar - 0x5c] = Y_halfwidth (double) -- [edi - 0x5c] in ASM at 0x70
  [PVar - 0x64] = Z_halfwidth (double) -- [edi - 0x64] in ASM at 0x67
  [PVar - 0x6c] = OTClr_L (double) -- [edi - 0x6c] in ASM at 0x57
  [PVar - 0x74] = OTClr_R (double) -- [edi - 0x74] in ASM at 0x5C
  [PVar - 0x7c] = OTClr (double) -- [edi - 0x7c] in ASM at 0x7D, 0x9C

Code structure:
  0x00-0x3: Standard SSE2 IFS preamble
  0x40-0x4A: Store coords to stack
  0x4B-0x64: z sign test → OTrap color selection
  0x65-0x88: |z|-Zhw, |y|-Yhw → max → possibly set OTClr
  0x8A-0xA7: |x|-Xhw → max → possibly set OTClr
  0xA9-0xBE: x^2+y^2+z^2 → sqrt → Pinch*r → add de_best
  0xC5-0xD3: |shape_raw| - RoomThick → Scale correction → store DE

Key notes:
  - The room is a hollow box: distance is measured from inside
  - Pinch > 0 makes walls bulge inward (spherical); Pinch < 0 = outward (pillow)
  - Three separate OTrap colors (L/R/main) selected by wall proximity and z sign
  - Not suitable for iteration - use as single dIFS object (1 iteration)
  - 218 bytes - moderate complexity
*/
