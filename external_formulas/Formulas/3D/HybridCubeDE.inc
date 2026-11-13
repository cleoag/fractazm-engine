// HybridCubeDE.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.
// Hand-ported from Pascal source (formulas.pas)
// DEoption: 11 (ABox/VaryScale analytic DE via w tracking)

// HybridCubeDE — AmazingBox/MandelBox-style formula WITH analytic DE tracking
// (formulas.pas line 4932)
// Original comment: "Amazing box without adding c x87 with option fold"
//
// Identical to HybridCube in box fold + sphere fold math, but also:
//   de = de * |m|    (tracks DE scale factor via w component)
//
// ASM: [ecx+8] = w = DE value, multiplied by m at label @@9 before position update.

constexpr unsigned int FORMULA_HYBRIDCUBEDE = 90012;

template<>
cpu_double3 execute_formula<FORMULA_HYBRIDCUBEDE>(cpu_double3 p, constant double* params, double &de) {
    const double fold  = params[0];
    const double scale = params[1];
    const double minR  = params[2];

    const double minR2 = minR * minR;

    // Box fold: clamp(v, -fold, fold)*2 - v
    double bx = fmin(fmax(p.x, -fold), fold) * 2.0 - p.x;
    double by = fmin(fmax(p.y, -fold), fold) * 2.0 - p.y;
    double bz = fmin(fmax(p.z, -fold), fold) * 2.0 - p.z;

    // Sphere fold
    double rr = bx * bx + by * by + bz * bz;
    double m;
    if (rr < minR2) {
        m = scale / minR2;
    } else if (rr < 1.0) {
        m = scale / rr;
    } else {
        m = scale;
    }

    // Analytic DE tracking (DEoption=11): track scale factor
    de = de * fabs(m);

    return cpu_double3(bx * m, by * m, bz * m);
}

cpu_double3 HybridCubeDE_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HYBRIDCUBEDE>(p, params, de);
}

constant FormulaInfo formula_info_HybridCubeDE = {
    .id = FORMULA_HYBRIDCUBEDE,
    .name = "HybridCubeDE",
    .param_count = 3,
    .complexity = 120,
    .uses_transcendental = false
};

/*
Original Pascal source: formulas.pas, procedure HybridCubeDE (line 4932)
AmazingBox/MandelBox-style formula WITH analytic distance estimation tracking.
Identical to HybridCube but also multiplies the DE accumulator by |m|.

Box fold (per axis):
  bx = clamp(x, -fold, fold)*2 - x
  by = clamp(y, -fold, fold)*2 - y
  bz = clamp(z, -fold, fold)*2 - z

Sphere fold (rr = bx²+by²+bz²):
  if rr < minR²:   m = scale / minR²
  elif rr < 1.0:   m = scale / rr
  else:            m = scale

DE tracking:
  de = de * |m|

Output: (bx*m, by*m, bz*m)
Engine does NOT add C for DEoption=11 (formula returns without C).

Parameters:
  [0] fold:  double (default: 1.0) — box fold radius
  [1] scale: double (default: 2.0) — scale factor
  [2] minR:  double (default: 0.5) — minimum sphere radius for clamping
*/
