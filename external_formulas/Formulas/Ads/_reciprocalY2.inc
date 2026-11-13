// _reciprocalY2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _reciprocalY2
// Hand-ported from ASM: 356 bytes, DEoption=-1

constexpr unsigned int FORMULA__RECIPROCALY2 = 7008;

// ASM trace:
//   Same as _reciprocalX2 but operates on edx (y).
//   Sign byte from [edx+7], result stored to [edx].

template<>
cpu_double3 execute_formula<FORMULA__RECIPROCALY2>(cpu_double3 p, constant double* params, double &de) {
    const double Limiter = params[0];
    double new_y = copysign(1.0, p.y) / (abs(p.y) + Limiter);
    return cpu_double3(p.x, new_y, p.z);
}

// Alternative direct function implementation
cpu_double3 _reciprocalY2_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.x, copysign(1.0, p.y) / (abs(p.y) + params[0]), p.z);
}

// Formula metadata
constant FormulaInfo formula_info__reciprocalY2 = {
    .id = FORMULA__RECIPROCALY2,
    .name = "_reciprocalY2",
    .param_count = 1,
    .complexity = 80,
    .uses_transcendental = false
};

/*
Original M3 Description:
Computes the reciprocal of y with limiter:
y' = sign(y) / (abs(y) + Limiter)

Parameters:
  [0] Limiter: double (default: 0.5)
*/
