// _NeoSqrZ.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _NeoSqrZ
// Hand-ported from ASM: 530 bytes, DEoption=-1

constexpr unsigned int FORMULA__NEOSQRZ = 7034;

// ASM trace: Same as _NeoSqrY but operates on ecx (z).
// A = Mul * z + fixZ
// z' = NeoSqr(A, fixSq) / Div - fixZ
// NeoSqr(A, B) = (A >= 0) ? A*(A-B) : A*(B-A)

template<>
cpu_double3 execute_formula<FORMULA__NEOSQRZ>(cpu_double3 p, constant double* params, double &de) {
    const double fixZ  = params[0];
    const double fixSq = params[1];
    const double Mul   = params[2];
    const double Div   = params[3];

    double A = Mul * p.z + fixZ;
    double neoSqr;
    if (A >= 0.0) {
        neoSqr = A * (A - fixSq);
    } else {
        neoSqr = A * (fixSq - A);
    }
    double result = neoSqr / Div - fixZ;

    return cpu_double3(p.x, p.y, result);
}

// Formula metadata
constant FormulaInfo formula_info__NeoSqrZ = {
    .id = FORMULA__NEOSQRZ,
    .name = "_NeoSqrZ",
    .param_count = 4,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
NeoSqr transform for z:
A = Mul * z + fixZ
z' = NeoSqr(A, fixSq) / Div - fixZ

Parameters:
  [0] fixZ: double (default: 0.0)
  [1] fixSq: double (default: 1.0)
  [2] Mul: double (default: 1.0)
  [3] Div: double (default: 1.0)
*/
