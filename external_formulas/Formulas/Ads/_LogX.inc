// _LogX.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _LogX
// Hand-ported from ASM: 428 bytes, DEoption=-1

constexpr unsigned int FORMULA__LOGX = 7019;

// ASM trace:
//   DD03        fld [ebx]           → x
//   DD47E0      fld [edi-0x20]      → Mul
//   DEC9        fmulp st1,st0       → x * Mul
//   DD47F0      fld [edi-0x10]      → fixX
//   DEC1        faddp st1,st0       → x*Mul + fixX = x'
//   DD13        fst [ebx]           → store x' (for sign extraction)
//   8A6307      mov ah,[ebx+7]      → sign byte
//   80E480      and ah, 0x80        → isolate sign
//   D9E1        fabs                → |x'|
//   DD47E8      fld [edi-0x18]      → fixLg
//   D9E1        fabs                → |fixLg|
//   DEC1        faddp st1,st0       → |x'| + |fixLg|
//   DD47D8      fld [edi-0x28]      → Base
//   D9C9        fxch st1            → swap: |x'|+|fixLg| in st0, Base in st1
//   D9F1        fyl2x               → Base * log2(|x'|+|fixLg|)
//   DD1B        fstp [ebx]          → store result
//   306307      xor [ebx+7], ah     → re-apply sign
//
// Formula by Luca GN 2013
// x_temp = Mul * x + fixX
// x' = sign(x_temp) * Base * log2(|x_temp| + |fixLg|)

template<>
cpu_double3 execute_formula<FORMULA__LOGX>(cpu_double3 p, constant double* params, double &de) {
    const double fixX  = params[0];
    const double fixLg = params[1];
    const double Mul   = params[2];
    const double Base  = params[3];

    double xp = Mul * p.x + fixX;
    double s = std::signbit(xp) ? -1.0 : 1.0;
    double result = s * Base * log2(abs(xp) + abs(fixLg));

    return cpu_double3(result, p.y, p.z);
}

// Alternative direct function implementation
cpu_double3 _LogX_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__LOGX>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__LogX = {
    .id = FORMULA__LOGX,
    .name = "_LogX",
    .param_count = 4,
    .complexity = 85,
    .uses_transcendental = true
};

/*
Original M3 Description:
Logarithm for x (formula by Luca GN 2013):
x_temp = Mul * x + fixX
x' = sign(x_temp) * Base * log2(|x_temp| + |fixLg|)

Default Base = 1/e = 0.367879441

Parameters:
  [0] fixX: double (default: 0.0)
  [1] fixLg: double (default: 0.01)
  [2] Mul: double (default: 1.0)
  [3] Base: double (default: 0.367879441)
*/
