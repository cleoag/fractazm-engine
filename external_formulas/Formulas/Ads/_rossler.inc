// _rossler.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _rossler
// Hand-ported from ASM: 86 bytes, DEoption=-1
// Rossler hyperchaotic 4D system.
// By Luca GN 2011, based on Jiang Xu et al.

constexpr unsigned int FORMULA__ROSSLER = 423;
template<> struct ExternalFormulaWritesWState<FORMULA__ROSSLER> { static constexpr bool value = true; };

// ASM trace (86 bytes, 0 branches):
//
// esi = PVar (note: reuses esi, no edi)
// eax=&x, edx=&y, ecx=&z, [ecx+8]=w
//
// x' = -(y + z) * Strength
// y' = (x + Alpha*y + w) * Strength
// z' = (Beta + x*z) * Strength
// w' = (Delta*w - Gamma*z) * Strength
//
// C handling: Transform (DEoption=-1), no C.

template<>
cpu_double3 execute_formula<FORMULA__ROSSLER>(cpu_double3 p, constant double* params, double &de) {
    const double Strength = params[0];  // [esi-0x10] (default: 1.0)
    const double Alpha    = params[1];  // [esi-0x18] (default: 0.5)
    const double Beta     = params[2];  // [esi-0x20] (default: 1.0)
    const double Gamma    = params[3];  // [esi-0x28]
    const double Delta    = params[4];  // [esi-0x30]

    double x = p.x, y = p.y, z = p.z, w = params[FORMULA_STATE_W_SLOT];

    double x_out = -(y + z) * Strength;
    double y_out = (x + Alpha * y + w) * Strength;
    double z_out = (Beta + x * z) * Strength;
    double w_out = (Delta * w - Gamma * z) * Strength;

    params[FORMULA_STATE_W_SLOT] = w_out;
    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info__rossler = {
    .id = FORMULA__ROSSLER,
    .name = "_rossler",
    .param_count = 5,
    .complexity = 86,
    .uses_transcendental = false
};

/*
Original M3 Description:
Rossler hyperchaotic system in 4D.
Adapted from Jiang Xu, Guoliang Cai, Song Zheng.
"International Journal of Nonlinear Science Vol.8(2009) No.1,pp.117-123"
Formula by Luca GN 2011

Full 4D system:
  x' = -(y + z) * Strength
  y' = (x + Alpha*y + w) * Strength
  z' = (Beta + x*z) * Strength
  w' = (Delta*w - Gamma*z) * Strength

Parameters:
  [0] Strength: double (default: 1.0) - overall scaling
  [1] Alpha: double (default: 0.5) - y feedback
  [2] Beta: double (default: 1.0) - z constant term
  [3] Gamma: double (default: 1.0) - z-to-w coupling
  [4] Delta: double (default: 1.0) - w feedback
*/
