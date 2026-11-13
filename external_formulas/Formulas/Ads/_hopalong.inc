// _hopalong.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _hopalong
// Hand-ported from ASM: 370 bytes, DEoption=-1

constexpr unsigned int FORMULA__HOPALONG = 7051;

// ASM trace:
//   ebx=&x, esi=PVar
//   [esi-0x10]=Str, [esi-0x18]=α, [esi-0x20]=β, [esi-0x28]=γ, [esi-0x30]=δ, [esi-0x38]=ε
//
//   t = β*x - γ*z + δ
//   D9C0/D9E1/D9FA  |t|; sqrt(|t|)
//   D9C9/D9EE/DED9  push 0; fcompp: compare 0 vs t
//   DFE0/80E441/7F02  test; jg +2 (skip fchs if t >= 0)
//   D9E0            fchs → negate sqrt if t < 0
//   DC02            + y → x' = y + sign(t)*sqrt(|t|)
//
//   y' = (α - x) * Str
//   z' = z + (α*y - β*x) * ε
//
// Hopalong attractor in 3D (sqrt version, rougher than _hopalm2 atan version).

template<>
cpu_double3 execute_formula<FORMULA__HOPALONG>(cpu_double3 p, constant double* params, double &de) {
    double x = p.x, y = p.y, z = p.z;

    const double Strength = params[0];  // default: 1.0
    const double Alpha    = params[1];  // default: -0.4
    const double Beta     = params[2];  // default: 1.0
    const double Gamma    = params[3];  // default: 0.1
    const double Delta    = params[4];  // default: 0.1
    const double Epsilon  = params[5];  // default: 0.2

    double t = Beta * x - Gamma * z + Delta;
    double new_x = y + sign(t) * sqrt(abs(t));
    double new_y = (Alpha - x) * Strength;
    double new_z = z + (Alpha * y - Beta * x) * Epsilon;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__hopalong = {
    .id = FORMULA__HOPALONG,
    .name = "_hopalong",
    .param_count = 6,
    .complexity = 90,
    .uses_transcendental = false
};

/*
Original M3 Description:
Hopalong system in 3D with five parameters and strength.
Uses sqrt(abs) for the hop (rougher look than _hopalm2 atan variant).
Formula by Luca GN 2011.
Works very well in combination with many formulas, gives a nice "rusty" look.

t = β*x - γ*z + δ
x' = y + sign(t)*sqrt(|t|)
y' = (α - x) * Strength
z' = z + (α*y - β*x) * ε

Note: ASM computes y + sign(t)*sqrt(|t|). Description says y - sign(t)*sqrt(|t|).
Metal follows the ASM.

Parameters:
  [0] Strength: double (default: 1.0)
  [1] Alpha: double (default: -0.4)
  [2] Beta: double (default: 1.0)
  [3] Gamma: double (default: 0.1)
  [4] Delta: double (default: 0.1)
  [5] Epsilon: double (default: 0.2)
*/
