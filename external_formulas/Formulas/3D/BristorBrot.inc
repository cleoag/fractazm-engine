// BristorBrot.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: BristorBrot
// Hand-ported from ASM: 295 bytes, DEscale=1, SIpower=2

constexpr unsigned int FORMULA_BRISTORBROT = 7037;

// ASM trace:
//   DD00/D8C8       x²
//   DD02/D8C8/D9E0  y²; -y²
//   DEC1            x² - y²
//   DD01/D8C8/D9E0  z²; -z²
//   DEC1            x² - y² - z²
//   DC4618/D9D0     +Cx; fnop
//   DD18            store x'
//
//   DD00/DC4FF0     x * X_mul
//   DD01/DC4FE0     z * Z_mul
//   DEC1            X_mul*x + Z_mul*z
//   DC0A            * y → y*(X_mul*x + Z_mul*z)
//   DC4620/D9D0     +Cy; fnop
//   DD1A            store y'
//
//   DD00/DC4FF0     x * X_mul
//   DD02/DC4FE8     y * Y_mul
//   DEC1            X_mul*x + Y_mul*y
//   DC09            * z → z*(X_mul*x + Y_mul*y)
//   DC4628/D9D0     +Cz; fnop
//   DD19/DD1A/DD18  store z', y', x'
//
// Doug Bristor formula from bugman123.com, with David Makin variation params.

template<>
cpu_double3 execute_formula<FORMULA_BRISTORBROT>(cpu_double3 p, double* params, double &de) {
    double x = p.x, y = p.y, z = p.z;

    const double X_mul = params[0];  // default: 2
    const double Y_mul = params[1];  // default: 1
    const double Z_mul = params[2];  // default: -1

    // C values appended by wrapper: params[3]=J1(Cx), params[4]=J2(Cy), params[5]=J3(Cz)
    const double Cx = params[3];
    const double Cy = params[4];
    const double Cz = params[5];

    // Pascal ASM adds C internally: x' += Cx, y' += Cy, z' += Cz
    double new_x = x*x - y*y - z*z + Cx;
    double new_y = y * (X_mul * x + Z_mul * z) + Cy;
    double new_z = z * (X_mul * x + Y_mul * y) + Cz;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_BristorBrot = {
    .id = FORMULA_BRISTORBROT,
    .name = "BristorBrot",
    .param_count = 3,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
An easy 3D formula from Doug Bristor (bugman123.com/Hypercomplex).
Additional user parameters added:

x' = x² - y² - z² + Cx
y' = y * (X_mul * x + Z_mul * z) + Cy
z' = z * (X_mul * x + Y_mul * y) + Cz

David Makin variation: X_mul=Y_mul=2, Z_mul=0.

Parameters:
  [0] X mul: double (default: 2)
  [1] Y mul: double (default: 1)
  [2] Z mul: double (default: -1)
*/
