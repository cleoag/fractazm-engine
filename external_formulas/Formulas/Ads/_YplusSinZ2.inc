// _YplusSinZ2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _YplusSinZ2
// Hand-ported from ASM: 414 bytes, DEoption=-1

constexpr unsigned int FORMULA__YPLUSSINZ2 = 7011;

// ASM trace:
//   DD01        fld [ecx]           → z
//   DC4EF0      fmul [esi-0x10]     → z * h (phase)
//   D9EB        fldpi               → pi
//   DEC9        fmulp st1,st0       → pi * h * z
//   D9FF        fcos                → cos(pi*h*z)
//   DC4EE8      fmul [esi-0x18]     → k * cos(pi*h*z)
//   DC66E8      fsub [esi-0x18]     → k*cos(pi*h*z) - k
//   DC06        fadd [esi]          → + 2.0 (constant)
//   DC4E08      fmul [esi+0x08]     → * 0.5 (constant)
//   DD01        fld [ecx]           → z
//   D9FE        fsin                → sin(z)
//   DEC9        fmulp st1,st0       → wave * sin(z)
//   DC02        fadd [edx]          → + y
//   DD1A        fstp [edx]          → store y'
//
// y' = y + 0.5 * (2 - k + k*cos(pi*h*z)) * sin(z)
// Constants: [PVar+0] = 2.0, [PVar+8] = 0.5

template<>
cpu_double3 execute_formula<FORMULA__YPLUSSINZ2>(cpu_double3 p, constant double* params, double &de) {
    const double h = params[0];   // phase
    const double k = params[1];   // amplitude

    double wave = 0.5 * (2.0 - k + k * cos(M_PI * h * p.z));
    double new_y = p.y + wave * sin(p.z);

    return cpu_double3(p.x, new_y, p.z);
}

// Alternative direct function implementation
cpu_double3 _YplusSinZ2_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__YPLUSSINZ2>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__YplusSinZ2 = {
    .id = FORMULA__YPLUSSINZ2,
    .name = "_YplusSinZ2",
    .param_count = 2,
    .complexity = 90,
    .uses_transcendental = true
};

/*
Original M3 Description:
A groovy waveform function that distorts nicely your fractal:
y = y + 0.5 * (2 - k + k*cos(pi*h*z)) * sin(z)

h = 0 will revert to the default _YplusSinZ.

Parameters:
  [0] h (phase): double (default: 3.0)
  [1] k (ampli): double (default: 1.0)
*/
