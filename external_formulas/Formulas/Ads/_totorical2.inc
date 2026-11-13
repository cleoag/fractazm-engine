// _totorical2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _totorical2
// Hand-ported from ASM: 403 bytes, DEoption=-1

constexpr unsigned int FORMULA__TOTORICAL2 = 7044;

// ASM trace:
//   8B7630        mov esi,[esi+0x30] → PVar
//   [esi-0x10] = R, [esi-0x18] = Gamma, [esi+0] = pi/2
//
//   DD00/D8C8/DD02/D8C8/DEC1/D9FA  sqrt(x²+y²)
//   DC66F0        fsub R → t = sqrt(x²+y²) - R
//   D9C0/D8C8     t²
//   DD01/D8C8     z²
//   DEC1/D9FA     sqrt(t²+z²) = x'
//   D9C1/DD01     push t, push z
//   D9F3          fpatan → atan2(t, z)
//   DC66E8        - Gamma
//   DC26          - pi/2  (from [CONSTANTS])
//   DD02/DD00     push y, push x
//   D9F3          fpatan → atan2(y, x) = z'
//   DD19/DD1A/DD18  store z', y', x'
//
// BradC toroidal coordinates (FractalForums).

template<>
cpu_double3 execute_formula<FORMULA__TOTORICAL2>(cpu_double3 p, constant double* params, double &de) {
    const double R     = params[0];  // default: 2.0
    const double Gamma = params[1];  // default: 0.0

    double t = sqrt(p.x * p.x + p.y * p.y) - R;

    double new_x = sqrt(t * t + p.z * p.z);
    double new_y = atan2(t, p.z) - Gamma - M_PI_2_F;
    double new_z = atan2(p.y, p.x);

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__totorical2 = {
    .id = FORMULA__TOTORICAL2,
    .name = "_totorical2",
    .param_count = 2,
    .complexity = 95,
    .uses_transcendental = true
};

/*
Original M3 Description:
Torical (generalized) coords by BradC (FractalForums).
Formula by Luca GN 2011.

t = sqrt(x² + y²) - R
x' = sqrt(t² + z²)
y' = atan2(t, z) - Gamma - pi/2
z' = atan2(y, x)

The -pi/2 offset makes this transform reversible with _invtorical2.

Parameters:
  [0] R: double (default: 2.0)
  [1] Gamma: double (default: 0.0)

[CONSTANTS]: pi/2 = 1.5707963267948966
*/
