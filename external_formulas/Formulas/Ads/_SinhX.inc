// _SinhX.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _SinhX
// Hand-ported from ASM: 140 bytes, DEoption=-1

constexpr unsigned int FORMULA__SINHX = 3456;

// ASM trace (Luca GN 2013):
//   temp = Mul * x + fixX
//   save sign of temp
//   x' = (std::signbit(temp) ? -1.0 : 1.0) * Base * log2(|temp| + |fixLg|)
//
// Despite the name "Sinh", the ASM actually implements a signed logarithmic
// compression, NOT asinh. The sign byte is extracted from the double's high
// byte (bit 7) and re-applied via XOR after computing the unsigned result.
// Note: description claims asinh, but ASM has no sqrt(v²+1) computation.

template<>
cpu_double3 execute_formula<FORMULA__SINHX>(cpu_double3 p, constant double* params, double &de) {
    const double fixX  = params[0];
    const double fixLg = params[1];
    const double Mul   = params[2];
    const double Base  = params[3]; // default: 1/e ≈ 0.367879441

    double x = p.x;

    // Compute temp = Mul*x + fixX
    double temp = Mul * x + fixX;

    // Signed log compression
    double sx = (std::signbit(temp) ? -1.0 : 1.0);
    double v = abs(temp) + abs(fixLg);

    // Base * log2(v) — this is the fyl2x instruction
    double new_x = sx * Base * log2(v);

    return cpu_double3(new_x, p.y, p.z);
}

// Alternative direct function implementation
cpu_double3 _SinhX_formula(cpu_double3 p, constant double* params, double &de) {
    double fX = params[0], fL = params[1], M = params[2], B = params[3];
    double t = M * p.x + fX;
    double s = (std::signbit(t) ? -1.0 : 1.0);
    return cpu_double3(s * B * log2(abs(t) + abs(fL)), p.y, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__SinhX = {
    .id = FORMULA__SINHX,
    .name = "_SinhX",
    .param_count = 4,
    .complexity = 140,
    .uses_transcendental = true
};

/*
Original M3 Description:
"Hyp.Sine for x" — but ASM actually implements signed log compression.
Formula by Luca GN 2013.

ASM-verified formula:
temp = Mul * x + fixX
x' = (std::signbit(temp) ? -1.0 : 1.0) * Base * log2(|temp| + |fixLg|)

Description claims: x' = ln(temp + sqrt(temp²+1))/ln(Base) + fixLg
But ASM has NO sqrt(v²+1) — it's a simple signed log.

Parameters:
  [0] fixX: double (default: 0)
  [1] fixLg: double (default: 0)
  [2] Mul: double (default: -2)
  [3] Base: double (default: 0.367879441 = 1/e)
*/
