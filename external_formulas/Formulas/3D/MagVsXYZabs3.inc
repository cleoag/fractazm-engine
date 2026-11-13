// MagVsXYZabs3.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MagVsXYZabs3
// Hand-ported from ASM: 407 bytes, DEoption=0
// MagVsXYZ variant by M.Benesi.
// Same core as MagVsXYZ but:
//   Sign-flips the cos term based on the sign of the original coordinate.
//   When coord >= 0: negate cos_term; when coord < 0: leave cos_term.
//   Creates mirror symmetry across each axis.

constexpr unsigned int FORMULA_MAGVSXYZABS3 = 5881;

// ASM trace (407 bytes):
//
// Setup, r², r, n_i, pow, atan2 — all identical to MagVsXYZ.
// Angle multiplier = Power (same as base MagVsXYZ).
//
// Output section per axis:
//   cos_term = n_i^Power * cos(Power*angle + CxM)
//
//   fld [coord]                ; load original coordinate
//   fldz                       ; 0
//   fcompp                     ; compare 0 with coord, pop both
//   fnstsw ax
//   and ah, 0x41
//   je skip_fchs               ; if coord < 0: skip (leave cos_term positive)
//   fchs                       ; if coord >= 0: negate cos_term
// skip_fchs:
//   fld Ci; fmul CiM; faddp   ; add Ci*CiM
//
// So: coord < 0  → result = cos_term + Ci*CiM
//     coord >= 0 → result = -cos_term + Ci*CiM
//
// C handling: Formula handles C internally (DEoption=0).

template<>
cpu_double3 execute_formula<FORMULA_MAGVSXYZABS3>(cpu_double3 p, constant double* params, double &de) {
    const double Power   = params[0];  // [edi-0x10] (default: 2.0)
    const double Axiom_1 = params[1];  // [edi-0x18] (default: π/2 ≈ 1.5707963)
    const double Axiom_2 = params[2];  // [edi-0x20] (default: π/2 ≈ 1.5707963)
    const double CxM     = params[3];  // [edi-0x28] (default: 1.0) — also CosShift
    const double CyM     = params[4];  // [edi-0x30] (default: 1.0)
    const double CzM     = params[5];  // [edi-0x38] (default: 1.0)
    // C values start after 7 PVar entries
    const double Cx      = params[7];  // J1
    const double Cy      = params[8];  // J2
    const double Cz      = params[9];  // J3

    double x = p.x, y = p.y, z = p.z;

    // a and b
    double a = Axiom_2;
    double b = Power * Axiom_1;

    // r² and r
    double r_sq = x * x + y * y + z * z;
    double r = sqrt(r_sq);

    // Per-axis magnitudes
    double n_x = sqrt(a * x * x + r_sq);
    double n_y = sqrt(a * y * y + r_sq);
    double n_z = sqrt(a * z * z + r_sq);

    // Original FYL2X/FPREM/F2XM1 sequence yields NaN at zero magnitude,
    // including positive powers. libm pow(0, positive) would hide that singularity.
    n_x = x87_log_power(fabs(n_x), Power);
    n_y = x87_log_power(fabs(n_y), Power);
    n_z = x87_log_power(fabs(n_z), Power);

    // Atan2 angles — same as MagVsXYZ
    double theta = atan2(r, b * fabs(x));
    double phi   = atan2(r, b * fabs(y));
    double tango = atan2(r, b * fabs(z));

    // Cos terms
    double cos_x = n_x * cos(Power * theta + CxM);
    double cos_y = n_y * cos(Power * phi   + CxM);
    double cos_z = n_z * cos(Power * tango + CxM);

    // DIFFERENCE: sign-flip cos_term based on coordinate sign
    // coord >= 0 → negate cos_term; coord < 0 → keep cos_term
    double out_x = (x >= 0.0 ? -cos_x : cos_x) + Cx * CxM;
    double out_y = (y >= 0.0 ? -cos_y : cos_y) + Cy * CyM;
    double out_z = (z >= 0.0 ? -cos_z : cos_z) + Cz * CzM;

    return cpu_double3(out_x, out_y, out_z);
}

// Formula metadata
constant FormulaInfo formula_info_MagVsXYZabs3 = {
    .id = FORMULA_MAGVSXYZABS3,
    .name = "MagVsXYZabs3",
    .param_count = 7,
    .complexity = 407,
    .uses_transcendental = true
};

/*
Original M3 Description:
MagVsXYZ variant by M.Benesi with coordinate-sign-dependent output.

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

  cos_x = n_x * cos(Power*theta + CxM)
  cos_y = n_y * cos(Power*phi   + CxM)
  cos_z = n_z * cos(Power*tango + CxM)

  x_out = (x >= 0 ? -cos_x : cos_x) + Cx * CxM
  y_out = (y >= 0 ? -cos_y : cos_y) + Cy * CyM
  z_out = (z >= 0 ? -cos_z : cos_z) + Cz * CzM

Differences from MagVsXYZ:
  1. Cos term is negated when the original coordinate is non-negative
  2. This creates mirror symmetry across each axis plane

ASM pattern per axis:
  fcompp 0 vs coord → je skip → fchs → skip: faddp Ci*CiM

C handling: NON-STANDARD — C scaled by CiM and added.
Engine should NOT add C; template handles it.

Parameters:
  [0] Power: double (default: 2.0) - exponent and angle multiplier
  [1] Axiom_1: double (default: 1.5707963 ≈ π/2) - atan scaling
  [2] Axiom_2: double (default: 1.5707963 ≈ π/2) - magnitude factor
  [3] CxM: double (default: 1.0) - Cx multiplier AND cosine shift
  [4] CyM: double (default: 1.0) - Cy multiplier
  [5] CzM: double (default: 1.0) - Cz multiplier
  [6..8] Cx, Cy, Cz: passed by engine from context
*/
