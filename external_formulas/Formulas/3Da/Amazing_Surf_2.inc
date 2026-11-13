// Amazing Surf 2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Amazing Surf 2
// Hand-ported from ASM (536 bytes, Capstone-verified)
// DEoption: 2 (analytic DE via w tracking)
// Extended API required: J1/J2/J3 (C components)
//
// Modified fold formula: fold X and Y are "infinitized" around the origin.
// Uses IEEE-754 bit manipulation to extract significand and create a repeating
// fold pattern that works over all scales.

constexpr unsigned int FORMULA_AMAZING_SURF_2 = 10186;

// The original deliberately rewrites the stored IEEE-754 exponent/mantissa.
// log2/exp2 is not equivalent for zero, subnormal, infinity or NaN inputs.
// In particular, enabled smooth at zero evaluates 0/0 before this rewrite;
// skipping that operation silently changes both coordinates and the DE scale.
static double asm_infinitized_fold(double v, int fiddler, int smooth_bit,
                                    double const0, double const1,
                                    double const_upper, double const_lower,
                                    double smooth_val) {
    constexpr uint64_t exponent_sign = UINT64_C(0xfff0000000000000);
    constexpr uint64_t fraction = UINT64_C(0x000fffffffffffff);
    constexpr uint64_t sign = UINT64_C(0x8000000000000000);
    constexpr uint64_t one = UINT64_C(0x3ff0000000000000);
    uint64_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    uint64_t scale_bits = bits & exponent_sign;
    if ((const_upper >= v) && (const_lower < v)) {
        if (fiddler & smooth_bit) {
            // For finite nonzero v, v/abs(v) is exactly +/-1. Taking that
            // quotient first avoids Double underflow/overflow in -1.5*v;
            // the original x87 product has sufficient exponent/precision.
            // Keep 0/0: its NaN bits are consumed by the following masks.
            v = v + smooth_val * (v / fabs(v));
            std::memcpy(&bits, &v, sizeof(bits));
            scale_bits = bits & exponent_sign;
        }
        bits = (bits & fraction) ^ one;
        std::memcpy(&v, &bits, sizeof(v));
    } else {
        scale_bits = (scale_bits & sign) ^ one;
    }
    const double folded = -fabs(fabs(v) + const0) + const1;
    double scale;
    std::memcpy(&scale, &scale_bits, sizeof(scale));
    double result = scale * folded;
    std::memcpy(&bits, &result, sizeof(bits));
    bits ^= (scale_bits & UINT64_C(0x0010000000000000)) << 11;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

template<>
cpu_double3 execute_formula<FORMULA_AMAZING_SURF_2>(cpu_double3 p, constant double* params, double &de) {
#if defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E = numeric::Extended80;
    const E fold(params[3]);
    const int fiddler=int(params[13]);
    E scaledX=fold*E(p.x), scaledY=fold*E(p.y);
    if (fiddler & 1) scaledX=abs(scaledX);
    if (fiddler & 2) scaledY=abs(scaledY);
    auto infinitized = [&](double v, int smoothBit) {
        constexpr uint64_t exponentSign=UINT64_C(0xfff0000000000000);
        constexpr uint64_t fraction=UINT64_C(0x000fffffffffffff);
        constexpr uint64_t sign=UINT64_C(0x8000000000000000);
        constexpr uint64_t one=UINT64_C(0x3ff0000000000000);
        uint64_t bits=std::bit_cast<uint64_t>(v), scaleBits=bits & exponentSign;
        if (E(params[16])>=E(v) && !(E(params[17])>=E(v))) {
            if (fiddler & smoothBit) {
                // 006D..0077: keep the product before division and its
                // subsequent Double store, including the 0/0 NaN rewrite.
                const E value(v);
                v=static_cast<double>(value+(E(params[18])*value)/abs(value));
                bits=std::bit_cast<uint64_t>(v);
                scaleBits=bits & exponentSign;
            }
            bits=(bits & fraction)^one;
            v=std::bit_cast<double>(bits);
        } else {
            scaleBits=(scaleBits & sign)^one;
        }
        // Both the folded value and scale product are explicit Double stores
        // before exponent parity is XORed into the resulting sign bit.
        const double folded=static_cast<double>(-abs(abs(E(v))+E(params[14]))+E(params[15]));
        const double result=static_cast<double>(E(std::bit_cast<double>(scaleBits))*E(folded));
        bits=std::bit_cast<uint64_t>(result);
        bits ^= (scaleBits & UINT64_C(0x0010000000000000)) << 11;
        return std::bit_cast<double>(bits);
    };
    const E x=E(infinitized(static_cast<double>(scaledX),4))/fold;
    const E y=E(infinitized(static_cast<double>(scaledY),8))/fold;
    const E z(p.z);
    E matrix[9];
    for (int i=0; i<9; ++i) matrix[i]=E(static_cast<float>(params[4+i]));
    // Original stack order at 0182 is z,y,x; the first matrix row becomes z.
    const E rz=(z*matrix[0]+y*matrix[1])+x*matrix[2];
    const E ry=(z*matrix[3]+y*matrix[4])+x*matrix[5];
    const E rx=(z*matrix[6]+y*matrix[7])+x*matrix[8];
    const E rr=(rz*rz+ry*ry)+rx*rx;
    const E m=!(rr>=E(params[2])) ? E(params[1])
        : (E(1)>=rr ? E(params[0])/rr : E(params[0]));
    de=static_cast<double>(E(de)*m);
    // 0200..020D adds Julia C before the final Double stores.
    return cpu_double3(static_cast<double>(rx*m+E(params[19])),
                       static_cast<double>(ry*m+E(params[20])),
                       static_cast<double>(rz*m+E(params[21])));
#else
    const double Scale = params[0];           // Scale
    const double scaleOverMinR2 = params[1];  // BOXSCALE pre-computed: Scale/sqr(MinR)
    const double minR2 = params[2];            // BOXSCALE pre-computed: sqr(MinR)
    const double FoldXY = params[3];          // FoldXY
    const int Fold_fiddler = int(params[13]); // Fold fiddler (integer) — after 9 matrix floats

    // [CONSTANTS] from the m3f [CONSTANTS] section:
    const double const0 = -1.5;              // [esi + 0x00]
    const double const1 = 0.5;               // [esi + 0x08]
    const double const_upper = 2.0;          // [esi + 0x10]
    const double const_lower = -2.0;         // [esi + 0x18]
    const double smooth_val = -1.5;          // [esi + 0x20]

    // Rotation matrix from 9 pre-computed float elements (from .3SingleAngles)
    RotMatrix3 RM = rotMatrix3FromFloats(params, 4);
    const double R00 = RM.col0.x, R01 = RM.col1.x, R02 = RM.col2.x;  // row 0
    const double R10 = RM.col0.y, R11 = RM.col1.y, R12 = RM.col2.y;  // row 1
    const double R20 = RM.col0.z, R21 = RM.col1.z, R22 = RM.col2.z;  // row 2

    // ---------------------------------------------------------------
    // Step 1: Scale x and y by FoldXY, optionally abs (ASM 0x0014-0x0033)
    // ---------------------------------------------------------------
    double fx = p.x * FoldXY;
    double fy = p.y * FoldXY;

    if (Fold_fiddler & 1) fx = fabs(fx);
    if (Fold_fiddler & 2) fy = fabs(fy);

    // ---------------------------------------------------------------
    // Step 2: Apply infinitized fold to x and y (ASM 0x0037-0x016E)
    // Uses IEEE-754 bit manipulation to create a repeating fold pattern.
    // ---------------------------------------------------------------
    double x_folded = asm_infinitized_fold(fx, Fold_fiddler, 4,
                                            const0, const1, const_upper, const_lower, smooth_val);
    double y_folded = asm_infinitized_fold(fy, Fold_fiddler, 8,
                                            const0, const1, const_upper, const_lower, smooth_val);

    // ---------------------------------------------------------------
    // Step 3: Divide folded values by FoldXY (ASM 0x0174-0x017B)
    // ---------------------------------------------------------------
    double x_val = x_folded / FoldXY;
    double y_val = y_folded / FoldXY;
    double z_val = p.z;  // z is NOT folded

    // ---------------------------------------------------------------
    // Step 4: Apply rotation matrix (ASM 0x0182-0x01B9)
    // The rotation input order is [z, y_val, x_val] and the matrix rows
    // compute: R1=M[2]*{z,y,x}, R2=M[0]*{z,y,x}, R3=M[1]*{z,y,x}
    // Output: z_out=R2, y_out=R3, x_out=R1
    // ---------------------------------------------------------------
    double rx = z_val * R00 + y_val * R01 + x_val * R02;  // → stored to z
    double ry = z_val * R10 + y_val * R11 + x_val * R12;  // → stored to y
    double rz = z_val * R20 + y_val * R21 + x_val * R22;  // → stored to x

    // ---------------------------------------------------------------
    // Step 5: Compute rr and sphere fold (ASM 0x01BC-0x01F2)
    // ---------------------------------------------------------------
    double rr = rx * rx + ry * ry + rz * rz;

    double m;
    // Original FCOM/JAE sends unordered radii to the minimum-radius branch.
    // FoldXY=0 can produce NaNs while the DE multiplier remains meaningful.
    if (!(rr >= minR2)) {
        m = scaleOverMinR2;
    } else if (rr <= 1.0) {
        m = Scale / rr;
    } else {
        m = Scale;
    }

    // DE tracking (ASM 0x01F2-0x01F7: fld [ecx+8], fmul st(1), fstp [ecx+8])
    de = de * m;

    // ---------------------------------------------------------------
    // Step 6: Scale and output (ASM 0x01FA-0x020D)
    // ASM stores: [ecx]=R2*m+Cz, [edx]=R3*m+Cy, [ebx]=R1*m+Cx
    // Preserve the fallback's existing Double product followed by Julia +C.
    // Output mapping: result.z = rx*m, result.y = ry*m, result.x = rz*m
    // ---------------------------------------------------------------
    cpu_double3 result;
    result.z = rx * m;
    result.y = ry * m;
    result.x = rz * m;
    return cpu_double3(result.x+params[19],result.y+params[20],result.z+params[21]);
#endif
}

// Alternative direct function implementation
cpu_double3 Amazing_Surf_2_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_AMAZING_SURF_2>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_Amazing_Surf_2 = {
    .id = FORMULA_AMAZING_SURF_2,
    .name = "Amazing Surf 2",
    .param_count = 7,
    .complexity = 1100,
    .uses_transcendental = false
};

/*
Original M3 Description:
Modified fold formula used here; fold X and Y is "infinitized" around the origin.

Mathematical Formula:
  1. Scale x,y by FoldXY, optionally abs based on Fold_fiddler bits
  2. "Infinitize" fold via IEEE-754 bit manipulation:
     a. Save sign+exponent bits of scaled value
     b. If in range [const_lower, const_upper]: extract significand to [1,2),
        apply fold = -(|sig + const0|) + const1, scale back by 2^exponent,
        flip sign based on exponent parity
     c. If out of range: construct +/-1.0 from sign, apply fold to raw value,
        multiply by +/-1.0, always flip sign (exponent of 1.0 has bit 20 set)
  3. Divide folded x,y back by FoldXY
  4. z is unchanged
  5. Apply 3x3 rotation matrix (from 3SingleAngles)
  6. Compute rr = rx^2 + ry^2 + rz^2
  7. Sphere fold (BOXSCALE): if rr < minR2: m=Scale/minR2, elif rr<1: m=Scale/rr, else m=Scale
  8. Scale by m, add C (engine adds C)

Expanded runtime parameters:
  [0] Scale
  [1] Scale/MinR^2 and [2] MinR^2 (BOXSCALE)
  [3] FoldXY
  [4..12] Rotation matrix (nine Singles widened to Double)
  [13] Fold fiddler (saved option index 6, after three rotation angles)

PVar layout in ASM (esi = PVar):
  [esi - 0x10] = Scale (DOUBLE, option 0)
  [esi - 0x18] = Scale/MinR^2 (BOXSCALE precomputed)
  [esi - 0x20] = MinR^2 (BOXSCALE precomputed)
  [esi - 0x28] = FoldXY (DOUBLE, option 2)
  [esi - 0x2C..0x4C] = 9 singles: rotation matrix (3SINGLEANGLES)
  [esi - 0x50] = Fold_fiddler (INTEGER)
  [esi + 0x00] = -1.5 (constant)
  [esi + 0x08] = 0.5 (constant)
  [esi + 0x10] = 2.0 (constant)
  [esi + 0x18] = -2.0 (constant)
  [esi + 0x20] = -1.5 (constant, smooth val)

NOTES:
  - DEoption is 2 (standard analytic DE, not VaryScale)
  - The infinitized fold preserves Double IEEE bit patterns; float ports need a separate numeric contract
  - No transcendentals used (bit manipulation + arithmetic only)
  - z axis is NOT folded (Surf variant)
  - Rotation applied after fold, before sphere fold
  - Fold_fiddler bits: bit 0 = abs(x), bit 1 = abs(y), bit 2 = smooth x, bit 3 = smooth y
*/
