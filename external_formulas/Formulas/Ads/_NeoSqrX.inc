// _NeoSqrX.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _NeoSqrX
// Hand-ported from ASM: 190 bytes, DEoption=-1

constexpr unsigned int FORMULA__NEOSQRX = 1234;

// ASM trace (Luca GN 2011):
//   A = Mul * x + fixX
//   NeoSqr(A, B):
//     if A >= 0: result = A * (A - B)
//     else:      result = A * (B - A)
//   x' = NeoSqr(A, fixSq) / Div - fixX
//
// Conditional quadratic: creates asymmetric folding around fixSq.
// ASM uses fldz/fcompp/fnstsw/and ah,0x41/jg for the comparison.
// Note: esi = PVar (overwrites esi with [esi+0x30]).

template<>
cpu_double3 execute_formula<FORMULA__NEOSQRX>(cpu_double3 p, constant double* params, double &de) {
    const double fixX  = params[0];
    const double fixSq = params[1];
    const double Mul   = params[2];
    const double Div   = params[3];

    double x = p.x;

    // A = Mul*x + fixX
    double A = Mul * x + fixX;

    // NeoSqr(A, fixSq)
    double neo;
    if (A >= 0.0) {
        neo = A * (A - fixSq);
    } else {
        neo = A * (fixSq - A);
    }

    // x' = neo / Div - fixX
    double new_x = neo / Div - fixX;

    return cpu_double3(new_x, p.y, p.z);
}

// Alternative direct function implementation
cpu_double3 _NeoSqrX_formula(cpu_double3 p, constant double* params, double &de) {
    double fX = params[0], fS = params[1], M = params[2], D = params[3];
    double A = M * p.x + fX;
    double neo = (A >= 0.0) ? A * (A - fS) : A * (fS - A);
    return cpu_double3(neo / D - fX, p.y, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__NeoSqrX = {
    .id = FORMULA__NEOSQRX,
    .name = "_NeoSqrX",
    .param_count = 4,
    .complexity = 190,
    .uses_transcendental = false
};

/*
Original M3 Description:
Formula by Luca GN 2011

NeoSqr(A, B) :=
  if A >= 0: A' = A * (A - B)
  else:      A' = A * (B - A)

x' = NeoSqr(Mul * x + fixX, fixSq) / Div - fixX

This transform is very strange, so use it wisely...

Note: NeoSqr is equivalent to: sign(A) * A * (B - |A|) = A*B - A*|A|
Or: -|A|*(|A|-B) when A>0, |A|*(|A|+B) when A<0... actually it's simpler
to just keep the conditional form.

Parameters:
  [0] fixX: double (default: 0.0)
  [1] fixSq: double (default: 1.0)
  [2] Mul: double (default: 1.0)
  [3] Div: double (default: 1.0)
*/
