// _RotatedAbs.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _RotatedAbs
// Hand-ported from ASM: 157 bytes, DEoption=-1
// Generalized abs transform: rotate, |x+add|-sub, inverse rotate

constexpr unsigned int FORMULA__ROTATEDABS = 5238;

// ASM trace:
//   FillCustomVBufWithVars type 6 (.3SingleAngles) stores M in row-major order:
//     [PVar-0x0c..14] = M row 0, [PVar-0x18..20] = M row 1, [PVar-0x24..2c] = M row 2
//   ASM forward rotation:
//     r0_asm = M[2] * p  (uses [edi-0x24/28/2c])
//     r1_asm = M[0] * p  (uses [edi-0x0c/10/14]) ← THIS IS THE FOLD COMPONENT
//     r2_asm = M[1] * p  (uses [edi-0x18/1c/20])
//   In Metal code: r0 = M row 0 * p = ASM's r1_asm (the fold component)
//   Abs-fold on r0: r0' = |r0 + AddX| - SubX
//   Inverse rotation: result = M^T * (r0', r1, r2)
//     new_x = r0'*m00 + r1*m10 + r2*m20
//     new_y = r0'*m01 + r1*m11 + r2*m21
//     new_z = r0'*m02 + r1*m12 + r2*m22
//
// "X" is purely conventional — any direction can be chosen via angles.
// With defaults (AddX=0, SubX=0): folds the row-0 direction (X with identity).
//
// C handling: DEoption=-1 (transform), no C addition.

template<>
cpu_double3 execute_formula<FORMULA__ROTATEDABS>(cpu_double3 p, constant double* params, double &de) {
    // Build rotation matrix from 3 Euler angles (raw .3SingleAngles input)
    RotMatrix3 R = rotMatrix3FromFloats(params, 0);
    const double m00 = R.col0.x, m01 = R.col1.x, m02 = R.col2.x;
    const double m10 = R.col0.y, m11 = R.col1.y, m12 = R.col2.y;
    const double m20 = R.col0.z, m21 = R.col1.z, m22 = R.col2.z;

    const double AddX = params[9];   // [PVar-0x34] (default: 0)
    const double SubX = params[10];   // [PVar-0x3c] (default: 0)

    double x = p.x, y = p.y, z = p.z;

    // Forward rotation: M * p
    double r0 = x * m00 + y * m01 + z * m02;
    double r1 = x * m10 + y * m11 + z * m12;
    double r2 = x * m20 + y * m21 + z * m22;

    // Abs-fold on the x-direction component (r0 = M row-0 * p, not r1)
    r0 = fabs(r0 + AddX) - SubX;

    // Inverse rotation: M^T * (r0, r1, r2)
    double new_x = r0 * m00 + r1 * m10 + r2 * m20;
    double new_y = r0 * m01 + r1 * m11 + r2 * m21;
    double new_z = r0 * m02 + r1 * m12 + r2 * m22;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__RotatedAbs = {
    .id = FORMULA__ROTATEDABS,
    .name = "_RotatedAbs",
    .param_count = 5,
    .complexity = 157,
    .uses_transcendental = false
};

/*
Original M3 Description:
A combination of:
  - Rotate by the given angles
  - Add a value to X, AbsX, Subtract a value from X
  - RotateBack by the given angles

It's a generalization of the Abs formula.
"X" is purely conventional — any direction can be used changing the angles!

Parameters:
  [0..8] Rotation matrix (3x3, from 3SingleAngles): double
         (host computes from 3 Euler angles, default: identity)
  [9]  AddX: double (default: 0) - offset before abs
  [10] SubX: double (default: 0) - subtracted after abs
*/
