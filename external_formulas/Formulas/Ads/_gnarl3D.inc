// _gnarl3D.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _gnarl3D
// Hand-ported from ASM: 147 bytes, DEoption=-1

constexpr unsigned int FORMULA__GNARL3D = 2727;

// ASM trace:
//   x' = (x - X_step * sin(z + sin(Alpha * (z + sin(Beta * z))))) * Strength
//   y' = (y - Y_step * sin(x + sin(Alpha * (x + sin(Beta * x))))) * Strength
//   z' = (z - Z_step * sin(y + sin(Alpha * (y + sin(Beta * y))))) * Strength
//
// Gnarl deformation in 3D (Mark Townsend / Luca GN 2011).
// Produces strange "smoke-like" shapes. Heavy sin usage - slow.

template<>
cpu_double3 execute_formula<FORMULA__GNARL3D>(cpu_double3 p, constant double* params, double &de) {
    const double Strength = params[0];
    const double X_step = params[1];
    const double Y_step = params[2];
    const double Z_step = params[3];
    const double Alpha = params[4];
    const double Beta = params[5];

    double x = p.x, y = p.y, z = p.z;

    // x' uses z-based sin chain
    double sx = sin(Beta * z);
    sx = sin(Alpha * (z + sx));
    sx = sin(z + sx);

    // y' uses x-based sin chain
    double sy = sin(Beta * x);
    sy = sin(Alpha * (x + sy));
    sy = sin(x + sy);

    // z' uses y-based sin chain
    double sz = sin(Beta * y);
    sz = sin(Alpha * (y + sz));
    sz = sin(y + sz);

    cpu_double3 result;
    result.x = (x - X_step * sx) * Strength;
    result.y = (y - Y_step * sy) * Strength;
    result.z = (z - Z_step * sz) * Strength;

    return result;
}

// Alternative direct function implementation
cpu_double3 _gnarl3D_formula(cpu_double3 p, constant double* params, double &de) {
    const double S = params[0], Xs = params[1], Ys = params[2], Zs = params[3];
    const double A = params[4], B = params[5];
    double x = p.x, y = p.y, z = p.z;

    double sx = sin(z + sin(A * (z + sin(B * z))));
    double sy = sin(x + sin(A * (x + sin(B * x))));
    double sz = sin(y + sin(A * (y + sin(B * y))));

    return cpu_double3((x - Xs * sx) * S,
                   (y - Ys * sy) * S,
                   (z - Zs * sz) * S);
}

// Formula metadata
constant FormulaInfo formula_info__gnarl3D = {
    .id = FORMULA__GNARL3D,
    .name = "_gnarl3D",
    .param_count = 6,
    .complexity = 147,
    .uses_transcendental = true
};

/*
Original M3 Description:
Gnarl deformation in 3D (2D formula is from Mark Townsend)
Formula by Luca GN 2011. Produces strange "smoke-like" shapes.
Handle with care - uses massive sin, slow and may be fuzzy.

x' = (x - stepX * sin(z + sin(a * (z + sin(b * z))))) * Strength
y' = (y - stepY * sin(x + sin(a * (x + sin(b * x))))) * Strength
z' = (z - stepZ * sin(y + sin(a * (y + sin(b * y))))) * Strength

Parameters:
  [0] Strength: double (default: 1.03)
  [1] X_step: double (default: 0.1)
  [2] Y_step: double (default: 0.1)
  [3] Z_step: double (default: 0.1)
  [4] Alpha: double (default: 3.0)
  [5] Beta: double (default: 2.0)
*/
