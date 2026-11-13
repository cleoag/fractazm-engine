// Makin4D p8a.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Makin4D p8a
// Hand-ported from ASM: 441 bytes, DEoption=4
// 4D formula: uses de parameter as w coordinate input/output

constexpr unsigned int FORMULA_MAKIN4D_P8A = 10334;

// ASM trace:
//   esi = context, edi = PVar, ebx = &x
//   Parameters: [edi-0x10] = w_add (double, default: 0)
//   C values: [esi+0x18]=Cx, [esi+0x20]=Cy, [esi+0x28]=Cz, [esi-0x38]=Cw
//
//   This is the same Makin formula as p4a but repeated THREE times
//   instead of twice, giving power 8 (= 2 applied 3 times).
//
//   Each iteration step (same as p4a):
//     z_complex = (x,y), w_complex = (z,w)
//     temp_w = 2*(x*z - y*w, x*w + y*z)   (complex multiply, doubled)
//     s = x^2 + y^2  (|z_complex|^2)
//     t = z^2 + w^2  (|w_complex|^2)
//     if t < s:
//       ratio = 1 - t/s
//       z_new = (x^2-y^2, 2*x*y) * ratio
//     else:
//       ratio = s/t - 1
//       z_new = (z^2-w^2, 2*z*w) * ratio
//     x = z_new_re, y = z_new_im, z = tw_re, w = tw_im
//
//   After THREE repetitions, add C.
//
//   Code structure is identical to p4a but with 3 copies of the block
//   instead of 2 (0x0E-0x8B, 0x8D-0x10A, 0x10C-0x189), then C addition.

template<>
cpu_double3 execute_formula<FORMULA_MAKIN4D_P8A>(cpu_double3 p, constant double* params, double &de) {
    const numeric::X87Scope extendedScope;
    const double w_add = params[0];   // [PVar-0x10]


    double x = p.x, y = p.y, z = p.z, w = de;

    // Three iterations (power 8 = 2^3)
    for (int iter = 0; iter < 3; iter++) {
        // The x86 path stores x/y/z/w back to double after each sub-step, but
        // the intermediate products and divisions still happen in x87 extended
        // precision. Use numeric::X87Real for the local temporaries and then
        // materialize back to double at the same points as the ASM stores.
        numeric::X87Real lx = x, ly = y, lz = z, lw = w;

        numeric::X87Real tw_re = lx * lz;
        tw_re -= ly * lw;
        tw_re += tw_re;

        numeric::X87Real tw_im = lx * lw;
        tw_im += ly * lz;
        tw_im += tw_im;

        numeric::X87Real s = lx * lx + ly * ly;     // |z_complex|^2
        numeric::X87Real t = lz * lz + lw * lw;     // |w_complex|^2

        numeric::X87Real tz_re, tz_im;
        // Match x87 FCOM/JAE semantics: unordered compares do not jump to the
        // "t >= s" block, so NaN inputs fall through into the first branch.
        // Keep the ratio math in extended precision so near-underflow inner
        // steps collapse to the same final double zeros as x87 instead of
        // reaching 0/0 early in pure double precision.
        if (s == numeric::X87Real(0.0) && t == numeric::X87Real(0.0) &&
            (lx != numeric::X87Real(0.0) || ly != numeric::X87Real(0.0) || lz != numeric::X87Real(0.0) || lw != numeric::X87Real(0.0))) {
            tz_re = numeric::X87Real(0.0);
            tz_im = numeric::X87Real(0.0);
            tw_re = numeric::X87Real(0.0);
            tw_im = numeric::X87Real(0.0);
        } else if (!(t >= s)) {
            numeric::X87Real ratio = (s - t) / s;
            tz_re = (lx * lx - ly * ly) * ratio;
            tz_im = numeric::X87Real(2.0) * lx * ly * ratio;
        } else {
            numeric::X87Real ratio = (s - t) / t;
            tz_re = (lz * lz - lw * lw) * ratio;
            tz_im = numeric::X87Real(2.0) * lz * lw * ratio;
        }

        volatile double x_store = static_cast<double>(tz_re);
        volatile double y_store = static_cast<double>(tz_im);
        volatile double z_store = static_cast<double>(tw_re);
        volatile double w_store = static_cast<double>(tw_im);
        x = x_store;
        y = y_store;
        z = z_store;
        w = w_store;
    }

    // Add C (C addition removed — engine adds C separately)
    w += w_add;

    de = w;
    return cpu_double3(x, y, z);
}

// Formula metadata
constant FormulaInfo formula_info_Makin4D_p8a = {
    .id = FORMULA_MAKIN4D_P8A,
    .name = "Makin4D p8a",
    .param_count = 1,
    .complexity = 441,
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
    (repeat three times then add C)

Parameters:
  [0] w add: double (default: 0)
  [1..4] Cx, Cy, Cz, Cw (engine-provided)

Note: Identical algorithm to Makin4D p4a but applied 3 times (power 8)
instead of 2 times (power 4). The description uses |z| and |w| (magnitudes)
but ASM computes |z|^2 and |w|^2 (squared magnitudes). Trust the ASM.
*/
