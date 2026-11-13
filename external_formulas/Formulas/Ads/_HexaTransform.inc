// _HexaTransform.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _HexaTransform
// Hand-ported from ASM: 144 bytes, DEoption=-1

constexpr unsigned int FORMULA__HEXATRANSFORM = 10007;
template<> struct ExternalFormulaWritesWState<FORMULA__HEXATRANSFORM> { static constexpr bool value = true; };
template<> struct ExternalFormulaAlwaysWritesJuliaXYZ<FORMULA__HEXATRANSFORM> { static constexpr bool value = true; };
template<> struct ExternalFormulaAlwaysWritesJuliaW<FORMULA__HEXATRANSFORM> { static constexpr bool value = true; };


// ASM trace:
//   esi = context, edi = PVar
//   Params: [edi-0x10]=Offset (double)
//   Constants: [edi+0x00]=k1=sqrt(3)/2, [edi+0x08]=k2=-0.5
//
//   --- Transform x/y/z/w ---
//   x' = |x| - Offset
//   y' = |y*k1 + x*k2| - Offset
//   z' = |z| - Offset
//   w' = |x*k2 - y*k1| - Offset     (fsubrp: st(0)-st(1))
//   fstp [ecx+8]  ; w'
//   fstp [ecx]    ; z'
//   fstp [edx]    ; y'
//   fstp [eax]    ; x'
//
//   --- Transform Cx/Cy/Cz/Cw (same pattern) ---
//   Cx' = |Cx| - Offset
//   Cy' = |Cy*k1 + Cx*k2| - Offset
//   Cz' = |Cz| - Offset
//   Cw' = |Cx*k2 - Cy*k1| - Offset
//   fstp [esi-0x38]; Cw → fstp [esi+0x28]; Cz → etc.
//
// Hexagonal transform (3D→4D) by Aexion.

template<>
cpu_double3 execute_formula<FORMULA__HEXATRANSFORM>(cpu_double3 p, constant double* params, double &de) {
    const double Offset = params[0];  // default: 0

    // Pre-computed constants (engine stores in CONSTANTS section)
    const double k1 = params[1];  // sqrt(3)/2
    const double k2 = params[2];

    double new_x = fabs(p.x) - Offset;
    double new_y = fabs(p.y * k1 + p.x * k2) - Offset;
    double new_z = fabs(p.z) - Offset;
    params[FORMULA_STATE_W_SLOT] = fabs(p.x * k2 - p.y * k1) - Offset;
    const double cx = params[3], cy = params[4], cz = params[5];
    params[WRITE_J1_SLOT] = fabs(cx) - Offset;
    params[WRITE_J2_SLOT] = fabs(cy * k1 + cx * k2) - Offset;
    params[WRITE_J3_SLOT] = fabs(cz) - Offset;
    params[WRITE_J4_SLOT] = fabs(cx * k2 - cy * k1) - Offset;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__hexatransform = {
    .id = FORMULA__HEXATRANSFORM,
    .name = "_HexaTransform",
    .param_count = 1,
    .complexity = 50,
    .uses_transcendental = false
};

/*
Original M3 Description:
Based on Aexions ideas, this performs a hexagonal transform to be used
as first formula as pre-transform, and choosing 'repeat from here' on
the next formula(s).

It is a 3d transform, but the result is 4d, so a combination with 4d
formulas would make a better sense (like the 4D ABox).

k1 = sqrt(3)/2, k2 = -0.5 (= 1/2), k3 = -k1

x'  = abs(x)  - Offset
y'  = abs(y*k1 + x*k2) - Offset
z'  = abs(z)  - Offset
w'  = abs(y*k3 + x*k2) - Offset    (= abs(x*k2 - y*k1))

Same pattern applied to Cx/Cy/Cz/Cw.

Parameters:
  [0] Offset: double (default: 0)
*/
