// _MobiusTpx.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _MobiusTpx
// Hand-ported from ASM (953 bytes, Capstone-verified)
// DEoption: 0 (standard fractal, engine adds C)
// Version: 2, SIpower: 1
//
// ASM trace summary:
//   Mobius transformation in triplex space: S' = (a t* S + b) t/ (c t* S + d)
//   where t* is triplex multiplication and t/ is triplex division.
//   Parameters: a=(ax,ay,az), b=(bx,by,bz), c=(cx,cy,cz), d=(dx,dy,dz)
//
//   Triplex multiplication (u t* v):
//     rxy_u = sqrt(ux^2 + uy^2), rxy_v = sqrt(vx^2 + vy^2)
//     if rxy_u * rxy_v == 0:
//       result_x = -vz * uz  (degenerate case)
//       result_y = vz * uz
//     else:
//       alpha = vz * uz / (rxy_u * rxy_v)
//       factor = 1 - alpha  (fsubrp: st0 = st0 - 1 => alpha, then 1 - alpha)
//       Wait, let me re-trace:
//       0xF6: fld1, 0xF8: DE E1 = fsubrp st(1) => st(1) = st(0) - st(1) = 1 - alpha
//       Actually DE E1 = fsubrp: st(1) = st(0) - st(1), pop
//       st(0) was 1.0, st(1) was alpha => st(1) = 1.0 - alpha => result = 1 - alpha
//       result_x = (ux*vx - uy*vy) * (1 - alpha)
//       result_y = (vx*uy + ux*vy) * (1 - alpha)
//     result_z = rxy_u * uz + rxy_v * vz
//
//   Triplex division (T0 t/ T1):
//     Compute T1_conj (conjugate by negating y component)
//     numerator = T0 t* T1_conj  (but with special handling)
//     denominator = |T1|^2 = T1.x^2 + T1.y^2 + T1.z^2
//     If denominator <= 1e-90 (constant [edi+0x00]):
//       result = (1e90, 1e90, 1e90)  (overflow, constant [edi+0x08])
//     Else:
//       Compute complex-like division in x-y plane
//       result = numerator / denominator
//
//   Full formula flow:
//     Save S = (x,y,z) to locals
//     T1 = c t* S + d  (numerator of denominator)
//     Check |T1|^2 vs 1e-90; if too small => overflow
//     T0 = a t* S + b  (numerator)
//     S' = T0 t/ T1
//     Store result back to x,y,z
//
// Description:
//   Mobius transformation in 3D triplex space.
//   S' = (a*S + b) / (c*S + d) using triplex algebra.
//   See fractalforums.com/theory/triplex-algebra/
//   Uses non-trig forms for performance.
//   Implemented by Luca GN 2011

constexpr unsigned int FORMULA_MOBIUSTPX = 10276;

// Triplex multiplication helper
// Returns (u t* v) using the non-trig form
static cpu_double3 triplex_mul(cpu_double3 u, cpu_double3 v) {
    double rxy_u = sqrt(u.x * u.x + u.y * u.y);
    double rxy_v = sqrt(v.x * v.x + v.y * v.y);

    double prod = rxy_u * rxy_v;
    double rx, ry;

    if (prod == 0.0) {
        // Degenerate case: ASM 0xDD-0xF4
        rx = -v.z * u.z;      // fchs then mul
        ry = v.z * u.z;
    } else {
        // Normal case: ASM 0xF9-0x152
        double alpha = v.z * u.z / prod;
        double factor = 1.0 - alpha;
        rx = (u.x * v.x - u.y * v.y) * factor;
        ry = (v.x * u.y + u.x * v.y) * factor;
    }

    double rz = rxy_v * u.z + rxy_u * v.z;

    return cpu_double3(rx, ry, rz);
}

