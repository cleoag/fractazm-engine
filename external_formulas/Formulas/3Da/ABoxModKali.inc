// ABoxModKali.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: ABoxModKali
// Hand-ported from ASM: 155 bytes, DEoption=2
// Modified Amazing Box (Kali variant): offset fold + sphere fold + C.

constexpr unsigned int FORMULA_ABOXMODKALI = 8900;

// ASM trace (155 bytes, SSE2, 2 branches):
//
// Uses SSE2 packed-double instructions (movupd, andpd, subpd, mulpd).
// The abs() is done via andpd with a sign-bit mask at [PVar+0].
//
// Step 1: Offset fold
//   vx = X_add - |x|
//   vy = Y_add - |y|
//   vz = Z_add - |z|
//
// Step 2: Compute r² = vx² + vy² + vz²
//
// Step 3: Sphere fold (three-way branch)
//   if r² < minR²:       scale = inner_scale  (= Scale/minR²)
//   elif r² < fixedR²:   scale = Scale / r²   (sphere inversion)
//   else:                 scale = Scale
//
// Step 4: Scale and add C
//   x' = vx * scale + Cx
//   y' = vy * scale + Cy
//   z' = vz * scale + Cz
//
// C handling: Formula adds C internally.

template<>
cpu_double3 execute_formula<FORMULA_ABOXMODKALI>(cpu_double3 p, constant double* params, double &de) {
    const double Scale   = params[0];   // raw .Double Scale
    const double innerScale = params[1]; // BOXSCALE pre-computed: Scale/sqr(MinR)
    const double minR2   = params[2];   // BOXSCALE pre-computed: sqr(MinR)
    const double Z_add   = params[3];   // raw .2Doubles Z add (type 14)
    const double Y_add   = params[4];   // raw .Double Y add
    const double X_add   = params[5];   // raw .Double X add
    const double fixedR2 = 1.0;         // [PVar+0x10] embedded constant

    // Step 1: Offset fold from the additive parameters.
    double vx = X_add - fabs(p.x);
    double vy = Y_add - fabs(p.y);
    double vz = Z_add - fabs(p.z);

    // Step 2: Squared distance of folded point
    double r2 = vx * vx + vy * vy + vz * vz;

    // Step 3: Sphere fold
    double scale;
    if (r2 < minR2) {
        scale = innerScale;
    } else if (r2 < fixedR2) {
        scale = Scale / r2;
    } else {
        scale = Scale;
    }

    // DE tracking: ASM uses SSE2 andpd for abs on input, then mulpd for scale.
    // The andpd on (z,w) pack applies abs to BOTH z and w (de) before fold,
    // not after. The fabs(de) here handles the de side.
    de = fabs(de) * scale;

    // Step 4: Apply scale (C addition handled by wrapper via ADDS_C=0)
    double x_out = vx * scale;
    double y_out = vy * scale;
    double z_out = vz * scale;

    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_ABoxModKali = {
    .id = FORMULA_ABOXMODKALI,
    .name = "ABoxModKali",
    .param_count = 5,
    .complexity = 155,
    .uses_transcendental = false
};

/*
Original M3 Description:
A modified Amazing Box as suggested by Kali, one iteration:

  x = X_add - abs(x)
  y = Y_add - abs(y)
  z = Z_add - abs(z)
  rr = x*x + y*y + z*z
  if rr < sqr(Min_R) then m = Scale/sqr(Min_R) else
  if rr < 1 then m = Scale/rr else m = Scale
  x = x * m + Cx
  y = y * m + Cy
  z = z * m + Cz

This variant uses additive offsets instead of the standard box fold.
Sphere fold with inner ball clamping and inversion zone.

Parameters:
  [0] Scale: double (default: 1.5) - raw .Double Scale [PVar-0x10]
  [1] Min_R: double (default: 0.6) - raw .Boxscale Min R, expanded to minR² and Scale/minR² in code
  [2] Z_add: double (default: 0.5) - raw .2Doubles Z add [PVar-0x30]
  [3] Y_add: double (default: 0.5) - raw .Double Y add [PVar-0x38]
  [4] X_add: double (default: 0.5) - raw .Double X add [PVar-0x40]
  [5..7] Cx, Cy, Cz: engine context only, not part of the public params array
*/
