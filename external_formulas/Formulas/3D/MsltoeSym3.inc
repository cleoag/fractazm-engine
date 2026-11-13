// MsltoeSym3.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MsltoeSym3
// Hand-ported from ASM: 119 bytes, DEoption=0

constexpr unsigned int FORMULA_MSLTOESYM3 = 7072;

// ASM trace:
//   Sort y,z by |value|: if |y| <= |z|, swap(y,z)
//   After sort: y has larger |abs| (=A), z has smaller (=B)
//   If y >= z: negate BOTH x AND z  (differs from Sym2 which only negates x)
//   T = 1 - z²/r²    (r² = x²+y²+z², pre-computed by engine)
//   new_x = (x² - y²) * T         (+Cx in ASM)
//   S = sqrt(y² + x²)
//   new_z = 2 * z * S              (+Cz in ASM - unlike MsltoeSym2!)
//   new_y = 2 * y * T * x * Y_mul (+Cy in ASM)
//
// Msltoe formula variant 3. Differs from Sym2:
// 1. Negates both x AND z when y >= z (Sym2 only negates x)
// 2. Adds Cz to z component (Sym2 does not)

template<>
cpu_double3 execute_formula<FORMULA_MSLTOESYM3>(cpu_double3 p, constant double* params, double &de) {
    const double Y_mul = params[0];  // default: 1.0

    double x = p.x, y = p.y, z = p.z;
    double r2 = x*x + y*y + z*z;

    // Sort: put larger |abs| value in y position
    if (abs(y) <= abs(z)) {
        double t = y; y = z; z = t;
    }
    // Sign adjustment: negate BOTH x and z if y >= z
    if (y >= z) {
        x = -x;
        z = -z;
    }

    double T = 1.0 - z*z / r2;
    double x2 = x*x;
    double y2 = y*y;

    double new_x = (x2 - y2) * T;
    double S = sqrt(y2 + x2);
    double new_z = 2.0 * z * S;
    double new_y = 2.0 * y * T * x * Y_mul;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_MsltoeSym3 = {
    .id = FORMULA_MSLTOESYM3,
    .name = "MsltoeSym3",
    .param_count = 1,
    .complexity = 119,
    .uses_transcendental = false
};

/*
Original M3 Description:
Variant of MsltoeSym2 with different symmetry folding.
If y > z then {x=-x, z=-z} (Sym2 only negates x).
Adds Cz to z component (Sym2 does not).
Good for julia mode with x, y, and z seeds.

C handling: Cx added to x, Cy added to y, Cz added to z (all three).

Parameters:
  [0] Y multiplier: double (default: 1.0)
*/
