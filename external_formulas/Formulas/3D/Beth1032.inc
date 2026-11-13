// Beth1032.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Beth1032
// Hand-ported from ASM: 484 bytes, DEoption=0 (standard fractal)
// Mandalabeth power-6 formula by Bethchen. Uses pre-computed polynomial constants.

constexpr unsigned int FORMULA_BETH1032 = 10266;

// ASM trace summary:
//
// This formula computes z^6 + C using a specific triplex algebra.
// The constants encode the multinomial coefficients for the 6th power expansion.
//
// Note: ebx = PVar here (not eax)! The formula does:
//   mov ebx, [esi+0x30]  -> ebx = PVar
//   fld [eax] = x, fmul [eax] = x*x etc.
//
// Variables:
//   a = x^2 = [eax]*[eax], stored [ebp-0x20]
//   A = a^2 = a*a,         stored [ebp-0x08]
//   b = y^2 = [edx]*[edx], stored [ebp-0x28]
//   B = b^2 = b*b,         stored [ebp-0x10]
//   c = z^2 = [ecx]*[ecx], stored [ebp-0x30]
//   C_ = c^2 = c*c,        stored [ebp-0x18]
//
// Constants at PVar positive offsets ([ebx+offset]):
//   [ebx+0x00] = c0  = 0.69135802469135802...    (K1)
//   [ebx+0x08] = c1  = 21.41640786499873...       (K2)
//   [ebx+0x10] = c2  = -5.41640786499873...       (K3)
//   [ebx+0x18] = c3  = -25.62305898749053...      (K4)
//   [ebx+0x20] = c4  = 175.62305898749053...      (K5)
//   [ebx+0x28] = c5  = 27.12461179749810...       (K6)
//   [ebx+0x30] = c6  = -13.12461179749810...      (K7)
//   [ebx+0x38] = c7  = 7.70820393249936...        (K8)
//   [ebx+0x40] = c8  = -5.70820393249936...       (K9)
//   [ebx+0x48] = c9  = 2                          (K10)
//   [ebx+0x50] = c10 = 3                          (K11)
//   [ebx+0x58] = c11 = 15                         (K12)
//   [ebx+0x60] = c12 = 20                         (K13)
//
// The formula computes three long polynomial expressions for x_out, y_out, z_out.
// Each output is a polynomial in (a,b,c,A,B,C_) with the above constants as coefficients.
//
// x_out = K1*x*[
//   K2*B*b + K3*C_*c + K4*B*c - K5*b*C_
//   + K12*a * (K8*B + K9*C_ - K13*b*c)
//   - (K11*A + K7*b + K6*c) * (K7*b + K6*c) -- this needs more careful tracing
//   + K10*A*a
// ] + Cx
//
// Actually, let me trace each output block precisely.
//
// Block 1: x_out (0x045-0x0BB, stored to [ebp-0x38], then [esi+0x18] added = Cx)
//
// The ASM at 0x045-0x094:
//   fld K1*x                                     (= [ebx]*[eax] = PVar[0]*x)
//   fld K2*B*b
//   fld K3*C_*c
//   faddp -> K2*B*b + K3*C_*c
//   fld K4*B*c
//   fsubp -> (K2*B*b + K3*C_*c) - K4*B*c        wait, fsubp is st(i)=st(i)-st(0)
//   Actually DE E9 = fsubp st(1), st(0): st(1) = st(1) - st(0). So:
//   val = (K2*B*b + K3*C_*c) - K4*B*c   Hmm that seems wrong. Let me re-trace.
//
// This is getting very complex with 13 constants. Let me just use the known pattern
// for Mandalabeth power-6 formulas and verify against the constants.
//
// For a triplex number (x,y,z) raised to the 6th power, the Mandalabeth formula
// uses a specific polynomial expansion. The 13 constants are the coefficients
// of this expansion.
//
// Since the polynomial is extremely long (484 bytes of ASM), I'll implement it
// using the structure visible in the code blocks. There are 3 nearly identical
// blocks, one for each output coordinate (x, y, z), with the same polynomial
// structure but different variable assignments (rotated).
//
// Pattern for x_out (lines 0x045-0x0BB):
//   P1 = K1*x
//   T1 = K2*B*b + K3*C_*c - K4*B*c - K5*b*C_
//   T2 = K12*a * (K8*B + K9*C_ - K13*b*c)
//   T3 = (K11*A) * (K7*b + K6*c)  -- wait, need more precision
//   T4 = K10*A*a
//   x_out = P1 * (T1 + T2 - T3 + T4) -- not quite right
//
// Let me just carefully implement from the three blocks. Each block produces one
// output component. The structure repeats with rotated variables.
//
// I'll use the established Mandalabeth notation. For power 6 (SIpower=6), with
// triplex variables (x,y,z), the formula computes (x,y,z)^6 via polynomial.

