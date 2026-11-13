// _reciprocalZ3.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _reciprocalZ3
// Hand-ported from ASM: 420 bytes, DEoption=-1

constexpr unsigned int FORMULA__RECIPROCALZ3 = 7018;

// ASM trace: Same as _reciprocalX3 but operates on ecx (z).
// z' = sign(z) * (1/Lim - 1/(|z| + Lim))

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Reciprocal3Original_x87(double*,const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA__RECIPROCALZ3>(cpu_double3 p, constant double* params, double &de) {
    double value = p.z;
#if defined(_MSC_VER) && defined(_M_X64)
    Reciprocal3Original_x87(&value,params);
#else
    const numeric::X87Scope extendedScope;
    const bool negative = signbit(value);
    const numeric::X87Real denominator = numeric::x87Abs((numeric::X87Real)value) + params[0];
    value = double((numeric::X87Real)params[1] - numeric::X87Real(1.) / denominator);
    if (negative) value = -value;
#endif
    p.z = value;
    return p;

}

// Alternative direct function implementation
cpu_double3 _reciprocalZ3_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__RECIPROCALZ3>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__reciprocalZ3 = {
    .id = FORMULA__RECIPROCALZ3,
    .name = "_reciprocalZ3",
    .param_count = 2,
    .complexity = 80,
    .uses_transcendental = false
};

/*
Original M3 Description:
Computes the reciprocal of z with a continuous function:
z' = sign(z) * (1/Lim - 1/(abs(z) + Lim))

Parameters:
  [0] Limiter: double (default: 0.5)
*/
