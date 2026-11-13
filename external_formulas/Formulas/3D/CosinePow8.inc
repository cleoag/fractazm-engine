// CosinePow8.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: CosinePow8
// Hand-ported from ASM: 206 bytes, DEoption=0
// Power-8 "Cosine" Mandelbulb - algebraic expansion (no trig calls)

constexpr unsigned int FORMULA_COSINEPOW8 = 5921;

// ASM trace:
//   Pure polynomial/algebraic power-8 Mandelbulb using the "cosine" variant.
//   Computes cos(8φ), sin(8φ), cos(8θ), sin(8θ) algebraically via
//   Chebyshev-like polynomial expansions in x², y², z².
//
//   Let rxy² = x²+y², r² = rxy²+z².
//
//   z_poly = cos(8φ)·r⁸ = z⁸ - 28·z⁶·rxy² + 70·z⁴·rxy⁴ - 28·z²·rxy⁶ + rxy⁸
//   z_out = Z_multiplier · z_poly + Cz
//
//   Q₂ = z⁶ - 7·z⁴·rxy² + 7·z²·rxy⁴ - rxy⁶
//   ratio = (8·z·Q₂) / (rxy⁷ + ε)     [ε=1e-40 guards rxy=0 singularity]
//   ASM: fdivp st(1): st(1) = 8*z*Q₂, st(0) = rxy⁷+ε → result = 8*z*Q₂/(rxy⁷+ε)
//   (NOTE: previous code had this ratio INVERTED — the ASM is (8zQ₂)/(rxy⁷+ε) not vice versa)
//
//   S₁ = cos(8θ)·rxy⁸ = x⁸ - 28·x⁶·y² + 70·x⁴·y⁴ - 28·x²·y⁶ + y⁸
//   P  = x⁶ - 7·x⁴·y² + 7·x²·y⁴ - y⁶     [sin(8θ)·rxy⁸ = 8·x·y·P]
//
//   x_out = S₁ · ratio + Cx
//   y_out = 8 · P · x · y · ratio + Cy
//
// Constants from M3 (hardcoded): 1e-40, 7, 8, 28, 70
// C handling: DEoption=0, formula reads and adds C internally.

template<>
cpu_double3 execute_formula<FORMULA_COSINEPOW8>(cpu_double3 p, constant double* params, double &de) {
    const double Z_multiplier = params[0];  // [PVar-0x10] (default: 1)
    const double Cx           = params[6];  // from engine context [esi+0x18]
    const double Cy           = params[7];  // from engine context [esi+0x20]
    const double Cz           = params[8];  // from engine context [esi+0x28]

    double x = p.x, y = p.y, z = p.z;

    // Squared terms
    double x2 = x * x;
    double y2 = y * y;
    double z2 = z * z;
    double rxy2 = x2 + y2;

    // Fourth powers
    double z4 = z2 * z2;
    double rxy4 = rxy2 * rxy2;
    double x4 = x2 * x2;
    double y4 = y2 * y2;

    // --- z output: cos(8φ)·r⁸ ---
    // z⁸ - 28·z⁶·rxy² + 70·z⁴·rxy⁴ - 28·z²·rxy⁶ + rxy⁸
    double z_poly = z4 * z4 + rxy4 * rxy4
                   + 70.0 * z4 * rxy4
                   - 28.0 * rxy2 * z2 * (rxy4 + z4);
    double new_z = Z_multiplier * z_poly + Cz;

    // --- Ratio: (rxy⁷ + ε) / (8·z·Q₂) ---
    // Q₂ = z⁶ - 7·z⁴·rxy² + 7·z²·rxy⁴ - rxy⁶
    double z6 = z2 * z4;
    double rxy6 = rxy2 * rxy4;
    double Q2 = z6 - 7.0 * z4 * rxy2 + 7.0 * z2 * rxy4 - rxy6;

    double rxy = sqrt(rxy2);
    double rxy7 = rxy4 * rxy2 * rxy;  // rxy⁷ = rxy⁴·rxy²·rxy
    // ASM: fdivp st(1): st(1)=8*z*Q2 divided by st(0)=rxy7+ε → ratio = 8*z*Q2/(rxy7+ε)
    double ratio = (8.0 * z * Q2) / (rxy7 + 1e-40);

    // --- y output: sin(8θ)·rxy⁸ · ratio ---
    // P = x⁶ - 7·x⁴·y² + 7·x²·y⁴ - y⁶
    double P = x4 * (x2 - 7.0 * y2) + y4 * (7.0 * x2 - y2);
    double new_y = 8.0 * P * x * y * ratio + Cy;

    // --- x output: cos(8θ)·rxy⁸ · ratio ---
    // S₁ = x⁸ - 28·x⁶·y² + 70·x⁴·y⁴ - 28·x²·y⁶ + y⁸
    double x2y2 = x2 * y2;
    double S1 = x4 * x4 + y4 * y4
               + 70.0 * x4 * y4
               - 28.0 * x2y2 * (x4 + y4);
    double new_x = ratio * S1 + Cx;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_CosinePow8 = {
    .id = FORMULA_COSINEPOW8,
    .name = "CosinePow8",
    .param_count = 4,
    .complexity = 206,
    .uses_transcendental = false
};

/*
Original M3 Description:
Power-8 "Cosine" Mandelbulb variant. Uses algebraic expansion of the
spherical trigonometric identities to compute the Mandelbulb iteration
without any trig function calls.

The formula is equivalent to the standard power-8 Mandelbulb:
  r = sqrt(x²+y²+z²)
  θ = atan2(y, x)
  φ = atan2(rxy, z)
  x' = r⁸ · sin(8φ) · cos(8θ) + Cx
  y' = r⁸ · sin(8φ) · sin(8θ) + Cy
  z' = r⁸ · cos(8φ) + Cz

but expressed entirely in polynomial form with a ratio term
sin⁸(φ)/sin(8φ) for the x-y components. This avoids expensive
atan2/sin/cos calls at the cost of a singularity at z=0 (where
sin(8φ) = 0), guarded by epsilon 1e-40 in the numerator.

Hardcoded constants (from M3 anonymous doubles):
  7 = C(7,1), 8 = C(8,1), 28 = C(8,2), 70 = C(8,4), 1e-40 = epsilon

Parameters:
  [0] Z_multiplier: double (default: 1) - scales z output polynomial
  [1] Cx: passed by engine from context
  [2] Cy: passed by engine from context
  [3] Cz: passed by engine from context
*/
