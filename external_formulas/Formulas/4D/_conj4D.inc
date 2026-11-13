// _conj4D.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _conj4D
// Hand-ported from ASM: 27 bytes, DEoption=4

constexpr unsigned int FORMULA__CONJ4D = 4516;

// ASM trace:
//   fld [edx]    ; y
//   fchs         ; -y
//   fstp [edx]   ; y = -y
//   fld [ecx]    ; z
//   fchs         ; -z
//   fstp [ecx]   ; z = -z
//   fld [ecx+8]  ; w
//   fchs         ; -w
//   fstp [ecx+8] ; w = -w
// 4D quaternion conjugation: (x, -y, -z, -w)

template<>
cpu_double3 execute_formula<FORMULA__CONJ4D>(cpu_double3 p, constant double* params, double &de) {
    (void)params;
    de = -de;
    return cpu_double3(p.x, -p.y, -p.z);
}

// Alternative direct function implementation
cpu_double3 _conj4D_formula(cpu_double3 p, constant double* params, double &de) {
    (void)params;
    de = -de;
    return cpu_double3(p.x, -p.y, -p.z);
}

// Formula metadata
constant FormulaInfo formula_info__conj4D = {
    .id = FORMULA__CONJ4D,
    .name = "_conj4D",
    .param_count = 0,
    .complexity = 27,
    .uses_transcendental = false
};

/*
Original M3 Description:
4D quaternion conjugation: conj(S) = (x,-y,-z,-w)
Formula by Luca GN 2011

Parameters:
  (none)
*/
