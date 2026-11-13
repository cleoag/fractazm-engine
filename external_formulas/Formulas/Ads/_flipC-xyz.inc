// _flipC-xyz.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _flipC-xyz
// Hand-ported from ASM: 299 bytes, DEoption=-1

constexpr unsigned int FORMULA__FLIPCXYZ = 6004;

// ASM trace:
//   8B7E30          mov edi,[esi+0x30]  → edi = PVar
//   D9D0            fnop
//   DD00            fld [eax]           → x
//   DD02            fld [edx]           → y
//   DD01            fld [ecx]           → z
//   DD4618          fld [esi+0x18]      → Cx
//   DD4620          fld [esi+0x20]      → Cy
//   DD4628          fld [esi+0x28]      → Cz
//   DD19            fstp [ecx]          → z' = Cz
//   DD1A            fstp [edx]          → y' = Cy
//   DD18            fstp [eax]          → x' = Cx
//   DD5E28          fstp [esi+0x28]     → Cz' = z_orig
//   DD5E20          fstp [esi+0x20]     → Cy' = y_orig
//   DD5E18          fstp [esi+0x18]     → Cx' = x_orig
//
// SPECIAL: Swaps coordinates with Julia constants.
// x ↔ Cx, y ↔ Cy, z ↔ Cz
// The Metal engine must handle C-modification as a side effect.

template<>
cpu_double3 execute_formula<FORMULA__FLIPCXYZ>(cpu_double3 p, constant double* params, double &de) {
    // Returns Julia C values as new coordinates.
    // Engine must also set: Cx=p.x, Cy=p.y, Cz=p.z (the old coords)
    // params layout: Julia C values passed by engine at indices [0],[1],[2]
    // NOTE: This requires special engine support for C-swapping.
    params[WRITE_J1_SLOT] = p.x;
    params[WRITE_J2_SLOT] = p.y;
    params[WRITE_J3_SLOT] = p.z;
    const double Cx = params[0];
    const double Cy = params[1];
    const double Cz = params[2];
    return cpu_double3(Cx, Cy, Cz);
}

// Alternative direct function implementation
cpu_double3 _flipCxyz_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(params[0], params[1], params[2]);
}

// Formula metadata
constant FormulaInfo formula_info__flipCxyz = {
    .id = FORMULA__FLIPCXYZ,
    .name = "_flipC-xyz",
    .param_count = 3,
    .complexity = 120,
    .uses_transcendental = false
};

/*
Original M3 Description:
Just for fun! Swaps x, y, z values with Cx, Cy, Cz.

x' = Cx, y' = Cy, z' = Cz
Cx' = x,  Cy' = y,  Cz' = z

SPECIAL: This formula modifies BOTH coordinates AND Julia constants.
The Metal engine must handle the bidirectional swap.

Parameters: none (reads Julia C from iteration context)
*/
