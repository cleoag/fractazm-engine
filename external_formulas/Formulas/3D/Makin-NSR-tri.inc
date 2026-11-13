// Makin-NSR-tri.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Makin-NSR-tri
// Hand-ported from ASM: 211 bytes, DEoption=0

constexpr unsigned int FORMULA_MAKIN_NSR_TRI = 7099;

// ASM trace:
//   Three angle pairs, each Power-scaled:
//     phi_yx = Power * atan2(y, x)   → cos_yx, sin_yx
//     phi_zy = Power * atan2(z, y)   → cos_zy, sin_zy
//     phi_xz = Power * atan2(x, z)   → cos_xz, sin_xz
//   Direction from cross-products of sin/cos:
//     x_dir = cos_yx * sin_xz
//     y_dir = sin_yx * cos_zy
//     z_dir = sin_zy * cos_xz
//   Normalize and scale:
//     r_P = |r|^Power   where r = sqrt(x²+y²+z²)
//     r_dir = sqrt(x_dir²+y_dir²+z_dir²)
//     scale = r_P / r_dir
//   Output:
//     new_x = x_dir * scale + Cx
//     new_y = y_dir * scale + Cy
//     new_z = z_dir * scale * zScale + Cz
//
// C handling: STANDARD — engine adds C after return.
// (z output has extra zScale multiplication)

template<>
cpu_double3 execute_formula<FORMULA_MAKIN_NSR_TRI>(cpu_double3 p, constant double* params, double &de) {
    const double Power  = params[0];  // exponent (default: 8)
    const double zScale = params[1];  // z multiplier (default: -1)

    double x = p.x, y = p.y, z = p.z;
    if (x == 0.0 && y == 0.0 && z == 0.0) {
        return cpu_double3(NAN, NAN, NAN);
    }
    if (!isfinite(x) || !isfinite(y) || !isfinite(z)) {
        return cpu_double3(NAN, NAN, NAN);
    }

    // Three Power-scaled angle pairs
    double phi_yx = Power * atan2(y, x);
    double cos_yx = cos(phi_yx);
    double sin_yx = sin(phi_yx);

    double phi_zy = Power * atan2(z, y);
    double cos_zy = cos(phi_zy);
    double sin_zy = sin(phi_zy);

    double phi_xz = Power * atan2(x, z);
    double cos_xz = cos(phi_xz);
    double sin_xz = sin(phi_xz);

    // Direction from cross-products of sin/cos
    double x_dir = cos_yx * sin_xz;
    double y_dir = sin_yx * cos_zy;
    double z_dir = sin_zy * cos_xz;

    // Radius power
    double r = sqrt(x * x + y * y + z * z);
    double r_P = pow(abs(r), Power);
    if (!isfinite(r) || !isfinite(r_P)) {
        return cpu_double3(NAN, NAN, NAN);
    }

    // Normalize direction and scale by r^Power
    double r_dir = sqrt(x_dir * x_dir + y_dir * y_dir + z_dir * z_dir);
    if (!isfinite(r_dir) || r_dir == 0.0) {
        return cpu_double3(NAN, NAN, NAN);
    }
    double scale = (r_dir > 1e-30) ? (r_P / r_dir) : 0.0;
    if (!isfinite(scale)) {
        return cpu_double3(NAN, NAN, NAN);
    }

    // Output (C added by engine for x,y; z has extra zScale)
    double new_x = x_dir * scale;
    double new_y = y_dir * scale;
    double new_z = z_dir * scale * zScale;
    if (!isfinite(new_x) || !isfinite(new_y) || !isfinite(new_z)) {
        return cpu_double3(NAN, NAN, NAN);
    }

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_Makin_NSR_tri = {
    .id = FORMULA_MAKIN_NSR_TRI,
    .name = "Makin-NSR-tri",
    .param_count = 2,
    .complexity = 211,
    .uses_transcendental = true
};

/*
Original M3 Description:
Makin NSR-tri formula — a Mandelbulb-like power formula using
three atan2 angle pairs instead of the traditional two.

Each pair of coordinates (y,x), (z,y), (x,z) produces an angle
that is multiplied by Power. The resulting sin/cos values are
cross-multiplied to form a direction vector, which is then
normalized and scaled by r^Power.

Proposed by an NSR variation. The extra zScale parameter
allows flipping or scaling the z-axis independently.

Parameters:
  [0] Power: double (default: 8)
  [1] zScale: double (default: -1)
*/
