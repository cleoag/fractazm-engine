// _HopSqrtY.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _HopSqrtY
// Hand-ported from ASM: 460 bytes, DEoption=-1

constexpr unsigned int FORMULA__HOPSQRTY = 7030;

// ASM trace: Same as _HopSqrtX but operates on edx (y).
// t = Mul * y + fixY
// y' = fixSq + sign(t) * sqrt(|t|) / Div

template<>
cpu_double3 execute_formula<FORMULA__HOPSQRTY>(cpu_double3 p, constant double* params, double &de) {
    const double fixY  = params[0];
    const double fixSq = params[1];
    const double Mul   = params[2];
    const double Div   = params[3];

    double t = Mul * p.y + fixY;
    double result = fixSq + sign(t) * sqrt(abs(t)) / Div;

    return cpu_double3(p.x, result, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__HopSqrtY = {
    .id = FORMULA__HOPSQRTY,
    .name = "_HopSqrtY",
    .param_count = 4,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
Sqrt for y as defined in Hopalong attractor:
t = Mul * y + fixY
y' = fixSq + sign(t) * sqrt(|t|) / Div

Parameters:
  [0] fixY: double (default: 0.0)
  [1] fixSq: double (default: 0.0)
  [2] Mul: double (default: 1.0)
  [3] Div: double (default: 1.0)
*/
