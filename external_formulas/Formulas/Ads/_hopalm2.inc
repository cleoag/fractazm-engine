// _hopalm2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _hopalm2
// Hand-ported from ASM: 446 bytes, DEoption=-1

constexpr unsigned int FORMULA__HOPALM2 = 7049;

// ASM trace:
//   8BD8/8B7630    ebx=&x, esi=PVar
//   [esi-0x10]=Strength, [esi-0x18]=α, [esi-0x20]=β, [esi-0x28]=γ,
//   [esi-0x30]=δ, [esi-0x38]=ε
//
//   DD03/DC4EE0       β*x
//   DD01/DC4ED8/D9E0  -γ*z
//   DEC1/DC46D0       βx - γz + δ = t
//   D9E8/D9F3         1; fpatan → atan(t)
//   DC02              + y → x' = y + atan(t)
//
//   DD46E8/DC23       α - x
//   DC4EF0            * Strength → y'
//
//   DD46E8/DC0A       α*y
//   DD03/DC4EE0/D9E0  -β*x
//   DEC1/DC4EC8       (αy - βx)*ε
//   DC01              + z → z'
//
// Modified hopalong with atan (smoother than sqrt version).

template<>
cpu_double3 execute_formula<FORMULA__HOPALM2>(cpu_double3 p, constant double* params, double &de) {
    double x = p.x, y = p.y, z = p.z;

    const double Strength = params[0];  // default: 1.0
    const double Alpha    = params[1];  // default: -0.4
    const double Beta     = params[2];  // default: 1.0
    const double Gamma    = params[3];  // default: 0.1
    const double Delta    = params[4];  // default: 0.1
    const double Epsilon  = params[5];  // default: 0.2

    double t = Beta * x - Gamma * z + Delta;
    double new_x = y + atan(t);
    double new_y = (Alpha - x) * Strength;
    double new_z = z + (Alpha * y - Beta * x) * Epsilon;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__hopalm2 = {
    .id = FORMULA__HOPALM2,
    .name = "_hopalm2",
    .param_count = 6,
    .complexity = 90,
    .uses_transcendental = true
};

/*
Original M3 Description:
Hopalong (modified) system in 3D with five parameters and strength.
Uses atan instead of +-sqrt(abs) for smoother results.
Formula by Luca GN 2011.
Works well in combination with many formulas, gives a nice "rusty" look.

t = β*x - γ*z + δ
x' = y + atan(t)
y' = (α - x) * Strength
z' = z + (α*y - β*x) * ε

Note: Description says "y - atan(t)" but ASM computes y + atan(t).

Parameters:
  [0] Strength: double (default: 1.0)
  [1] Alpha: double (default: -0.4)
  [2] Beta: double (default: 1.0)
  [3] Gamma: double (default: 0.1)
  [4] Delta: double (default: 0.1)
  [5] Epsilon: double (default: 0.2)
*/
