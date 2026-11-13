// _RotateBack.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _RotateBack
// Hand-ported from ASM: 84 bytes, DEoption=-1

constexpr unsigned int FORMULA__ROTATEBACK = 7061;

// ASM trace:
//   Inverse rotation: uses the TRANSPOSE of the forward rotation matrix.
//   Same .3SingleAngles parameter type as _Rotate.
//
//   _RotateBack applies R^T (inverse for orthogonal rotation):
//     x' = dot(R.col0, p)
//     y' = dot(R.col1, p)
//     z' = dot(R.col2, p)

template<>
cpu_double3 execute_formula<FORMULA__ROTATEBACK>(cpu_double3 p, constant double* params, double &de) {
    // params[0..2] = raw Euler angles in degrees (from .3SingleAngles)
    RotMatrix3 R = rotMatrix3FromFloats(params, 0);
    // Apply transpose (inverse for orthogonal rotation)
    return cpu_double3(dot(R.col0, p), dot(R.col1, p), dot(R.col2, p));
}

// Formula metadata
constant FormulaInfo formula_info__RotateBack = {
    .id = FORMULA__ROTATEBACK,
    .name = "_RotateBack",
    .param_count = 3,
    .complexity = 84,
    .uses_transcendental = false
};

/*
Original M3 Description:
Inverse 3D rotation (undo _Rotate). Uses the transpose of the
rotation matrix, which equals the inverse for orthogonal matrices.

x' = x*m[0] + y*m[3] + z*m[6]
y' = x*m[1] + y*m[4] + z*m[7]
z' = x*m[2] + y*m[5] + z*m[8]

Parameters (.3SingleAngles type):
  [0] Angle X (degrees)
  [1] Angle Y (degrees)
  [2] Angle Z (degrees)
  buildRotMatrix3() computes the 3x3 rotation matrix from raw angles.
*/
