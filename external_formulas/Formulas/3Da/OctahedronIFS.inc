// OctahedronIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: OctahedronIFS
// Hand-ported from ASM: 237 bytes, DEoption=2
// Octahedral fold IFS with two rotation matrices
// Similar to Menger3 but NO abs() and octahedral fold instead of standard sort

constexpr unsigned int FORMULA_OCTAHEDRONIFS = 10030;

// ASM trace summary:
//   000D-0011: load z, y, x (NO abs step)
//   0013-004A: R1 rotation on (x, y, z) -> (rx, ry, rz)
//   004C-007C: octahedral fold (compare-swap with sign flips)
//     004C: if rx < ry, swap -> max in st0
//     0056: negate, compare with st1 (sign-aware fold)
//     0062: compare with rz, swap
//     006E: negate, compare again, swap
//     007C: reorder via fxch st(1)
//   007E-00B5: R2 rotation on folded values -> (r2x, r2y, r2z)
//   00B7-00E3: Scale, DE tracking, Menger offsets (no conditional z-fold)

template<>
cpu_double3 execute_formula<FORMULA_OCTAHEDRONIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    const double CScaleX = params[1];
    const double CScaleY = params[2];
    const double CScaleZ = params[3];

    // Rotation1 from 3 Euler angles (degrees)
    RotMatrix3 R1 = rotMatrix3FromFloats(params, 4);

    // Rotation2 from 3 Euler angles (degrees)
    RotMatrix3 R2 = rotMatrix3FromFloats(params, 13);

    // Step 1: NO abs() -- OctahedronIFS works with signed values

    // Step 2: Apply R1 rotation
    // After R1, the x87 live stack is [row0, row1, row2].
    cpu_double3 rv = R1 * cpu_double3(p.x, p.y, p.z);
    double st0 = rv.x;
    double st1 = rv.y;
    double st2 = rv.z;

    // Step 3: Octahedral fold, keeping the x87 live-stack order exactly.
    if (st0 < st1) { double t = st0; st0 = st1; st1 = t; }
    st0 = -st0;
    if (!(st0 < st1)) { double t = st0; st0 = st1; st1 = t; }
    st0 = -st0;
    if (st0 < st2) { double t = st0; st0 = st2; st2 = t; }
    st0 = -st0;
    if (!(st0 < st2)) { double t = st0; st0 = st2; st2 = t; }
    st0 = -st0;
    { double t = st0; st0 = st1; st1 = t; } // 0x7C: fxch st(1)

    // Step 4: R2 stage, using the post-fold live stack ordering from the asm.
    const double m50 = params[13], m54 = params[14], m58 = params[15];
    const double m5c = params[16], m60 = params[17], m64 = params[18];
    const double m68 = params[19], m6c = params[20], m70 = params[21];

    double r2y = st1 * m5c + st0 * m60 + st2 * m64;
    double r2x = st1 * m50 + st0 * m54 + st2 * m58;
    double r2z = st1 * m68 + st0 * m6c + st2 * m70;

    // Step 5: Scale + DE tracking
    de = de * Scale;

    // Step 6: Offsets + Store
    double k = Scale - 1.0;
    return cpu_double3(
        r2x * Scale - k * CScaleX,
        r2y * Scale - k * CScaleY,
        r2z * Scale - k * CScaleZ
    );
}

// Formula metadata
constant FormulaInfo formula_info_OctahedronIFS = {
    .id = FORMULA_OCTAHEDRONIFS,
    .name = "OctahedronIFS",
    .param_count = 10,
    .complexity = 860,
    .uses_transcendental = true
};

/*
Original M3 Description:
Octahedron IFS fractal with two rotation matrices.

Mathematical Formula:
  1. NO absolute values (unlike Menger3)
  2. First rotation: [rx,ry,rz] = R1 * [x,y,z]
  3. Octahedral fold: compare-swap network with alternating negation
     - Produces octahedral symmetry via sign-aware folding
     - Phase 1: ascending swap, negate+swap, negate
     - Phase 2: ascending swap vs z, negate+swap, negate
     - Final reorder via fxch st(1)
  4. Second rotation: [r2x,r2y,r2z] = R2 * [folded values]
  5. Scale + DE tracking: de *= Scale
  6. Uniform offsets (all three axes, no conditional z-fold):
     x = r2x * Scale - (Scale-1) * CScaleX
     y = r2y * Scale - (Scale-1) * CScaleY
     z = r2z * Scale - (Scale-1) * CScaleZ

Parameters:
  [0] Scale: double (default: 2)
  [1] CScale_X: double (default: 1)
  [2] CScale_Y: double (default: 0)
  [3] CScale_Z: double (default: 0)
  [4-6] Rotation1: 3 angles in degrees (default: 0)
  [7-9] Rotation2: 3 angles in degrees (default: 0)

PVar layout:
  [PVar - 0x10] = Scale
  [PVar - 0x18] = CScale_X
  [PVar - 0x20] = CScale_Y
  [PVar - 0x28] = CScale_Z
  Rotation1 matrix: [PVar - 0x2C..0x4C] (9 singles)
  Rotation2 matrix: [PVar - 0x50..0x70] (9 singles)

Key differences from Menger3:
  - No abs() step (works with signed coordinates)
  - Octahedral fold instead of standard sort
  - CScaleX default=1, CScaleY/Z default=0 (vs all 1 for Menger3)
  - Scale default=2 (vs 3 for Menger3)
  - No conditional z-fold
*/
