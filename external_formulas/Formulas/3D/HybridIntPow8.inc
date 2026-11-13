// HybridIntPow8.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.
// Ported from Pascal HybridItIntPow8 (formulas.pas:4709)
// DEoption: 0 (standard log-DE, engine adds C after return)
// Hand-ported from ASM

// "White's Mandelbulb Power 8" — polynomial form (NOT spherical coordinates).
// The iconic 3D fractal discovered by Daniel White and Paul Nylander (2009).
// This is the correct tricomplex multiplication form, distinct from the naive
// spherical-coordinates approximation.
//
// ASM trace summary:
//   r    = x*x + y*y  (2D radius squared, NOT full 3D)
//   rr   = r*r
//   sz   = z*z
//   zzzz = sz*sz
//
//   z_new = 8 * dZmul * z * sqrt(r) * (r - sz) * (zzzz - 6*r*sz + rr)
//   a     = 1 + (zzzz*(70*rr + zzzz) - 28*sz*r*(zzzz + rr)) / (rr*rr + 1e-40)
//   y_new = 8 * a * x * y * (sx*sx*(sx - 7*sy) + sy*sy*(7*sx - sy))
//   x_new = a * (sx*sx*sx*sx + sy*sy*(70*sx*sx + sy*sy) - 28*sx*sy*(sy*sy + sx*sx))
//
// Verification: x formula = Re((x+iy)^8) = x^8 + 70x^4y^4 + y^8 - 28x^6y^2 - 28x^2y^6  ✓
//
// Engine adds Cx, Cy, Cz after return (DEoption=0).

constexpr unsigned int FORMULA_HYBRIDINTPOW8 = 90008;

template<>
cpu_double3 execute_formula<FORMULA_HYBRIDINTPOW8>(cpu_double3 p, constant double* params, double &de) {
    (void)de;
    const double dZmul = params[0];

    const double sx   = p.x * p.x;
    const double sy   = p.y * p.y;
    const double sz   = p.z * p.z;
    const double zzzz = sz * sz;
    const double r    = sx + sy;     // 2D radius squared
    const double rr   = r * r;

    // z component (ASM: fchs = negate applied, giving the form below)
    const double new_z = 8.0 * dZmul * p.z * sqrt(r) * (r - sz) * (zzzz - 6.0 * r * sz + rr);

    // Factor a for x and y components
    const double a = 1.0 + (zzzz * (70.0 * rr + zzzz) - 28.0 * sz * r * (zzzz + rr)) / (rr * rr + 1e-40);

    // x component: Re((x+iy)^8) scaled by a
    const double new_x = a * (sx*sx*sx*sx + sy*sy*(70.0*sx*sx + sy*sy) - 28.0*sx*sy*(sy*sy + sx*sx));

    // y component: 8*Im((x+iy)^8)/xy form, scaled by a
    const double new_y = 8.0 * a * p.x * p.y * (sx*sx*(sx - 7.0*sy) + sy*sy*(7.0*sx - sy));

    return cpu_double3(new_x, new_y, new_z);
}

cpu_double3 HybridIntPow8_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HYBRIDINTPOW8>(p, params, de);
}

constant FormulaInfo formula_info_HybridIntPow8 = {
    .id = FORMULA_HYBRIDINTPOW8,
    .name = "HybridIntPow8",
    .param_count = 1,
    .complexity = 180,
    .uses_transcendental = false
};

/*
Original Pascal source: formulas.pas:4709, fHIntFunctions[8]
"White's Mandelbulb Power 8" polynomial form (tricomplex multiplication).

sx = x*x, sy = y*y, sz = z*z, zzzz = sz*sz
r  = sx + sy  (2D radius squared)
rr = r*r

z' = 8 * dZmul * z * sqrt(r) * (r - sz) * (zzzz - 6*r*sz + rr)
a  = 1 + (zzzz*(70*rr + zzzz) - 28*sz*r*(zzzz + rr)) / (rr*rr + 1e-40)
x' = a * (sx^4 + sy^2*(70*sx^2 + sy^2) - 28*sx*sy*(sy^2 + sx^2))
y' = 8 * a * x * y * (sx^2*(sx - 7*sy) + sy^2*(7*sx - sy))

params[0] = dZmul (z-multiplier, default 1.0)
Engine adds Cx, Cy, Cz after return (DEoption=0).
*/
