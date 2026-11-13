// _juliax2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _juliax2
// Hand-ported from ASM: 81 bytes, DEoption=-1

constexpr unsigned int FORMULA__JULIAX2 = 10002;
template<> struct ExternalFormulaAlwaysWritesJuliaXYZ<FORMULA__JULIAX2> { static constexpr bool value = true; };

// ASM trace:
//   ebx = &x, esi = context, edi = PVar
//   Params: [edi-0x10]=Separation, [edi-0x18..0x28]=seeds(L), [edi-0x30..0x40]=seeds(R)
//
//   fld [ebx]       ; x
//   ftst             ; compare x with 0
//   fnstsw ax
//   shr ah, 1        ; CF = C0 (sign bit)
//   jae 0x38         ; jump if x >= 0
//   --- x < 0 path ---
//   fadd [edi-0x10]  ; x += Separation
//   fstp [ebx]       ; store x
//   Cx = X_seed_L, Cy = Y_seed_L, Cz = Z_seed_L
//   --- x >= 0 path (at 0x38) ---
//   fsub [edi-0x10]  ; x -= Separation
//   fstp [ebx]       ; store x
//   Cx = X_seed_R, Cy = Y_seed_R, Cz = Z_seed_R
//
// Double Julia set: splits space at x=0 with different seeds per side.

template<>
cpu_double3 execute_formula<FORMULA__JULIAX2>(cpu_double3 p, constant double* params, double &de) {
    const double Separation = params[0];  // default: 2
    const double X_seed_L   = params[1];  // default: 0.5
    const double Y_seed_L   = params[2];  // default: 0.5
    const double Z_seed_L   = params[3];  // default: 0
    const double X_seed_R   = params[4];  // default: -1
    const double Y_seed_R   = params[5];  // default: 0
    const double Z_seed_R   = params[6];  // default: 0

    // Selected seeds are written to persistent Julia state by the wrapper.

    cpu_double3 result = p;

    if (!(p.x >= 0.0)) { // FTST unordered follows the negative branch.
        result.x = p.x + Separation;
        params[WRITE_J1_SLOT] = X_seed_L;
        params[WRITE_J2_SLOT] = Y_seed_L;
        params[WRITE_J3_SLOT] = Z_seed_L;
    } else {
        result.x = p.x - Separation;
        params[WRITE_J1_SLOT] = X_seed_R;
        params[WRITE_J2_SLOT] = Y_seed_R;
        params[WRITE_J3_SLOT] = Z_seed_R;
    }

    return result;
}

// Formula metadata
constant FormulaInfo formula_info__juliax2 = {
    .id = FORMULA__JULIAX2,
    .name = "_juliax2",
    .param_count = 7,
    .complexity = 40,
    .uses_transcendental = false
};

/*
Original M3 Description:
Shows a double Julia set, with chosen seeds. Just *don't* turn on Julia mode,
and use as a pretransform!

if x < 0:
  x' = x + Separation
  Cx = X_seed_L, Cy = Y_seed_L, Cz = Z_seed_L
else:
  x' = x - Separation
  Cx = X_seed_R, Cy = Y_seed_R, Cz = Z_seed_R

Parameters:
  [0] Separation: double (default: 2)
  [1] X seed (L): double (default: 0.5)
  [2] Y seed (L): double (default: 0.5)
  [3] Z seed (L): double (default: 0)
  [4] X seed (R): double (default: -1)
  [5] Y seed (R): double (default: 0)
  [6] Z seed (R): double (default: 0)
*/
