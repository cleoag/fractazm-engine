// _FlipXYc.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _FlipXYc
// Hand-ported from ASM: 356 bytes, DEoption=-1

constexpr unsigned int FORMULA__FLIPXYC = 7004;

// ASM trace:
//   8B7E30          mov edi,[esi+0x30]  → edi = PVar
//   837FF400        cmp dword [edi-0x0C], 0  → Integer param "Flip also C"
//   740C            je +12              → skip C flip if param == 0
//   DD4618+DD4620   fld Cx, fld Cy
//   DD5E18+DD5E20   fstp → Cx'=Cy, Cy'=Cx  (swap Julia Cx,Cy)
//   DD02+DD00       fld y, fld x
//   DD1A+DD18       fstp → y'=x, x'=y       (always swap coords)
//
// SPECIAL: Optionally swaps Julia Cx,Cy (if Integer param != 0).

template<>
cpu_double3 execute_formula<FORMULA__FLIPXYC>(cpu_double3 p, double* params, double &de) {
    // Coordinate swap always happens. C swap handled by engine.
    // Integer param at params[0] controls C-flip (engine side effect).
    if (params[0] != 0.0) {
        params[WRITE_J1_SLOT] = params[2];
        params[WRITE_J2_SLOT] = params[1];
    }
    return cpu_double3(p.y, p.x, p.z);
}

// Alternative direct function implementation
cpu_double3 _FlipXYc_formula(cpu_double3 p, double* params, double &de) {
    return cpu_double3(p.y, p.x, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__FlipXYc = {
    .id = FORMULA__FLIPXYC,
    .name = "_FlipXYc",
    .param_count = 1,
    .complexity = 80,
    .uses_transcendental = false
};

/*
Original M3 Description:
Exchanging x and y values. If "Flip also C" != 0, also swaps Cx and Cy.

x' = y, y' = x (always)
if Flip_also_C != 0: Cx' = Cy, Cy' = Cx (engine side effect)

Note: Changing C does not work in Julia mode.

Parameters:
  [0] Flip also C: integer (default: 0)
*/
