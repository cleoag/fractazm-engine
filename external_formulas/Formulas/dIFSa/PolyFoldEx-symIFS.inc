// PolyFoldEx-symIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: PolyFoldEx-symIFS
// Hand-ported from ASM: 249 bytes, Capstone-verified
// DEoption: 21 (dIFS fold — position transform only, no shape DE)
// Extended polyfold symmetric: two polyfolds (XY + XZ) with alternating reflection

constexpr unsigned int FORMULA_POLYFOLDEX_SYMIFS = 7809;

template<>
cpu_double3 execute_formula<FORMULA_POLYFOLDEX_SYMIFS>(cpu_double3 p, constant double* params, double &de) {
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

    // === FIRST POLYFOLD (XY plane, with symmetric fold) ===
    // Step 3: Quantize azimuthal angle into sectors
    double angle_xy = Atan2_fixer - atan2(rot_y, rot_x);
    int n_xy = int(rint(angle_xy * INV_2PI * double(PolyFold_Order)));
    double sector_xy = double(n_xy) / double(PolyFold_Order) * TWO_PI;

    double cs1 = cos(sector_xy);
    double ss1 = sin(sector_xy);

    // Step 4: 2D rotation + symmetric fold in XY
    double new_y = rot_y * cs1 + rot_x * ss1;
    double new_x = rot_x * cs1 - rot_y * ss1;

    // Symmetric fold: negate y for even sector indices
    if ((n_xy & 1) == 0) {
        new_y = -new_y;
    }

    // z unchanged from rotation
    double new_z = rot_z;

    // === SECOND POLYFOLD (XZ plane, with symmetric fold) ===
    // Step 5: Load x and z, polyfold in xz
    // ASM: atan2(z, x) via fpatan with ST(1)=z, ST(0)=x
    double angle_xz = Atan2Z_fixer - atan2(new_z, new_x);
    int n_xz = int(rint(angle_xz * INV_2PI * double(PolyFoldZ_Order)));
    double sector_xz = double(n_xz) / double(PolyFoldZ_Order) * TWO_PI;

    double cs2 = cos(sector_xz);
    double ss2 = sin(sector_xz);

    // Step 6: 2D rotation + symmetric fold in XZ
    double out_z = new_z * cs2 + new_x * ss2;
    double out_x = new_x * cs2 - new_z * ss2;

    // Symmetric fold: negate z for even sector indices
    if ((n_xz & 1) == 0) {
        out_z = -out_z;
    }

    // NOTE: DEoption=21 — no w tracking, no shape DE
    return cpu_double3(out_x, new_y, out_z);
}

// Alternative direct function implementation
cpu_double3 PolyFoldEx_symIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_POLYFOLDEX_SYMIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_PolyFoldEx_symIFS = {
    .id = FORMULA_POLYFOLDEX_SYMIFS,
    .name = "PolyFoldEx-symIFS",
    .param_count = 10,
    .complexity = 820,
    .uses_transcendental = true
};

/*
Original M3 Description:
Poly fold Ex is a 3D extension of Poly Folding (sym) in 3D, as it
affects z too.
You can get strange Sierpinski solids with this.
Remember; Do not change the fixers or you get a discontinue transform,
with branch cuts.

Mathematical Formula:
  DEoption=21 (dIFS fold — position transform only):
    - Two sequential polyfold operations (XY + XZ)
    - Both with symmetric fold (negate for even sectors)
    - No w tracking, no shape DE

  Step 1: Add offsets:
    ax = x + Add_X;  ay = y + Add_Y;  az = z + Add_Z

  Step 2: Apply rotation matrix (from .3SingleAngles):
    rot_x = R[0] * (ax, ay, az)
    rot_y = R[1] * (ax, ay, az)
    rot_z = R[2] * (ax, ay, az)

  Step 3: First polyfold-sym (XY plane):
    angle = Atan2_fixer - atan2(rot_y, rot_x)
    n = round(angle * INV_2PI * PolyFold_Order)
    sector = n / PolyFold_Order * TWO_PI
    new_y = rot_y*cs + rot_x*ss (negated if n even)
    new_x = rot_x*cs - rot_y*ss

  Step 4: Second polyfold-sym (XZ plane):
    angle_xz = Atan2Z_fixer - atan2(new_z, new_x)
    n_xz = round(angle_xz * INV_2PI * PolyFoldZ_Order)
    sector_xz = n_xz / PolyFoldZ_Order * TWO_PI
    out_z = new_z*cs + new_x*ss (negated if n_xz even)
    out_x = new_x*cs - new_z*ss

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
  - First half (0x00-0x9C): identical to PolyFold-symIFS (XY polyfold+sym)
  - Intermediate store: y at [esi-0x70], x at [esi-0x78], z at [esi-0x68]
  - Second half (0x9-0xF8): same polyfold+sym pattern on XZ plane
  - Second pass reads back stored x,z: fld [esi-0x68], fld [esi-0x78]

Key notes:
  - Symmetric version of PolyFoldExIFS (same structure, adds negate)
  - Both polyfold passes include the "reflection cheat" for continuity
  - XZ polyfold: atan2(z, x) → fold → negate z for even sectors
  - 249 bytes vs 211 for non-sym (38 extra bytes for two sym checks)
  - Changing fixers from pi creates branch cuts (as warned in desc)
*/
