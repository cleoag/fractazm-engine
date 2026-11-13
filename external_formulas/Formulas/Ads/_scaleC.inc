// _scaleC.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _scaleC
// Hand-ported from ASM: 239 bytes, DEoption=-1

constexpr unsigned int FORMULA__SCALEC = 6003;

// ASM trace:
//   8B7E30          mov edi,[esi+0x30]  → edi = PVar
//   DD4618          fld [esi+0x18]      → Cx (Julia x constant)
//   DC4FF0          fmul [edi-0x10]     → Cx * ScaleCx
//   DD5E18          fstp [esi+0x18]     → Cx' = Cx * ScaleCx
//   DD4620          fld [esi+0x20]      → Cy
//   DC4FE8          fmul [edi-0x18]     → Cy * ScaleCy
//   DD5E20          fstp [esi+0x20]     → Cy' = Cy * ScaleCy
//   DD4628          fld [esi+0x28]      → Cz
//   DC4FE0          fmul [edi-0x20]     → Cz * ScaleCz
//   DD5E28          fstp [esi+0x28]     → Cz' = Cz * ScaleCz
//
// SPECIAL: This formula does NOT modify x,y,z coordinates.
// It modifies the Julia constants (Cx, Cy, Cz) in the iteration context.
// The Metal engine must handle C-modification as a side effect.

template<>
cpu_double3 execute_formula<FORMULA__SCALEC>(cpu_double3 p, constant double* params, double &de) {
    const double ScaleCx = params[0];
    const double ScaleCy = params[1];
    const double ScaleCz = params[2];
    // C values appended after PVar entries
    const double Cx = params[3];
    const double Cy = params[4];
    const double Cz = params[5];

    params[WRITE_J1_SLOT] = Cx * ScaleCx;
    params[WRITE_J2_SLOT] = Cy * ScaleCy;
    params[WRITE_J3_SLOT] = Cz * ScaleCz;

    return p;
}

// Alternative direct function implementation
cpu_double3 _scaleC_formula(cpu_double3 p, constant double* params, double &de) {
    return p;
}

// Formula metadata
constant FormulaInfo formula_info__scaleC = {
    .id = FORMULA__SCALEC,
    .name = "_scaleC",
    .param_count = 3,
    .complexity = 90,
    .uses_transcendental = false
};

/*
Original M3 Description:
A pretransform to scale C components.

This formula ONLY modifies the Julia constants, not coordinates:
Cx' = Cx * ScaleCx
Cy' = Cy * ScaleCy
Cz' = Cz * ScaleCz

The Metal iteration engine must apply C-scaling as a side effect.

Parameters:
  [0] ScaleCx: double (default: 1)
  [1] ScaleCy: double (default: 1)
  [2] ScaleCz: double (default: 1)
*/
