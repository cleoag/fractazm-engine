// Trans-KochIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Trans-KochIFS
// Hand-ported from ASM: 317 bytes, Capstone-verified
// DEoption: 21 (dIFS fold — but DOES track w via KochScale)
// Polyfold-sym + abs(x) + Koch angle rotations/folds + scale + offset

constexpr unsigned int FORMULA_TRANS_KOCHIFS = 7522;

template<>
cpu_double3 execute_formula<FORMULA_TRANS_KOCHIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Add_X = params[0];
    const double Add_Y = params[1];
    const double Add_Z = params[2];
    // This formula uses the 13-slot hybrid contract:
    // raw .3SingleAngles + trailing sin/cos from .SingleAngle KochAngle.
    RotMatrix3 R = rotMatrix3FromFloats(params, 3);
    const int PolyFold_Order = int(params[12]);
    const double sin_Koch = params[13];
    const double cos_Koch = params[14];
    const double KochSub = params[15];
    const double KochScale = params[16];
    const double KochFixX = params[17];
    const double KochFixZ = params[18];

    // Mathematical constants (from M3 CONSTANTS section)
    const double INV_2PI = 0.159154943;    // 1 / (2*pi)
    const double TWO_PI = 6.283185307;     // 2*pi
    const double PI = 3.14159265358979;    // pi (used as Atan2_fixer, hardcoded)

    // Step 1: Add offsets to position
    double ax = p.x + Add_X;
    double ay = p.y + Add_Y;
    double az = p.z + Add_Z;

    // Step 2: Apply 3x3 rotation matrix
    cpu_double3 rv = R * cpu_double3(ax, ay, az);
    double rot_x = rv.x;
    double rot_y = rv.y;
    double rot_z = rv.z;

    // Step 3: Polyfold-sym in XY plane (Atan2_fixer = pi, from constant)
    double angle = PI - atan2(rot_y, rot_x);
    int n = int(rint(angle * INV_2PI * double(PolyFold_Order)));
    double sector = double(n) / double(PolyFold_Order) * TWO_PI;

    double cs = cos(sector);
    double ss = sin(sector);

    // Step 4: 2D rotation + symmetric fold + ABS(x)
    double new_y = rot_y * cs + rot_x * ss;
    double new_x = rot_x * cs - rot_y * ss;

    // Symmetric fold: negate y for even sector indices
    if ((n & 1) == 0) {
        new_y = -new_y;
    }

    // ASM: fabs on x component (UNLIKE plain PolyFold-symIFS)
    new_x = abs(new_x);

    double x = new_x;
    double y = new_y;
    double z = rot_z;

    // Step 5: First Koch rotation (by +KochAngle in xz plane)
    // ASM: x' = x*cos_Koch - z*sin_Koch, z' = x*sin_Koch + z*cos_Koch
    double x1 = x * cos_Koch - z * sin_Koch;
    double z1 = x * sin_Koch + z * cos_Koch;

    // Step 6: First Koch fold (tent fold centered at cos_Koch * KochSub)
    // ASM: x = |cos_Koch * KochSub - x'|  (fsubrp + fabs)
    x = abs(cos_Koch * KochSub - x1);
    z = z1;

    // Step 7: Second Koch rotation (by -KochAngle in xz plane, inverse)
    // ASM: x'' = x*cos_Koch + z*sin_Koch, z'' = z*cos_Koch - x*sin_Koch
    double x2 = x * cos_Koch + z * sin_Koch;
    double z2 = z * cos_Koch - x * sin_Koch;

    // Step 8: Second Koch fold (tent fold centered at sin_Koch * KochSub)
    // ASM: x = |sin_Koch * KochSub - x''|  (fsubrp + fabs)
    x = abs(sin_Koch * KochSub - x2);
    z = z2;

    // Step 9: Scale all coordinates AND w (DE tracking)
    // ASM: x *= KochScale, y *= KochScale, z *= KochScale, w *= KochScale
    x *= KochScale;
    y *= KochScale;
    z *= KochScale;
    de *= KochScale;    // w tracking (unusual for DEoption=21)

    // Step 10: Subtract position offsets
    // ASM: x -= KochFixX, z -= KochFixZ
    x -= KochFixX;
    z -= KochFixZ;

    return cpu_double3(x, y, z);
}

