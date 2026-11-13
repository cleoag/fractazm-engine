// _FlipYZc.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _FlipYZc
// Hand-ported from ASM: 356 bytes, DEoption=-1

constexpr unsigned int FORMULA__FLIPYZC = 7006;

// ASM trace:
//   Same structure as _FlipXYc but swaps y,z and optionally Cy,Cz.
//   cmp [edi-0x0C], 0 → check Integer param
//   If != 0: swap Cy,Cz via fld/fstp [esi+0x20],[esi+0x28]
//   Always: y' = z, z' = y

template<>
cpu_double3 execute_formula<FORMULA__FLIPYZC>(cpu_double3 p, constant double* params, double &de) {
    if (params[0] != 0.0) {
        params[WRITE_J2_SLOT] = params[3];
        params[WRITE_J3_SLOT] = params[2];
    }
    return cpu_double3(p.x, p.z, p.y);
}

// Alternative direct function implementation
cpu_double3 _FlipYZc_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.x, p.z, p.y);
}

// Formula metadata
constant FormulaInfo formula_info__FlipYZc = {
    .id = FORMULA__FLIPYZC,
    .name = "_FlipYZc",
    .param_count = 1,
    .complexity = 80,
    .uses_transcendental = false
};

/*
Original M3 Description:
Exchanging y and z values. If "Flip also C" != 0, also swaps Cy and Cz.

y' = z, z' = y (always)
if Flip_also_C != 0: Cy' = Cz, Cz' = Cy (engine side effect)

Parameters:
  [0] Flip also C: integer (default: 0)
*/
