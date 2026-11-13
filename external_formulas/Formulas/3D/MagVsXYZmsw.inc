// MagVsXYZmsw.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MagVsXYZmsw
// Hand-ported from ASM: 476 bytes, DEoption=0
// MagVsXYZ variant by M.Benesi — "modified sign weight".
// Same core as MagVsXYZ but:
//   Conditionally negates n_i^Power when BOTH Ci >= 0 AND coord_i >= 0.
//   Then standard output with Power as angle multiplier.

constexpr unsigned int FORMULA_MAGVSXYZMSW = 3050;

// ASM trace (476 bytes):
//
// Setup, r², r, n_i, pow, atan2 — all identical to MagVsXYZ.
// Angle multiplier = Power (same as base MagVsXYZ).
//
// After atan2, BEFORE output — conditional n_i negation:
//
// For each axis (x shown):
//   fld Cx; fldz; fcompp       ; compare 0 with Cx
//   and ah, 0x41; jle skip     ; if Cx < 0: skip (jle = je after AND)
//   fld x; fldz; fcompp        ; compare 0 with x
//   and ah, 0x41; jle skip     ; if x < 0: skip
//   n_x = -n_x                 ; negate only if BOTH Cx >= 0 AND x >= 0
// skip:
//
// Output (standard MagVsXYZ):
//   x = n_x * cos(Power*theta + CxM) + Cx * CxM
//   (n_x may have been negated above)
//
// C handling: Formula handles C internally (DEoption=0).

template<>
cpu_double3 execute_formula<FORMULA_MAGVSXYZMSW>(cpu_double3 p, constant double* params, double &de) {
    const double Power   = params[0];  // [edi-0x10] (default: 2.0)
    const double Axiom_1 = params[1];  // [edi-0x18] (default: π/2 ≈ 1.5707963)
    const double Axiom_2 = params[2];  // [edi-0x20] (default: π/2 ≈ 1.5707963)
    const double CxM     = params[3];  // [edi-0x28] (default: 1.0) — also CosShift
    const double CyM     = params[4];  // [edi-0x30] (default: 1.0)
    const double CzM     = params[5];  // [edi-0x38] (default: 1.0)
    const double Cx      = params[7];  // from engine context
    const double Cy      = params[8];  // from engine context
    const double Cz      = params[9];  // from engine context

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

    // DIFFERENCE: conditional negate n_i when BOTH Ci >= 0 AND coord_i >= 0
    if (Cx >= 0.0 && x >= 0.0) n_x = -n_x;
    if (Cy >= 0.0 && y >= 0.0) n_y = -n_y;
    if (Cz >= 0.0 && z >= 0.0) n_z = -n_z;

    // Atan2 angles — same as MagVsXYZ
    double theta = atan2(r, b * fabs(x));
    double phi   = atan2(r, b * fabs(y));
    double tango = atan2(r, b * fabs(z));

    // Output: standard MagVsXYZ (with possibly negated n_i)
    double out_x = n_x * cos(Power * theta + CxM) + Cx * CxM;
    double out_y = n_y * cos(Power * phi   + CxM) + Cy * CyM;
    double out_z = n_z * cos(Power * tango + CxM) + Cz * CzM;

    return cpu_double3(out_x, out_y, out_z);
}

// Formula metadata
constant FormulaInfo formula_info_MagVsXYZmsw = {
    .id = FORMULA_MAGVSXYZMSW,
    .name = "MagVsXYZmsw",
    .param_count = 7,
    .complexity = 476,
    .uses_transcendental = true
};

/*
Original M3 Description:
MagVsXYZ variant by M.Benesi — "modified sign weight".
Conditionally negates the magnitude term based on the signs of both
the C value and the corresponding coordinate.

Mathematical Formula:
  a = Axiom_2
  b = Power * Axiom_1
  r = sqrt(x²+y²+z²)

  n_x = sqrt(a*x²+r²) ^ Power
  n_y = sqrt(a*y²+r²) ^ Power
  n_z = sqrt(a*z²+r²) ^ Power

  // Conditional negate:
  if (Cx >= 0 AND x >= 0) n_x = -n_x
  if (Cy >= 0 AND y >= 0) n_y = -n_y
  if (Cz >= 0 AND z >= 0) n_z = -n_z

  theta = atan2(r, b*|x|)
  phi   = atan2(r, b*|y|)
  tango = atan2(r, b*|z|)

  x = n_x * cos(Power*theta + CxM) + Cx * CxM
  y = n_y * cos(Power*phi   + CxM) + Cy * CyM
  z = n_z * cos(Power*tango + CxM) + Cz * CzM

Differences from MagVsXYZ:
  n_i^Power is negated when both the C value and coordinate
  are non-negative, creating asymmetric fractal shapes that
  depend on the C value quadrant.

ASM pattern per axis:
  fcompp 0 vs Ci → jle skip_outer
    fcompp 0 vs coord → jle skip_inner
      n_i = -n_i
  skip:

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
