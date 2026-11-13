// QuickDudley.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: QuickDudley
// Hand-ported from ASM: 443 bytes, DEscale=2, SIpower=2

constexpr unsigned int FORMULA_QUICKDUDLEY = 7024;

// ASM trace: identical to QuickDudleyKM except z' uses FADDP (DEC1)
// instead of FSUBP (DEE9) for the 2xz term.
//
// QuickDudley original formula:
// x' = x² - 2zy + Cx
// y' = z² + 2yx + Cy
// z' = y² + 2xz + Cz    (note: + instead of -)

template<>
cpu_double3 execute_formula<FORMULA_QUICKDUDLEY>(cpu_double3 p, constant double* params, double &de) {
    double x = p.x, y = p.y, z = p.z;
    double new_x = x*x - 2.0*z*y;
    double new_y = z*z + 2.0*y*x;
    double new_z = y*y + 2.0*x*z;
    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_QuickDudley = {
    .id = FORMULA_QUICKDUDLEY,
    .name = "QuickDudley",
    .param_count = 0,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
From a formula by QuickDudley (original multiplication table).
Has nice minibrots.
x' = x² - 2zy + Cx
y' = z² + 2yx + Cy
z' = y² + 2xz + Cz

By Luca GN 2015. DEscale=2, SIpower=2.
Parameters: none (uses Julia constants from engine)
*/
