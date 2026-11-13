// _planeFold.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _planeFold
// Hand-ported from ASM: 458 bytes, DEoption=-1

constexpr unsigned int FORMULA__PLANEFOLD = 7022;

// ASM trace:
//   8B7630      mov esi,[esi+0x30]  → PVar
//   DD46E0      fld [esi-0x20]      → Nz
//   DD46E8      fld [esi-0x18]      → Ny
//   DD46F0      fld [esi-0x10]      → Nx → ST: Nx, Ny, Nz
//   DD01/02/00  fld z, y, x         → ST: x, y, z, Nx, Ny, Nz
//   D9C0        fld st0             → dup x
//   D8CC        fmul st0,st4        → x * Nx
//   D9C2        fld st2             → dup y
//   D8CE        fmul st0,st6        → y * Ny
//   DEC1        faddp               → x*Nx + y*Ny
//   D9C3        fld st3             → dup z
//   D8CF        fmul st0,st7        → z * Nz
//   DEC1        faddp               → dot = x*Nx + y*Ny + z*Nz
//   D8C0        fadd st0,st0        → dd = 2 * dot
//   DCCC        fmul st4,st0        → Nx' = Nx * dd
//   DCCD        fmul st5,st0        → Ny' = Ny * dd
//   DECE        fmulp st6,st0       → Nz' = Nz * dd, pop
//   DEE3        fsubrp st3,st0      → x' = x - Nx*dd
//   DEE3        fsubrp st3,st0      → y' = y - Ny*dd (now at st0 position)
//   DEE3        fsubrp st3,st0      → z' = z - Nz*dd
//   DD18/1A/19  store x', y', z'
//
// Reflection through plane with normal N:
// dd = 2 * dot(p, N)
// p' = p - N * dd
// Normal should be normalized to avoid scaling.
//
// NOTE: D8C0 = FADD st(0),st(0) = 2*value (not FMUL!)

template<>
cpu_double3 execute_formula<FORMULA__PLANEFOLD>(cpu_double3 p, constant double* params, double &de) {
    const double Nx = params[0];
    const double Ny = params[1];
    const double Nz = params[2];

    double dd = 2.0 * (p.x * Nx + p.y * Ny + p.z * Nz);

    return cpu_double3(
        p.x - Nx * dd,
        p.y - Ny * dd,
        p.z - Nz * dd
    );
}

// Alternative direct function implementation
cpu_double3 _planeFold_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__PLANEFOLD>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__planeFold = {
    .id = FORMULA__PLANEFOLD,
    .name = "_planeFold",
    .param_count = 3,
    .complexity = 80,
    .uses_transcendental = false
};

/*
Original M3 Description:
A folding on an arbitrary plane with normal N:

dd = 2 * (x*Nx + y*Ny + z*Nz)
x' = x - Nx * dd
y' = y - Ny * dd
z' = z - Nz * dd

The normal must be normalized to not scale the result and disturb the DE.

Parameters:
  [0] Normal X: double (default: 1.0)
  [1] Normal Y: double (default: 0.0)
  [2] Normal Z: double (default: 0.0)
*/
