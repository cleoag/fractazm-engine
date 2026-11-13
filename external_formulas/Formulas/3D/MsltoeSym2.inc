// MsltoeSym2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MsltoeSym2
// Hand-ported from ASM: 110 bytes, DEoption=0

constexpr unsigned int FORMULA_MSLTOESYM2 = 7071;

// ASM trace:
//   Sort y,z by |value|: if |y| <= |z|, swap(y,z)
//   After sort: y has larger |abs| (=A), z has smaller (=B)
//   If y >= z: negate x
//   T = 1 - z²/r²    (r² = x²+y²+z², pre-computed by engine)
//   new_x = (x² - y²) * T         (+Cx in ASM)
//   S = sqrt(y² + x²)
//   new_z = 2 * z * S              (NO +Cz in ASM!)
//   new_y = 2 * y * T * x * Y_mul (+Cy in ASM)
//
// Msltoe formula from fractalforums.com.
// NOTE: Unlike MsltoeSym3, this variant does NOT add Cz to z.

template<>
cpu_double3 execute_formula<FORMULA_MSLTOESYM2>(cpu_double3 p, constant double* params, double &de) {
    const double Y_mul = params[0];  // default: 1.0
    const double Cx = params[1];
    const double Cy = params[2];

    double x = p.x, y = p.y, z = p.z;
    double r2 = x*x + y*y + z*z;

    // Sort: put larger |abs| value in y position
    if (abs(y) <= abs(z)) {
        double t = y; y = z; z = t;
    }
    // Sign adjustment: negate x if y >= z
    if (y >= z) x = -x;

    double T = 1.0 - z*z / r2;
    double x2 = x*x;
    double y2 = y*y;

    double new_x = (x2 - y2) * T + Cx;
    double S = sqrt(y2 + x2);
    double new_z = 2.0 * z * S;
    double new_y = 2.0 * y * T * x * Y_mul + Cy;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_MsltoeSym2 = {
    .id = FORMULA_MSLTOESYM2,
    .name = "MsltoeSym2",
    .param_count = 1,
    .complexity = 110,
    .uses_transcendental = false
};

/*
Original M3 Description:
Msltoe formula from fractalforums.com.
Symmetry via abs/exchange on y,z. Computes zr = 1 - z^2/r^2,
then applies quadratic-like mapping with Y_multiplier.
Good for julia mode.

NOTE: Does NOT add Cz to z component (unlike MsltoeSym3).
C handling: Cx added to x, Cy added to y, Cz NOT added to z.

Parameters:
  [0] Y multiplier: double (default: 1.0)
*/
