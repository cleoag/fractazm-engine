// _Scalingplus.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _Scalingplus
// Hand-ported from ASM: 53 bytes, DEoption=4

constexpr unsigned int FORMULA__SCALINGPLUS = 371;

// ASM trace (PVar at [esi+0x30]):
//   fld [ecx+8]       ; w
//   fld [PVar-0x28]   ; W_scale (params[3])
//   fmulp st(1)       ; w *= W_scale
//   fstp [ecx+8]      ; store w
//   fld [ecx]         ; z
//   fld [PVar-0x20]   ; Z_scale (params[2])
//   fmulp st(1)       ; z *= Z_scale
//   fstp [ecx]        ; store z
//   fld [edx]         ; y
//   fld [PVar-0x18]   ; Y_scale (params[1])
//   fmulp st(1)       ; y *= Y_scale
//   fstp [edx]        ; store y
//   fld [eax]         ; x
//   fld [PVar-0x10]   ; X_scale (params[0])
//   fmulp st(1)       ; x *= X_scale
//   fstp [eax]        ; store x

template<>
cpu_double3 execute_formula<FORMULA__SCALINGPLUS>(cpu_double3 p, constant double* params, double &de) {
    const double X_scale = params[0];
    const double Y_scale = params[1];
    const double Z_scale = params[2];
    const double W_scale = params[3];

    cpu_double3 result;
    result.x = p.x * X_scale;
    result.y = p.y * Y_scale;
    result.z = p.z * Z_scale;
    de *= W_scale;

    return result;
}

// Alternative direct function implementation
cpu_double3 _Scalingplus_formula(cpu_double3 p, constant double* params, double &de) {
    const double X_scale = params[0];
    const double Y_scale = params[1];
    const double Z_scale = params[2];
    const double W_scale = params[3];

    cpu_double3 result;
    result.x = p.x * X_scale;
    result.y = p.y * Y_scale;
    result.z = p.z * Z_scale;
    de *= W_scale;

    return result;
}

// Formula metadata
constant FormulaInfo formula_info__Scalingplus = {
    .id = FORMULA__SCALINGPLUS,
    .name = "_Scalingplus",
    .param_count = 4,
    .complexity = 50,
    .uses_transcendental = false
};

/*
Original M3 Description:
Just like normal scaling, but with selectable values for each dimension.
Useful for "coordinate fixing" especially in different coordinate systems.

Mathematical Formula:
x' = x * X_scale
y' = y * Y_scale
z' = z * Z_scale
w' = w * W_scale

Parameters:
  [0] x_scale: double (default: 1.0)
  [1] y_scale: double (default: 1.0)
  [2] z_scale: double (default: 1.0)
  [3] w_scale: double (default: 1.0)
*/
