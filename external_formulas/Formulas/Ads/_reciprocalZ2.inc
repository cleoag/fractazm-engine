// _reciprocalZ2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _reciprocalZ2
// Hand-ported from ASM: 356 bytes, DEoption=-1

constexpr unsigned int FORMULA__RECIPROCALZ2 = 7009;

// ASM trace:
//   Same as _reciprocalX2 but operates on ecx (z).
//   Sign byte from [ecx+7], result stored to [ecx].

template<>
cpu_double3 execute_formula<FORMULA__RECIPROCALZ2>(cpu_double3 p, constant double* params, double &de) {
    const double Limiter = params[0];
    double new_z = copysign(1.0, p.z) / (abs(p.z) + Limiter);
    return cpu_double3(p.x, p.y, new_z);
}

// Alternative direct function implementation
cpu_double3 _reciprocalZ2_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.x, p.y, copysign(1.0, p.z) / (abs(p.z) + params[0]));
}

// Formula metadata
constant FormulaInfo formula_info__reciprocalZ2 = {
    .id = FORMULA__RECIPROCALZ2,
    .name = "_reciprocalZ2",
    .param_count = 1,
    .complexity = 80,
    .uses_transcendental = false
};

/*
Original M3 Description:
Computes the reciprocal of z with limiter:
z' = sign(z) / (abs(z) + Limiter)

Parameters:
  [0] Limiter: double (default: 0.5)
*/
