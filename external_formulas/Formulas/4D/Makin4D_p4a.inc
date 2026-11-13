// Makin4D p4a.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Makin4D p4a
// Hand-ported from ASM: 314 bytes, DEoption=4
// 4D formula: uses de parameter as w coordinate input/output

constexpr unsigned int FORMULA_MAKIN4D_P4A = 10326;

// ASM trace:
//   esi = context, edi = PVar, ebx = &x
//   eax/ebx = &x, edx = &y, ecx = &z, [ecx+8] = w
//   Parameters: [edi-0x10] = w_add (double, default: 0)
//   C values: [esi+0x18]=Cx, [esi+0x20]=Cy, [esi+0x28]=Cz, [esi-0x38]=Cw
//
//   Formula does the Makin iteration TWICE then adds C.
//   Each iteration step:
//     z_complex = (x, y), w_complex = (z, w)
//     temp_w = 2 * z_complex * w_complex  (complex multiply, doubled)
//     s = |z_complex|^2 = x^2 + y^2
//     t = |w_complex|^2 = z^2 + w^2
//     if t < s:  (ASM: fcom st1 checks t < s via CF)
//       ratio = -t/s + 1 = 1 - t/s
//       z_new = (x^2 - y^2, 2*x*y) * ratio   (= sqr(z_complex) * (1-t/s))
//     else:
//       ratio = s/t - 1
//       z_new = (z^2 - w^2, 2*z*w) * ratio   (= sqr(w_complex) * (s/t-1))
//
//   After two repetitions:
//     x += Cx, y += Cy, z += Cz, w += Cw + w_add

template<>
cpu_double3 execute_formula<FORMULA_MAKIN4D_P4A>(cpu_double3 p, constant double* params, double &de) {
    const double w_add = params[0];  // [PVar-0x10] default: 0

    // C values

    double x = p.x, y = p.y, z = p.z, w = de;

    // Repeat the iteration twice (power 4 = two applications of power 2)
    for (int iter = 0; iter < 2; iter++) {
        // Complex multiply: w_new = 2 * (x,y) * (z,w)
        // Real part: x*z - y*w, doubled
        // Imag part: x*w + y*z, doubled
        double tw_re = 2.0 * (x * z - y * w);
        double tw_im = 2.0 * (x * w + y * z);

        double s = x * x + y * y;     // |z_complex|^2
        double t = z * z + w * w;     // |w_complex|^2

        double tz_re, tz_im;
        if (t < s) {
            // z_new = sqr(z_complex) * (1 - t/s)
            // ASM: fdivrp → t/s, fchs → -t/s, fld1; faddp → 1-t/s
            double ratio = 1.0 - t / s;
            tz_re = (x * x - y * y) * ratio;
            tz_im = 2.0 * x * y * ratio;
        } else {
            // z_new = sqr(w_complex) * (s/t - 1)
            // ASM: fdivp → s/t, fld1; fchs; faddp → s/t-1
            double ratio = s / t - 1.0;
            tz_re = (z * z - w * w) * ratio;
            tz_im = 2.0 * z * w * ratio;
        }

        // Update: z_complex = tz, w_complex = tw
        x = tz_re;
        y = tz_im;
        z = tw_re;   // stored to [ecx] first, then...
        w = tw_im;   // stored to [ecx+8]
        // Wait: ASM stores tw_im to [edx], tw_re to [ebx]
        // then tz to [ecx+8] and [ecx]
        // Actually: let me re-check the store order
        // 0x60: fadd st0 → tw_im doubled; fmul st2 → tw_im*ratio
        //   fstp [edx] → y = tw_im*ratio? No...
        // Let me re-trace more carefully:
        //
        // At the branch t < s:
        //   0x47: fdivrp st1 → st0 = t/s (was s at st1, t at st0: st1/st0 → reversed = t/s? No!)
        //   Actually FDIVRP st1: st1 = st0/st1, pop. st0=t, st1=s → st1 = t/s, pop → st0=t/s
        //   Wait: t was at st0, s at st1. FDIVRP st1 = st1 = st0/st1 = t/s. pop → st0 = t/s
        //   Hmm no: FDIVRP st(i), st(0): st(i) = st(0)/st(i). So st1 = st0/st1 = t/s. pop.
        //   0x49: fchs → -t/s
        //   0x4B: fld1 → 1; faddp → 1 - t/s
        //   0x4: fld [ebx] → x;  fmul st0 → x^2
        //   0x53: fld [edx] → y;  fmul st0 → y^2
        //   0x57: fsubp st1 → x^2 - y^2
        //   0x59: fmul st1 → (x^2-y^2) * (1-t/s)    ← this is tz_re
        //   0x5C: fld [ebx] → x; fmul [edx] → x*y
        //   0x60: fadd st0 → 2*x*y; fmul st2 → 2*x*y * (1-t/s)  ← tz_im
        //   0x64: fstp [edx] → y = tz_im
        //   0x66: fstp [ebx] → x = tz_re
        //   0x68: fstp st0 → pop ratio
        //   Then tw stored:
        //   0x88: fstp [ecx+8] → w = tw_im
        //   0x8B: fstp [ecx]   → z = tw_re
        //   Wait, these are at 0x88 which is AFTER the if/else
        //   And tw_re/tw_im were computed at 0x0E-0x29 and are still on stack

        // OK so the mapping is:
        //   x = tz_re, y = tz_im (the "z" complex)
        //   z = tw_re, w = tw_im (the "w" complex)
    }

    // Add C (C addition removed — engine adds C separately)
    w += w_add;

    de = w;
    return cpu_double3(x, y, z);
}

// Formula metadata
constant FormulaInfo formula_info_Makin4D_p4a = {
    .id = FORMULA_MAKIN4D_P4A,
    .name = "Makin4D p4a",
    .param_count = 1,
    .complexity = 314,
    .uses_transcendental = false
};

/*
Original M3 Description:
Formula proposed by David Makin at the forums (July 21, 2014).

    // here z and w are the complex
    // numbers containing x,y,z and w coords;
    double s=|z|
    double t=|w|
    complex wt = w
    w = 2.0*z*w
    if s>=t
      z = sqr(z)*(1.0 - t/s)
    else
      z = (s/t-1.0)*sqr(wt)
    endif
    (repeat twice then add C)

Parameters:
  [0] w add: double (default: 0)
  [1..4] Cx, Cy, Cz, Cw (engine-provided)

Note: "z" and "w" in the description refer to complex numbers (x,y) and (z,w).
The formula computes power 4 by applying the base step twice.
*/
