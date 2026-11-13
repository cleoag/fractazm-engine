// Beth2422.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Beth2422
// Hand-ported from ASM: 446 bytes, DEoption=4
// 4D formula: uses de parameter as w coordinate input/output
// Beth (Mandalabeth) formula - Version 2 ABI: ebx=PVar, [ebp+0xc]=&w

constexpr unsigned int FORMULA_BETH2422 = 10330;

// ASM trace:
//   esi = context, ebx = PVar (from [esi+0x30])
//   [ebp+0xc] = pointer to w
//   Parameters: [PVar-0x10] = W add (double, default: 0) — the only user parameter
//   Constants from [CONSTANTS] section (positive PVar offsets, fixed):
//     [ebx+0x00] = 2.5
//     [ebx+0x08] = 3
//     [ebx+0x10] = -10
//     [ebx+0x18] = 7
//     [ebx+0x20] = -6
//
//   KEY DIFFERENCE from Beth1241:
//   Beth1241 at 0x0086: DE E9 = FSUBP st(1) — SUBTRACTS c4*crossterms
//   Beth2422 at 0x0086: DE C1 = FADDP st(1) — ADDS c4*crossterms
//   This difference occurs at all 4 component positions (0x86, 0xD8, 0x12C, 0x18E).
//
//   So the polynomial for Beth2422 is:
//     x' = 2.5*x * [7*(B4+C4+D4) + (-6)*(b2*c2+c2*d2+d2*b2) + (-10)*a2*(b2+c2+d2) + 3*A4] + Cx
//        = 2.5*x * [7*(B4+C4+D4) - 6*(b2*c2+c2*d2+d2*b2) - 10*a2*(b2+c2+d2) + 3*A4] + Cx
//
//   Note: Constants are from [CONSTANTS] section and stored at positive PVar
//   offsets in VarBuffer. They are NOT in the dOptionValue[] array.

template<>
cpu_double3 execute_formula<FORMULA_BETH2422>(cpu_double3 p, constant double* params, double &de) {
    // User parameter (the only one)
    const double w_add = params[0];   // [PVar-0x10] default: 0

    // Fixed constants from [CONSTANTS] section of .m3f file
    // These are stored at positive PVar offsets in VarBuffer, NOT in dOptionValue[].
    const double c0 = 2.5;    // [ebx+0x00]
    const double c1 = 3.0;    // [ebx+0x08]
    const double c2 = -10.0;  // [ebx+0x10]
    const double c3 = 7.0;    // [ebx+0x18]
    const double c4 = -6.0;   // [ebx+0x20]

    double x = p.x, y = p.y, z = p.z, w = de;

    // Squares
    double a2 = x * x;
    double b2 = y * y;
    double c2_ = z * z;
    double d2 = w * w;

    // 4th powers
    double A4 = a2 * a2;
    double B4 = b2 * b2;
    double C4 = c2_ * c2_;
    double D4 = d2 * d2;

    // ASM at 0x0086: DE C1 = FADDP st(1) → st(1) = st(1) + st(0)
    // So: c3*(4th powers) + c4*(cross terms) [ADDITION, not subtraction like Beth1241]
    double nx = c0 * x * (c3 * (B4 + C4 + D4)
                        + c4 * (b2*c2_ + c2_*d2 + d2*b2)
                        + c2 * a2 * (b2 + c2_ + d2)
                        + c1 * A4);

    double ny = c0 * y * (c3 * (C4 + D4 + A4)
                        + c4 * (c2_*d2 + d2*a2 + a2*c2_)
                        + c2 * b2 * (c2_ + d2 + a2)
                        + c1 * B4);

    double nz = c0 * z * (c3 * (D4 + A4 + B4)
                        + c4 * (d2*a2 + a2*b2 + b2*d2)
                        + c2 * c2_ * (d2 + a2 + b2)
                        + c1 * C4);

    double nw = c0 * w * (c3 * (A4 + B4 + C4)
                        + c4 * (a2*b2 + b2*c2_ + c2_*a2)
                        + c2 * d2 * (a2 + b2 + c2_)
                        + c1 * D4) + w_add;

    de = nw;
    return cpu_double3(nx, ny, nz);
}

// Formula metadata
constant FormulaInfo formula_info_Beth2422 = {
    .id = FORMULA_BETH2422,
    .name = "Beth2422",
    .param_count = 1,
    .complexity = 446,
    .uses_transcendental = false
};

/*
Original M3 Description:
This is one of the plenty 'Mandalabeth' formulas made by Bethchen at fractalforums.com.

Similar to Beth1241 but with different constants AND different cross-term sign.
Beth1241 uses FSUBP (subtraction) for cross-terms, Beth2422 uses FADDP (addition).

Parameters:
  [0] W add: double (default: 0) — the only user-adjustable parameter

Fixed constants (from [CONSTANTS] section, hardcoded):
  c0 = 2.5, c1 = 3, c2 = -10, c3 = 7, c4 = -6

Note: Version=2 ABI means w is passed via [ebp+0xc] stack argument
and ebx=PVar. In the C++ port, we use the de parameter for w.
The constants are at positive PVar offsets in VarBuffer — they are NOT
stored in the dOptionValue[] array and must be hardcoded.
*/
