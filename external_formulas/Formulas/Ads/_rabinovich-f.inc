// _rabinovich-f.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _rabinovich-f
// Hand-ported from ASM: 100 bytes, DEoption=-1

constexpr unsigned int FORMULA__RABINOVICH_F = 7070;

// ASM trace (Rabinovich-Fabrikant chaotic system):
//   new_x = (y*(x² + z - Alpha) + x*Gamma) * Strength
//   new_y = (x*(Alpha + z*Beta - x²) + y*Gamma) * Strength
//   new_z = ((Alpha - Beta)*z * (Delta + x*y)) * Strength
//
// Generalized Rabinovich-Fabrikant system.
// Standard RF: dx=y(z-1+x²)+γx, dy=x(3z+1-x²)+γy, dz=-2z(α+xy)

template<>
cpu_double3 execute_formula<FORMULA__RABINOVICH_F>(cpu_double3 p, constant double* params, double &de) {
    const double Strength = params[0];  // default: small (e.g. 0.01)
    const double Alpha    = params[1];  // default: 1.0
    const double Beta     = params[2];  // default: 0.1 (or 3.0)
    const double Gamma    = params[3];  // default: 0.87
    const double Delta    = params[4];  // default: -2.0

    double x = p.x, y = p.y, z = p.z;
    double x2 = x * x;

    double new_x = (y * (x2 + z - Alpha) + x * Gamma) * Strength;
    double new_y = (x * (Alpha + z * Beta - x2) + y * Gamma) * Strength;
    double new_z = ((Alpha - Beta) * z * (Delta + x * y)) * Strength;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__rabinovich_f = {
    .id = FORMULA__RABINOVICH_F,
    .name = "_rabinovich-f",
    .param_count = 5,
    .complexity = 100,
    .uses_transcendental = false
};

/*
Original M3 Description:
Generalized Rabinovich-Fabrikant chaotic system in 3D.

x' = (y*(x² + z - α) + x*γ) * Strength
y' = (x*(α + z*β - x²) + y*γ) * Strength
z' = ((α - β)*z * (δ + x*y)) * Strength

Standard RF system: α=1, β=0.1, γ=0.87, δ=-2, Strength=small.

Parameters:
  [0] Strength: double (default: small step size)
  [1] Alpha: double (default: 1.0)
  [2] Beta: double (default: 0.1)
  [3] Gamma: double (default: 0.87)
  [4] Delta: double (default: -2.0)
*/
