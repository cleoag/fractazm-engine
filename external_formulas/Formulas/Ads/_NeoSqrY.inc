// _NeoSqrY.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _NeoSqrY
// Hand-ported from ASM: 530 bytes, DEoption=-1

constexpr unsigned int FORMULA__NEOSQRY = 7033;

// ASM trace:
//   DD02        fld [edx]           → y
//   DC4EE0      fmul [esi-0x20]     → y*Mul
//   DC46F0      fadd [esi-0x10]     → A = y*Mul + fixY
//   D9C0/D9EE   dup A, push 0
//   DED9        fcompp              → compare 0 vs A
//   DFE0/80E441/7F0D  test and branch
//
//   A >= 0 path:  A*(A - fixSq)
//   A < 0  path:  A*(fixSq - A)
//
//   DC76D8      fdiv [esi-0x28]     → / Div
//   DC66F0      fsub [esi-0x10]     → - fixY
//   DD1A        fstp [edx]          → y'
//
// NeoSqr(A, B):
//   if A >= 0: A*(A-B)
//   else:      A*(B-A)
//
// y' = NeoSqr(Mul*y + fixY, fixSq) / Div - fixY

template<>
cpu_double3 execute_formula<FORMULA__NEOSQRY>(cpu_double3 p, constant double* params, double &de) {
    const double fixY  = params[0];
    const double fixSq = params[1];
    const double Mul   = params[2];
    const double Div   = params[3];

    double A = Mul * p.y + fixY;
    double neoSqr;
    if (A >= 0.0) {
        neoSqr = A * (A - fixSq);
    } else {
        neoSqr = A * (fixSq - A);
    }
    double result = neoSqr / Div - fixY;

    return cpu_double3(p.x, result, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__NeoSqrY = {
    .id = FORMULA__NEOSQRY,
    .name = "_NeoSqrY",
    .param_count = 4,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
NeoSqr transform for y (Luca GN 2011):
A = Mul * y + fixY
NeoSqr(A, B) = (A >= 0) ? A*(A-B) : A*(B-A)
y' = NeoSqr(A, fixSq) / Div - fixY

Parameters:
  [0] fixY: double (default: 0.0)
  [1] fixSq: double (default: 1.0)
  [2] Mul: double (default: 1.0)
  [3] Div: double (default: 1.0)
*/
