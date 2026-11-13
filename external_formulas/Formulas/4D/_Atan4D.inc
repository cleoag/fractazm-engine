// _Atan4D.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _Atan4D
// Hand-ported from ASM: 285 bytes, DEoption=4
// Quaternion arctangent: atan(q + P) * Strength

constexpr unsigned int FORMULA__ATAN4D = 6594;

// ASM trace (285 bytes, 3 branches):
//
// Step 1: Add phase offsets
//   x += Px, y += Py, z += Pz, w += Pw
//
// Step 2: r = sqrt(y² + z² + w²)
//
// Step 3: Branch on threshold
//   if r <= threshold → simple: x = atan(x), done
//   if x == 0 AND |r-1| <= threshold → simple: x = atan(x), done
//
// Step 4: Complex quaternion atan computation
//   x' = [atan2(x, 1-r) + atan2(x, 1+r)] * Scale
//   ratio = [(1-r)² + x²] / [(1+r)² + x²]
//   factor = ln(ratio) * LogScale / r
//
// Step 5: Output with Strength scaling
//   x_out = x' * Strength
//   y_out = y * Strength * factor
//   z_out = z * Strength * factor
//   w_out = w * Strength * factor
//
// Math note: With Scale=0.5, LogScale=-0.25:
//   Re(atan(q)) = 0.5 * [atan2(x,1-r) + atan2(x,1+r)]
//               = 0.5 * atan(2x / (1 - |q|²))
//   Im factor = -0.25 * ln([(1-r)²+x²]/[(1+r)²+x²]) / r
//
// For 3D (w=0): r = sqrt(y² + z²), Pw=0.
//
// C handling: Formula does NOT add C. Engine handles C externally.

template<>
cpu_double3 execute_formula<FORMULA__ATAN4D>(cpu_double3 p, constant double* params, double &de) {
    const double Px       = params[0];   // [edi-0x10] (default: 0.0)
    const double Py       = params[1];   // [edi-0x18] (default: 0.0)
    const double Pz       = params[2];   // [edi-0x20] (default: 0.0)
    const double Pw       = params[3];   // [edi-0x28] (default: 0.0)
    const double Strength = params[4];   // [edi-0x30] (default: 1.4)
    // [CONSTANTS] embedded as compile-time values:
    const double Scale    = 0.5;        // [edi+0x00]
    const double thresh   = 1e-5;       // [edi+0x08]
    const double LogScale = 0.25;       // [edi+0x10]

    // Step 1: Add phase offsets
    double x = p.x + Px;
    double y = p.y + Py;
    double z = p.z + Pz;
    double w = de + Pw;

    // Step 2: Imaginary magnitude
    double r = sqrt(y * y + z * z + w * w);

    // Step 3: Threshold checks
    if (r <= thresh) {
        // Simple path: pure real or near-zero imaginary
        de = w;
        return cpu_double3(atan(x), y, z);
    }

    if (x == 0.0 && fabs(r - 1.0) <= thresh) {
        // Singularity guard: x=0, r≈1 (near ±i on unit sphere)
        de = w;
        return cpu_double3(atan(x), y, z);
    }

    // Step 4: Complex quaternion arctangent
    // Keep the x87 operand ordering literally to minimize avoidable drift.
    double x_prime =
        (atan2(x, 1.0 - r) - atan2(-x, 1.0 + r)) * Scale;

    // ratio = [(1+r)² + x²] / [(1-r)² + x²]
    double one_minus_r = 1.0 - r;
    double one_plus_r  = 1.0 + r;
    double numer = one_plus_r  * one_plus_r  + x * x;
    double denom = one_minus_r * one_minus_r + x * x;
    double ratio = numer / denom;

    // factor = ln(ratio) * LogScale / r
    double factor = log(ratio) * LogScale / r;

    // Step 5: Output with Strength
    double x_out = x_prime * Strength;
    double y_out = y * Strength * factor;
    double z_out = z * Strength * factor;
    de = w * Strength * factor;

    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info__Atan4D = {
    .id = FORMULA__ATAN4D,
    .name = "_Atan4D",
    .param_count = 5,
    .complexity = 285,
    .uses_transcendental = true
};

/*
Original M3 Description:
Quaternion arctangent: atan(Q+P) * Strength.
Q is a quaternion (x,y,z,w), P is a phase offset.

atan(q) = Re_part + Im_part * v̂ where v̂ = Im(q)/|Im(q)|

Re_part = [atan2(x, 1-r) + atan2(x, 1+r)] * Scale
Im_part = ln([(1-r)²+x²] / [(1+r)²+x²]) * LogScale / r

With Scale=0.5, LogScale=-0.25 these give the standard quaternion atan.

Full 4D:
  x += Px, y += Py, z += Pz, w += Pw
  r = sqrt(y²+z²+w²)
  [threshold checks → fallback to atan(x)]
  x' = [atan2(x,1-r) + atan2(x,1+r)] * Scale * Strength
  factor = ln([(1-r)²+x²]/[(1+r)²+x²]) * LogScale / r
  y' = y * Strength * factor
  z' = z * Strength * factor
  w' = w * Strength * factor

3D projection (w=0):
  r = sqrt(y²+z²) after phase offset.
  Same computation with w=0.

Parameters:
  [0] Px: double (default: 0.0) - x phase offset [edi-0x10]
  [1] Py: double (default: 0.0) - y phase offset [edi-0x18]
  [2] Pz: double (default: 0.0) - z phase offset [edi-0x20]
  [3] Strength: double (default: 1.0) - amplitude [edi-0x30]
  [4] Scale: double (default: 0.5) - atan scale [edi]
  [5] threshold: double (default: ~1e-10) - branch threshold [edi+0x08]
  [6] LogScale: double (default: -0.25) - log ratio scale [edi+0x10]
  4D-only: Pw [edi-0x28]=0.0
*/
