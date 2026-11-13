// _reciprocalX3b.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _reciprocalX3b
// Hand-ported from ASM: 140 bytes, DEoption=-1

constexpr unsigned int FORMULA__RECIPROCALX3B = 4812;

// ASM trace:
//   sign = sign(x)
//   term1 = 1/Lim1 - 1/(|mul1*x| + Lim1)
//   term2 = 1/Lim2 - 1/(|mul2*x²| + Lim2)
//   x' = sign * (term1 + term2)
//
// Reciprocal-style compression of x. Continuous, bounded function.
// .DReci2 params store value AND precomputed 1/value at PVar offsets.
// PVar layout: Lim1, 1/Lim1, Lim2, 1/Lim2, mul1, mul2
// Sign is extracted from x's high byte and re-applied via XOR.

template<>
cpu_double3 execute_formula<FORMULA__RECIPROCALX3B>(cpu_double3 p, constant double* params, double &de) {
    // DReci2 expands each limiter into its value and reciprocal.
    const double value = p.x;
    double result = params[1] - 1.0 / (abs(params[4] * value) + params[0]);
    result += params[3] - 1.0 / (abs(params[5] * value * value) + params[2]);
    if (std::signbit(value)) result = -result;
    return cpu_double3(result, p.y, p.z);
}

// Alternative direct function implementation
cpu_double3 _reciprocalX3b_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__RECIPROCALX3B>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__reciprocalX3b = {
    .id = FORMULA__RECIPROCALX3B,
    .name = "_reciprocalX3b",
    .param_count = 4,
    .complexity = 140,
    .uses_transcendental = false
};

/*
Original M3 Description:
Computes a reciprocal-style function of x with a different (continuous) function,
fine-tuneable:

x' = sign(x)(1/Lim1 + 1/Lim2 - 1/(|m1*x| + Lim1) - 1/(m2*x² + Lim2))

The DE is almost untouched, works fine with Raystep 0.5.
Don't go too low with the limiter to avoid noise.

Note: .DReci2 params store value AND precomputed reciprocal.
PVar layout: [Lim1, 1/Lim1, Lim2, 1/Lim2, mul1, mul2]

Parameters:
  [0] Limiter1: double (default: 0.67)
  [1] Limiter2: double (default: 0.67)
  [2] mul1: double (default: 1)
  [3] mul2: double (default: 1)
*/
