// _reciprocalY3b.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _reciprocalY3b
// Hand-ported from ASM: 140 bytes, DEoption=-1

constexpr unsigned int FORMULA__RECIPROCALY3B = 5823;

// Same as _reciprocalX3b but applied to Y axis.
// y' = sign(y)(1/Lim1 - 1/(|mul1*y|+Lim1) + 1/Lim2 - 1/(|mul2*y²|+Lim2))

template<>
cpu_double3 execute_formula<FORMULA__RECIPROCALY3B>(cpu_double3 p, constant double* params, double &de) {
    // DReci2 expands each limiter into its value and reciprocal.
    const double value = p.y;
    double result = params[1] - 1.0 / (abs(params[4] * value) + params[0]);
    result += params[3] - 1.0 / (abs(params[5] * value * value) + params[2]);
    if (std::signbit(value)) result = -result;
    return cpu_double3(p.x, result, p.z);
}

// Alternative direct function implementation
cpu_double3 _reciprocalY3b_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__RECIPROCALY3B>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__reciprocalY3b = {
    .id = FORMULA__RECIPROCALY3B,
    .name = "_reciprocalY3b",
    .param_count = 4,
    .complexity = 140,
    .uses_transcendental = false
};

/*
Original M3 Description:
Computes a reciprocal-style function of y (same as X3b but for Y axis).
y' = sign(y)(1/Lim1 + 1/Lim2 - 1/(|m1*y| + Lim1) - 1/(m2*y² + Lim2))

Parameters:
  [0] Limiter1: double (default: 0.67)
  [1] Limiter2: double (default: 0.67)
  [2] mul1: double (default: 1)
  [3] mul2: double (default: 1)
*/
