// _reciprocalX2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _reciprocalX2
// Hand-ported from ASM: 356 bytes, DEoption=-1

constexpr unsigned int FORMULA__RECIPROCALX2 = 7007;

// ASM trace:
//   DD00          fld [eax]          → x
//   8A5007        mov dl,[eax+7]     → save sign byte
//   D9E1          fabs               → |x|
//   DC46F0        fadd [esi-0x10]    → |x| + Limiter
//   80E280        and dl, 0x80       → isolate sign bit
//   D9E8          fld1               → 1.0
//   DEF1          fdivrp             → 1.0 / (|x| + Limiter)  (DE F0+i = FDIVRP)
//   DD18          fstp [eax]         → store result
//   305007        xor [eax+7], dl    → re-apply original sign
//
// Signed reciprocal with limiter to prevent singularity.

template<>
cpu_double3 execute_formula<FORMULA__RECIPROCALX2>(cpu_double3 p, constant double* params, double &de) {
    const double Limiter = params[0];
    double new_x = copysign(1.0, p.x) / (abs(p.x) + Limiter);
    return cpu_double3(new_x, p.y, p.z);
}

// Alternative direct function implementation
cpu_double3 _reciprocalX2_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(copysign(1.0, p.x) / (abs(p.x) + params[0]), p.y, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__reciprocalX2 = {
    .id = FORMULA__RECIPROCALX2,
    .name = "_reciprocalX2",
    .param_count = 1,
    .complexity = 80,
    .uses_transcendental = false
};

/*
Original M3 Description:
Computes the reciprocal of x with limiter:
x' = sign(x) / (abs(x) + Limiter)

Bigger Limiter values lead to less noise.
Disturbs the DE, so handle with care.

Parameters:
  [0] Limiter: double (default: 0.5)
*/
