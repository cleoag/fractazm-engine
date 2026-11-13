// _gnarl2D.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _gnarl2D
// Hand-ported from ASM: 103 bytes, DEoption=-1

constexpr unsigned int FORMULA__GNARL2D = 7063;

// ASM trace:
//   helper(v) = sin(v + sin(Alpha * (v + sin(Beta * v))))
//
//   new_x = (x - X_step * helper(y)) * Strength
//   new_y = (y - Y_step * helper(x)) * Strength
//   z unchanged
//
// 2D gnarl attractor (Mark Townsend pattern). Heavy sin usage.

template<>
cpu_double3 execute_formula<FORMULA__GNARL2D>(cpu_double3 p, constant double* params, double &de) {
    const double Strength = params[0];  // default: 1.03
    const double X_step   = params[1];  // default: 0.1
    const double Y_step   = params[2];  // default: 0.1
    const double Alpha    = params[3];  // default: 3.0
    const double Beta     = params[4];  // default: 2.0

    double x = p.x, y = p.y;

    // helper(v) = sin(v + sin(Alpha * (v + sin(Beta * v))))
    double gy = sin(Beta * y);
    gy = sin(Alpha * (y + gy));
    gy = sin(y + gy);

    double gx = sin(Beta * x);
    gx = sin(Alpha * (x + gx));
    gx = sin(x + gx);

    double new_x = (x - X_step * gy) * Strength;
    double new_y = (y - Y_step * gx) * Strength;

    return cpu_double3(new_x, new_y, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__gnarl2D = {
    .id = FORMULA__GNARL2D,
    .name = "_gnarl2D",
    .param_count = 5,
    .complexity = 103,
    .uses_transcendental = true
};

/*
Original M3 Description:
2D Gnarl deformation (Mark Townsend formula, ported by Luca GN 2011).
Applies sinusoidal distortion in the xy plane. Z unchanged.

g(v) = sin(v + sin(Alpha * (v + sin(Beta * v))))

x' = (x - X_step * g(y)) * Strength
y' = (y - Y_step * g(x)) * Strength
z' = z

Parameters:
  [0] Strength: double (default: 1.03)
  [1] X step: double (default: 0.1)
  [2] Y step: double (default: 0.1)
  [3] Alpha: double (default: 3.0)
  [4] Beta: double (default: 2.0)
*/
