// _dynamic.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _dynamic
// Hand-ported from ASM: 126 bytes, DEoption=-1

constexpr unsigned int FORMULA__DYNAMIC = 7074;

// ASM trace:
//   new_x = (x - Alpha*cos(Gamma*y) - Beta*sin(Delta*z)) * Strength
//   new_y = (y - Alpha*cos(Gamma*z) - Beta*sin(Delta*x)) * Strength
//   new_z = (z - Alpha*cos(Gamma*x) - Beta*sin(Delta*y)) * Strength
//
// Cyclic 3D deformation by Luca GN (2011), inspired by
// Mark Townsend's 2D formula. Each axis is deformed via
// cos/sin of the other two axes. Can be very slow.

template<>
cpu_double3 execute_formula<FORMULA__DYNAMIC>(cpu_double3 p, constant double* params, double &de) {
    const double Strength = params[0];  // default: 1.0
    const double Alpha    = params[1];  // default: 0.1
    const double Beta     = params[2];  // default: 0.1
    const double Gamma    = params[3];  // default: 1.5
    const double Delta    = params[4];  // default: 1.5

    double x = p.x, y = p.y, z = p.z;

    double new_x = (x - Alpha * cos(Gamma * y) - Beta * sin(Delta * z)) * Strength;
    double new_y = (y - Alpha * cos(Gamma * z) - Beta * sin(Delta * x)) * Strength;
    double new_z = (z - Alpha * cos(Gamma * x) - Beta * sin(Delta * y)) * Strength;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__dynamic = {
    .id = FORMULA__DYNAMIC,
    .name = "_dynamic",
    .param_count = 5,
    .complexity = 126,
    .uses_transcendental = true
};

/*
Original M3 Description:
Dynamic 3D deformation by Luca GN (2011), inspired by
Mark Townsend's 2D formula. Each axis is deformed via
cos/sin of the other two axes. Can be very slow.

x' = (x - Alpha*cos(Gamma*y) - Beta*sin(Delta*z)) * Strength
y' = (y - Alpha*cos(Gamma*z) - Beta*sin(Delta*x)) * Strength
z' = (z - Alpha*cos(Gamma*x) - Beta*sin(Delta*y)) * Strength

Cyclic structure: x deformed by y,z; y by z,x; z by x,y.

Parameters:
  [0] Strength: double (default: 1.0)
  [1] Alpha: double (default: 0.1)
  [2] Beta: double (default: 0.1)
  [3] Gamma: double (default: 1.5)
  [4] Delta: double (default: 1.5)
*/
