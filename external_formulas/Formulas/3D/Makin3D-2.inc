// Makin3D-2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Makin3D-2
// Hand-ported from ASM: 390 bytes, DEscale=2, SIpower=2

constexpr unsigned int FORMULA_MAKIN3D_2 = 7036;

// ASM trace:
//   DD03        fld [ebx]           → x
//   D8C8        fmul st0,st0        → x²
//   DD01/DC0A   fld z; fmul [edx]   → z*y
//   D8C0        fadd st0,st0        → 2zy  (D8C0 = FADD st0,st0)
//   DEC1        faddp               → x² + 2zy
//   D9D0        fnop
//   DD02/D8C8   fld y; fmul st0,st0 → y²
//   DD01/DC0B   fld z; fmul [ebx]   → z*x
//   D8C0/DEC1   2zx; y² + 2zx
//   D9E0/D9D0   fchs → -(y² + 2zx)
//   DD01/D8C8   fld z; fmul st0,st0 → z²
//   D9E0        fchs                → -z²
//   DD02/DC0B   fld y; fmul [ebx]   → y*x
//   D8C0/DEC1   2xy; -z² + 2xy
//   D9D0        fnop
//   DC4628/DD19 z' = -z² + 2xy + Cz
//   DC4620/DD1A y' = -(y² + 2zx) + Cy
//   DC4618/DD1B x' = x² + 2zy + Cx
//
// From WIP3D5 by David Makin (FractalForums), ported by Luca GN 2011.

template<>
cpu_double3 execute_formula<FORMULA_MAKIN3D_2>(cpu_double3 p, constant double* params, double &de) {
    double x = p.x, y = p.y, z = p.z;

    double new_x = x*x + 2.0 * z * y;
    double new_y = -(y*y + 2.0 * z * x);
    double new_z = -(z*z) + 2.0 * x * y;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_Makin3D_2 = {
    .id = FORMULA_MAKIN3D_2,
    .name = "Makin3D-2",
    .param_count = 0,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
From WIP3D5 by David Makin:
x' = x² + 2zy + Cx
y' = -y² - 2zx + Cy
z' = -z² + 2xy + Cz

DEscale=2, SIpower=2.
Parameters: none (uses Julia constants from engine)

Note: The original description incorrectly lists "+Cx" for all three.
ASM trace confirms Cx/Cy/Cz are applied to their respective axes.
*/
