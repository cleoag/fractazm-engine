// PolyFoldExIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: PolyFoldExIFS
// Hand-ported from ASM: 211 bytes, Capstone-verified
// DEoption: 21 (dIFS fold — position transform only, no shape DE)
// Extended polyfold: two polyfolds (XY + XZ), discontinuous (no sym)

constexpr unsigned int FORMULA_POLYFOLDEXIFS = 8765;

template<>
cpu_double3 execute_formula<FORMULA_POLYFOLDEXIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Add_X = params[0];
    const double Add_Y = params[1];
    const double Add_Z = params[2];
    // params[3..5] = 3 Euler angles (degrees) for rotation matrix
    RotMatrix3 R = rotMatrix3FromFloats(params, 3);
    const int PolyFold_Order = int(params[12]);
    const double Atan2_fixer = params[13];
    const int PolyFoldZ_Order = int(params[14]);
    const double Atan2Z_fixer = params[15];

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

    // === FIRST POLYFOLD (XY plane) ===
    // Step 3: Quantize azimuthal angle into sectors
    double angle_xy = Atan2_fixer - atan2(rot_y, rot_x);
    double n_xy = rint(angle_xy * INV_2PI * double(PolyFold_Order));
    double sector_xy = n_xy / double(PolyFold_Order) * TWO_PI;

    double cs1 = cos(sector_xy);
    double ss1 = sin(sector_xy);

    // Step 4: 2D rotation in rot_x-rot_y plane (no sym fold)
    double new_y = rot_y * cs1 + rot_x * ss1;
    double new_x = rot_x * cs1 - rot_y * ss1;
    // z unchanged from rotation: new_z = rot_z
    double new_z = rot_z;

    // === SECOND POLYFOLD (XZ plane) ===
    // Step 5: Load the just-computed x and z, polyfold in xz
    // ASM: atan2(z, x) via fpatan with ST(1)=z, ST(0)=x
    double angle_xz = Atan2Z_fixer - atan2(new_z, new_x);
    double n_xz = rint(angle_xz * INV_2PI * double(PolyFoldZ_Order));
    double sector_xz = n_xz / double(PolyFoldZ_Order) * TWO_PI;

    double cs2 = cos(sector_xz);
    double ss2 = sin(sector_xz);

    // Step 6: 2D rotation in x-z plane (no sym fold)
    // ASM: z_out = z*cs + x*ss, x_out = x*cs - z*ss
    double out_z = new_z * cs2 + new_x * ss2;
    double out_x = new_x * cs2 - new_z * ss2;

    // NOTE: DEoption=21 — no w tracking, no shape DE
    return cpu_double3(out_x, new_y, out_z);
}

// Alternative direct function implementation
cpu_double3 PolyFoldExIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_POLYFOLDEXIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_PolyFoldExIFS = {
    .id = FORMULA_POLYFOLDEXIFS,
    .name = "PolyFoldExIFS",
    .param_count = 10,
    .complexity = 780,
    .uses_transcendental = true
};

/*
Original M3 Description:
Poly fold Ex is a discontinue function; 3D extension of Poly Folding in 3D,
as it affects z too.
You can get strange Sierpinski solids with this.
Remember; Do not change the fixer or you get a discontinue transform,
with branch cuts.

Mathematical Formula:
  DEoption=21 (dIFS fold — position transform only):
    - Two sequential polyfold operations (XY + XZ)
    - Both discontinuous (no symmetric fold)
    - No w tracking, no shape DE

  Step 1: Add offsets:
    ax = x + Add_X;  ay = y + Add_Y;  az = z + Add_Z

  Step 2: Apply rotation matrix (from .3SingleAngles):
    rot_x = R[0] * (ax, ay, az)
    rot_y = R[1] * (ax, ay, az)
    rot_z = R[2] * (ax, ay, az)

  Step 3: First polyfold (XY plane):
    angle = Atan2_fixer - atan2(rot_y, rot_x)
    n = round(angle * INV_2PI * PolyFold_Order) / PolyFold_Order * TWO_PI
    Rotate (rot_x, rot_y) → (new_x, new_y), z unchanged

  Step 4: Second polyfold (XZ plane):
    angle_xz = Atan2Z_fixer - atan2(new_z, new_x)
    n = round(angle_xz * INV_2PI * PolyFoldZ_Order) / PolyFoldZ_Order * TWO_PI
    Rotate (new_x, new_z) → (out_x, out_z), y unchanged

  Step 5: Output:
    result = (out_x, new_y, out_z)

Parameters:
  [0] Add_X: double (default: 0)
  [1] Add_Y: double (default: 0)
  [2] Add_Z: double (default: 0)
  [3..11] Rotation: 9 floats (from .3SingleAngles, row-major 3x3 matrix)
  [12] PolyFold_Order: int (default: 4) — XY polygon sides
  [13] Atan2_fixer: double (default: pi) — XY angle offset
  [14] PolyFoldZ_Order: int (default: 4) — XZ polygon sides
  [15] Atan2Z_fixer: double (default: pi) — XZ angle offset

PVar layout (verified against x87 ASM offsets):
  Constants (positive offsets):
  [PVar + 0x00] = 0.159154943 (1/(2*pi))
  [PVar + 0x08] = 6.283185307 (2*pi)

  Options (negative offsets):
  [PVar - 0x10] = Add_X (double)
  [PVar - 0x18] = Add_Y (double)
  [PVar - 0x20] = Add_Z (double)
  [PVar - 0x24 .. -0x44] = Rotation matrix (9 singles, row-major)
  [PVar - 0x48] = PolyFold_Order (integer)
  [PVar - 0x50] = Atan2_fixer (double, default pi)
  [PVar - 0x54] = PolyFoldZ_Order (integer)
  [PVar - 0x5c] = Atan2Z_fixer (double, default pi)

Code structure:
  - First half (0x00-0x89): identical to PolyFoldIFS (XY polyfold)
  - Second half (0x8C-0xD2): same polyfold pattern on XZ plane
  - Reads back stored x,z for second pass: fld [esi-0x68], fld [esi-0x78]

Key notes:
  - 3D extension: polyfolds both azimuthal (XY) and elevation (XZ) angles
  - Both passes are discontinuous (no sym fold)
  - Second polyfold uses new x and z from first pass as input
  - y is unchanged by second polyfold (only x and z are rotated)
  - Changing Atan2 fixers from pi creates branch cuts (as warned in desc)
*/
