// BenesiPine1.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: BenesiPine1
// Hand-ported from ASM: 272 bytes, DEoption=0 (handles C internally)
// Benesi pinetree formula

constexpr unsigned int FORMULA_BENESIPINE1 = 3211;

// ASM trace:
//   STEP 1: "Benesi fold 1" - two rotation-like transforms with abs
//     Let p0=sqrt(2/3), p1=sqrt(1/3), p2=sqrt(1/2) (default values)
//     Phase 1a:
//       tx = (x*p0 - z*p1) * p2
//       z  = |x*p1 + z*p0|
//       x  = |tx - y*p2|
//       y  = |tx + y*p2|
//     Phase 1b:
//       tx = (x+y) * p2
//       y  = (y-x) * p2
//       x  = tx*p0 + z*p1
//       z  = z*p0 - tx*p1
//     Phase 1c: scale
//       x  = x*Scale - xOff
//       y  = y*Scale
//       z  = z*Scale
//   STEP 2: "Benesi pinetree" - Mandelbrot-like iteration
//     K = 2*x / sqrt(y²+z²)
//     new_x = x²-y²-z² + Cx
//     new_y = 2*K*y*z + Cy
//     new_z = K*(y²-z²) + Cz
//
// C handling: Formula handles C internally (adds Cx/Cy/Cz in step 2).
//   The engine must NOT add C after this formula returns.

template<>
cpu_double3 execute_formula<FORMULA_BENESIPINE1>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];  // (default: 2.0)
    const double xOff  = params[1];  // (default: 2.0)
    // [CONSTANTS] embedded as compile-time values:
    const double p0 = 0.816496580927726;   // [PVar+0x00] sqrt(2/3)
    const double p1 = 0.577350269189626;   // [PVar+0x08] sqrt(1/3)
    const double p2 = 0.707106781186548;   // [PVar+0x10] sqrt(1/2)

    // C values passed by engine from context

    double x = p.x, y = p.y, z = p.z;

    // STEP 1a: First rotation with abs
    double tx = (x * p0 - z * p1) * p2;
    z = fabs(x * p1 + z * p0);
    x = fabs(tx - y * p2);
    y = fabs(tx + y * p2);

    // STEP 1b: Second rotation
    tx = (x + y) * p2;
    y  = (y - x) * p2;
    x  = tx * p0 + z * p1;
    z  = z * p0 - tx * p1;

    // STEP 1c: Scale
    x = x * Scale - xOff;
    y = y * Scale;
    z = z * Scale;

    // STEP 2: Benesi pinetree iteration
    double x2 = x * x;
    double y2 = y * y;
    double z2 = z * z;

    double K = 2.0 * x / sqrt(y2 + z2);

    double new_x = x2 - y2 - z2;
    double new_y = 2.0 * K * y * z;
    double new_z = K * (y2 - z2);

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_BenesiPine1 = {
    .id = FORMULA_BENESIPINE1,
    .name = "BenesiPine1",
    .param_count = 2,
    .complexity = 272,
    .uses_transcendental = true
};

/*
Original M3 Description:
The cool Benesi pinetree formula.

STEP 1: "Benesi fold 1" - rotations using sqrt(2/3), sqrt(1/3), sqrt(1/2)
with absolute values, followed by scaling.
STEP 2: "Benesi pinetree" - Mandelbrot-like iteration:
  t = 2*x/sqrt(y²+z²)
  x = x²-y²-z²
  y = 2*t*y*z
  z = t*(y²-z²)

NOTE: This formula handles C internally. The engine must NOT add C.

Parameters:
  [0] Scale: double (default: 2.0)
  [1] xOff: double (default: 2.0)
  [2] Double: double (default: 0.816496580927726 = sqrt(2/3))
  [3] Double: double (default: 0.5773502691896258 = sqrt(1/3))
  [4] Double: double (default: 0.7071067811865475 = sqrt(1/2))
  [5..7] Cx, Cy, Cz: passed by engine from context
*/
