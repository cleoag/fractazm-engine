// _reciprocalX3.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _reciprocalX3
// Hand-ported from ASM: 419 bytes, DEoption=-1

constexpr unsigned int FORMULA__RECIPROCALX3 = 7016;

// ASM trace:
//   DD00        fld [eax]           → x
//   8A5007      mov dl,[eax+7]      → save sign byte
//   D9E1        fabs                → |x|
//   DC46F0      fadd [esi-0x10]     → |x| + Limiter
//   80E280      and dl, 0x80        → isolate sign bit
//   D9E8        fld1                → 1.0
//   DEF1        fdivrp st1,st0      → 1/(|x|+Lim)
//   D9E0        fchs                → -1/(|x|+Lim)
//   DC46E8      fadd [esi-0x18]     → 1/Lim - 1/(|x|+Lim)
//   DD18        fstp [eax]          → store
//   305007      xor [eax+7], dl     → re-apply sign
//
// .DReci2 param: [PVar-0x10] = Limiter, [PVar-0x18] = 1/Limiter (pre-computed)
// x' = sign(x) * (1/Lim - 1/(|x| + Lim))
// Continuous function, no singularity at x=0 (returns 0 there).

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Reciprocal3Original_x87(double*,const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA__RECIPROCALX3>(cpu_double3 p, constant double* params, double &de) {
    double value = p.x;
#if defined(_MSC_VER) && defined(_M_X64)
    Reciprocal3Original_x87(&value,params);
#else
    const numeric::X87Scope extendedScope;
    const bool negative = signbit(value);
    const numeric::X87Real denominator = numeric::x87Abs((numeric::X87Real)value) + params[0];
    value = double((numeric::X87Real)params[1] - numeric::X87Real(1.) / denominator);
    if (negative) value = -value;
#endif
    p.x = value;
    return p;

}

// Alternative direct function implementation
cpu_double3 _reciprocalX3_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__RECIPROCALX3>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__reciprocalX3 = {
    .id = FORMULA__RECIPROCALX3,
    .name = "_reciprocalX3",
    .param_count = 2,
    .complexity = 80,
    .uses_transcendental = false
};

/*
Original M3 Description:
Computes the reciprocal of x with a continuous function:
x' = sign(x) * (1/Lim - 1/(abs(x) + Lim))

The DE is almost untouched, works fine with Raystep 0.5.
Don't go too low with the limiter to avoid noise.

Parameters:
  [0] Limiter: double (default: 0.5)
*/
