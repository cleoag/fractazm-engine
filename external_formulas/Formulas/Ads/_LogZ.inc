// _LogZ.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _LogZ
// Hand-ported from ASM: 432 bytes, DEoption=-1

constexpr unsigned int FORMULA__LOGZ = 7021;

// ASM trace: Same as _LogX but operates on ecx (z).
// z_temp = Mul * z + fixZ
// z' = sign(z_temp) * Base * log2(|z_temp| + |fixLg|)

template<>
cpu_double3 execute_formula<FORMULA__LOGZ>(cpu_double3 p, constant double* params, double &de) {
    const double fixZ  = params[0];
    const double fixLg = params[1];
    const double Mul   = params[2];
    const double Base  = params[3];

    double zp = Mul * p.z + fixZ;
    double s = std::signbit(zp) ? -1.0 : 1.0;
    double result = s * Base * log2(abs(zp) + abs(fixLg));

    return cpu_double3(p.x, p.y, result);
}

// Alternative direct function implementation
cpu_double3 _LogZ_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__LOGZ>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__LogZ = {
    .id = FORMULA__LOGZ,
    .name = "_LogZ",
    .param_count = 4,
    .complexity = 85,
    .uses_transcendental = true
};

/*
Original M3 Description:
Logarithm for z (formula by Luca GN 2013):
z_temp = Mul * z + fixZ
z' = sign(z_temp) * Base * log2(|z_temp| + |fixLg|)

Parameters:
  [0] fixZ: double (default: 0.0)
  [1] fixLg: double (default: 0.01)
  [2] Mul: double (default: 1.0)
  [3] Base: double (default: 0.367879441)
*/
