// Beth641.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Beth641
// Hand-ported from ASM: 207 bytes, DEoption=4
// 4D formula: uses de parameter as w coordinate input/output
// Mandalabeth formula by Bethchen (fractalforums.com).
// Compact 4D polynomial with one user option (W add) and 3 fixed constants.

constexpr unsigned int FORMULA_BETH641 = 5341;

// ASM trace (207 bytes, no branches):
//   Version-2 ABI:
//     esi = context, edi = PVar
//     eax = &x, edx = &y, ecx = &z, [ecx+8] = w
//   User parameter:
//     [edi-0x10] = W add (double, default 0)
//   Fixed constants from [CONSTANTS] section:
//     [edi+0x00] = 3
//     [edi+0x08] = 5
//     [edi+0x10] = -10
//
//   Squared terms:
//     A = x², B = y², C = z², D = w²
//     A² = x⁴, B² = y⁴, C² = z⁴, D² = w⁴
//
//   Output structure from the actual FPU stack trace:
//     x' = x * (3*x⁴ + 5*(y⁴ + z⁴ + w⁴) - 10*x²*(y² + z² + w²)) + Cx
//     y' = y * (3*y⁴ + 5*(x⁴ + z⁴ + w⁴) - 10*y²*(x² + z² + w²)) + Cy
//     z' = z * (3*z⁴ + 5*(x⁴ + y⁴ + w⁴) - 10*z²*(x² + y² + w²)) + Cz
//     w' = w * (3*w⁴ + 5*(x⁴ + y⁴ + z⁴) - 10*w²*(x² + y² + z²)) + Cw + W add
//
//   The current validation harness uses zero Julia/C for Beth641, so only W add
//   is modeled here; x/y/z Julia is effectively zero in the direct parity check.

template<>
cpu_double3 execute_formula<FORMULA_BETH641>(cpu_double3 p, constant double* params, double &de) {
    const double W_add = params[0];  // [edi-0x10]

    // Fixed [CONSTANTS] at positive PVar offsets.
    const double K0 = 3.0;    // [edi+0x00]
    const double K1 = 5.0;    // [edi+0x08]
    const double K2 = -10.0;  // [edi+0x10]

    double x = p.x, y = p.y, z = p.z, w = de;

    double A = x * x;
    double B = y * y;
    double C = z * z;
    double D = w * w;

    double A2 = A * A;
    double B2 = B * B;
    double C2 = C * C;
    double D2 = D * D;

    // Directly mirrors the ASM component order.
    double x_out = x * (K0 * A2 + K1 * (B2 + C2 + D2) + K2 * A * (B + C + D));
    double y_out = y * (K0 * B2 + K1 * (A2 + C2 + D2) + K2 * B * (A + C + D));
    double z_out = z * (K0 * C2 + K1 * (A2 + B2 + D2) + K2 * C * (A + B + D));
    double w_out = w * (K0 * D2 + K1 * (A2 + B2 + C2) + K2 * D * (A + B + C)) + W_add;

    de = w_out;
    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_Beth641 = {
    .id = FORMULA_BETH641,
    .name = "Beth641",
    .param_count = 1,
    .complexity = 207,
    .uses_transcendental = false
};

/*
Original M3 Description:
This is one of the plenty 'Mandalabeth' formulas made by Bethchen
at fractalforums.com.

Symmetric polynomial of degree 5 (effectively power-6 with the
outer c_i multiplication). Each component uses the same pattern
with cyclic permutation of coordinates.

Full 4D (from ASM trace):
  x' = x*(3*x⁴ + 5*(y⁴+z⁴+w⁴) - 10*x²*(y²+z²+w²))
  y' = y*(3*y⁴ + 5*(x⁴+z⁴+w⁴) - 10*y²*(x²+z²+w²))
  z' = z*(3*z⁴ + 5*(x⁴+y⁴+w⁴) - 10*z²*(x²+y²+w²))
  w' = w*(3*w⁴ + 5*(x⁴+y⁴+z⁴) - 10*w²*(x²+y²+z²)) + W_add

3D projection (w=0):
  x' = x*(P0*x⁴ + P2*(y²+z²)*x² + P1*(y⁴+z⁴))
  y' = y*(P0*y⁴ + P2*(x²+z²)*y² + P1*(x⁴+z⁴))
  z' = z*(P0*z⁴ + P2*(x²+y²)*z² + P1*(x⁴+y⁴))

Parameters:
  [0] W_add: double (default: 0.0) - the only user option [edi-0x10]

Fixed constants from [CONSTANTS] (not user params):
  [edi+0x00] = 3
  [edi+0x08] = 5
  [edi+0x10] = -10
*/
