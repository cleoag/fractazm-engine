// _updateC_4d.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _updateC_4d
// Hand-ported from ASM: 446 bytes, DEoption=-1

constexpr unsigned int FORMULA__UPDATEC_4D = 7015;

template<>
struct ExternalFormulaWritesWState<FORMULA__UPDATEC_4D> {
    static constexpr bool value = true;
};

// ASM trace:
//   8B7508      mov esi,[ebp+8]     → esi = context
//   DD4108      fld [ecx+8]         → w
//   DD01        fld [ecx]           → z
//   DD02        fld [edx]           → y
//   DD00        fld [eax]           → x  → ST: x, y, z, w
//   DD5E18      fstp [esi+0x18]     → Cx = x
//   DD5E20      fstp [esi+0x20]     → Cy = y
//   DD5E28      fstp [esi+0x28]     → Cz = z
//   DD5EC8      fstp [esi-0x38]     → Cw = w
//
// Copies x,y,z,w to Cx,Cy,Cz,Cw (Julia constants).
// Coordinates are NOT modified.
// ENGINE SIDE EFFECT: Sets Julia constants from current position.

template<>
cpu_double3 execute_formula<FORMULA__UPDATEC_4D>(cpu_double3 p, constant double* params, double &de) {
    // ASM: Cx=x, Cy=y, Cz=z, Cw=w (copy position to Julia constants)
    // C values via WRITE_J slots; de carries w for 4D
    params[WRITE_J1_SLOT] = p.x;
    params[WRITE_J2_SLOT] = p.y;
    params[WRITE_J3_SLOT] = p.z;
    params[WRITE_J4_SLOT] = params[FORMULA_STATE_W_SLOT];  // w coordinate
    return p;
}

// Alternative direct function implementation
cpu_double3 _updateC_4d_formula(cpu_double3 p, constant double* params, double &de) {
    return p;
}

// Formula metadata
constant FormulaInfo formula_info__updateC_4d = {
    .id = FORMULA__UPDATEC_4D,
    .name = "_updateC_4d",
    .param_count = 0,
    .complexity = 70,
    .uses_transcendental = false
};

/*
Original M3 Description:
Moves x, y, z, w values to Cx, Cy, Cz, Cw.

This allows you to calculate any 4d Mandelbrot-like set in warped coordinates.
IFS are not affected by C.
Remember that all transforms MUST be used as pretransforms, put this AFTER all of them,
then put the set (or the sets). Check "repeat from here" in the first M-set.

Parameters: none
*/
