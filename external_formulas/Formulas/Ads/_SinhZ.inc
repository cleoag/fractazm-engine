// _SinhZ.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _SinhZ
// Hand-ported from ASM: 170 bytes, DEoption=-1

constexpr unsigned int FORMULA__SINHZ = 7890;

// ASM trace (Luca GN 2013):
//   temp = Mul * z + fixZ
//   v = temp + sqrt(temp² + 1)      // this IS the asinh computation
//   z' = Base * log2(v) - fixLg
//
// Unlike _SinhX (which is just signed log), _SinhZ actually computes asinh:
//   asinh(temp) = ln(temp + sqrt(temp²+1))
//   z' = Base * log2(temp + sqrt(temp²+1)) - fixLg
//
// Note: esi = PVar (overwrites esi with [esi+0x30]).

template<>
cpu_double3 execute_formula<FORMULA__SINHZ>(cpu_double3 p, constant double* params, double &de) {
    const double fixZ = params[0];
    const double fixLg = params[1];
    const double Mul  = params[2];
    const double Base = params[3]; // default: 1/e ≈ 0.367879441

    double z = p.z;

    // temp = Mul*z + fixZ
    double temp = Mul * z + fixZ;

    // v = temp + sqrt(temp²+1) — the asinh argument
    double v = temp + sqrt(temp * temp + 1.0);

    // Base * log2(v) - fixLg
    double new_z = Base * log2(max(v, 1e-30)) - fixLg;

    return cpu_double3(p.x, p.y, new_z);
}

// Alternative direct function implementation
cpu_double3 _SinhZ_formula(cpu_double3 p, constant double* params, double &de) {
    double fZ = params[0], fL = params[1], M = params[2], B = params[3];
    double t = M * p.z + fZ;
    double v = t + sqrt(t*t + 1.0);
    return cpu_double3(p.x, p.y, B * log2(max(v, 1e-30)) - fL);
}

// Formula metadata
constant FormulaInfo formula_info__SinhZ = {
    .id = FORMULA__SINHZ,
    .name = "_SinhZ",
    .param_count = 4,
    .complexity = 170,
    .uses_transcendental = true
};

/*
Original M3 Description:
Hyp.Sine for z + some bells & whistles.
Formula by Luca GN 2013.

Mathematical Formula:
temp = Mul * z + fixZ
z' = Base * log2(temp + sqrt(temp² + 1)) - fixLg

This actually computes asinh via ln(v + sqrt(v²+1)), unlike _SinhX
which only does signed log compression.

Parameters:
  [0] fixZ: double (default: 0)
  [1] fixLg: double (default: 0)
  [2] Mul: double (default: -2)
  [3] Base: double (default: 0.367879441 = 1/e)
*/
