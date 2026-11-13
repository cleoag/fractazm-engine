// _Tanh4D.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _Tanh4D
// Hand-ported from ASM: 488 bytes, DEoption=4
// 4D formula: uses de parameter as w coordinate input/output

constexpr unsigned int FORMULA_TANH4D = 10335;

// ASM trace:
//   esi = context, edi = PVar, ebx = &x
//   Parameters:
//     [edi-0x10] = Mulx (double, default: 2.0)
//     [edi-0x18] = Muly (double, default: 0.0)
//     [edi-0x20] = Mulz (double, default: 0.0)
//     [edi-0x28] = Mulw (double, default: 0.0)
//     [edi-0x30] = Final Mul (double, default: 1.0)
//   Constants:
//     [edi+0x00] = 1e-200 (epsilon for division safety)
//
//   Algorithm:
//   Step 1: Non-commutative quaternion multiply Q = (x,y,z,w) q* M = (Mulx,Muly,Mulz,Mulw)
//     Hamilton product:
//     x' = x*Mx - y*My - z*Mz - w*Mw
//     y' = x*My + y*Mx + z*Mw - w*Mz    (wait, let me verify from ASM)
//     Actually ASM at 0x10-0xB8:
//       x' = x*Mx - y*My - z*Mz - w*Mw       (fchs on y*My, z*Mz, w*Mw terms)
//       Actually: x*Mx + (-y*My) + (-z*Mz) + (-w*Mw)? No...
//       0x10: x; 0x13: *Mx; 0x18: y; 0x1B: *My; 0x1E: fchs; 0x20: faddp → x*Mx - y*My
//       0x22: z; 0x25: *Mz; 0x28: fchs; 0x2A: faddp → x*Mx-y*My-z*Mz
//       0x2C: w; 0x2: *Mw; 0x32: fchs; 0x34: faddp → x*Mx-y*My-z*Mz-w*Mw = x'
//       Wait that's the Hamilton real part, but Mulx is at [edi-0x10], Muly at [edi-0x18]...
//       Hmm, but the description says this is tanh with Hamilton product.
//
//     Let me just trace all 4 outputs:
//     x'  = x*Mx - y*My - z*Mz - w*Mw           (standard Hamilton real)
//     y'  = x*My + y*Mx + z*Mw - w*Mz            (Hamilton i)
//       0x38: x*My + 0x43: y*Mx (faddp) + 0x4D: z*Mw (faddp) + 0x57: w*Mz (fchs, faddp)
//       → x*My + y*Mx + z*Mw - w*Mz  BUT look at ASM:
//       0x3B: x*My ([edi-0x18]) + 0x43: y*Mx ([edi-0x10]) + 0x4D: z*Mw ([edi-0x28])
//       + 0x57: w*Mz ([edi-0x20]), fchs → -w*Mz
//       = x*My + y*Mx + z*Mw - w*Mz ← standard Hamilton
//     z'  = x*Mz - y*Mw + z*Mx + w*My            (Hamilton j)
//       0x63: x*Mz + 0x6E: y*Mw, fchs → -y*Mw + 0x7A: z*Mx + 0x84: w*My
//     w'  = x*Mw + y*Mz - z*My + w*Mx            (Hamilton k)
//       0x8B: x*Mw + 0x98: y*Mz + 0xA2: z*My, fchs → -z*My + 0xAC: w*Mx
//
//   Step 2: Compute |imag| = sqrt(w'^2 + y'^2 + z'^2) and exp(x')
//     0xBA: w'^2 + y'^2 + z'^2 → |imag|^2
//     0xCB: fsqrt → |imag|
//     0xCD: fld st0 → dup |imag|; fsincos → cos(|imag|), sin(|imag|)
//     0xD3: exp(x') via fld x'; fldl2e; fmulp; fld1; fprem; f2xm1; faddp; fscale
//     0xE9: fld st0 → dup exp(x')
//     0xEB: exp(x') * cos(|imag|) → stored to [ebx] as real part
//     0xEF: exp(x') * sin(|imag|)  (for imaginary scaling)
//     0xF1: check |imag| vs epsilon; if |imag| > epsilon: divide by |imag|
//     0x100: scale y', z', w' by (exp(x') * sin(|imag|) / |imag|)
//
//   Step 3: Compute tanh from exp result
//     Let E = exp(Q*M), which is now (e_re, e_y, e_z, e_w)
//     tanh = (E-1) / (E+1) for the real part, and similar for imaginary
//     Actually: tanh(Q*M/2) = (exp(Q*M)-1)/(exp(Q*M)+1)
//
//     The ASM computes the quaternion division:
//     denominator = |E+1|^2 = (e_re+1)^2 + e_y^2 + e_z^2 + e_w^2
//     inv_denom = 1/denominator
//     numerator for each component:
//       For real: (e_re+1)*(e_re-1) + e_y^2 + e_z^2 + e_w^2
//       For y: (e_re+1)*e_y - e_y*(e_re-1)  = 2*e_y
//         Wait, quaternion division (A/B) = A * conj(B) / |B|^2
//         numerator A = E-1 = (e_re-1, e_y, e_z, e_w)
//         denominator B = E+1 = (e_re+1, e_y, e_z, e_w)
//         conj(B) = (e_re+1, -e_y, -e_z, -e_w)
//         A * conj(B): real = (e_re-1)(e_re+1) + e_y^2 + e_z^2 + e_w^2
//         A * conj(B): i = -(e_re-1)*e_y + e_y*(e_re+1) = 2*e_y
//         A * conj(B): j = -(e_re-1)*e_z + e_z*(e_re+1) = 2*e_z
//         A * conj(B): k = -(e_re-1)*e_w + e_w*(e_re+1) = 2*e_w
//         |B|^2 = (e_re+1)^2 + e_y^2 + e_z^2 + e_w^2
//
//   Step 4: Multiply result by Final_Mul

