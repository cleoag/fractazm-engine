// _BT4_Transform4.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _BT4_Transform4
// Hand-ported from ASM: 203 bytes, DEoption=-1

constexpr unsigned int FORMULA__BT4_TRANSFORM4 = 7103;

// ASM trace (constants c0=sqrt(2/3), c1=sqrt(1/3), c2=sqrt(1/2)):
//   Phase 1: rotation
//     t = (x*c0 - z*c1)*c2
//     y_s = y*c2
//     R = x*c1 + z*c0
//   Phase 2: square (like T2), but NO sqrt (like T3)
//     A = (t+y_s)²,  B = (t-y_s)²,  C = R²
//     x4 = |A+C - Offset| * Scale * c2
//     y4 = |B+C - Offset| * Scale * c2
//     z4 = |A+B - Offset| * Scale
//   Phase 3: second rotation
//     s = (x4+y4)*c2,  d = (y4-x4)*c2
//     out_x = s*c0 + z4*c1
//     out_z = z4*c0 - s*c1
//     out_y = d
//
// "Hollowing out" transform by M.Benesi.
// Combines T2 squaring with T3's removal of sqrt.
// C handling: Transform (DEoption=-1), no C involvement.

template<>
cpu_double3 execute_formula<FORMULA__BT4_TRANSFORM4>(cpu_double3 p, constant double* params, double &de) {
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

    // Phase 2: square, sum (no sqrt!), offset, abs, scale
    double A = (t + y_s) * (t + y_s);
    double B = (t - y_s) * (t - y_s);
    double C = R * R;

    double x4 = abs(A + C - Offset) * Scale * c2;
    double y4 = abs(B + C - Offset) * Scale * c2;
    double z4 = abs(A + B - Offset) * Scale;

    // Phase 3: second rotation
    double s = x4 + y4;   // x4,y4 already include *c2 from Phase 2
    double d = y4 - x4;
    double new_x = s * c0 + z4 * c1;
    double new_z = z4 * c0 - s * c1;
    double new_y = d;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__BT4_Transform4 = {
    .id = FORMULA__BT4_TRANSFORM4,
    .name = "_BT4_Transform4",
    .param_count = 2,
    .complexity = 203,
    .uses_transcendental = false
};

/*
Original M3 Description:
"Hollowing out" transform by M.Benesi.
Use sparingly (every few iterations, maybe the next to last
iteration you want to increase the detail level of).

Same as T2 but with sqrt removed from the reduction step:
tx=(x*sqrt(2/3)-z*sqrt(1/3))*sqrt(1/2);
y=y*sqrt(1/2);
z=(x*sqrt(1/3) + z*sqrt(2/3))^2;
tx=(tx-y)^2;
ty=(tx+y)^2;

x=abs((ty+z)-offset)*scale*sqrt(1/2);  // no square roots!
y=abs((tx+z)-offset)*scale*sqrt(1/2);
z=abs((tx+ty)-offset)*scale;

tx=y+x; y=y-x;
x=z*sqrt(1/3)+tx*sqrt(2/3);
z=z*sqrt(2/3)-tx*sqrt(1/3);

Parameters:
  [0] Scale: double (default: 2.0)
  [1] Offset: double (default: 2.0)
*/
