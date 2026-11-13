// _BT1_4D_Transform1.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _BT1_4D_Transform1
// Hand-ported from ASM: 254 bytes, DEoption=-1
// BulbTracer1 4D Transform — complex 4D rotation/fold with abs values.

constexpr unsigned int FORMULA__BT1_4D_TRANSFORM1 = 8340;

template<>
struct ExternalFormulaWritesWState<FORMULA__BT1_4D_TRANSFORM1> {
    static constexpr bool value = true;
};

// ASM trace (254 bytes, 0 branches):
//
// Uses 5 anonymous constants A,B,C,D,E and 2 named params Scale, Offset.
// All purely algebraic with abs() folds.
//
// Step 1: Compute intermediates from (x,y,z,w)
//   T1 = |x*E + w*D|
//   T2 = x*D - w*E
//   T3 = (T2*A - z*B) * C
//
// Step 2: Fold operations
//   z_temp  = |T2*B + z*A|
//   x_minus = |T3 - y*C|
//   x_plus  = |T3 + y*C|
//
// Step 3: Combine
//   T4 = (x_minus + x_plus) * C
//   y_new = (x_plus - x_minus) * C
//
// Step 4: Rotate T4/z_temp
//   T5 = T4*A + z_temp*B
//   z_new = z_temp*A - T4*B
//
// Step 5: Final with D,E rotation + scaling
//   w_new = T1*D - T5*E
//   x_new = (T5*D + T1*E) * Scale - Offset
//   y_new = y_new * Scale
//   z_new = z_new * Scale
//
// C handling: Transform (DEoption=-1), no C.

template<>
cpu_double3 execute_formula<FORMULA__BT1_4D_TRANSFORM1>(cpu_double3 p, constant double* params, double &de) {
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

    // Step 1: Compute intermediates
    double T1 = fabs(x * E + w * D);
    double T2 = x * D - w * E;
    double T3 = (T2 * A - z * B) * C;

    // Step 2: Fold operations with abs
    double z_temp  = fabs(T2 * B + z * A);
    double x_minus = fabs(T3 - y * C);
    double x_plus  = fabs(T3 + y * C);

    // Step 3: Combine folded values
    double T4    = (x_minus + x_plus) * C;
    double y_new = (x_plus - x_minus) * C;

    // Step 4: Rotate (T4, z_temp) by (A, B)
    double T5    = T4 * A + z_temp * B;
    double z_new = z_temp * A - T4 * B;

    // Step 5: Final rotation by (D, E) + scaling
    params[FORMULA_STATE_W_SLOT] = T1 * D - T5 * E;
    double x_new = (T5 * D + T1 * E) * Scale - Offset;
    y_new *= Scale;
    z_new *= Scale;

    return cpu_double3(x_new, y_new, z_new);
}

// Formula metadata
constant FormulaInfo formula_info__BT1_4D_Transform1 = {
    .id = FORMULA__BT1_4D_TRANSFORM1,
    .name = "_BT1_4D_Transform1",
    .param_count = 7,
    .complexity = 254,
    .uses_transcendental = false
};

/*
Original M3 Description:
BulbTracer1 4D Transform 1. A complex algebraic transform using
absolute-value folds and rotations in 4D space. Used as a modifier
transform in fractal formula chains.

The transform applies paired rotations using (A,B) and (D,E) parameter
pairs, with abs-fold operations creating fractal symmetry.

Full 4D:
  T1 = |x*E + w*D|
  T2 = x*D - w*E
  T3 = (T2*A - z*B)*C
  z' = |T2*B + z*A|
  x_m = |T3 - y*C|,  x_p = |T3 + y*C|
  T4 = (x_m + x_p)*C,  y' = (x_p - x_m)*C
  T5 = T4*A + z'*B,  z' = z'*A - T4*B
  w' = T1*D - T5*E
  x' = (T5*D + T1*E)*Scale - Offset
  y' *= Scale, z' *= Scale

Parameters:
  [0] A: double [edi]
  [1] B: double [edi+0x08]
  [2] C: double [edi+0x10]
  [3] D: double [edi+0x18]
  [4] E: double [edi+0x20]
  [5] Scale: double [edi-0x10]
  [6] Offset: double [edi-0x18]
*/
