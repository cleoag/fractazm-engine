// HybridCube.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.
// Hand-ported from Pascal source (formulas.pas)
// DEoption: 0 (standard log-DE, engine adds C after return)

// HybridCube — AmazingBox/MandelBox-style formula, no DE tracking (formulas.pas line 4852)
// Original comment: "Amazing box x87 with options fold fold x2"
//
// ASM box fold pattern (for each axis):
//   result = |x+fold| - |x-fold| - x
//          = clamp(x, -fold, fold)*2 - x
//
// Sphere fold (rr = r²):
//   if rr < minR²: m = scale / minR²      (clamped: from [esi-24])
//   elif rr < 1.0: m = scale / rr         (linear: fdivr scale/rr)
//   else:          m = scale              (from [esi-16])
//
// Parameter layout in ASM (esi = PVar, calling conv: params[n] = [esi-(16+8n)]):
//   [esi-16] = scale        (ASM params[0])
//   [esi-24] = scale/minR²  (ASM params[1], precomputed by BOXSCALE)
//   [esi-32] = minR²        (ASM params[2], precomputed by BOXSCALE)
//   [esi-40] = fold         (ASM params[3])
//
// C++ port uses a collapsed 3-param user-facing layout (fold, scale, minR)
// and recomputes minR² at runtime instead of consuming the BOXSCALE slots.
//
// NOTE: C is added by the Pascal ASM itself (fadd [esi+24/32/40]),
// but for the C++ port we return without C — engine adds it (DEoption=0).

constexpr unsigned int FORMULA_HYBRIDCUBE = 90011;

template<>
cpu_double3 execute_formula<FORMULA_HYBRIDCUBE>(cpu_double3 p, constant double* params, double &de) {
    (void)de;  // DEoption=0 — unused

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

    return cpu_double3(bx * m, by * m, bz * m);
}

cpu_double3 HybridCube_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HYBRIDCUBE>(p, params, de);
}

constant FormulaInfo formula_info_HybridCube = {
    .id = FORMULA_HYBRIDCUBE,
    .name = "HybridCube",
    .param_count = 3,
    .complexity = 120,
    .uses_transcendental = false
};

/*
Original Pascal source: formulas.pas, procedure HybridCube (line 4852)
AmazingBox/MandelBox-style formula without analytic DE tracking.

Box fold (per axis):
  bx = clamp(x, -fold, fold)*2 - x   (equivalent to |x+fold|-|x-fold|-x)
  by = clamp(y, -fold, fold)*2 - y
  bz = clamp(z, -fold, fold)*2 - z

Sphere fold (rr = bx²+by²+bz²):
  if rr < minR²:   m = scale / minR²
  elif rr < 1.0:   m = scale / rr
  else:            m = scale

Output: (bx*m, by*m, bz*m)
Engine adds Cx, Cy, Cz after return (DEoption=0).

Parameters:
  [0] fold:  double (default: 1.0) — box fold radius
  [1] scale: double (default: 2.0) — scale factor
  [2] minR:  double (default: 0.5) — minimum sphere radius for clamping
*/
