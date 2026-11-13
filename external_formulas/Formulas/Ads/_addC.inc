// _addC.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _addC
// Hand-ported from ASM: 53 bytes, DEoption=-1

constexpr unsigned int FORMULA__ADDC = 7280;

// ASM trace:
//   fld [ecx]         ; z
//   fld [edx]         ; y
//   fld [eax]         ; x
//   fld [esi+0x18]    ; Cx (Julia constant x)
//   fmul [PVar-0x10]  ; Cx * X_mul (params[0])
//   faddp st(1)       ; x + Cx*X_mul
//   fstp [eax]        ; store x
//   fld [esi+0x20]    ; Cy (Julia constant y)
//   fmul [PVar-0x18]  ; Cy * Y_mul (params[1])
//   faddp st(1)       ; y + Cy*Y_mul
//   fstp [edx]        ; store y
//   fld [esi+0x28]    ; Cz (Julia constant z)
//   fmul [PVar-0x20]  ; Cz * Z_mul (params[2])
//   faddp st(1)       ; z + Cz*Z_mul
//   fstp [ecx]        ; store z
//
// Note: 'c' is the Julia constant vector from the iteration context.
// In Mandelbrot mode c = initial point; in Julia mode c = Julia constant.

template<>
cpu_double3 execute_formula<FORMULA__ADDC>(cpu_double3 p, constant double* params, double &de) {
    const double X_mul = params[0];
    const double Y_mul = params[1];
    const double Z_mul = params[2];

    // c.x/c.y/c.z are the Julia constants from the iteration context
    // Provided by the renderer at params[3..5] after the three mapped options
    const double Cx = params[3];
    const double Cy = params[4];
    const double Cz = params[5];

    cpu_double3 result;
    result.x = p.x + Cx * X_mul;
    result.y = p.y + Cy * Y_mul;
    result.z = p.z + Cz * Z_mul;

    return result;
}

// Alternative direct function implementation
cpu_double3 _addC_formula(cpu_double3 p, constant double* params, double &de) {
    const double X_mul = params[0];
    const double Y_mul = params[1];
    const double Z_mul = params[2];

    const double Cx = params[3];
    const double Cy = params[4];
    const double Cz = params[5];

    cpu_double3 result;
    result.x = p.x + Cx * X_mul;
    result.y = p.y + Cy * Y_mul;
    result.z = p.z + Cz * Z_mul;

    return result;
}

// Formula metadata
constant FormulaInfo formula_info__addC = {
    .id = FORMULA__ADDC,
    .name = "_addC",
    .param_count = 3,
    .complexity = 50,
    .uses_transcendental = false
};

/*
Original M3 Description:
Add Julia constants (C) scaled by per-axis multipliers.
This transform is used in hybrid formulas to re-add the C constant
at specific iteration steps.

Mathematical Formula:
x' = x + Cx * X_multiplier
y' = y + Cy * Y_multiplier
z' = z + Cz * Z_multiplier

Parameters:
  [0] Cx_multiplier: double (default: 1)
  [1] Cy_multiplier: double (default: 1)
  [2] Cz_multiplier: double (default: 1)
  [3..5] Cx, Cy, Cz: Julia constants (from iteration context)
*/