// _FoldingOct.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _FoldingOct
// Hand-ported from ASM: 209 bytes, DEoption=-1

constexpr unsigned int FORMULA__FOLDINGOCT = 2153;

// ASM trace:
//   558BEC5657      push ebp; mov ebp,esp; push esi; push edi
//   8B7508          mov esi,[ebp+8]
//   89C7            mov edi,eax        → edi = ptr to x
//   DD01            fld [ecx]          → st0 = z
//   D9E1            fabs               → st0 = |z|
//   DD02            fld [edx]          → st0 = y, st1 = |z|
//   D9E1            fabs               → st0 = |y|
//   DD07            fld [edi]          → st0 = x, st1 = |y|, st2 = |z|
//   D9E1            fabs               → st0 = |x|
//   --- Three-pass bubble sort: ensure st0 >= st1 >= st2 ---
//   D8D1+DFE0+80E441+7402+D9C9   compare & conditional swap st0,st1
//   D8D2+DFE0+80E441+7402+D9CA   compare & conditional swap st0,st2
//   D9C9                          fxch st1
//   D8D2+DFE0+80E441+7402+D9CA   compare & conditional swap st0,st2
//   --- After sort: st0=max, st1=mid, st2=min ---
//   DD1A            fstp [edx]         → y' = max (largest)
//   DD1            fstp [edi]         → x' = mid (middle)
//   DD19            fstp [ecx]         → z' = min (smallest)
//   89F8            mov eax,edi        → restore eax
//   5F5E5DC20800    pop edi; pop esi; pop ebp; ret 8
//
// Octahedral fold: takes |x|, |y|, |z| and sorts them so x >= y >= z.
// This is used in octahedron/Sierpinski-type IFS constructions.

template<>
cpu_double3 execute_formula<FORMULA__FOLDINGOCT>(cpu_double3 p, constant double* params, double &de) {
    double a = abs(p.x);
    double b = abs(p.y);
    double c = abs(p.z);

    // Sort descending: a >= b >= c
    if (a < b) { double t = a; a = b; b = t; }
    if (a < c) { double t = a; a = c; c = t; }
    if (b < c) { double t = b; b = c; c = t; }

    return cpu_double3(a, b, c);
}

// Alternative direct function implementation
cpu_double3 _FoldingOct_formula(cpu_double3 p, constant double* params, double &de) {
    double a = abs(p.x), b = abs(p.y), c = abs(p.z);
    if (a < b) { double t = a; a = b; b = t; }
    if (a < c) { double t = a; a = c; c = t; }
    if (b < c) { double t = b; b = c; c = t; }
    return cpu_double3(a, b, c);
}

// Formula metadata
constant FormulaInfo formula_info__FoldingOct = {
    .id = FORMULA__FOLDINGOCT,
    .name = "_FoldingOct",
    .param_count = 0,
    .complexity = 190,
    .uses_transcendental = false
};

/*
Original M3 Description:
Octahedral fold: takes absolute values of x, y, z and sorts them
in descending order so that x >= y >= z.

The ASM uses a three-pass bubble sort with fcom/fnstsw/fxch.
This places the largest absolute coordinate in x, middle in y,
smallest in z — the standard fold for octahedron-based fractals.

Parameters: none
*/
