// _FlipXZc.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _FlipXZc
// Hand-ported from ASM: 356 bytes, DEoption=-1

constexpr unsigned int FORMULA__FLIPXZC = 7005;

// ASM trace:
//   Same structure as _FlipXYc but swaps x,z and optionally Cx,Cz.
//   cmp [edi-0x0C], 0 → check Integer param
//   If != 0: swap Cx,Cz via fld/fstp [esi+0x18],[esi+0x28]
//   Always: x' = z, z' = x

template<>
cpu_double3 execute_formula<FORMULA__FLIPXZC>(cpu_double3 p, constant double* params, double &de) {
    if (params[0] != 0.0) {
        params[WRITE_J1_SLOT] = params[3];
        params[WRITE_J3_SLOT] = params[1];
    }
    return cpu_double3(p.z, p.y, p.x);
}

// Alternative direct function implementation
cpu_double3 _FlipXZc_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.z, p.y, p.x);
}

// Formula metadata
constant FormulaInfo formula_info__FlipXZc = {
    .id = FORMULA__FLIPXZC,
    .name = "_FlipXZc",
    .param_count = 1,
    .complexity = 80,
    .uses_transcendental = false
};

/*
Original M3 Description:
Exchanging x and z values. If "Flip also C" != 0, also swaps Cx and Cz.

x' = z, z' = x (always)
if Flip_also_C != 0: Cx' = Cz, Cz' = Cx (engine side effect)

Parameters:
  [0] Flip also C: integer (default: 0)
*/
