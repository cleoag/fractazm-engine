// _TranslateC4d.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _TranslateC4d
// Hand-ported from ASM: 83 bytes, DEoption=-1

constexpr unsigned int FORMULA__TRANSLATEC4D = 10001;

template<>
struct ExternalFormulaWritesWState<FORMULA__TRANSLATEC4D> {
    static constexpr bool value = true;
};

template<>
struct ExternalFormulaUsesWrittenCForSameIterationAdd<FORMULA__TRANSLATEC4D> {
    static constexpr bool value = true;
};

// ASM trace:
//   esi = context, edi = PVar
//   Params: [edi-0x10]=x_add, [edi-0x18]=y_add, [edi-0x20]=z_add, [edi-0x28]=w_add
//           [edi-0x30]=Cx_add, [edi-0x38]=Cy_add, [edi-0x40]=Cz_add, [edi-0x48]=Cw_add
//
//   fld [ecx+8]    ; w
//   fld [ecx]      ; z
//   fld [edx]      ; y
//   fld [eax]      ; x
//   fadd [edi-0x10]; x += x_add
//   fstp [eax]
//   fadd [edi-0x18]; y += y_add
//   fstp [edx]
//   fadd [edi-0x20]; z += z_add
//   fstp [ecx]
//   fadd [edi-0x28]; w += w_add
//   fstp [ecx+8]
//   ... then same for Cx/Cy/Cz/Cw ...
//
// Simple translation of all 4D coordinates and C constants by parameter offsets.

template<>
cpu_double3 execute_formula<FORMULA__TRANSLATEC4D>(cpu_double3 p, double* params, double &de) {
    (void)de;

    const double x_add = params[0];
    const double y_add = params[1];
    const double z_add = params[2];
    const double w_add = params[3];
    const double cx_add = params[4];
    const double cy_add = params[5];
    const double cz_add = params[6];
    const double cw_add = params[7];

    const double cx_in = params[8];
    const double cy_in = params[9];
    const double cz_in = params[10];
    const double cw_in = params[11];

    const double cx_out = cx_in + cx_add;
    const double cy_out = cy_in + cy_add;
    const double cz_out = cz_in + cz_add;
    const double cw_out = cw_in + cw_add;

    params[WRITE_J1_SLOT] = cx_out;
    params[WRITE_J2_SLOT] = cy_out;
    params[WRITE_J3_SLOT] = cz_out;
    params[WRITE_J4_SLOT] = cw_out;
    params[FORMULA_STATE_W_SLOT] = params[FORMULA_STATE_W_SLOT] + w_add;

    // ADDS_C=0 formulas receive wrapper-side +C after return. Subtract the
    // translated Julia constants here so the final committed position remains
    // x+dx, y+dy, z+dz while still exposing updated J1..J4 downstream.
    return cpu_double3(
        p.x + x_add - cx_out,
        p.y + y_add - cy_out,
        p.z + z_add - cz_out
    );
}

// Formula metadata
constant FormulaInfo formula_info__translatec4d = {
    .id = FORMULA__TRANSLATEC4D,
    .name = "_TranslateC4d",
    .param_count = 8,
    .complexity = 30,
    .uses_transcendental = false
};

/*
Original M3 Description:
Add a constant to the 4d vector and C components:

x = x + x_add
y = y + y_add
z = z + z_add
w = w + w_add
Cx = Cx + Cx_add
Cy = Cy + Cy_add
Cz = Cz + Cz_add
Cw = Cw + Cw_add

Useful if you want to translate the whole coordinate system.

Parameters:
  [0] x_add: double (default: 0)
  [1] y_add: double (default: 0)
  [2] z_add: double (default: 0)
  [3] w_add: double (default: 0) — 4D only
  [4] Cx_add: double (default: 0) — modifies C
  [5] Cy_add: double (default: 0) — modifies C
  [6] Cz_add: double (default: 0) — modifies C
  [7] Cw_add: double (default: 0) — 4D only, modifies C
*/
