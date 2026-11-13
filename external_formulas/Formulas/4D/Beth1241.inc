// Beth1241.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Beth1241
// Hand-ported from ASM: 446 bytes, DEoption=4
// 4D formula: uses de parameter as w coordinate input/output
// Beth (Mandalabeth) formula - Version 2 ABI: ebx=PVar, [ebp+0xc]=&w

constexpr unsigned int FORMULA_BETH1241 = 10329;

// ASM trace:
//   esi = context, ebx = PVar (from [esi+0x30])
//   eax = &x, edx = &y, ecx = &z
//   [ebp+0xc] = second stack argument = pointer to w (Version=2 ABI)
//   Parameters: [PVar-0x10] = W add (double, default: 0) — the only user parameter
//   Constants from [CONSTANTS] section (positive PVar offsets, fixed):
//     [ebx+0x00] = -0.5
//     [ebx+0x08] = 3
//     [ebx+0x10] = 10
//     [ebx+0x18] = 15
//     [ebx+0x20] = 30
//   C values: [esi+0x18]=Cx, [esi+0x20]=Cy, [esi+0x28]=Cz (engine adds these)
//   Cw = [esi-0x38] (formula adds this internally in ASM; in 3D rendering Cw=0)
//
//   The formula computes a 4D Mandalabeth (generalized Mandelbrot with
//   polynomial terms). Uses squares and 4th powers of each coordinate.
//
//   Let a2=x^2, b2=y^2, c2=z^2, d2=w^2 (squares)
//   Let A4=x^4, B4=y^4, C4=z^4, D4=w^4 (4th powers)
//
//   Each output component (from ASM with FSUBP at 0x0086):
//     x' = -0.5*x * [15*(B4+C4+D4) - 30*(b2*c2+c2*d2+d2*b2) + 10*a2*(b2+c2+d2) + 3*A4] + Cx
//     y' = -0.5*y * [15*(C4+D4+A4) - 30*(c2*d2+d2*a2+a2*c2) + 10*b2*(c2+d2+a2) + 3*B4] + Cy
//     z' = -0.5*z * [15*(D4+A4+B4) - 30*(d2*a2+a2*b2+b2*d2) + 10*c2*(d2+a2+b2) + 3*C4] + Cz
//     w' = -0.5*w * [15*(A4+B4+C4) - 30*(a2*b2+b2*c2+c2*a2) + 10*d2*(a2+b2+c2) + 3*D4] + Cw + w_add
//
//   Note: Constants are from [CONSTANTS] section of .m3f file and stored at
//   positive PVar offsets. They are NOT user parameters and are NOT in the
//   dOptionValue[] array. They must be hardcoded here.

template<>
cpu_double3 execute_formula<FORMULA_BETH1241>(cpu_double3 p, constant double* params, double &de) {
    // User parameter (the only one)
    const double w_add = params[0];   // [PVar-0x10] default: 0

    // Fixed constants from [CONSTANTS] section of .m3f file
    // These are stored at positive PVar offsets in VarBuffer, NOT in dOptionValue[].
    const double c0 = -0.5;   // [ebx+0x00]
    const double c1 = 3.0;    // [ebx+0x08]
    const double c2 = 10.0;   // [ebx+0x10]
    const double c3 = 15.0;   // [ebx+0x18]
    const double c4 = 30.0;   // [ebx+0x20]

    double x = p.x, y = p.y, z = p.z, w = de;

    // Squares
    double a2 = x * x;  // x^2
    double b2 = y * y;  // y^2
    double c2_ = z * z; // z^2 (renamed to avoid conflict with c2 constant)
    double d2 = w * w;  // w^2

    // 4th powers
    double A4 = a2 * a2;  // x^4
    double B4 = b2 * b2;  // y^4
    double C4 = c2_ * c2_;// z^4
    double D4 = d2 * d2;  // w^4

    // ASM at 0x0086: DE E9 = FSUBP st(1),st(0) → st(1) = st(1) - st(0)
    // So: c3*(4th powers) - c4*(cross terms)
    double nx = c0 * x * (c3 * (B4 + C4 + D4)
                        - c4 * (b2*c2_ + c2_*d2 + d2*b2)
                        + c2 * a2 * (b2 + c2_ + d2)
                        + c1 * A4);

    double ny = c0 * y * (c3 * (C4 + D4 + A4)
                        - c4 * (c2_*d2 + d2*a2 + a2*c2_)
                        + c2 * b2 * (c2_ + d2 + a2)
                        + c1 * B4);

    double nz = c0 * z * (c3 * (D4 + A4 + B4)
                        - c4 * (d2*a2 + a2*b2 + b2*d2)
                        + c2 * c2_ * (d2 + a2 + b2)
                        + c1 * C4);

    double nw = c0 * w * (c3 * (A4 + B4 + C4)
                        - c4 * (a2*b2 + b2*c2_ + c2_*a2)
                        + c2 * d2 * (a2 + b2 + c2_)
                        + c1 * D4) + w_add;

    de = nw;
    return cpu_double3(nx, ny, nz);
}

// Formula metadata
constant FormulaInfo formula_info_Beth1241 = {
    .id = FORMULA_BETH1241,
    .name = "Beth1241",
    .param_count = 1,
    .complexity = 446,
    .uses_transcendental = false
};

/*
Original M3 Description:
This is one of the plenty 'Mandalabeth' formulas made by Bethchen at fractalforums.com.

The formula is a power-4 generalized Mandelbrot in 4D using polynomial expansion.
Each component is computed as:
  coord' = c0 * coord * polynomial(other_coords^2, other_coords^4)
  (engine adds Cx/Cy/Cz; Cw is 0 in 3D rendering)

Parameters:
  [0] W add: double (default: 0) — the only user-adjustable parameter

Fixed constants (from [CONSTANTS] section, hardcoded):
  c0 = -0.5, c1 = 3, c2 = 10, c3 = 15, c4 = 30

Note: Version=2 ABI means w is passed via [ebp+0xc] stack argument
and ebx=PVar. In the C++ port, we use the de parameter for w.
The constants are at positive PVar offsets in VarBuffer — they are NOT
stored in the dOptionValue[] array and must be hardcoded.
*/
