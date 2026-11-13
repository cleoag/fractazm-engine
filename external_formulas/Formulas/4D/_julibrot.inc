// _julibrot.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _julibrot
// Hand-ported from ASM: 50 bytes, DEoption=4

constexpr unsigned int FORMULA__JULIBROT = 1370;

// ASM trace:
//   fld [PVar-0x10]  ; params[0] = x1_seed
//   fstp [ecx]       ; z = x1_seed
//   fld [PVar-0x18]  ; params[1] = y1_seed
//   fstp [ecx+8]     ; w = y1_seed
//   fld [PVar-0x20]  ; params[2] = x2_seed
//   fstp [esi+0x18]  ; Cx = x2_seed
//   fld [PVar-0x28]  ; params[3] = y2_seed
//   fstp [esi+0x20]  ; Cy = y2_seed
//
// This transform sets z, w coordinates and Julia constants Cx, Cy from params.
// Used to create Julibrot slices through 4D Mandelbrot space.
// x and y are left unchanged.

template<>
cpu_double3 execute_formula<FORMULA__JULIBROT>(cpu_double3 p, constant double* params, double &de) {
    cpu_double3 result = p;

    // z = x1_seed (params[0])
    result.z = params[0];
    de = params[1];
    params[WRITE_J1_SLOT] = params[2];
    params[WRITE_J2_SLOT] = params[3];

    return result;
}

// Alternative direct function implementation
cpu_double3 _julibrot_formula(cpu_double3 p, constant double* params, double &de) {
    cpu_double3 result = p;
    result.z = params[0];
    de = params[1];
    params[WRITE_J1_SLOT] = params[2];
    params[WRITE_J2_SLOT] = params[3];
    return result;
}

// Formula metadata
constant FormulaInfo formula_info__julibrot = {
    .id = FORMULA__JULIBROT,
    .name = "_julibrot",
    .param_count = 4,
    .complexity = 50,
    .uses_transcendental = false
};

/*
Original M3 Description:
Julibrot transform: sets z, w and Julia constants from parameters.
Used to slice through 4D Mandelbrot/Julia space.

Mathematical Formula:
z = x1_seed
w = y1_seed
Cx = x2_seed
Cy = y2_seed
(x and y unchanged)

Parameters:
  [0] x1_seed: double (default: -0.844)
  [1] y1_seed: double (default: 0.206)
  [2] x2_seed: double (default: 0.363)
  [3] y2_seed: double (default: -0.106)
*/
