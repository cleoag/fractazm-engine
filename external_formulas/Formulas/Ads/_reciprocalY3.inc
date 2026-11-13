// _reciprocalY3.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _reciprocalY3
// Hand-ported from ASM: 419 bytes, DEoption=-1

constexpr unsigned int FORMULA__RECIPROCALY3 = 7017;

// ASM trace: Same as _reciprocalX3 but operates on edx (y).
// y' = sign(y) * (1/Lim - 1/(|y| + Lim))

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Reciprocal3Original_x87(double*,const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA__RECIPROCALY3>(cpu_double3 p, constant double* params, double &de) {
    double value = p.y;
#if defined(_MSC_VER) && defined(_M_X64)
    Reciprocal3Original_x87(&value,params);
#else
    const numeric::X87Scope extendedScope;
    const bool negative = signbit(value);
    const numeric::X87Real denominator = numeric::x87Abs((numeric::X87Real)value) + params[0];
    value = double((numeric::X87Real)params[1] - numeric::X87Real(1.) / denominator);
    if (negative) value = -value;
#endif
    p.y = value;
    return p;

}

// Alternative direct function implementation
cpu_double3 _reciprocalY3_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__RECIPROCALY3>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__reciprocalY3 = {
    .id = FORMULA__RECIPROCALY3,
    .name = "_reciprocalY3",
    .param_count = 2,
    .complexity = 80,
    .uses_transcendental = false
};

/*
Original M3 Description:
Computes the reciprocal of y with a continuous function:
y' = sign(y) * (1/Lim - 1/(abs(y) + Lim))

Parameters:
  [0] Limiter: double (default: 0.5)
*/
