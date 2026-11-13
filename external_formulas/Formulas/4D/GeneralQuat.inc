// GeneralQuat.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: GeneralQuat
// Hand-ported from ASM: 188 bytes, DEoption=4
// 4D formula: uses de parameter as w coordinate input/output

constexpr unsigned int FORMULA_GENERALQUAT = 10321;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void GeneralQuatOriginal_x87(double* state, const int* selectors);
#endif

// ASM trace:
//   esi = context, edi = PVar
//   eax = &x, edx = &y, ecx = &z, [ecx+8] = w
//   8 Integer parameters at [PVar-0x0C..-0x28] (4 bytes each)
//   C values: [esi+0x18]=Cx, [esi+0x20]=Cy, [esi+0x28]=Cz, [esi-0x38]=Cw
//
//   Phase 1: Load x,y,z,w and compute base terms:
//     fld [ecx+8]; fld [ecx]; fld [eax]; fld [edx]  → stack [y,x,z,w]
//     [esp+0x00] = x²-y²         (fld st1; fmul st0; fld st1; fmul st0; fsubp)
//     [esp+0x08] = 2*x*y         (fld st1; fmul st1; fadd st0)
//     [esp+0x10] = 2*x*z         (fld st1; fmul st3; fadd st0)
//     [esp+0x18] = 2*x*w         (fld st1; fmul st4; fadd st0)
//
//   Phase 2: Build 8 cross-products on FPU stack via fmul/fxch/fld:
//     st0..st7 = [y*z, y*z, z², y*w, y*w, z*w, z*w, w²]
//
//   Phase 3: Loop (ecx=7, 8 iterations):
//     For each Integer param p at [edi], [edi-4], ...:
//       if p>=1: slot = (p-1)&3, results[slot] += term
//       if p<=-1: slot = (-p-1)&3, results[slot] -= term   (fchs before fadd)
//     sub edi,4; dec ecx; jns loop
//
//   Phase 4: Load results, add C, store:
//     x' = results[0] + [esi+0x18]   → [eax]
//     y' = results[1] + [esi+0x20]   → [edx]
//     z' = results[2] + [esi+0x28]   → [ecx]
//     w' = results[3] + [esi-0x38]   → [ecx+8]
//
// NOTE: DEoption=4 (4D formula). The `de` parameter carries w coordinate.
// C is added INTERNALLY — engine must NOT add C again for DEoption=4.

