// _RotatedFolding.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _RotatedFolding
// Hand-ported from ASM: 173 bytes, DEoption=-1
// Generalized Mandelbox fold: rotate, box-fold, inverse rotate

constexpr unsigned int FORMULA__ROTATEDFOLDING = 8481;

// ASM trace: same matrix layout as _RotatedAbs (type 6 .3SingleAngles)
//   [PVar-0x0c..14] = M row 0 (fold direction), [PVar-0x18..20] = M row 1, [PVar-0x24..2c] = M row 2
//   In Metal: r0 = M row-0 * p = the fold component (not r1!)
//   Mandelbox box fold on r0:
//     r0' = |r0 + fold| - |r0 - fold| - r0
//     This is equivalent to:
//       if r0 > fold:  r0' = 2*fold - r0   (fold back from above)
//       if r0 < -fold: r0' = -2*fold - r0   (fold back from below)
//       otherwise:     r0' = r0              (unchanged)
//   Inverse rotation: result = M^T * (r0', r1, r2)
//
// The fold direction is determined by the rotation angles.
// With identity rotation, this folds along the X axis (row 0).
//
// C handling: DEoption=-1 (transform), no C addition.

template<>
cpu_double3 execute_formula<FORMULA__ROTATEDFOLDING>(cpu_double3 p, constant double* params, double &de) {
    // Build rotation matrix from 3 Euler angles (raw .3SingleAngles input)
    RotMatrix3 R = rotMatrix3FromFloats(params, 0);
    const double m00 = R.col0.x, m01 = R.col1.x, m02 = R.col2.x;
    const double m10 = R.col0.y, m11 = R.col1.y, m12 = R.col2.y;
    const double m20 = R.col0.z, m21 = R.col1.z, m22 = R.col2.z;

    const double fold = params[9];  // [PVar-0x34] Fold_of_x (default: 1.0)

    double x = p.x, y = p.y, z = p.z;

    // Forward rotation: M * p
    double r0 = x * m00 + y * m01 + z * m02;
    double r1 = x * m10 + y * m11 + z * m12;
    double r2 = x * m20 + y * m21 + z * m22;

    // Mandelbox box fold on r0 (x-direction = M row-0 component): |r0+fold| - |r0-fold| - r0
    // Clamps r0 to [-fold, fold] via reflection
    r0 = fabs(r0 + fold) - fabs(r0 - fold) - r0;

    // Inverse rotation: M^T * (r0, r1, r2)
    double new_x = r0 * m00 + r1 * m10 + r2 * m20;
    double new_y = r0 * m01 + r1 * m11 + r2 * m21;
    double new_z = r0 * m02 + r1 * m12 + r2 * m22;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__RotatedFolding = {
    .id = FORMULA__ROTATEDFOLDING,
    .name = "_RotatedFolding",
    .param_count = 4,
    .complexity = 173,
    .uses_transcendental = false
};

/*
Original M3 Description:
Generalized Mandelbox box fold with rotatable fold axis.

Rotates the point, applies a 1D Mandelbox-style fold (reflecting values
outside [-fold, fold] back inside), then rotates back. The fold direction
is determined by the rotation angles.

The fold operation: r1' = |r1+fold| - |r1-fold| - r1
is equivalent to:
  if r1 > fold:   r1' = 2*fold - r1
  if r1 < -fold:  r1' = -2*fold - r1
  otherwise:       r1' = r1

Parameters:
  [0..8] Rotation matrix (3x3, from 3SingleAngles_Fold_rotation): double
         (host computes from 3 Euler angles, default: identity)
  [9]  Fold_of_x: double (default: 1.0) - fold limit
*/
