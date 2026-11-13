// MagVsXYZ.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MagVsXYZ
// Hand-ported from ASM: 362 bytes, DEoption=0
// MagVsXYZ formula by M.Benesi — Magnitude vs XYZ coordinates.
// "Midway of Cantor dust and Burning Ship, but in 3D"
//
// The formula computes per-axis radial magnitudes, raises them to Power,
// then combines with atan2-based angles and cosine modulation.

constexpr unsigned int FORMULA_MAGVSXYZ = 2979;

// ASM trace (362 bytes, 0 branches):
//
// Setup:
//   a = Axiom_2                          [edi-0x20]
//   b = Power * Axiom_1                  [edi-0x10] * [edi-0x18]
//
// Compute r² and r:
//   r_sq = x² + y² + z²
//   r = sqrt(r_sq)                       (r stays on FPU stack throughout)
//
// Per-axis magnitudes:
//   n_x = sqrt(a*x² + r_sq)
//   n_y = sqrt(a*y² + r_sq)
//   n_z = sqrt(a*z² + r_sq)
//
// Power each magnitude (fyl2x/fprem/f2xm1/fscale = pow):
//   n_x = |n_x| ^ Power                 (fabs before fyl2x; n_x always ≥ 0)
//   n_y = |n_y| ^ Power
//   n_z = |n_z| ^ Power
//
// Atan2 angles:
//   theta = atan2(r, b*|x|)             NOTE: ASM puts r in ST(1), b*|x| in ST(0)
//   phi   = atan2(r, b*|y|)             for fpatan → arctan(ST(1)/ST(0))
//   tango = atan2(r, b*|z|)
//
//   NOTE: M3 description says atan2(b*|x|, r) but ASM computes atan2(r, b*|x|).
//   These are complementary: atan2(a,b) + atan2(b,a) = π/2 for positive a,b.
//   We follow the ASM faithfully.
//
// Output (MagVsXYZ — no fabs):
//   x = n_x * cos(Power*theta + CxM) + Cx * CxM
//   y = n_y * cos(Power*phi   + CxM) + Cy * CyM
//   z = n_z * cos(Power*tango + CxM) + Cz * CzM
//
//   NOTE: [edi-0x28] (CxM) serves DUAL purpose: both as the cosine shift
//   for ALL axes and as the Cx multiplier. The M3 description lists
//   "CosShift" and "CxM" as separate parameters, but in the ASM they
//   share the same memory offset [edi-0x28].
//
// C handling: Formula handles C internally (DEoption=0).
//   Cx/Cy/Cz read from [esi+0x18/0x20/0x28].

template<>
cpu_double3 execute_formula<FORMULA_MAGVSXYZ>(cpu_double3 p, constant double* params, double &de) {
    const double Power   = params[0];  // [edi-0x10] (default: 2.0)
    const double Axiom_1 = params[1];  // [edi-0x18] (default: π/2 ≈ 1.5707963)
    const double Axiom_2 = params[2];  // [edi-0x20] (default: π/4 ≈ 0.78539816)
    const double CxM     = params[3];  // [edi-0x28] (default: 1.0) — also CosShift
    const double CyM     = params[4];  // [edi-0x30] (default: 1.0)
    const double CzM     = params[5];  // [edi-0x38] (default: 1.0)
    const double Cx      = params[7];  // from engine context
    const double Cy      = params[8];  // from engine context
    const double Cz      = params[9];  // from engine context

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

    // Raise to Power (ASM uses fabs before fyl2x; sqrt results are non-negative)
    n_x = pow(n_x, Power);
    n_y = pow(n_y, Power);
    n_z = pow(n_z, Power);
    if (!std::isfinite(n_x) || !std::isfinite(n_y) || !std::isfinite(n_z)) {
        return cpu_double3(qnan, qnan, qnan);
    }

    // Atan2 angles — ASM: fpatan(ST(1)=r, ST(0)=b*|coord|) = atan2(r, b*|coord|)
    double theta = atan2(r, b * fabs(x));
    double phi   = atan2(r, b * fabs(y));
    double tango = atan2(r, b * fabs(z));
    if (!std::isfinite(theta) || !std::isfinite(phi) || !std::isfinite(tango)) {
        return cpu_double3(qnan, qnan, qnan);
    }

    // Output: n_i * cos(Power*angle + CxM) + Ci * CiM
    // Note: CxM is used as cosine shift for ALL three axes
    double out_x = n_x * cos(Power * theta + CxM) + Cx * CxM;
    double out_y = n_y * cos(Power * phi   + CxM) + Cy * CyM;
    double out_z = n_z * cos(Power * tango + CxM) + Cz * CzM;
    if (!std::isfinite(out_x) || !std::isfinite(out_y) || !std::isfinite(out_z)) {
        return cpu_double3(qnan, qnan, qnan);
    }

    return cpu_double3(out_x, out_y, out_z);
}

// Formula metadata
constant FormulaInfo formula_info_MagVsXYZ = {
    .id = FORMULA_MAGVSXYZ,
    .name = "MagVsXYZ",
    .param_count = 9,
    .complexity = 362,
    .uses_transcendental = true
};

/*
Original M3 Description:
MagVsXYZ, formula by M.Benesi.
Shows off in one quadrant only.
"Midway" of Cantor dust and Burning Ship, but in 3D!

Mathematical Formula:
  a = Axiom_2
  b = Power * Axiom_1
  r = sqrt(x²+y²+z²)

  n_x = sqrt(a*x²+r²) ^ Power
  n_y = sqrt(a*y²+r²) ^ Power
  n_z = sqrt(a*z²+r²) ^ Power

  theta = atan2(r, b*|x|)      (ASM: fpatan with r/b*|x|)
  phi   = atan2(r, b*|y|)
  tango = atan2(r, b*|z|)

  x = n_x * cos(Power*theta + CxM) + Cx * CxM
  y = n_y * cos(Power*phi   + CxM) + Cy * CyM
  z = n_z * cos(Power*tango + CxM) + Cz * CzM

NOTE: params[3] (CxM at [edi-0x28]) serves dual purpose as both
the cosine shift for all axes AND the Cx multiplier. The M3
description lists "CosShift" and "CxM" separately but they share
one parameter slot in the compiled code.

C handling: NON-STANDARD — CxM/CyM/CzM scale the C contributions.
Engine should NOT add C; template handles it.

Parameters:
  [0] Power: double (default: 2.0) - exponent and angle multiplier
  [1] Axiom_1: double (default: 1.5707963 ≈ π/2) - atan scaling
  [2] Axiom_2: double (default: 0.78539816 ≈ π/4) - magnitude factor
  [3] CxM: double (default: 1.0) - Cx multiplier AND cosine shift
  [4] CyM: double (default: 1.0) - Cy multiplier
  [5] CzM: double (default: 1.0) - Cz multiplier
  [6..8] Cx, Cy, Cz: passed by engine from context
*/
