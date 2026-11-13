// _Transform.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _Transform
// Hand-ported from ASM: 154 bytes, DEoption=-1

constexpr unsigned int FORMULA__TRANSFORM = 8896;
template<> struct ExternalFormulaWritesWState<FORMULA__TRANSFORM> { static constexpr bool value = true; };
template<> struct ExternalFormulaUsesXYZState<FORMULA__TRANSFORM> { static constexpr bool value = true; };
template<> struct ExternalFormulaAppendsJuliaMode<FORMULA__TRANSFORM> { static constexpr bool value = true; };
template<> struct ExternalFormulaAlwaysWritesJuliaXYZ<FORMULA__TRANSFORM> { static constexpr bool value = true; };

// ASM trace:
//   Shift: x += X_add, y += Y_add, z += Z_add
//   3x3 rotation matrix multiply (9 doubles from .3DoubleAngles, pre-computed):
//     rx = x*M[0] + y*M[1] + z*M[2]
//     ry = x*M[3] + y*M[4] + z*M[5]
//     rz = x*M[6] + y*M[7] + z*M[8]
//   Scale: rx *= Scale, ry *= Scale, rz *= Scale
//   If Copy_to_C != 0: Cx=rx, Cy=ry, Cz=rz (copy to Julia constants)
//   Output: (rx*Scale, ry*Scale, rz*Scale)
//
// General transform: shift, rotate, scale (and optionally copy to C).

template<>
cpu_double3 execute_formula<FORMULA__TRANSFORM>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    const double X_add = params[1];
    const double Y_add = params[2];
    const double Z_add = params[3];
    // Rotation matrix from 9 pre-computed double matrix elements (from .3DoubleAngles)
    RotMatrix3 R = rotMatrix3FromFloats(params, 4);
    // params[13] = Copy_to_C (integer flag)

    // Shift
    double x = p.x + X_add;
    double y = p.y + Y_add;
    double z = p.z + Z_add;

    // 3x3 rotation matrix multiply
    cpu_double3 rv = R * cpu_double3(x, y, z);
    double rx = rv.x;
    double ry = rv.y;
    double rz = rv.z;

    // Scale
    cpu_double3 result;
    result.x = rx * Scale;
    result.y = ry * Scale;
    result.z = rz * Scale;

    params[FORMULA_STATE_W_SLOT] *= Scale;
    params[FORMULA_STATE_X_SLOT] *= Scale; // Deriv1, with signed Scale.
    params[WRITE_J1_SLOT] = params[14];
    params[WRITE_J2_SLOT] = params[15];
    params[WRITE_J3_SLOT] = params[16];
    if (params[13] != 0.0 && params[20] == 0.0) {
        params[WRITE_J1_SLOT] = result.x;
        params[WRITE_J2_SLOT] = result.y;
        params[WRITE_J3_SLOT] = result.z;
    }

    return result;
}

// Formula metadata
constant FormulaInfo formula_info__Transform = {
    .id = FORMULA__TRANSFORM,
    .name = "_Transform",
    .param_count = 14,
    .complexity = 154,
    .uses_transcendental = false
};

/*
Original M3 Description:
A general transform to shift, scale and rotate Z and C (by option).
Works with all 3D + 4D bulb and box formulas also in julia mode.

Mathematical Formula:
x = x + X_add, y = y + Y_add, z = z + Z_add
[rx,ry,rz] = RotationMatrix * [x,y,z]
x' = rx * Scale, y' = ry * Scale, z' = rz * Scale
If Copy_to_C and not DoJulia: Cx=x', Cy=y', Cz=z'. W and Deriv1 also scale.

Parameters:
  [0] Scale: double (default: 1)
  [1] X_add: double (default: 0)
  [2] Y_add: double (default: 0)
  [3] Z_add: double (default: 0)
  [4..12] Rotation matrix (9 doubles, pre-computed from 3 angles)
  [13] Copy_to_C: int (default: 1)
*/
