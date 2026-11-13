// MagVsXYZabs2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MagVsXYZabs2
// Hand-ported from ASM: 399 bytes, DEoption=0
// MagVsXYZ variant by M.Benesi.
// Same core as MagVsXYZ but:
//   1. Angle multiplier is Axiom_1 (not Power), same as MagVsXYZabs
//   2. C contribution is always SUBTRACTED as absolute value: cos_term - |Ci*CiM|

constexpr unsigned int FORMULA_MAGVSXYZABS2 = 4767;

// ASM trace (399 bytes):
//
// Setup, r², r, n_i, pow, atan2 — all identical to MagVsXYZ.
//
// Output (DIFFERENCE 1: Axiom_1 as angle multiplier, same as MagVsXYZabs):
//   cos_term_x = n_x^Power * cos(Axiom_1*theta + CxM)
//
// Output (DIFFERENCE 2: conditional add/sub on C contribution):
//   c_term = Cx * CxM
//   fldz; fcomp st(1) — compare 0 with c_term
//   and ah, 0x41; jne → fsubp (c_term >= 0: subtract)
//                  else → faddp (c_term < 0: add)
//
//   c_term >= 0: cos_term - c_term = cos_term - |c_term|
//   c_term < 0:  cos_term + c_term = cos_term - |c_term|
//   BOTH paths yield: cos_term - |c_term|
//
// C handling: Formula handles C internally (DEoption=0).

template<>
cpu_double3 execute_formula<FORMULA_MAGVSXYZABS2>(cpu_double3 p, constant double* params, double &de) {
    const double Power   = params[0];  // [edi-0x10] (default: 2.0)
    const double Axiom_1 = params[1];  // [edi-0x18] (default: π/2 ≈ 1.5707963)
    const double Axiom_2 = params[2];  // [edi-0x20] (default: π/2 ≈ 1.5707963)
    const double CxM     = params[3];  // [edi-0x28] (default: 1.0) — also CosShift
    const double CyM     = params[4];  // [edi-0x30] (default: 1.0)
    const double CzM     = params[5];  // [edi-0x38] (default: 1.0)
    // C values start after 8 PVar entries (7 options + 1 unused constant)
    const double Cx      = params[8];  // J1
    const double Cy      = params[9];  // J2
    const double Cz      = params[10]; // J3

    double x = p.x, y = p.y, z = p.z;
    const double qnan = NAN;

    // a and b
    double a = Axiom_2;
    double b = Power * Axiom_1;

    // r² and r
    double r_sq = x * x + y * y + z * z;
    if (r_sq == 0.0 || !std::isfinite(r_sq)) {
        return cpu_double3(qnan, qnan, qnan);
    }
    double r = sqrt(r_sq);
    if (!std::isfinite(r)) {
        return cpu_double3(qnan, qnan, qnan);
    }

    // Per-axis magnitudes
    double n_x = sqrt(a * x * x + r_sq);
    double n_y = sqrt(a * y * y + r_sq);
    double n_z = sqrt(a * z * z + r_sq);

    // Raise to Power
    n_x = pow(n_x, Power);
    n_y = pow(n_y, Power);
    n_z = pow(n_z, Power);
    if (!std::isfinite(n_x) || !std::isfinite(n_y) || !std::isfinite(n_z)) {
        return cpu_double3(qnan, qnan, qnan);
    }

    // Atan2 angles — same as MagVsXYZ
    double theta = atan2(r, b * fabs(x));
    double phi   = atan2(r, b * fabs(y));
    double tango = atan2(r, b * fabs(z));
    if (!std::isfinite(theta) || !std::isfinite(phi) || !std::isfinite(tango)) {
        return cpu_double3(qnan, qnan, qnan);
    }

    // Output: DIFFERENCE 1 — Axiom_1 as angle multiplier (not Power)
    // DIFFERENCE 2 — always subtracts |Ci*CiM| from cos term
    double out_x = n_x * cos(Axiom_1 * theta + CxM) - fabs(Cx * CxM);
    double out_y = n_y * cos(Axiom_1 * phi   + CxM) - fabs(Cy * CyM);
    double out_z = n_z * cos(Axiom_1 * tango + CxM) - fabs(Cz * CzM);
    if (!std::isfinite(out_x) || !std::isfinite(out_y) || !std::isfinite(out_z)) {
        return cpu_double3(qnan, qnan, qnan);
    }

    return cpu_double3(out_x, out_y, out_z);
}

// Formula metadata
constant FormulaInfo formula_info_MagVsXYZabs2 = {
    .id = FORMULA_MAGVSXYZABS2,
    .name = "MagVsXYZabs2",
    .param_count = 9,
    .complexity = 399,
    .uses_transcendental = true
};

/*
Original M3 Description:
MagVsXYZ variant by M.Benesi.
Same core computation as MagVsXYZ family.

Mathematical Formula:
  a = Axiom_2
  b = Power * Axiom_1
  r = sqrt(x²+y²+z²)

  n_x = sqrt(a*x²+r²) ^ Power
  n_y = sqrt(a*y²+r²) ^ Power
  n_z = sqrt(a*z²+r²) ^ Power

  theta = atan2(r, b*|x|)
  phi   = atan2(r, b*|y|)
  tango = atan2(r, b*|z|)

  x = n_x * cos(Axiom_1*theta + CxM) - |Cx * CxM|
  y = n_y * cos(Axiom_1*phi   + CxM) - |Cy * CyM|
  z = n_z * cos(Axiom_1*tango + CxM) - |Cz * CzM|

Differences from MagVsXYZ:
  1. Angle multiplier is Axiom_1 (not Power)
  2. C contribution is subtracted as absolute value: cos_term - |Ci*CiM|

NOTE: The ASM uses fcomp/fnstsw/jne conditional that produces
faddp when c_term < 0 and fsubp when c_term >= 0, but both paths
yield the same result: cos_term - |c_term|.

C handling: NON-STANDARD — C terms are subtracted as |Ci*CiM|.
Engine should NOT add C; template handles it.

Parameters:
  [0] Power: double (default: 2.0) - exponent for magnitudes
  [1] Axiom_1: double (default: 1.5707963 ≈ π/2) - atan scaling AND angle multiplier
  [2] Axiom_2: double (default: 1.5707963 ≈ π/2) - magnitude factor
  [3] CxM: double (default: 1.0) - Cx multiplier AND cosine shift
  [4] CyM: double (default: 1.0) - Cy multiplier
  [5] CzM: double (default: 1.0) - Cz multiplier
  [6..8] Cx, Cy, Cz: passed by engine from context
*/
