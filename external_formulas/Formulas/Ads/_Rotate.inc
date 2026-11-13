// _Rotate.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _Rotate
// Hand-ported from ASM: 84 bytes, DEoption=-1

constexpr unsigned int FORMULA__ROTATE = 7060;

// ASM trace:
//   Uses .3SingleAngles parameter: 3 user-facing angle values,
//   engine pre-computes a 3x3 rotation matrix stored as 9 single-precision floats.
//
//   Matrix layout (dword ptr, single precision!):
//     m[0]=[PVar-0x0c], m[1]=[PVar-0x10], m[2]=[PVar-0x14]
//     m[3]=[PVar-0x18], m[4]=[PVar-0x1c], m[5]=[PVar-0x20]
//     m[6]=[PVar-0x24], m[7]=[PVar-0x28], m[8]=[PVar-0x2c]
//
//   x' = x*m[0] + y*m[1] + z*m[2]
//   y' = x*m[3] + y*m[4] + z*m[5]
//   z' = x*m[6] + y*m[7] + z*m[8]

template<>
cpu_double3 execute_formula<FORMULA__ROTATE>(cpu_double3 p, double* params, double &de) {
    // params[0..8] = pre-computed rotation matrix (9 floats from .3SingleAngles)
    // Pascal FillCustomVBufWithVars calls BuildRotMatrix and stores the matrix in PVar
    RotMatrix3 R = rotMatrix3FromFloats(params, 0);
    return R * p;
}

// Formula metadata
constant FormulaInfo formula_info__Rotate = {
    .id = FORMULA__ROTATE,
    .name = "_Rotate",
    .param_count = 3,
    .complexity = 84,
    .uses_transcendental = false
};

/*
Original M3 Description:
3D rotation transform. User specifies 3 Euler angles;
engine pre-computes a 3x3 rotation matrix (9 single-precision floats).

x' = x*m[0] + y*m[1] + z*m[2]
y' = x*m[3] + y*m[4] + z*m[5]
z' = x*m[6] + y*m[7] + z*m[8]

Parameters (.3SingleAngles type):
  User sees: 3 angle values (X, Y, Z rotation in degrees)
  Engine provides: 9 double matrix elements as params[0..8]
*/
