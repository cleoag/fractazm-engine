// _CelticMode.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _CelticMode
// Hand-ported from ASM: 104 bytes, DEoption=-1

constexpr unsigned int FORMULA__CELTICMODE = 7058;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void CelticModeOriginal_x87(double* xyz, const double* params);
#endif

// ASM trace:
//   x' = x*Evil_rune + |x + Cx*Good_rune| - Cx*Good_rune
//
//   if Mystic_rune != 0:
//     z' = |z + Cz*Mystic_rune| - Cz*Heretic_rune
//   else:
//     z unchanged
//   y unchanged
//
// Celtic fold modifier: applies abs-fold to x (and optionally z)
// using C values as fold centers.

template<>
cpu_double3 execute_formula<FORMULA__CELTICMODE>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double xyz[] = {p.x, p.y, p.z};
    CelticModeOriginal_x87(xyz, params);
    return cpu_double3(xyz[0], xyz[1], xyz[2]);
#else
    const double Good_rune    = params[0];  // default: -1
    const double Evil_rune    = params[1];  // default: 0
    const double Mystic_rune  = params[2];  // default: 0
    const double Heretic_rune = params[3];  // default: 0

    // The wrapper appends Julia C immediately after the four options.
    const double Cx = params[4];
    const double Cz = params[6];

    double new_x = (fabs(p.x + Cx * Good_rune) - Cx * Good_rune) + p.x * Evil_rune;
    double new_z = p.z;
    if (Mystic_rune != 0.0) {
        new_z = fabs(p.z + Cz * Mystic_rune) - Cz * Heretic_rune;
    }

    return cpu_double3(new_x, p.y, new_z);
#endif
}

// Formula metadata
constant FormulaInfo formula_info__CelticMode = {
    .id = FORMULA__CELTICMODE,
    .name = "_CelticMode",
    .param_count = 4,
    .complexity = 104,
    .uses_transcendental = false
};

/*
Original M3 Description:
Celtic fold modifier. Applies abs-fold to x using Cx as fold center,
and optionally to z using Cz. Named with "rune" themed parameters.

x' = x * Evil_rune + |x + Cx * Good_rune| - Cx * Good_rune
if Mystic_rune != 0:
  z' = |z + Cz * Mystic_rune| - Cz * Heretic_rune
y unchanged

NOTE: This transform reads Cx/Cz from the engine context inline.
The Metal engine must provide C values to this formula.

Parameters:
  [0] Good rune: double (default: -1)
  [1] Evil rune: double (default: 0)
  [2] Mystic rune: double (default: 0)
  [3] Heretic rune: double (default: 0)
*/
