// Makin3D-1.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Makin3D-1
// Hand-ported from ASM: 390 bytes, DEscale=1, SIpower=2

constexpr unsigned int FORMULA_MAKIN3D_1 = 7035;

// ASM trace:
//   DD03        fld [ebx]           → x
//   D8C8        fmul st0,st0        → x²
//   DD02/D8C8/DEE9  y²; x²-y²
//   DD01/D8C8/DEE9  z²; x²-y²-z²
//   DD03/DC0A/D8C0  x; x*y; 2xy  (D8C0 = FADD st0,st0)
//   DD01/D8C0       z; 2z         (D8C0 = FADD st0,st0)
//   DD03/DC22       x; x-y        (DC22 = FSUB [edx])
//   DEC9            2z*(x-y)
//   DC4628/DD19     z' = 2z(x-y) + Cz
//   DC4620/DD1A     y' = 2xy + Cy
//   DC4618/DD1B     x' = x²-y²-z² + Cx
//
// From WIP3D5 by David Makin (FractalForums), ported by Luca GN 2011.
// NOTE: D8C0 is FADD st0,st0 (doubles value), not FMUL.

template<>
cpu_double3 execute_formula<FORMULA_MAKIN3D_1>(cpu_double3 p, constant double* params, double &de) {
    double x = p.x, y = p.y, z = p.z;

    double new_x = x*x - y*y - z*z;
    double new_y = 2.0 * x * y;
    double new_z = 2.0 * z * (x - y);

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_Makin3D_1 = {
    .id = FORMULA_MAKIN3D_1,
    .name = "Makin3D-1",
    .param_count = 0,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
From WIP3D5 by David Makin:
x' = x² - y² - z² + Cx
y' = 2xy + Cy
z' = 2z(x - y) + Cz

DEscale=1, SIpower=2.
Parameters: none (uses Julia constants from engine)
*/
