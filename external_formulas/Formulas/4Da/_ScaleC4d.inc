// _ScaleC4d.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _ScaleC4d
// Hand-ported from ASM: 81 bytes, DEoption=5

constexpr unsigned int FORMULA__SCALEC4D = 9525;

// ASM trace:
//   Part 1: Scale position by params[0]
//     fld w, z, y, x, Scale
//     w *= Scale, z *= Scale, y *= Scale, x *= Scale
//     store all back
//
//   Part 2: Scale Julia constants by params[1]
//     fld Cw, Cz, Cy, Cx, C_Scale
//     Cw *= C_Scale, Cz *= C_Scale, Cy *= C_Scale, Cx *= C_Scale
//     store all back
//
// Scales both position (x,y,z,w) and Julia constants (Cx,Cy,Cz,Cw)
// by independent scale factors.

template<>
cpu_double3 execute_formula<FORMULA__SCALEC4D>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    const double C_Scale = params[1];

    // The wrapper appends Julia constants immediately after the option params
    // and, for ADDS_C=0 formulas, adds them after the formula returns. The asm
    // scales C in-place before that add, so mirror that contract here.
    params[2] *= C_Scale;
    params[3] *= C_Scale;
    params[4] *= C_Scale;
    params[5] *= C_Scale;
    params[WRITE_J1_SLOT] = params[2];
    params[WRITE_J2_SLOT] = params[3];
    params[WRITE_J3_SLOT] = params[4];
    params[WRITE_J4_SLOT] = params[5];

    // Pascal external formulas run inside the generic Mandel/Julia wrapper.
    // The runtime adds C after the formula returns for ADDS_C=0 formulas.
    // To implement x' = x * Scale with that contract, the formula must return
    // the scaled delta relative to the (possibly scaled) Julia constant.
    cpu_double3 result = p * Scale - cpu_double3(params[2], params[3], params[4]);

    // Same contract for w/de: subtract scaled Cw here; the wrapper adds it back.
    de = de * Scale - params[5];

    return result;
}

// Alternative direct function implementation
cpu_double3 _ScaleC4d_formula(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    const double C_Scale = params[1];
    params[2] *= C_Scale;
    params[3] *= C_Scale;
    params[4] *= C_Scale;
    params[5] *= C_Scale;
    params[WRITE_J1_SLOT] = params[2];
    params[WRITE_J2_SLOT] = params[3];
    params[WRITE_J3_SLOT] = params[4];
    params[WRITE_J4_SLOT] = params[5];
    de = de * Scale - params[5];
    return p * Scale - cpu_double3(params[2], params[3], params[4]);
}

// Formula metadata
constant FormulaInfo formula_info__ScaleC4d = {
    .id = FORMULA__SCALEC4D,
    .name = "_ScaleC4d",
    .param_count = 2,
    .complexity = 81,
    .uses_transcendental = false
};

/*
Original M3 Description:
Scale position (x,y,z,w) by Scale and Julia constants (Cx,Cy,Cz,Cw) by Scale_C.
DEoption=5: affects DE and Julia constants.

Mathematical Formula:
x' = x * Scale, y' = y * Scale, z' = z * Scale, w' = w * Scale
Cx' = Cx * Scale_C, Cy' = Cy * Scale_C, Cz' = Cz * Scale_C, Cw' = Cw * Scale_C

Parameters:
  [0] Scale: double (default: 1)
  [1] Scale_C: double (default: 1)
*/
