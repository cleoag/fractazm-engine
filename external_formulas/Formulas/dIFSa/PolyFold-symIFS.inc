// PolyFold-symIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: PolyFold-symIFS
// Hand-ported from ASM: 161 bytes, Capstone-verified
// DEoption: 21 (dIFS fold — position transform only, no shape DE)
// Polyfold symmetric: quantize azimuthal angle into sectors with alternating reflection

constexpr unsigned int FORMULA_POLYFOLD_SYMIFS = 8627;

template<>
cpu_double3 execute_formula<FORMULA_POLYFOLD_SYMIFS>(cpu_double3 p, constant double* params, double &de) {
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
    // Row 0 → rot_x (used as x in polyfold)
    // Row 1 → rot_y (used as y in polyfold)
    // Row 2 → rot_z (passed through as z output)
    cpu_double3 rv = R * cpu_double3(ax, ay, az);
    double rot_x = rv.x;
    double rot_y = rv.y;
    double rot_z = rv.z;

    // Step 3: Polyfold — quantize azimuthal angle into sectors
    // ASM: atan2(rot_y, rot_x) then fsubr(Atan2_fixer)
    double angle = Atan2_fixer - atan2(rot_y, rot_x);

    // Quantize: n = round(angle * INV_2PI * PolyFold_Order)
    int n = int(rint(angle * INV_2PI * double(PolyFold_Order)));

    // Sector angle = n / PolyFold_Order * TWO_PI
    double sector = double(n) / double(PolyFold_Order) * TWO_PI;

    double cs = cos(sector);
    double ss = sin(sector);

    // Step 4: 2D rotation in the rot_x-rot_y plane
    // ASM: new_y = rot_y*cs + rot_x*ss → stored as y
    //      new_x = rot_x*cs - rot_y*ss → stored as x
    double new_x = rot_x * cs - rot_y * ss;
    double new_y = rot_y * cs + rot_x * ss;

    // Step 5: Symmetric fold — negate y for even sector indices
    // ASM: if (n & 1) == 0: fchs (the "reflection cheat")
    if ((n & 1) == 0) {
        new_y = -new_y;
    }

    // Step 6: Output position
    // z passes through as rot_z (no modification after rotation)
    // NOTE: DEoption=21 — no w tracking, no shape DE
    return cpu_double3(new_x, new_y, rot_z);
}

// Alternative direct function implementation
cpu_double3 PolyFold_symIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_POLYFOLD_SYMIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_PolyFold_symIFS = {
    .id = FORMULA_POLYFOLD_SYMIFS,
    .name = "PolyFold-symIFS",
    .param_count = 8,
    .complexity = 540,
    .uses_transcendental = true
};

/*
Original M3 Description:
Poly fold reflects objects belonging to a quadrant to all other quadrants.
The "sym" means that it is a "symmetrized" version of normal polyfold;
You can use "Add Y" without adding discontinuity!
This is done using a "reflection cheat" on even/odd quadrants, it works
perfectly for even orders and fairly good for odd ones.
You can get Sierpinski and Barnsley style fractals with this.
Try to put it as 1st transform...

Mathematical Formula:
  DEoption=21 (dIFS fold — position transform only):
    - No IFS scale/translate (no Scale, no X/Y/Z_add like DEoption=20)
    - No w tracking (DE not modified)
    - No shape DE output
    - Pure position transformation for use between shape formulas

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
    new_y = rot_y*cs + rot_x*ss

  Step 5: Symmetric fold (reflection cheat):
    If n is even: new_y = -new_y

  Step 6: Output:
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
    R[0][0] at -0x24, R[0][1] at -0x28, R[0][2] at -0x2c
    R[1][0] at -0x30, R[1][1] at -0x34, R[1][2] at -0x38
    R[2][0] at -0x3c, R[2][1] at -0x40, R[2][2] at -0x44
  [PVar - 0x48] = PolyFold_Order (integer)
  [PVar - 0x50] = Atan2_fixer (double, default pi)

dIFS state structure (via esi):
  [esi - 0x78] = x position (double)
  [esi - 0x70] = y position (double)
  [esi - 0x68] = z position (double)
  No w tracking (DEoption=21)
  No shape DE output (DEoption=21)

Pure x87 FPU execution (no SSE2):
  - All computation done on x87 stack
  - No packed SIMD operations (unlike DEoption=20 formulas)
  - Stack depth peaks at 7 registers during rotation computation
  - fist + mov for sector parity check (integer branch on n & 1)

Rotation matrix computation order (interleaved for stack efficiency):
  Row 2 (→ rot_z) computed first: [0x3c, 0x40, 0x44]
  Row 0 (→ rot_x) computed second: [0x24, 0x28, 0x2c]
  Row 1 (→ rot_y) computed third: [0x30, 0x34, 0x38]
  Final stack order: ST(0)=rot_x, ST(1)=rot_y, ST(2)=rot_z

Key notes:
  - DEoption=21 = pure fold transform (no scale, no DE, no w)
  - Polyfold quantizes angle to N sectors for polygon symmetry
  - Atan2_fixer=pi centers the fold at angle=pi (negative x-axis)
  - Symmetric fold negates y for even sector indices
  - "Reflection cheat" works perfectly for even PolyFold_Order
  - Odd orders produce slight discontinuity (approximate symmetry)
  - Pure x87 FPU (no SSE2) — characteristic of DEoption=21 folds
  - Position offsets added BEFORE rotation (in original space)
*/
