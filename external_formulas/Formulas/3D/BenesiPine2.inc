// BenesiPine2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: BenesiPine2
// Hand-ported from ASM: 298 bytes, DEoption=0 (handles C internally)
// Benesi pinetree formula variant 2

constexpr unsigned int FORMULA_BENESIPINE2 = 8088;

// ASM trace:
//   STEP 1: "Benesi fold 2" - rotation with squares/sqrt/abs
//     Let p0=sqrt(2/3), p1=sqrt(1/3), p2=sqrt(1/2) (default values)
//     Phase 1a:
//       tx = (x*p0 - z*p1) * p2
//       y  = y * p2
//       z  = (x*p1 + z*p0)²
//       tx2 = (tx - y)²
//       ty2 = (tx + y)²
//     Phase 1b: sqrt/abs/scale
//       x  = |sqrt(ty2+z) - offset| * Scale * p2
//       y  = |sqrt(tx2+z) - offset| * Scale * p2
//       z  = |sqrt(tx2+ty2) - offset| * Scale
//     Phase 1c: second rotation
//       tx = y + x;  y = y - x
//       x  = tx*p0 + z*p1
//       z  = z*p0 - tx*p1
//   STEP 2: "Benesi pinetree" (same as BenesiPine1)
//     K = 2*x / sqrt(y²+z²)
//     new_x = x²-y²-z² + Cx
//     new_y = 2*K*y*z + Cy
//     new_z = K*(y²-z²) + Cz
//
// C handling: Formula handles C internally (adds Cx/Cy/Cz in step 2).
//   The engine must NOT add C after this formula returns.

template<>
cpu_double3 execute_formula<FORMULA_BENESIPINE2>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];  // (default: 2.0)
    const double xOff  = params[1];  // (default: 2.0)
    // [CONSTANTS] embedded as compile-time values (positive PVar offsets):
    const double p0 = 0.816496580927726;   // [PVar+0x00] sqrt(2/3)
    const double p1 = 0.577350269189626;   // [PVar+0x08] sqrt(1/3)
    const double p2 = 0.707106781186548;   // [PVar+0x10] sqrt(1/2)

    // C values passed by engine from context

    double x = p.x, y = p.y, z = p.z;

    // STEP 1a: Initial rotation and squaring
    double tx = (x * p0 - z * p1) * p2;
    y = y * p2;
    z = x * p1 + z * p0;
    z = z * z;                    // z = (x*p1 + z*p0)²
    double tx2 = (tx - y) * (tx - y);  // tx2 = (tx-y)²
    double ty2 = (tx + y) * (tx + y);  // ty2 = (tx+y)²

    // STEP 1b: sqrt/abs/scale
    x = fabs(sqrt(ty2 + z) - xOff) * Scale * p2;
    y = fabs(sqrt(tx2 + z) - xOff) * Scale * p2;
    z = fabs(sqrt(tx2 + ty2) - xOff) * Scale;

    // STEP 1c: Second rotation
    tx = y + x;
    y  = y - x;
    x  = tx * p0 + z * p1;
    z  = z * p0 - tx * p1;

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
constant FormulaInfo formula_info_BenesiPine2 = {
    .id = FORMULA_BENESIPINE2,
    .name = "BenesiPine2",
    .param_count = 2,
    .complexity = 298,
    .uses_transcendental = true
};

/*
Original M3 Description:
The cool Benesi pinetree formula variant 2. Use in conjunction with
BenesiPine1 or other formulas. (Three iters of pine1 + one of pine2)

STEP 1: "Benesi fold 2" - rotation with squaring, sqrt/abs/scale, rotation back
  tx=(x*sqrt(2/3)-z*sqrt(1/3))*sqrt(1/2)
  y=y*sqrt(1/2)
  z=(x*sqrt(1/3)+z*sqrt(2/3))²
  tx2=(tx-y)²,  ty2=(tx+y)²
  x=|sqrt(ty2+z)-offset|*Scale*sqrt(1/2)
  y=|sqrt(tx2+z)-offset|*Scale*sqrt(1/2)
  z=|sqrt(tx2+ty2)-offset|*Scale
  tx=y+x; y=y-x
  x=tx*sqrt(2/3)+z*sqrt(1/3)
  z=z*sqrt(2/3)-tx*sqrt(1/3)
STEP 2: "Benesi pinetree" (same as Pine1)
  K=2*x/sqrt(y²+z²);  x=x²-y²-z²;  y=2*K*y*z;  z=K*(y²-z²)

NOTE: This formula handles C internally. The engine must NOT add C.

Parameters:
  [0] Scale: double (default: 2.0)
  [1] xOff: double (default: 2.0)
  [2] Double: double (default: 0.816496580927726 = sqrt(2/3))
  [3] Double: double (default: 0.5773502691896258 = sqrt(1/3))
  [4] Double: double (default: 0.7071067811865475 = sqrt(1/2))
  [5..7] Cx, Cy, Cz: passed by engine from context
*/