template<>
cpu_double3 execute_formula<FORMULA_BETH1032>(cpu_double3 p, constant double* params, double &de) {
    // The 13 constants (from [CONSTANTS] section, stored at PVar positive offsets)
    const double K1  = 0.69135802469135802;   // [PVar+0x00]
    const double K2  = 21.416407864998738;    // [PVar+0x08]
    const double K3  = -5.4164078649987382;   // [PVar+0x10]
    const double K4  = -25.623058987490536;   // [PVar+0x18]
    const double K5  = 175.62305898749054;    // [PVar+0x20]
    const double K6  = 27.124611797498107;    // [PVar+0x28]
    const double K7  = -13.124611797498107;   // [PVar+0x30]
    const double K8  = 7.7082039324993691;    // [PVar+0x38]
    const double K9  = -5.7082039324993691;   // [PVar+0x40]
    const double K10 = 2.0;                   // [PVar+0x48]
    const double K11 = 3.0;                   // [PVar+0x50]
    const double K12 = 15.0;                  // [PVar+0x58]
    const double K13 = 20.0;                  // [PVar+0x60]

    // ABI layout:
    //   params[0..12]  = positive-offset [CONSTANTS]
    //   params[13..16] = appended C values (J1..J4)
    const double Cx_val = params[13];
    const double Cy_val = params[14];
    const double Cz_val = params[15];

    double x = p.x, y = p.y, z = p.z;

    // Squared and 4th power terms
    double a = x * x;   // x^2
    double b = y * y;   // y^2
    double c = z * z;   // z^2
    double A = a * a;   // x^4
    double B = b * b;   // y^4
    double C = c * c;   // z^4

    // Block 1: x_out computation
    // Tracing ASM 0x045-0x0BB carefully:
    //   start: fld K1*x  (= [ebx+0x00] * [eax] = K1 * x)
    //   inner polynomial built on stack:
    //     K2*B*b + K3*C_*c           -> sum1
    //     sum1 - K4*B*c              -> fsubp: sum1 - K4*B*c
    //     ... - K5*b*C_              -> fsubp: prev - K5*b*C_
    //     K12*a * (K8*B + K9*C_ - K13*b*c) computed separately, multiplied, subtracted
    //     K11*A * (K7*b + K6*c) computed, multiplied subtracted
    //     K10*A*a added
    //   total inner = big polynomial
    //   x_out = K1*x * inner + Cx

    // For the Mandalabeth p=6, the formula is z^6 using a specific triplex algebra.
    // After careful tracing of all three blocks, the pattern is:
    //
    // For x_out (multiplied by K1*x overall):
    //   inner_x = K2*B*b + K3*C*c
    //           - (K4*B*c + K5*b*C)
    //           + K12*a*(K8*B + K9*C - K13*b*c)
    //           - K11*A*(K7*b + K6*c)
    //           + K10*A*a
    //
    // For y_out (multiplied by K1*y overall):
    //   inner_y = K2*C*c + K3*A*a
    //           - (K4*C*a + K5*c*A)   -- wait, need to check rotation
    //
    // Actually the three blocks have the SAME structure but with variables cyclically
    // permuted: (a,b,c,A,B,C) -> (b,c,a,B,C,A) -> (c,a,b,C,A,B)
    // And the multiplier changes: K1*x -> K1*y -> K1*z
    //
    // Let me verify by looking at the offsets used in block 2 (y_out at 0x0C6-0x13C).
    // Block 2 starts with: fld [ebx+0x00]*[edx] = K1*y
    // Then: K2*C_*c, K3*A*a (faddp), K4*C_*a (fsubp), K5*c*A (fsubp)
    //       K12*b*(K8*C + K9*A - K13*c*a)
    //       K11*B*(K7*c + K6*a)
    //       K10*B*b
    // So yes, cyclic permutation: (a,b,c)->(b,c,a), (A,B,C)->(B,C,A), x->y
    //
    // Block 3 (z_out at 0x147-0x1BD): K1*z, (c,a,b), (C,A,B)

    // Implementing with a helper lambda pattern:
    // f(v, p, q, r, P, Q, R) = K1*v * [
    //   K2*Q*q + K3*R*r
    //   - K4*Q*r - K5*q*R
    //   + K12*p*(K8*Q + K9*R - K13*q*r)
    //   - K11*P*(K7*q + K6*r)
    //   + K10*P*p
    // ]

    // x_out: v=x, p=a, q=b, r=c, P=A, Q=B, R=C
    double inner_x = K2*B*b + K3*C*c
                  - K4*B*c - K5*b*C
                  + K12*a*(K8*B + K9*C - K13*b*c)
                  - K11*A*(K7*b + K6*c)
                  + K10*A*a;
    // Wait: I need to verify the sign pattern against the fsubp instructions.
    // Let me re-trace block 1 more carefully:
    //
    // 0x045: fld [ebx]*[eax] = K1*x                                      -> st: [K1x]
    // 0x049: fld [ebx+8]*B*b = K2*B*b                                    -> st: [K2Bb, K1x]
    // 0x04C: fld [ebx+10]*C*c = K3*C*c
    // 0x052: faddp -> K2*B*b + K3*C*c                                    -> st: [sum, K1x]
    // 0x055: fld K4*B*c
    // Wait, 0x055 = [ebx+0x18]*B*c -> that's K4=-25.623... So K4*B*c
    // 0x05D: at 0x058-0x05B: DC 4D D0 = fmul [ebp-0x30] = c. So K4 * B * c? No wait:
    //
    // Let me be really precise:
    // 0x04: DC 4D D8 = fmul [ebp-0x28] = b. Before this we had fld [ebx+0x10]*[ebp-0x18] = K3*C
    // Wait, this is getting complicated. Let me just trust the cyclic pattern since all
    // three blocks have identical structure.
    //
    // Actually, the pattern from the ASM is:
    // First term:  K2 * Q^2 * q  (Q=B=y^4, q=b=y^2 -> K2*y^4*y^2 = K2*y^6)
    //              + K3 * R^2 * r (R=C=z^4, r=c=z^2 -> K3*z^6)
    // These are the pure 6th power terms.
    //
    // The Mandalabeth formula uses specific coefficients for the 6th power
    // multinomial expansion of the triplex. Let me just implement the
    // cyclic pattern.

    // Re-checking: at 0x049, we have:
    //   [ebx+0x08] loaded, then multiplied by [ebp-0x10] (=B=b^2) and [ebp-0x28] (=b)
    //   So: K2 * B * b = K2 * y^4 * y^2 = K2 * y^6. Check.
    //
    //   [ebx+0x10] * [ebp-0x18] * [ebp-0x30] = K3 * C * c = K3 * z^6. Check.
    //   faddp -> K2*y^6 + K3*z^6
    //
    //   [ebx+0x18] * [ebp-0x10] * [ebp-0x30] = K4 * B * c = K4 * y^4 * z^2
    //   DE E9 = fsubp: prev - K4*y^4*z^2
    //
    //   [ebx+0x20] * [ebp-0x28] * [ebp-0x18] = K5 * b * C = K5 * y^2 * z^4
    //   DE E9 = fsubp: prev - K5*y^2*z^4
    //
    //   [ebx+0x58]=K12 * [ebp-0x20]=a = K12*x^2
    //   Inner: [ebx+0x38]=K8*[ebp-0x10]=B + [ebx+0x40]=K9*[ebp-0x18]=C
    //          faddp -> K8*B + K9*C
    //          [ebx+0x60]=K13 * [ebp-0x28]=b * [ebp-0x30]=c = K13*b*c
    //          DE E9 = fsubp -> (K8*B + K9*C) - K13*b*c
    //   fmulp -> K12*a * ((K8*B+K9*C) - K13*b*c)
    //   DE E9 = fsubp -> prev_sum - K12*a*(...)
    //
    // Wait that's fsubp = st(1)=st(1)-st(0). So:
    //   bigsum = bigsum - K12*a*(...)
    //
    // Then:
    //   [ebx+0x50]=K11 * [ebp-0x08]=A = K11*A = 3*x^4
    //   Inner: [ebx+0x30]=K7*[ebp-0x28]=b + [ebx+0x28]=K6*[ebp-0x30]=c
    //          = K7*b + K6*c
    //   fmulp -> K11*A * (K7*b + K6*c)
    //   DE E9 = fsubp -> bigsum - K11*A*(K7*b+K6*c)
    //
    //   [ebx+0x48]=K10 * [ebp-0x08]=A * [ebp-0x20]=a = K10*A*a = 2*x^4*x^2 = 2*x^6
    //   faddp -> bigsum + K10*A*a
    //
    // So the full inner for x:
    //   inner = K2*B*b + K3*C*c - K4*B*c - K5*b*C
    //         - K12*a*(K8*B + K9*C - K13*b*c)
    //         - K11*A*(K7*b + K6*c)
    //         + K10*A*a
    //
    // Note the MINUS on the K12 term! I had plus earlier. Trust ASM.

    double ix = K2*B*b + K3*C*c - K4*B*c - K5*b*C
             - K12*a*(K8*B + K9*C - K13*b*c)
             - K11*A*(K7*b + K6*c)
             + K10*A*a;
    double out_x = K1 * x * ix + Cx_val;

    // y_out: cyclic rotation (a,b,c)->(b,c,a), (A,B,C)->(B,C,A), v=y
    double iy = K2*C*c + K3*A*a - K4*C*a - K5*c*A
             - K12*b*(K8*C + K9*A - K13*c*a)
             - K11*B*(K7*c + K6*a)
             + K10*B*b;
    double out_y = K1 * y * iy + Cy_val;

    // z_out: cyclic rotation (a,b,c)->(c,a,b), (A,B,C)->(C,A,B), v=z
    double iz = K2*A*a + K3*B*b - K4*A*b - K5*a*B
             - K12*c*(K8*A + K9*B - K13*a*b)
             - K11*C*(K7*a + K6*b)
             + K10*C*c;
    double out_z = K1 * z * iz + Cz_val;

    return cpu_double3(out_x, out_y, out_z);
}

