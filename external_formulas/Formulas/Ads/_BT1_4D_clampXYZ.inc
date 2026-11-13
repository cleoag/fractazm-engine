// _BT1_4D_clampXYZ.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _BT1_4D_clampXYZ
// Hand-ported from ASM: 310 bytes, DEoption=-1
// BulbTracer1 4D Transform with radius-preserving normalization.

constexpr unsigned int FORMULA__BT1_4D_CLAMPXYZ = 4267;

template<>
struct ExternalFormulaWritesWState<FORMULA__BT1_4D_CLAMPXYZ> {
    static constexpr bool value = true;
};

// ASM trace (310 bytes, 0 branches):
//
// Same as _BT1_4D_Transform1 but with radius normalization at the end.
// The output is scaled so that the point's distance from origin is
// preserved (clamped to the original radius).
//
// Step 1: r_orig = sqrt(x² + y² + z²)
//
// Steps 2-5: Identical to _BT1_4D_Transform1
//   (4D rotation/fold with abs values using params A,B,C,D,E)
//   T1 = |x*E + w*D|
//   T2 = x*D - w*E
//   T3 = (T2*A - z*B) * C
//   z_temp = |T2*B + z*A|
//   x_minus = |T3 - y*C|
//   x_plus  = |T3 + y*C|
//   T4 = (x_minus + x_plus) * C
//   y_new = (x_plus - x_minus) * C
//   T5 = T4*A + z_temp*B
//   z_new = z_temp*A - T4*B
//   w_new = T1*D - T5*E  (stored to the hidden w output)
//   x_pre = T5*D + T1*E
//
// Step 6: Radius normalization
//   r_new = sqrt(x_pre² + y_new² + z_new²)
//   norm = r_orig / r_new
//
// Step 7: Final output
//   x' = x_pre * norm * Scale - Offset
//   y' = y_new * norm * Scale
//   z' = z_new * norm * Scale
//
// C handling: Transform (DEoption=-1), no C.

template<>
cpu_double3 execute_formula<FORMULA__BT1_4D_CLAMPXYZ>(cpu_double3 p, constant double* params, double &de) {
    (void)de;

    const double A      = params[2];   // [edi]      (default: ?)
    const double B      = params[3];   // [edi+0x08] (default: ?)
    const double C      = params[4];   // [edi+0x10] (default: ?)
    const double D      = params[5];   // [edi+0x18] (default: ?)
    const double E      = params[6];   // [edi+0x20] (default: ?)
    const double Scale  = params[0];   // [edi-0x10] (default: ?)
    const double Offset = params[1];   // [edi-0x18] (default: ?)

    double x = p.x, y = p.y, z = p.z;
    double w = params[FORMULA_STATE_W_SLOT];

    // Step 1: Original radius
    double r_orig = sqrt(x * x + y * y + z * z);

    // Steps 2-5: Same computation as _BT1_4D_Transform1
    double T1 = fabs(x * E + w * D);
    double T2 = x * D - w * E;
    double T3 = (T2 * A - z * B) * C;

    double z_temp  = fabs(T2 * B + z * A);
    double x_minus = fabs(T3 - y * C);
    double x_plus  = fabs(T3 + y * C);

    double T4    = (x_minus + x_plus) * C;
    double y_new = (x_plus - x_minus) * C;

    double T5    = T4 * A + z_temp * B;
    double z_new = z_temp * A - T4 * B;

    params[FORMULA_STATE_W_SLOT] = T1 * D - T5 * E;
    double x_pre = T5 * D + T1 * E;

    // Step 6: Radius normalization
    double r_new = sqrt(x_pre * x_pre + y_new * y_new + z_new * z_new);
    double norm = (r_new > 1e-30) ? (r_orig / r_new) : 1.0;

    // Step 7: Final output with normalization + scaling
    double x_out = x_pre * norm * Scale - Offset;
    double y_out = y_new * norm * Scale;
    double z_out = z_new * norm * Scale;

    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info__BT1_4D_clampXYZ = {
    .id = FORMULA__BT1_4D_CLAMPXYZ,
    .name = "_BT1_4D_clampXYZ",
    .param_count = 7,
    .complexity = 310,
    .uses_transcendental = false
};

/*
Original M3 Description:
BulbTracer1 4D Transform with radius-preserving ("clamping") normalization.

Identical to _BT1_4D_Transform1 but adds a normalization step that
rescales the output so that the point's distance from the origin
matches the original input distance. This prevents the transform
from changing the point's overall magnitude.

norm = r_orig / r_new
x' = x_pre * norm * Scale - Offset
y' = y_new * norm * Scale
z' = z_new * norm * Scale

The "clamp" in the name refers to this radius preservation.

Parameters:
  [0] A: double [edi]
  [1] B: double [edi+0x08]
  [2] C: double [edi+0x10]
  [3] D: double [edi+0x18]
  [4] E: double [edi+0x20]
  [5] Scale: double [edi-0x10]
  [6] Offset: double [edi-0x18]
*/
