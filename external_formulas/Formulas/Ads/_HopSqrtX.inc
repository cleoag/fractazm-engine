// _HopSqrtX.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _HopSqrtX
// Hand-ported from ASM: 460 bytes, DEoption=-1

constexpr unsigned int FORMULA__HOPSQRTX = 7029;

// ASM trace:
//   DD03        fld [ebx]           → x
//   DC4EE0      fmul [esi-0x20]     → x*Mul
//   DC46F0      fadd [esi-0x10]     → t = x*Mul + fixX
//   D9C0        fld st0             → dup t
//   D9E1        fabs                → |t|
//   D9FA        fsqrt               → √|t|
//   D9C9        fxch st1            → swap: t on top
//   D9EE        fldz                → 0
//   DED9        fcompp              → compare 0 vs t, pop both
//   DFE0        fnstsw ax
//   80E441      and ah, 0x41
//   7F02        jg +2               → jump if t > 0 (skip fchs)
//   D9E0        fchs                → negate √|t|
//   DC76D8      fdiv [esi-0x28]     → / Div
//   DC46E8      fadd [esi-0x18]     → + fixSq
//   DD1B        fstp [ebx]          → store x'
//
// Sqrt for x as defined in Hopalong attractor:
// t = Mul * x + fixX
// x' = fixSq + sign(t) * sqrt(|t|) / Div

template<>
cpu_double3 execute_formula<FORMULA__HOPSQRTX>(cpu_double3 p, constant double* params, double &de) {
    const double fixX  = params[0];
    const double fixSq = params[1];
    const double Mul   = params[2];
    const double Div   = params[3];

    double t = Mul * p.x + fixX;
    double result = fixSq + sign(t) * sqrt(abs(t)) / Div;

    return cpu_double3(result, p.y, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__HopSqrtX = {
    .id = FORMULA__HOPSQRTX,
    .name = "_HopSqrtX",
    .param_count = 4,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
Sqrt for x as defined in Hopalong attractor (Luca GN 2011):
t = Mul * x + fixX
x' = fixSq + sign(t) * sqrt(|t|) / Div

Parameters:
  [0] fixX: double (default: 0.0)
  [1] fixSq: double (default: 0.0)
  [2] Mul: double (default: 1.0)
  [3] Div: double (default: 1.0)
*/
