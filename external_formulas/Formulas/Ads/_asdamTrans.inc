// _asdamTrans.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _asdamTrans
// Hand-ported from ASM: 127 bytes, DEoption=-1
// Radial-dependent rotation transform (adds rotated scaled copy of point)

constexpr unsigned int FORMULA__ASDAMTRANS = 9341;

// ASM trace:
//   s = [PVar-0x34] / sqrt(x²+y²+z²)    (Term_Scale / |p|)
//   Compute scaled point: sx=x*s, sy=y*s, sz=z*s
//   FillCustomVBufWithVars type 6 stores matrix row-major: M[0] at [PVar-0x0c..14],
//     M[1] at [PVar-0x18..20], M[2] at [PVar-0x24..2c]
//   In Metal: Metal row 0 = M[0] → adds to x, row 1 = M[1] → adds to y, row 2 = M[2] → adds to z
//   ASM confirms: r1 (using [PVar-0x0c..14]=M[0]) stored to &x, r2 (M[1]) to &y, r0 (M[2]) to &z
//   Output: new = p + R * (s * p)   (no permutation needed in Metal)
//     new_x = x + row0
//     new_y = y + row1
//     new_z = z + row2
//
// C handling: DEoption=-1 (transform), no C addition.

template<>
cpu_double3 execute_formula<FORMULA__ASDAMTRANS>(cpu_double3 p, constant double* params, double &de) {
    // Build rotation matrix from 3 Euler angles (raw .3SingleAngles input)
    RotMatrix3 R = rotMatrix3FromFloats(params, 0);
    const double m00 = R.col0.x, m01 = R.col1.x, m02 = R.col2.x;
    const double m10 = R.col0.y, m11 = R.col1.y, m12 = R.col2.y;
    const double m20 = R.col0.z, m21 = R.col1.z, m22 = R.col2.z;

    // Scale parameter
    const double Term_Scale = params[9];  // [PVar-0x34] (default: 0.5)

    double x = p.x, y = p.y, z = p.z;

    // Compute s = Term_Scale / |p|
    double r = sqrt(x * x + y * y + z * z);
    double s = Term_Scale / r;

    // Scaled point
    double sx = x * s;
    double sy = y * s;
    double sz = z * s;

    // Apply rotation matrix rows
    double row0 = sx * m00 + sy * m01 + sz * m02;
    double row1 = sx * m10 + sy * m11 + sz * m12;
    double row2 = sx * m20 + sy * m21 + sz * m22;

    // Add rotated scaled point to original: p + R*(s*p)
    return cpu_double3(x + row0, y + row1, z + row2);
}

// Formula metadata
constant FormulaInfo formula_info__asdamTrans = {
    .id = FORMULA__ASDAMTRANS,
    .name = "_asdamTrans",
    .param_count = 4,
    .complexity = 127,
    .uses_transcendental = true
};

/*
Original M3 Description:
Radial-dependent rotation transform. Computes a scale factor inversely
proportional to the distance from the origin, applies a rotation matrix
to the scaled point, and adds the result to the original point.

  s = Term_Scale / |p|
  result = p + M * (s * p)

where M is a 3x3 rotation matrix (from 3 Euler angles). The output uses
a row permutation (row1→x, row2→y, row0→z) inherent to the
3SingleAngles storage layout.

With default parameters (identity rotation, Term_Scale=0.5):
  result = p + 0.5 * p / |p| = p * (1 + 0.5/|p|)
which pushes points outward by a fixed distance of 0.5 along the radial
direction.

Parameters:
  [0..8] Rotation matrix (3x3, from 3SingleAngles_Term_Rotation): double
         (host computes from 3 Euler angles, default: identity)
  [9]  Term_Scale: double (default: 0.5)
*/
