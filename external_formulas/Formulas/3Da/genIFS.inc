// genIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: genIFS
// Hand-ported from ASM: 143 bytes, DEoption=2

constexpr unsigned int FORMULA_GENIFS = 429;

// ASM trace:
//   Per-axis: conditional abs + offset
//   z' = (abs_z ? abs(z) : z) + offset_z
//   y' = (abs_y ? abs(y) : y) + offset_y
//   x' = (abs_x ? abs(x) : x) + offset_x
//   3x3 rotation matrix multiply:
//   rx = x'*M[0] + y'*M[1] + z'*M[2]
//   ry = x'*M[3] + y'*M[4] + z'*M[5]
//   rz = x'*M[6] + y'*M[7] + z'*M[8]
//   Scale all by Scale, add Julia constants:
//   x_out = rx * Scale + Cx
//   y_out = ry * Scale + Cy
//   z_out = rz * Scale + Cz
//
// Generic IFS with abs folding, rotation, and uniform scaling.

template<>
cpu_double3 execute_formula<FORMULA_GENIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    const int abs_x = (int)params[1];
    const int abs_y = (int)params[2];
    const int abs_z = (int)params[3];
    const double offset_x = params[4];
    const double offset_y = params[5];
    const double offset_z = params[6];
    // Rotation matrix from 9 pre-computed float elements (from .3SingleAngles)
    RotMatrix3 R = rotMatrix3FromFloats(params, 7);
    // Julia constants

    // Conditional abs + offset
    double x = (abs_x != 0) ? abs(p.x) : p.x;
    double y = (abs_y != 0) ? abs(p.y) : p.y;
    double z = (abs_z != 0) ? abs(p.z) : p.z;
    x += offset_x;
    y += offset_y;
    z += offset_z;

    // 3x3 rotation matrix multiply
    cpu_double3 rv = R * cpu_double3(x, y, z);
    double rx = rv.x;
    double ry = rv.y;
    double rz = rv.z;

    // Scale and add Julia constants
    cpu_double3 result;
    result.x = rx * Scale;
    result.y = ry * Scale;
    result.z = rz * Scale;

    // DE modification for DEoption=2
    // The original FMUL stores the signed scale in the W/derivative lane.
    de *= Scale;

    return result;
}

// Alternative direct function implementation
cpu_double3 genIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_GENIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_genIFS = {
    .id = FORMULA_GENIFS,
    .name = "genIFS",
    .param_count = 10,
    .complexity = 143,
    .uses_transcendental = true
};

/*
Original M3 Description:
Generic IFS with per-axis abs folding, 3x3 rotation matrix, uniform scale,
and Julia constant addition.

Mathematical Formula:
x' = (abs_x ? |x| : x) + offset_x
y' = (abs_y ? |y| : y) + offset_y
z' = (abs_z ? |z| : z) + offset_z
[rx, ry, rz] = R * [x', y', z']  (3x3 rotation)
x_out = rx * Scale
y_out = ry * Scale
z_out = rz * Scale

Parameters:
  [0] Scale: double (default: -1.6)
  [1] Abs_X: int (default: 1)
  [2] Abs_Y: int (default: 1)
  [3] Abs_Z: int (default: 0)
  [4] Offset_X: double (default: -1)
  [5] Offset_Y: double (default: -1)
  [6] Offset_Z: double (default: 0)
  [7..15] Rotation matrix (9 floats, pre-computed from 3 angles)
  [16..18] Cx, Cy, Cz: Julia constants (from iteration context)
*/
