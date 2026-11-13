// _LogY.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _LogY
// Hand-ported from ASM: 432 bytes, DEoption=-1

constexpr unsigned int FORMULA__LOGY = 7020;

// ASM trace: Same as _LogX but operates on edx (y).
// y_temp = Mul * y + fixY
// y' = sign(y_temp) * Base * log2(|y_temp| + |fixLg|)

template<>
cpu_double3 execute_formula<FORMULA__LOGY>(cpu_double3 p, constant double* params, double &de) {
    const double fixY  = params[0];
    const double fixLg = params[1];
    const double Mul   = params[2];
    const double Base  = params[3];

    double yp = Mul * p.y + fixY;
    double s = std::signbit(yp) ? -1.0 : 1.0;
    double result = s * Base * log2(abs(yp) + abs(fixLg));

    return cpu_double3(p.x, result, p.z);
}

// Alternative direct function implementation
cpu_double3 _LogY_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__LOGY>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__LogY = {
    .id = FORMULA__LOGY,
    .name = "_LogY",
    .param_count = 4,
    .complexity = 85,
    .uses_transcendental = true
};

/*
Original M3 Description:
Logarithm for y (formula by Luca GN 2013):
y_temp = Mul * y + fixY
y' = sign(y_temp) * Base * log2(|y_temp| + |fixLg|)

Parameters:
  [0] fixY: double (default: 0.0)
  [1] fixLg: double (default: 0.01)
  [2] Mul: double (default: 1.0)
  [3] Base: double (default: 0.367879441)
*/
