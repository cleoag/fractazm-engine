// Benesi2Pow6.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Benesi2Pow6
// Hand-ported from ASM: 196 bytes, DEoption=0
// Benesi Type 2 (Pine Tree) power-6 formula - tricomplex cube

constexpr unsigned int FORMULA_BENESI2POW6 = 8746;

// ASM trace:
//   B = x² - y² - z²
//   rxy² = x² + y²
//   rxy = sqrt(rxy²)
//
//   x-component: Benesi T2 cube real part
//     x' = B · (B² - 12·x²·(y²+z²)) + Cx
//     The constant 12 = 3·(v² + w²)/x² where v=2xy, w=2xz in the
//     tricomplex multiplication (u+vj+wk)³.
//
//   Elevation angle (φ) terms via (E+iF)³:
//     E = x²+y²-z²  [= r²·cos(2φ)]
//     F = 2z·rxy     [= r²·sin(2φ)]
//     K₁ = E³ - 3EF² = E(E²-3F²)   [= r⁶·cos(6φ)]
//     K₂ = 3E²F - F³ = F(3E²-F²)   [= r⁶·sin(6φ)]
//
//   Azimuthal angle (θ) terms - sin(6θ)·rxy⁶:
//     L = x²-y², N = 2xy
//     P = L²-N²  [cos(4θ)·rxy⁴]
//     Q = 2NL    [sin(4θ)·rxy⁴]
//     S = N·P + L·Q = 2xy(3x⁴-10x²y²+3y⁴)  [sin(6θ)·rxy⁶]
//
//   inv = 1/(x²+y²)³
//
//   y' = |Cy| - |K₂|
//   z' = |Cz| - |S·K₁|·inv
//
// C handling: DEoption=0. x uses +Cx, y/z use |C|-|...| pattern.

template<>
cpu_double3 execute_formula<FORMULA_BENESI2POW6>(cpu_double3 p, constant double* params, double &de) {
    // [CONSTANTS] embedded as compile-time values:
    const double coeff      = params[0];  // [PVar+0x00]
    // C values passed by engine from context
    const double Cx         = params[1];  // [esi+0x18]
    const double Cy         = params[2];  // [esi+0x20]
    const double Cz         = params[3];  // [esi+0x28]

    double x = p.x, y = p.y, z = p.z;

    // Squared terms
    double x2 = x * x;
    double y2 = y * y;
    double z2 = z * z;
    double rxy2 = x2 + y2;

    // B = x² - y² - z²
    double B = x2 - y2 - z2;

    // x output: Benesi T2 cube real part
    // C_term = B² - coeff·x²·(y²+z²)
    double C_term = B * B - coeff * x2 * (y2 + z2);
    double new_x = B * C_term + Cx;

    // Elevation angle terms: (E+iF)³
    double E = rxy2 - z2;              // x²+y²-z²
    double rxy = sqrt(rxy2);
    double F = 2.0 * z * rxy;         // 2z·sqrt(x²+y²)

    double E2 = E * E;
    double F2 = F * F;
    double K1 = E * (E2 - 3.0 * F2);  // E³ - 3EF²
    double K2 = F * (3.0 * E2 - F2);  // 3E²F - F³

    // Azimuthal angle terms: sin(6θ)·rxy⁶
    double L = x2 - y2;               // cos(2θ)·rxy²
    double N = 2.0 * x * y;          // sin(2θ)·rxy²
    double P = L * L - N * N;         // cos(4θ)·rxy⁴
    double Q = 2.0 * N * L;          // sin(4θ)·rxy⁴
    double S = N * P + L * Q;         // sin(6θ)·rxy⁶

    double rxy6 = rxy2 * rxy2 * rxy2;
    // z output: |Cz| - |sin(6θ) * K₁|
    // x87 keeps tiny x/y tails alive longer than ARM double. Preserve the
    // finite tail collapse for underflowed nonzero x/y, but let the true
    // origin singularity go through as raw 0/0 -> NaN like the reference.
    double sin6theta;
    if (rxy6 == 0.0 && (x != 0.0 || y != 0.0)) {
        sin6theta = 0.0;
    } else {
        sin6theta = S / rxy6;
    }
    // y output: |Cy| - |K₂|
    double new_y = fabs(Cy) - fabs(K2);

    // z output: |Cz| - |sin(6θ) * K₁|
    double new_z = fabs(Cz) - fabs(sin6theta * K1);

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_Benesi2Pow6 = {
    .id = FORMULA_BENESI2POW6,
    .name = "Benesi2Pow6",
    .param_count = 3,
    .complexity = 196,
    .uses_transcendental = true
};

/*
Original M3 Description:
Benesi Type 2 (Pine Tree) power-6 formula. Uses tricomplex cubing
of the T2 squaring operation.

The x-component is the real part of the tricomplex cube:
  B = x² - y² - z²
  x' = B · (B² - 12·x²·(y²+z²)) + Cx

The y and z components use spherical angle expansions with absolute-value
C handling:
  y' = |Cy| - |r⁶·sin(6φ)|
  z' = |Cz| - |sin(6θ)·cos(6φ)·r⁶|

where φ is the elevation angle and θ is the azimuthal angle.
All computed algebraically without trig calls.

The |Cy|/|Cz| pattern means the fractal is reflected for negative C values.

Parameters:
  [0] coeff: constant (value: 12) - tricomplex cube coefficient (3·(2²+2²))
  [1] Cx: passed by engine from context
  [2] Cy: passed by engine from context
  [3] Cz: passed by engine from context
*/
