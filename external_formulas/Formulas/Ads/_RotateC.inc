// _RotateC.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _RotateC
// Hand-ported from ASM: 92 bytes, DEoption=-1

constexpr unsigned int FORMULA__ROTATEC = 7062;

// ASM trace:
//   Same rotation matrix as _Rotate, but applied to C (Cx, Cy, Cz)
//   instead of x, y, z. Position is unchanged.
//
//   Cx' = Cx*m[0] + Cy*m[1] + Cz*m[2]
//   Cy' = Cx*m[3] + Cy*m[4] + Cz*m[5]
//   Cz' = Cx*m[6] + Cy*m[7] + Cz*m[8]
//
//   Stores results back to [esi+0x18], [esi+0x20], [esi+0x28] (Cx, Cy, Cz).

template<>
cpu_double3 execute_formula<FORMULA__ROTATEC>(cpu_double3 p, constant double* params, double &de) {
    // Rotation matrix from 3SingleAngles
    RotMatrix3 R = rotMatrix3FromFloats(params, 0);
    // C values appended after 9 matrix entries
    const double Cx = params[9];
    const double Cy = params[10];
    const double Cz = params[11];

    // Rotate C: Cx' = Cx*m[0] + Cy*m[1] + Cz*m[2], etc.
    params[WRITE_J1_SLOT] = Cx * R.m[0][0] + Cy * R.m[0][1] + Cz * R.m[0][2];
    params[WRITE_J2_SLOT] = Cx * R.m[1][0] + Cy * R.m[1][1] + Cz * R.m[1][2];
    params[WRITE_J3_SLOT] = Cx * R.m[2][0] + Cy * R.m[2][1] + Cz * R.m[2][2];

    return p;  // position unchanged
}

// Formula metadata
constant FormulaInfo formula_info__RotateC = {
    .id = FORMULA__ROTATEC,
    .name = "_RotateC",
    .param_count = 9,
    .complexity = 92,
    .uses_transcendental = false
};

/*
Original M3 Description:
Rotates the C (Julia constant) values using a 3x3 rotation matrix.
Position is unchanged; only Cx/Cy/Cz are rotated.

Cx' = Cx*m[0] + Cy*m[1] + Cz*m[2]
Cy' = Cx*m[3] + Cy*m[4] + Cz*m[5]
Cz' = Cx*m[6] + Cy*m[7] + Cz*m[8]

NOTE: This is a side-effect transform that modifies C in the engine context.
The Metal engine must apply the rotation to C values.

Parameters (.3SingleAngles type):
  User sees: 3 angle values (X, Y, Z rotation in degrees)
  Engine provides: 9 double matrix elements as params[0..8]
*/