// Alternative direct function implementation
cpu_double3 Trans_KochIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_TRANS_KOCHIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_Trans_KochIFS = {
    .id = FORMULA_TRANS_KOCHIFS,
    .name = "Trans-KochIFS",
    .param_count = 13,
    .complexity = 1110,
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
  DEoption=21 (dIFS fold with w tracking via KochScale):
    - PolyFold-sym + abs(x) + Koch angle rotations/folds + scale + offset
    - DOES track w (de *= KochScale) — unusual for DEoption=21

  Step 1: Add offsets + 3x3 rotation (same as PolyFold-symIFS)

  Step 2: Polyfold-sym in XY plane:
    - Uses pi as Atan2_fixer (hardcoded constant, not user parameter)
    - Standard sym fold: negate y for even sectors
    - ADDITIONAL: x = abs(x) after the polyfold rotation

  Step 3: First Koch rotation (xz plane, by +KochAngle):
    x' = x*cos(Koch) - z*sin(Koch)
    z' = x*sin(Koch) + z*cos(Koch)

  Step 4: First Koch fold (tent fold):
    x = |cos(Koch) * KochSub - x'|

  Step 5: Second Koch rotation (xz plane, by -KochAngle, inverse):
    x'' = x*cos(Koch) + z*sin(Koch)
    z'' = z*cos(Koch) - x*sin(Koch)

  Step 6: Second Koch fold (tent fold):
    x = |sin(Koch) * KochSub - x''|

  Step 7: Scale + offset:
    x *= KochScale, y *= KochScale, z *= KochScale
    w *= KochScale (DE tracking)
    x -= KochFixX, z -= KochFixZ

Parameters:
  [0] Add_X: double (default: 0)
  [1] Add_Y: double (default: 0)
  [2] Add_Z: double (default: 0)
  [3..11] Rotation: 9 floats (from .3SingleAngles, row-major 3x3 matrix)
  [12] PolyFold_Order: int (default: 5) — polygon sides
  [13] sin_KochAngle: double (precomputed from .SingleAngle KochAngle=-30)
  [14] cos_KochAngle: double (precomputed from .SingleAngle KochAngle=-30)
  [15] KochSub: double (default: 0.333333) — Koch fold center scale
  [16] KochScale: double (default: 3) — iteration scale
  [17] KochFixX: double (default: 1) — x offset after scale
  [18] KochFixZ: double (default: 0) — z offset after scale

PVar layout (verified against x87 ASM offsets):
  Constants (positive offsets):
  [PVar + 0x00] = 0.159154943 (1/(2*pi))
  [PVar + 0x08] = 6.283185307 (2*pi)
  [PVar + 0x10] = 3.14159265... (pi, used as Atan2_fixer)

  Options (negative offsets):
  [PVar - 0x10] = Add_X (double)
  [PVar - 0x18] = Add_Y (double)
  [PVar - 0x20] = Add_Z (double)
  [PVar - 0x24 .. -0x44] = Rotation matrix (9 singles, row-major)
  [PVar - 0x48] = PolyFold_Order (integer)
  [PVar - 0x4c] = sin(KochAngle) (single, from .SingleAngle)
  [PVar - 0x50] = cos(KochAngle) (single, from .SingleAngle)
  [PVar - 0x58] = KochSub (double)
  [PVar - 0x60] = KochScale (double)
  [PVar - 0x68] = KochFixX (double)
  [PVar - 0x70] = KochFixZ (double)

dIFS state structure (via esi):
  [esi - 0x78] = x position (double)
  [esi - 0x70] = y position (double)
  [esi - 0x68] = z position (double)
  [esi + 0x70] = w (DE scale tracking — USED by this formula!)

Code structure:
  0x00-0x9D: Rotation + Polyfold-sym (same as PolyFold-symIFS)
    BUT with fabs at 0x90 (abs of x component) — key difference
  0xA0-0xBF: First Koch rotation (2D rotation of x,z by KochAngle)
  0xC2-0xD0: First Koch fold: x = |cos_Koch * KochSub - x|
  0xD3-0xF2: Second Koch rotation (inverse) + second Koch fold
  0x105-0x128: Scale x,y,z,w by KochScale
  0x12A-0x139: Subtract KochFixX from x, KochFixZ from z

Key notes:
  - Most complex polyfold formula (317 bytes)
  - Combines polyfold-sym with Koch fractal folds
  - abs(x) after polyfold ensures only positive x enters Koch folds
  - Two Koch folds create fractal structure along xz diagonal
  - Koch fold centers: cos(Koch)*KochSub and sin(Koch)*KochSub
  - Forward rotation → fold → inverse rotation → fold pattern
  - Atan2_fixer is pi (constant), not a user parameter
  - KochAngle default -30° gives sin=-0.5, cos=0.8660
  - KochSub=1/3 gives standard Koch geometry
  - KochScale=3 matches Koch fractal self-similarity ratio
  - w *= KochScale means DE accumulates properly across iterations
  - KochFixX/Z are the "fixed point" offsets for the IFS contraction
*/
