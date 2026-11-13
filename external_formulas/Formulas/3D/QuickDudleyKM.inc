// QuickDudleyKM.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: QuickDudleyKM
// Hand-ported from ASM: 438 bytes, DEscale=2, SIpower=2

constexpr unsigned int FORMULA_QUICKDUDLEYKM = 7023;

// ASM trace:
//   DD03        fld [ebx]           → x
//   D8C8        fmul st0,st0        → x²
//   DD01        fld [ecx]           → z
//   DC0A        fmul [edx]          → z*y
//   D8C0        fadd st0,st0        → 2*z*y  (D8C0 = FADD st0,st0)
//   DEE9        fsubp st1,st0       → x² - 2zy (1st result)
//   DD01        fld [ecx]           → z
//   D8C8        fmul st0,st0        → z²
//   DD02        fld [edx]           → y
//   DC0B        fmul [ebx]          → y*x
//   D8C0        fadd st0,st0        → 2*y*x
//   DEC1        faddp st1,st0       → z² + 2yx (2nd result)
//   DD02        fld [edx]           → y
//   D8C8        fmul st0,st0        → y²
//   DD01        fld [ecx]           → z
//   DC0B        fmul [ebx]          → z*x
//   D8C0        fadd st0,st0        → 2*z*x
//   DEE9        fsubp st1,st0       → y² - 2zx (3rd result)
//   DC4628/20/18 + Cx/Cy/Cz
//
// QuickDudley formula (Kerry Mitchell variant):
// x' = x² - 2zy + Cx
// y' = z² + 2yx + Cy
// z' = y² - 2xz + Cz

template<>
cpu_double3 execute_formula<FORMULA_QUICKDUDLEYKM>(cpu_double3 p, constant double* params, double &de) {
    double x = p.x, y = p.y, z = p.z;
    // Julia constants handled by engine (added to result)
    double new_x = x*x - 2.0*z*y;
    double new_y = z*z + 2.0*y*x;
    double new_z = y*y - 2.0*x*z;
    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_QuickDudleyKM = {
    .id = FORMULA_QUICKDUDLEYKM,
    .name = "QuickDudleyKM",
    .param_count = 0,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
From a formula by QuickDudley, converted by Kerry Mitchell.
x' = x² - 2zy + Cx
y' = z² + 2yx + Cy
z' = y² - 2xz + Cz

By Luca GN 2015. DEscale=2, SIpower=2.
Parameters: none (uses Julia constants from engine)
*/
