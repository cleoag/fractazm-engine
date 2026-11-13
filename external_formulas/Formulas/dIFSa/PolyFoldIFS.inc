// PolyFoldIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: PolyFoldIFS
// Hand-ported from ASM: 141 bytes, Capstone-verified
// DEoption: 21 (dIFS fold — position transform only, no shape DE)
// Polyfold (discontinuous): quantize azimuthal angle into sectors, no reflection

constexpr unsigned int FORMULA_POLYFOLDIFS = 4644;

template<>
cpu_double3 execute_formula<FORMULA_POLYFOLDIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Add_X = params[0];
    const double Add_Y = params[1];
    const double Add_Z = params[2];
    // params[3..5] = 3 Euler angles (degrees) for rotation matrix
    RotMatrix3 R = rotMatrix3FromFloats(params, 3);
    const int PolyFold_Order = int(params[12]);
    const double Atan2_fixer = params[13];

    // Mathematical constants (from M3 CONSTANTS section)
    const double INV_2PI = 0.159154943;    // 1 / (2*pi)
    const double TWO_PI = 6.283185307;     // 2*pi

    // Step 1: Add offsets to position
    double ax = p.x + Add_X;
    double ay = p.y + Add_Y;
    double az = p.z + Add_Z;

    // Step 2: Apply 3x3 rotation matrix (built from 3 Euler angles)
    cpu_double3 rv = R * cpu_double3(ax, ay, az);
    double rot_x = rv.x;
    double rot_y = rv.y;
    double rot_z = rv.z;

    // Step 3: Polyfold — quantize azimuthal angle into sectors
    // ASM: atan2(rot_y, rot_x) then fsubr(Atan2_fixer)
    double angle = Atan2_fixer - atan2(rot_y, rot_x);

    // Quantize: n = round(angle * INV_2PI * PolyFold_Order)
    // NOTE: Unlike PolyFold-symIFS, sector index n is NOT stored — no parity check
    double n_float = rint(angle * INV_2PI * double(PolyFold_Order));

    // Sector angle = n / PolyFold_Order * TWO_PI
    double sector = n_float / double(PolyFold_Order) * TWO_PI;

    double cs = cos(sector);
    double ss = sin(sector);

    // Step 4: 2D rotation in the rot_x-rot_y plane
    // ASM: new_y = rot_y*cs + rot_x*ss → stored as y (NO negate — discontinuous)
    //      new_x = rot_x*cs - rot_y*ss → stored as x
    double new_x = rot_x * cs - rot_y * ss;
    double new_y = rot_y * cs + rot_x * ss;

    // NOTE: No symmetric fold — this creates discontinuity at sector boundaries
    // DEoption=21 — no w tracking, no shape DE
    return cpu_double3(new_x, new_y, rot_z);
}

// Alternative direct function implementation
cpu_double3 PolyFoldIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_POLYFOLDIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_PolyFoldIFS = {
    .id = FORMULA_POLYFOLDIFS,
    .name = "PolyFoldIFS",
    .param_count = 8,
    .complexity = 520,
    .uses_transcendental = true
};

/*
Original M3 Description:
Poly fold is a discontinue function. It reflects objects belonging to a
quadrant to all other quadrants.
You can get Sierpinski and Barnsley style fractals with this, if you can
handle it. Try to put it as 1st transform...

Mathematical Formula:
  DEoption=21 (dIFS fold — position transform only):
    - Identical to PolyFold-symIFS EXCEPT no symmetric fold
    - No negate for even sector indices → discontinuous at sector boundaries
    - No w tracking, no shape DE

  Step 1: Add offsets:
    ax = x + Add_X;  ay = y + Add_Y;  az = z + Add_Z

  Step 2: Apply rotation matrix (from .3SingleAngles):
    rot_x = R[0] * (ax, ay, az)
    rot_y = R[1] * (ax, ay, az)
    rot_z = R[2] * (ax, ay, az)

  Step 3: Polyfold (azimuthal in rot_x-rot_y plane):
    angle = Atan2_fixer - atan2(rot_y, rot_x)
    n = round(angle * INV_2PI * PolyFold_Order)
    sector = n / PolyFold_Order * TWO_PI
    cs = cos(sector), ss = sin(sector)

  Step 4: 2D rotation to reference sector:
    new_x = rot_x*cs - rot_y*ss
    new_y = rot_y*cs + rot_x*ss    (NO negate — discontinuous)

  Step 5: Output:
    result = (new_x, new_y, rot_z)

Parameters:
  [0] Add_X: double (default: 0)
  [1] Add_Y: double (default: 0)
  [2] Add_Z: double (default: 0)
  [3..11] Rotation: 9 floats (from .3SingleAngles, row-major 3x3 matrix)
  [12] PolyFold_Order: int (default: 5) — polygon sides
  [13] Atan2_fixer: double (default: 3.14159265...) — angle offset (pi centers folds)

PVar layout (verified against x87 ASM offsets):
  Constants (positive offsets):
  [PVar + 0x00] = 0.159154943 (1/(2*pi))
  [PVar + 0x08] = 6.283185307 (2*pi)
  [PVar + 0x10] = 1.57079632675 (pi/2, unused)
  [PVar + 0x18] = 0.017453293 (pi/180, unused)

  Options (negative offsets):
  [PVar - 0x10] = Add_X (double)
  [PVar - 0x18] = Add_Y (double)
  [PVar - 0x20] = Add_Z (double)
  [PVar - 0x24 .. -0x44] = Rotation matrix (9 singles, row-major)
  [PVar - 0x48] = PolyFold_Order (integer)
  [PVar - 0x50] = Atan2_fixer (double, default pi)

Key difference from PolyFold-symIFS:
  - PolyFoldIFS: after frndint, goes DIRECTLY to fidiv (no fist/mov)
  - PolyFold-symIFS: after frndint, stores n as int, checks parity, negates y
  - This means PolyFoldIFS is 20 bytes shorter (141 vs 161)
  - Discontinuity at sector boundaries (all sectors map to same reference)
  - PolyFold-symIFS's "reflection cheat" eliminates this discontinuity
*/
