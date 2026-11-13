// _BT5b_Transform5b.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _BT5b_Transform5b
// Hand-ported from ASM: 227 bytes, DEoption=-1

constexpr unsigned int FORMULA__BT5B_TRANSFORM5B = 7104;

// ASM trace (constants c0=sqrt(2/3), c1=sqrt(1/3), c2=sqrt(1/2)):
//   Phase 1: rotation (same as all Benesi transforms)
//     t = (x*c0 - z*c1)*c2
//     y_s = y*c2
//     R = x*c1 + z*c0
//   Phase 2: 8th power, then 4th root of sum
//     A = (t+y_s)^8,  B = (t-y_s)^8,  C = R^8
//     x5 = |(A+C)^(1/4) - Offset| * Scale * c2
//     y5 = |(B+C)^(1/4) - Offset| * Scale * c2
//     z5 = |(A+B)^(1/4) - Offset| * Scale
//   Phase 3: second rotation (same as all Benesi transforms)
//     s = (x5+y5)*c2,  d = (y5-x5)*c2
//     out_x = s*c0 + z5*c1
//     out_z = z5*c0 - s*c1
//     out_y = d
//
// Benesi T5b: Uses 8th power with 4th root (double sqrt).
// C handling: Transform (DEoption=-1), no C involvement.

template<>
cpu_double3 execute_formula<FORMULA__BT5B_TRANSFORM5B>(cpu_double3 p, constant double* params, double &de) {
    const double Scale  = params[0];  // (default: 2.0)
    const double Offset = params[1];  // (default: 2.0)

    const double c0 = 0.816496580927726;  // sqrt(2/3)
    const double c1 = 0.577350269189626;  // sqrt(1/3)
    const double c2 = 0.707106781186548;  // sqrt(1/2)

    double x = p.x, y = p.y, z = p.z;

    // Phase 1: rotation
    double t = (x * c0 - z * c1) * c2;
    double y_s = y * c2;
    double R = x * c1 + z * c0;

    // Phase 2: 8th power via repeated squaring
    double tp = t + y_s;
    double tp2 = tp * tp;  double tp4 = tp2 * tp2;  double A = tp4 * tp4;  // (t+y_s)^8

    double tm = t - y_s;
    double tm2 = tm * tm;  double tm4 = tm2 * tm2;  double B = tm4 * tm4;  // (t-y_s)^8

    double R2 = R * R;  double R4 = R2 * R2;  double C = R4 * R4;  // R^8

    // 4th root via double sqrt
    double x5 = abs(sqrt(sqrt(A + C)) - Offset) * Scale * c2;
    double y5 = abs(sqrt(sqrt(B + C)) - Offset) * Scale * c2;
    double z5 = abs(sqrt(sqrt(A + B)) - Offset) * Scale;

    // Phase 3: second rotation
    double s = x5 + y5;   // x5,y5 already include *c2 from Phase 2
    double d = y5 - x5;
    double new_x = s * c0 + z5 * c1;
    double new_z = z5 * c0 - s * c1;
    double new_y = d;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__BT5b_Transform5b = {
    .id = FORMULA__BT5B_TRANSFORM5B,
    .name = "_BT5b_Transform5b",
    .param_count = 2,
    .complexity = 227,
    .uses_transcendental = false
};

/*
Original M3 Description:
Benesi Transform 5b by M.Benesi.
Another variant in the Benesi transform family.

Like T2 but uses 8th power (three repeated squarings) instead
of 2nd power (one squaring), and 4th root (double sqrt) instead
of square root, for the intermediate reduction step.

This creates sharper creases in the fractal geometry compared
to the smoother T2 variant.

Parameters:
  [0] Scale: double (default: 2.0)
  [1] Offset: double (default: 2.0)
*/