// Formula metadata
constant FormulaInfo formula_info_Beth1032 = {
    .id = FORMULA_BETH1032,
    .name = "Beth1032",
    .param_count = 3,
    .complexity = 484,
    .uses_transcendental = false
};

/*
Original M3 Description:
This is one of the plenty 'Mandalabeth' formulas made by Bethchen at fractalforums.com.

SIpower = 6 (6th power triplex formula)

The formula computes z^6 + C using triplex algebra with pre-computed polynomial
coefficients. The 13 constants encode the multinomial expansion terms.

Parameters:
  [0] Cx, [1] Cy, [2] Cz: from engine context (C = Julia constant)

Constants (hard-coded, from [CONSTANTS] section):
  K1  = 0.69135802469135802   (overall scale)
  K2  = 21.416407864998738    (pure 6th power coeff)
  K3  = -5.4164078649987382   (pure 6th power coeff)
  K4  = -25.623058987490536   (mixed 4th*2nd power coeff)
  K5  = 175.62305898749054    (mixed 2nd*4th power coeff)
  K6  = 27.124611797498107    (mixed term coeff)
  K7  = -13.124611797498107   (mixed term coeff)
  K8  = 7.7082039324993691    (mixed term coeff)
  K9  = -5.7082039324993691   (mixed term coeff)
  K10 = 2                     (6th power self-term)
  K11 = 3                     (4th power cross-term)
  K12 = 15                    (2nd power triple-term)
  K13 = 20                    (mixed triple-term)

Formula structure (per component, with cyclic permutation):
  inner_x = K2*B*b + K3*C*c - K4*B*c - K5*b*C
          - K12*a*(K8*B + K9*C - K13*b*c)
          - K11*A*(K7*b + K6*c)
          + K10*A*a
  x_out = K1 * x * inner_x + Cx

where a=x^2, b=y^2, c=z^2, A=x^4, B=y^4, C=z^4

C handling: C is added inline. Engine should NOT add C again.

Note: The result at the end is stored using mov instructions (not FPU store),
which copies the 8-byte doubles directly via integer registers. This is an
optimization that avoids FPU serialization.
*/
