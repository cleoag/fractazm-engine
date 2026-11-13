// Makin4D p8b.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Makin4D p8b
// Hand-ported from ASM: 426 bytes, DEoption=4
// 4D formula: uses de parameter as w coordinate input/output

constexpr unsigned int FORMULA_MAKIN4D_P8B = 10333;

// ASM trace:
//   esi = context, edi = PVar, ebx = &x
//   eax/ebx = &x, edx = &y, ecx = &z, [ecx+8] = w
//   Parameters:
//     [edi-0x10] = w_add (double, default: 0)
//     [edi-0x18] = xy_mul (double, default: 0) — not referenced by the ASM
//   C values: [esi+0x18]=Cx, [esi+0x20]=Cy, [esi+0x28]=Cz, [esi-0x38]=Cw
//
//   This formula applies the Makin iteration THREE times (power 8 = 2^3).
//   Each step:
//     z_complex = (x,y), w_complex = (z,w)
//     m = |z|^2 + |w|^2  (total magnitude)
//     temp_w = 2 * z * w  (complex multiply, doubled)
//     temp_z = 2 * sqr(z) (complex square, doubled)
//     n = |temp_z| = sqrt(temp_z_re^2 + temp_z_im^2)
//     z_new = temp_z * (1 - m/n)
//     w_new = temp_w  (already computed)
//     Then update z,w and (z,w) are stored to corresponding memory
//
//   The key difference from p4a is the intermediate formula:
//     w = 2*z*w (same)
//     z = 2*sqr(z)*(1 - (|z|+|w|)/|2*sqr(z)|)
//       Wait, description says: double m = |z|+|w|, but ASM computes
//       m = x^2+y^2 + z^2+w^2 = |z|^2+|w|^2
//
//   From ASM carefully:
//   Block at 0x0E-0x84 (first iteration):
//     0x0E: x^2+y^2 = |z|^2                         (st0)
//     0x1A: z^2+w^2 = |w|^2                          (st0), |z|^2 at st1
//     0x27: faddp → m = |z|^2+|w|^2                  (st0=m)
//     0x2A: 2*(x*z - y*w)                             (st0=tw_re, st1=m)
//     0x38: 2*(x*w + y*z)                             (st0=tw_im, st1=tw_re, st2=m)
//     0x46: 2*(x^2-y^2)                               (st0=tz_re, rest below)
//     0x53: 4*x*y                                     (st0=tz_im, st1=tz_re, st2=tw_im, st3=tw_re, st4=m)
//     store tz_im → [edx], tz_re → [ebx]
//     0x5: reload [ebx]^2 + [edx]^2 = tz_re^2+tz_im^2
//     0x69: fsqrt → n = |temp_z|
//     0x6B: fdivr st3 → st0 = m/n (st3=m, fdivr = st3/st0 = m/n)
//     0x6D: fchs → -m/n
//     0x6: fld1 → 1; faddp → 1 - m/n
//     0x73: fld [ebx] = tz_re; fmul st1 → tz_re*(1-m/n); fstp [ebx]
//     0x79: fld [edx] = tz_im; fmulp st1 → tz_im*(1-m/n); fstp [edx]
//     0x7: fstp [ecx+8] → w = tw_im
//     0x82: fstp [ecx] → z = tw_re
//     0x84: fstp st0 → pop m
//   Then the block repeats two more times (total 3 iterations for power 8)
//   Finally add C.

template<>
cpu_double3 execute_formula<FORMULA_MAKIN4D_P8B>(cpu_double3 p, constant double* params, double &de) {
    const double w_add = params[0];   // [PVar-0x10]


    double x = p.x, y = p.y, z = p.z, w = de;

    // Three iterations of the base step (2^3 = power 8)
    for (int iter = 0; iter < 3; iter++) {
        // m = |z_complex|^2 + |w_complex|^2 = x^2+y^2+z^2+w^2
        double s = x*x + y*y;
        double t = z*z + w*w;
        double m = s + t;

        // temp_w = 2 * (x,y) * (z,w) complex multiply
        double tw_re = 2.0 * (x*z - y*w);
        double tw_im = 2.0 * (x*w + y*z);

        // temp_z = 2 * sqr(x,y) = 2*(x^2-y^2, 2xy) = (2(x^2-y^2), 4xy)
        double tz_re = 2.0 * (x*x - y*y);
        double tz_im = 4.0 * x * y;

        // n = |temp_z| = sqrt(tz_re^2 + tz_im^2) = 2*(x^2+y^2) in real arithmetic.
        // Rewriting 1 - m/n as (s - t)/(2*s) avoids catastrophic cancellation on
        // platforms where long double is only 64-bit, while remaining algebraically
        // equivalent to the x87 sequence.
        double scale = (s - t) / (2.0 * s);

        // z_new = temp_z * scale
        x = tz_re * scale;
        y = tz_im * scale;

        // w_new = temp_w (already computed)
        z = tw_re;   // stored to [ecx]
        w = tw_im;   // stored to [ecx+8]
    }

    // Add C (C addition removed — engine adds C separately)
    w += w_add;

    de = w;
    return cpu_double3(x, y, z);
}

// Formula metadata
constant FormulaInfo formula_info_Makin4D_p8b = {
    .id = FORMULA_MAKIN4D_P8B,
    .name = "Makin4D p8b",
    .param_count = 1,
    .complexity = 426,
    .uses_transcendental = true
};

/*
Original M3 Description:
Formula proposed by David Makin at the forums (July 21, 2014).

    // here z and w are the complex
    // numbers containing x,y,z and w coords;
    double m = |z|+|w|
    w = 2.0*z*w
    z = 2.0*sqr(z)
    double n = cabs(z)
    z = z*(1-m/n)
    (three times then add C)

NOTE: Description says |z|+|w| (magnitudes) but ASM computes
|z|^2+|w|^2 (squared magnitudes). Trust the ASM.

Parameters:
  [0] w add: double (default: 0)
  [1..4] Cx, Cy, Cz, Cw (engine-provided)

The "xy mul" parameter in the .m3 file appears unused in the code.
*/