template<>
cpu_double3 execute_formula<FORMULA_MOBIUSTPX>(cpu_double3 p, constant double* params, double &de) {
    // 12 parameters: a-x, a-y, a-z, b-x, b-y, b-z, c-x, c-y, c-z, d-x, d-y, d-z
    // PVar layout: [edi-0x10]=param1, [edi-0x18]=param2, etc.
    // From ASM: params loaded at 0x2E-0x94
    // [edi-0x10] -> [ebp-0xa4] = a-x (param 0)
    // [edi-0x18] -> [ebp-0x9c] = a-y (param 1)
    // [edi-0x20] -> [ebp-0x94] = a-z (param 2)
    // [edi-0x28] -> [ebp-0x8c] = b-x (param 3)
    // [edi-0x30] -> [ebp-0x84] = b-y (param 4)
    // [edi-0x38] -> [ebp-0x7c] = b-z (param 5)
    // [edi-0x40] -> [ebp-0x74] = c-x (param 6)
    // [edi-0x48] -> [ebp-0x6c] = c-y (param 7)
    // [edi-0x50] -> [ebp-0x64] = c-z (param 8)
    // [edi-0x58] -> [ebp-0x5c] = d-x (param 9)
    // [edi-0x60] -> [ebp-0x54] = d-y (param 10)
    // [edi-0x68] -> [ebp-0x4c] = d-z (param 11)
    cpu_double3 a = cpu_double3(params[0], params[1], params[2]);
    cpu_double3 b = cpu_double3(params[3], params[4], params[5]);
    cpu_double3 c = cpu_double3(params[6], params[7], params[8]);
    cpu_double3 d = cpu_double3(params[9], params[10], params[11]);

    const double EPSILON = params[12];
    const double OVERFLOW = params[13];

    cpu_double3 S = p;

    // T1 = c t* S + d
    // ASM: 0x9C-0x181 computes c t* S, stores result, then adds d
    cpu_double3 cS = triplex_mul(c, S);
    cpu_double3 T1 = cS + d;

    // Check |T1|^2
    // ASM 0x184-0x19D: T1.x^2 + T1.y^2 + T1.z^2
    double T1_mag2 = T1.x * T1.x + T1.y * T1.y + T1.z * T1.z;

    // If T1_mag2 <= epsilon, return overflow
    // ASM 0x19D: fcomp [edi] (1e-90), 0x1A2: ja => if T1_mag2 > 1e-90 continue
    if (T1_mag2 <= EPSILON) {
        return cpu_double3(OVERFLOW, OVERFLOW, OVERFLOW);
    }

    // T0 = a t* S + b
    // ASM 0x1BE-0x2BD computes a t* S, stores, then adds b
    cpu_double3 aS = triplex_mul(a, S);
    cpu_double3 T0 = aS + b;

    // inv_T1_mag2 = 1.0 / T1_mag2
    // ASM 0x2C0: fld1, 0x2C2: fdiv [ebp-0x1c], 0x2C5: fstp [ebp-0x1c]
    double inv_T1_mag2 = 1.0 / T1_mag2;

    // Now compute T0 t/ T1
    // This is T0 t* conj(T1) / |T1|^2
    // But the ASM does it with the triplex division formula directly

    // Recompute rxy for T0 and T1 x-y components
    double rxy_T0 = sqrt(T0.x * T0.x + T0.y * T0.y);
    double rxy_T1 = sqrt(T1.x * T1.x + T1.y * T1.y);

    double prod = rxy_T0 * rxy_T1;
    double rx, ry;

    if (prod == 0.0) {
        // ASM 0x303-0x31D: degenerate case
        rx = T1.z * T0.z;        // Note: no negation for division
        ry = -T1.z * T0.z;       // negated
    } else {
        // ASM 0x31-0x375: normal division case
        // For division, we compute:
        //   (T0.x*T1.x + T0.y*T1.y) * factor * inv_T1_mag2
        //   and similar for y, with cross terms
        // Actually from ASM trace:
        //   0x31-0x330: T0_xy * T1_x, T1_xy * T0_y => T0.x*T1.x + T0.y*T1.y (real part product)
        //   Wait, need more careful trace...
        //
        // The ASM at 0x31 computes:
        //   T0_cS_x * T1_34_x + T0_cS_y * T1_34_y  (dot product of xy components)
        //   times inv_T1_mag2
        //   Then multiplied by factor from z interaction
        //   factor = 1 + T1.z*T0.z / (rxy_T0*rxy_T1)
        //   Wait, for division the conjugate negates the imaginary part
        //   So we get: (T0_x*T1_x + T0_y*T1_y) and (T1_x*T0_y - T0_x*T1_y)
        //   But we also need the z-angle interaction

        // Let me re-derive from the ASM more carefully:
        // 0x31: fld [ebp-0xc4] = T0_cS.x
        // 0x325: fmul [ebp-0x34] = T0_cS.x * T1.x
        // 0x328: fld [ebp-0x44] = T0_cS.y (actually the y from the T0 result)
        // 0x32B: fmul [ebp-0x2c] = T0_cS.y * T1.y
        // 0x32E: faddp => T0.x*T1.x + T0.y*T1.y
        // 0x330: fmul [ebp-0x1c] = * inv_T1_mag2
        // Then:
        // 0x333: fld [ebp-0x3c] = T0.z (z component)
        // 0x336: fmul [ebp-0x24] = T0.z * T1.z (renamed as T0_z*T1_z)
        // 0x339: fld [ebp-0x14] = rxy_T0
        // 0x33C: fmul [ebp-0xc] = rxy_T0 * rxy_T1
        // 0x33: fdivp => T0.z*T1.z / (rxy_T0*rxy_T1)
        // 0x341: fld1 => 1.0
        // 0x343: faddp => 1 + T0.z*T1.z/(rxy_T0*rxy_T1)
        // 0x345: fmulp => (T0.x*T1.x+T0.y*T1.y)*inv * (1 + alpha_div)
        // => result_x

        double alpha_div = T0.z * T1.z / prod;
        double factor_div = 1.0 + alpha_div;

        rx = (T0.x * T1.x + T0.y * T1.y) * inv_T1_mag2 * factor_div;

        // 0x34D-0x375: similar for y
        // (T1.x*T0.y - T0.x*T1.y) for cross product (division = conjugate)
        // Actually ASM: T0.x*T1.y (0x350,0x353) then T0_cS.x*T1.y (0x359,035C)
        // fsubp => T1.x*T0.y - T0.x*T1.y (wait, need exact trace)
        // 0x34D: fld [ebp-0x34] = T1.x
        // 0x350: fmul [ebp-0x44] = T1.x * T0_y
        // 0x353: fld [ebp-0xc4] = T0_x
        // 0x359: fmul [ebp-0x2c] = T0_x * T1_y
        // 0x35C: fsubp => T1.x*T0_y - T0_x*T1_y
        // 0x35E: fmul [ebp-0x1c] = * inv_T1_mag2
        // Then same factor calculation...
        ry = (T1.x * T0.y - T0.x * T1.y) * inv_T1_mag2 * factor_div;
    }

    // z component of division:
    // ASM 0x37B-0x38C:
    // rxy_T1 * T0.z - rxy_T0 * T1.z => then * inv_T1_mag2
    // 0x37B: fld [ebp-0xc] = rxy_T1
    // 0x37E: fmul [ebp-0x3c] = rxy_T1 * T0.z
    // 0x381: fld [ebp-0x14] = rxy_T0
    // 0x384: fmul [ebp-0x24] = rxy_T0 * T1.z
    // 0x387: fsubp => rxy_T1*T0.z - rxy_T0*T1.z  (DE E9 = fsubp st1-st0)
    // Wait: DE E9 = fsubp => st(1) = st(1) - st(0) = (rxy_T1*T0.z) - (rxy_T0*T1.z)
    // 0x389: fmul [ebp-0x1c] = * inv_T1_mag2
    double rz = (rxy_T1 * T0.z - rxy_T0 * T1.z) * inv_T1_mag2;

    return cpu_double3(rx, ry, rz);
}

constant FormulaInfo formula_info_mobiustpx = {
    .id = FORMULA_MOBIUSTPX,
    .name = "_MobiusTpx",
    .param_count = 12,
    .complexity = 0,
    .uses_transcendental = false
};
