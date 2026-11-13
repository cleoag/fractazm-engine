// _tospherical.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _tospherical
// Hand-ported from ASM: 180 bytes, DEoption=-1

constexpr unsigned int FORMULA__TOSPHERICAL = 2345;

// ASM trace (Luca GN 2011):
//   r = sqrt(x² + y² + z²)          // radius
//   theta = atan2(sqrt(x² + y²), z)  // polar angle from z-axis
//   phi = atan2(y, x)                // azimuthal angle
//   x' = r, y' = theta, z' = phi
//
// Cartesian to spherical coordinate conversion. No parameters.
// Uses fpatan for atan2 (handles full angle range).

template<>
cpu_double3 execute_formula<FORMULA__TOSPHERICAL>(cpu_double3 p, constant double* params, double &de) {
    double x = p.x, y = p.y, z = p.z;

    double x2 = x * x, y2 = y * y, z2 = z * z;

    // Radius
    double r = sqrt(x2 + y2 + z2);

    // Polar angle (from z-axis)
    double rxy = sqrt(x2 + y2);
    double theta = atan2(rxy, z);

    // Azimuthal angle
    double phi = atan2(y, x);

    return cpu_double3(r, theta, phi);
}

// Alternative direct function implementation
cpu_double3 _tospherical_formula(cpu_double3 p, constant double* params, double &de) {
    double x = p.x, y = p.y, z = p.z;
    double r = sqrt(x*x + y*y + z*z);
    return cpu_double3(r, atan2(sqrt(x*x+y*y), z), atan2(y, x));
}

// Formula metadata
constant FormulaInfo formula_info__tospherical = {
    .id = FORMULA__TOSPHERICAL,
    .name = "_tospherical",
    .param_count = 0,
    .complexity = 180,
    .uses_transcendental = true
};

/*
Original M3 Description:
Spherical coords. Fast conversion without parameters.
Formula by Luca GN 2011

From Wikipedia:
x' = sqrt(x² + y² + z²)         // r
y' = atan2(sqrt(x² + y²), z)    // theta (polar)
z' = atan2(y, x)                 // phi (azimuthal)

Parameters: none
*/