template<>
cpu_double3 execute_formula<FORMULA_TANH4D>(cpu_double3 p, constant double* params, double &de) {
    const double Mx    = params[0];   // [PVar-0x10] Mulx, default: 2.0
    const double My    = params[1];   // [PVar-0x18] Muly, default: 0.0
    const double Mz    = params[2];   // [PVar-0x20] Mulz, default: 0.0
    const double Mw    = params[3];   // [PVar-0x28] Mulw, default: 0.0
    const double fmul  = params[4];   // [PVar-0x30] Final Mul, default: 1.0
    const double eps   = 1e-200;      // [PVar+0x00]

    double x = p.x, y = p.y, z = p.z, w = de;

    // Step 1: Hamilton quaternion multiply Q * M
    double qx = x*Mx - y*My - z*Mz - w*Mw;
    double qy = x*My + y*Mx + z*Mw - w*Mz;
    double qz = x*Mz - y*Mw + z*Mx + w*My;
    double qw = x*Mw + y*Mz - z*My + w*Mx;

    // Step 2: Compute exp(Q*M)
    // exp(a + b*i + c*j + d*k) = exp(a) * (cos(|v|) + sin(|v|)/|v| * (b*i+c*j+d*k))
    // where |v| = sqrt(b^2 + c^2 + d^2)
    double imag_len = sqrt(qw*qw + qy*qy + qz*qz);
    double cos_v = cos(imag_len);
    double sin_v = sin(imag_len);
    double exp_a = exp(qx);

    double e_re = exp_a * cos_v;
    double scale = exp_a * sin_v;
    if (imag_len > eps) {
        scale /= imag_len;
    }
    double e_y = qy * scale;
    double e_z = qz * scale;
    double e_w = qw * scale;

    // Step 3: tanh = (E-1) * conj(E+1) / |E+1|^2
    // |E+1|^2 = (e_re+1)^2 + e_y^2 + e_z^2 + e_w^2
    double denom = (e_re + 1.0) * (e_re + 1.0) + e_y*e_y + e_z*e_z + e_w*e_w;
    double inv_d = 1.0 / denom;

    // Real part: (e_re-1)(e_re+1) + e_y^2 + e_z^2 + e_w^2
    double t_re = ((e_re + 1.0) * (e_re - 1.0) + e_y*e_y + e_z*e_z + e_w*e_w);
    // Imaginary parts: 2 * e_component
    // From the quaternion division: i-part = (e_re+1)*e_y - e_y*(e_re-1) = 2*e_y
    // But actually need to verify ASM for exact expression...
    // ASM at 0x163-0x17B: (e_re+1)*e_y + e_y*(-(e_re-1)) = (e_re+1)*e_y - e_y*(e_re-1) = 2*e_y
    double t_y = (e_re + 1.0) * e_y - e_y * (e_re - 1.0);   // = 2*e_y
    double t_z = (e_re + 1.0) * e_z - e_z * (e_re - 1.0);   // = 2*e_z
    double t_w = (e_re + 1.0) * e_w - e_w * (e_re - 1.0);   // = 2*e_w

    // Step 4: Apply inv_denom and Final_Mul
    double nx = inv_d * t_re * fmul;
    double ny = inv_d * t_y  * fmul;
    double nz = inv_d * t_z  * fmul;
    double nw = inv_d * t_w  * fmul;

    de = nw;
    return cpu_double3(nx, ny, nz);
}

// Formula metadata
constant FormulaInfo formula_info_Tanh4D = {
    .id = FORMULA_TANH4D,
    .name = "_Tanh4D",
    .param_count = 5,
    .complexity = 488,
    .uses_transcendental = true
};

/*
Original M3 Description:
*WARNING* - This function is almost useless for standard fractal use because
tanh is convergent for large arguments, and DE works for divergent fractals only.

tanh(x) := (exp(2*x)-1)/(exp(2*x)+1)

This function computes f*tanh(Q q* M / 2), following Hamiltonian theory.
Q is a quat (x,y,z,w).

tanh(Q) := final * (exp(Q q* M)-1) q/ (exp(Q q* M)+1)

Parameters:
  [0] Mulx: double (default: 2.0)
  [1] Muly: double (default: 0.0)
  [2] Mulz: double (default: 0.0)
  [3] Mulw: double (default: 0.0)
  [4] Final Mul: double (default: 1.0)
  [5] epsilon constant (default: 1e-200)

Note: This is a transform (pretransform), not a standard fractal formula.
The tanh function converges, making it unsuitable for standard DE rendering.
*/