template<>
cpu_double3 execute_formula<FORMULA_GENERALQUAT>(cpu_double3 p, double* params, double &de) {
    // Integer parameters (slot selectors for cross-terms):
    const int IJ_yz = int(params[0]);  // [PVar-0x0C] default: 4
    const int JI_yz = int(params[1]);  // [PVar-0x10] default: 4
    const int JJ_zz = int(params[2]);  // [PVar-0x14] default: -2
    const int IK_yw = int(params[3]);  // [PVar-0x18] default: -3
    const int KI_yw = int(params[4]);  // [PVar-0x1C] default: -3
    const int JK_zw = int(params[5]);  // [PVar-0x20] default: 1
    const int KJ_zw = int(params[6]);  // [PVar-0x24] default: 1
    const int KK_ww = int(params[7]);  // [PVar-0x28] default: 2

    const int pvals[8] = { IJ_yz, JI_yz, JJ_zz, IK_yw, KI_yw, JK_zw, KJ_zw, KK_ww };
#if defined(_MSC_VER) && defined(_M_X64)
    // Preserve the original x87 exponent range and qword accumulator spills.
    // In particular x*x-y*y can be finite even if both products exceed DBL_MAX.
    double state[] = {p.x, p.y, p.z, de, params[8], params[9], params[10], params[11]};
    GeneralQuatOriginal_x87(state, pvals);
    de = state[3];
    return cpu_double3(state[0], state[1], state[2]);
#else
    const numeric::X87Scope extendedScope;

    // C values (engine-provided):

    const numeric::X87Real x = p.x, y = p.y, z = p.z, w = de;

    // Phase 1: Base terms (fixed part of quaternion square)
    double results[4] = {
        double(x*x - y*y),   // Original qword stores after each expression.
        double((x*y)+(x*y)),
        double((x*z)+(x*z)),
        double((x*w)+(x*w))
    };

    // Phase 2+3: 8 cross-terms, each assigned to a slot by its parameter
    // Parameter value: +n → add to slot (n-1), -n → subtract from slot (n-1)
    const numeric::X87Real terms[8] = { y*z, y*z, z*z, y*w, y*w, z*w, z*w, w*w };

    for (int i = 0; i < 8; i++) {
        // Original DEC/JNS dispatch tests the sign of the wrapped uint32
        // result; INT_MIN therefore selects positive slot3. Keep the NEG/SUB
        // path unsigned too, including zero, to avoid signed-overflow UB.
        unsigned int selector = static_cast<unsigned int>(pvals[i]) - 1u;
        numeric::X87Real term = terms[i];
        if ((selector & 0x80000000u) != 0u) {
            selector = 0u - selector - 2u;
            term = -term;
        }
        results[selector & 3u] = double(term + results[selector & 3u]);
    }

    // Phase 4: Add C (internally — engine must NOT add C for DEoption=4)
    // C values appended by wrapper: params[8]=J1(Cx), params[9]=J2(Cy), params[10]=J3(Cz), params[11]=J4(Cw)
    results[0] += params[8];   // x' += Cx
    results[1] += params[9];   // y' += Cy
    results[2] += params[10];  // z' += Cz
    results[3] += params[11];  // w' += Cw

    de = results[3];  // w' → de

    return cpu_double3(results[0], results[1], results[2]);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_GeneralQuat = {
    .id = FORMULA_GENERALQUAT,
    .name = "GeneralQuat",
    .param_count = 8,
    .complexity = 188,
    .uses_transcendental = false
};

/*
Original M3 Description:
A generalization of a power 2 formula with 4 components.

The user values are in the range of 1 to 4, positive or negative.
They represent the 4D vector component to which the term is added,
or subtracted if negative.
  1: real part (X)
  2: i(maginary) part (Y)
  3: j (Z)
  4: k (W)

The squaring:
  (x,iy,jz,kw)² = xx + i·2xy + j·2xz + ij·yz + ji·yz - yy
                 + jj·zz + k·2xw + ik·yw + ki·yw + jk·zw + kj·zw + kk·ww

Base (fixed):              Configurable cross-terms (8 params select slot):
  x' = x²-y²          ij·yz, ji·yz, jj·zz, ik·yw, ki·yw,
  y' = 2xy          jk·zw, kj·zw, kk·ww
  z' = 2xz
  w' = 2xw

Default values give commutative quaternion multiplication:
  x' = x²-y² + 2wz
  y' = 2xy - z² + w²
  z' = 2xz - 2yw
  w' = 2xw + 2yz

Verification with defaults (IJ=4, JI=4, JJ=-2, IK=-3, KI=-3, JK=1, KJ=1, KK=2):
  y*z → slot 3 (w'), y*z → slot 3 (w')       → w' += 2yz
  z² → slot 1 neg (y')                        → y' -= z²
  y*w → slot 2 neg (z'), y*w → slot 2 neg     → z' -= 2yw
  z*w → slot 0 (x'), z*w → slot 0             → x' += 2zw
  w² → slot 1 (y')                            → y' += w²
  ✓ matches description

NOTE: DEoption=4 (4D formula).
  - `de` parameter = w coordinate (input/output)
  - C added internally — engine must NOT add C
  - Cw at [esi-0x38] is specific to 4D context struct layout

Parameters:
  [0] IJ yz: integer (default: 4)  — y*z → add to w'
  [1] JI yz: integer (default: 4)  — y*z → add to w'
  [2] JJ zz: integer (default: -2) — z²  → subtract from y'
  [3] IK yw: integer (default: -3) — y*w → subtract from z'
  [4] KI yw: integer (default: -3) — y*w → subtract from z'
  [5] JK zw: integer (default: 1)  — z*w → add to x'
  [6] KJ zw: integer (default: 1)  — z*w → add to x'
  [7] KK ww: integer (default: 2)  — w²  → add to y'
  [8..11] Cx, Cy, Cz, Cw (engine-provided)

PVar layout:
  [PVar-0x0C] = IJ yz (int32, 4 bytes)
  [PVar-0x10] = JI yz
  [PVar-0x14] = JJ zz
  [PVar-0x18] = IK yw
  [PVar-0x1C] = KI yw
  [PVar-0x20] = JK zw
  [PVar-0x24] = KJ zw
  [PVar-0x28] = KK ww
  Context: [esi+0x18]=Cx, [esi+0x20]=Cy, [esi+0x28]=Cz, [esi-0x38]=Cw
*/
