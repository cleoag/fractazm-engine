#include "ifs_original_state.h"
// Numerical reference: immutable original M3F CODE.

// Metal Template for Formula: DevilIFS
// Hand-ported from ASM: 330 bytes, Capstone-verified
// DEoption: 20 (dIFS shape -- SSE2 IFS transform + x87 Devil/Citrus surface DE)
// Citrus surface (from MathWorld) with 3SingleAngles rotation

constexpr unsigned int FORMULA_DEVILIFS = 10214;

// ASM trace summary:
//   0x00-0x40: Standard SSE2 IFS preamble (Scale, X/Y/Z_add, Apply_scale_add)
//   0x4B-0x54: Initialize [esp+0x18]=0, [esp+0x20]=0 (pos/neg accumulators)
//   0x55-0xA7: 3x3 rotation matrix multiply (3SingleAngles CitrusRot)
//              edi adjusted -0x2c, then restored +0x2c
//   0xAA-0xC7: Compute x_sq, y_sq, z_sq (stored to stack)
//   0xC8-0xE9: Devil surface computation:
//     term1 = x_sq^2 + x_sq*z_sq + x_sq*z_sq + y_sq*[edi+0x08] + z_sq^2
//     = x_sq^2 + 2*x_sq*z_sq + z_sq^2 + y_sq*CitrusSub
//     Hmm, actually: x_sq^2 (=x^4), x_sq*z_sq, both added. Then y_sq*Const, z_sq^2 (=z^4)
//     More carefully: x^4 + x^2*z^2 + x^2*z^2 = x^4 + 2*x^2*z^2
//     Plus y^2 * constant + z^4
//     = (x^2+z^2)^2 + y^2*constant
//     This matches the MathWorld "Devil" surface form!
//   0xED-0xFC: Second term: x_sq * [edi] + y_sq^2 = x^2*constant + y^4
//   0x100-0x116: Fourth root difference (same pattern as Mitre/Citrus)
//   0x117-0x12B: Spherical cutoff: sqrt(x_sq+y_sq+z_sq) - YZ_add, min with shape
//   0x13A-0x149: Scale correction + store

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void DevilIFSOriginal_x87(double*,const double*,const float*,int);
#endif

template<>
cpu_double3 execute_formula<FORMULA_DEVILIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[10] = {p.x,p.y,p.z,params[DIFS_VARYSCALE_SLOT]};
    float rotation[9];
    for (int i=0;i<9;++i) rotation[i]=float(params[7+i]);
    DevilIFSOriginal_x87(state,params,rotation,int(params[6]));
    params[DIFS_VARYSCALE_SLOT]=state[3];
    de=state[9];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    Arm64ApplyIFSState native(p,params);
    using E = Arm64ApplyIFSState::E;
    const auto rotated=native.rotate(params+7);
    // The original stores each square and both accumulators as Double.
    const E xx=double(E(rotated.x)*rotated.x), yy=double(E(rotated.y)*rotated.y),
            zz=double(E(rotated.z)*rotated.z);
    const E xz=xx*zz;
    const E positive=double((((xx*xx+xz)+xz)+yy*params[17])+zz*zz);
    const E negative=double(xx*params[16]+yy*yy);
    E distance=(sqrt(sqrt(positive))-sqrt(sqrt(negative)))*params[18];
    const E clip=sqrt(xx+(yy+zz))-params[0];
    if (clip >= distance) distance=clip; // unordered retains the surface
    return native.finish(distance,de);
#else

    // Raw param layout: [0] Devil_limit, [1] Scale (.2Doubles raw=1), [2] Z_add (.2Doubles raw=1),
    // [3] Y_add, [4] X_add, [5] Citrus_sub, [6] Apply_scale_add,
    // [7..9] 3 angles (.3SingleAngles raw=3) → buildRotMatrix3
    const double Devil_limit     = params[0];
    const double Scale           = params[1];    // .2Doubles — 1 raw value
    const double Z_add           = params[2];    // .2Doubles — 1 raw value
    const double Y_add           = params[3];
    const double X_add           = params[4];
    const double Citrus_sub      = params[5];
    const int Apply_scale_add   = int(params[6]);
    RotMatrix3 Rot = rotMatrix3FromFloats(params, 7);

    // Constants from [CONSTANTS]:
    const double K_CONST0 = 0.36;   // [PVar + 0x00]
    const double K_CONST1 = 0.25;   // [PVar + 0x08]
    const double K_CONST2 = 2.0;    // [PVar + 0x10]

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    const cpu_double3 transformed(x,y,z);
    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

    // Step 2: Initialize accumulators
    double pos_acc = 0.0;  // [esp+0x18]
    double neg_acc = 0.0;  // [esp+0x20]

    // Step 3: Apply 3x3 rotation (3SingleAngles CitrusRot → buildRotMatrix3)
    double rx = x * Rot.col0.x + y * Rot.col1.x + z * Rot.col2.x;
    double ry = x * Rot.col0.y + y * Rot.col1.y + z * Rot.col2.y;
    double rz = x * Rot.col0.z + y * Rot.col1.z + z * Rot.col2.z;
    x = rx; y = ry; z = rz;

    // Step 4: Compute squared values
    // 0xAA: fld x, fmul st(0) -> x_sq, fstp [esp] -> store
    // 0xB4: fld y, fmul st(0) -> y_sq, fstp [esp+8]
    // 0xBE: fld z, fmul st(0) -> z_sq, fstp [esp+0x10]
    double x_sq = x * x;
    double y_sq = y * y;
    double z_sq = z * z;

    // Step 5: Devil surface term 1 (positive accumulator)
    // ASM at 0xC8-0xE9:
    // 0xC8: fld x_sq
    // 0xCB: fmul st(0) -> x_sq^2 = x^4
    // 0xCD: fld x_sq
    // 0xD0: fmul z_sq -> x_sq * z_sq
    // 0xD4: DC C1 = fadd st(1), st(0) -> st1 = x^4 + x_sq*z_sq
    // 0xD6: DE C1 = faddp -> st0 = (x^4 + x_sq*z_sq) + x_sq*z_sq = x^4 + 2*x_sq*z_sq
    // Wait: fadd st(1), st(0) adds st0 to st1 but doesn't pop.
    // Then faddp st(1) adds st0 to st1 and pops.
    // Before 0xD4: st0 = x_sq*z_sq, st1 = x^4
    // 0xD4: DC C1 = fadd st(1), st(0) -> st1 = x^4 + x_sq*z_sq, st0 = x_sq*z_sq
    // 0xD6: DE C1 = faddp st(1) -> st0 = x_sq*z_sq + (x^4 + x_sq*z_sq) = x^4 + 2*x_sq*z_sq
    double term_a = x_sq * x_sq + 2.0 * x_sq * z_sq;

    // 0xD8: fld y_sq
    // 0xDC: fmul [edi+0x08] = K_CONST1 (0.25) -> y_sq * 0.25
    // 0xDF: faddp -> term_a + y_sq*0.25
    term_a += y_sq * K_CONST1;

    // 0xE1: fld z_sq
    // 0xE5: fmul st(0) -> z^4
    // 0xE7: faddp -> term_a + z^4
    term_a += z_sq * z_sq;

    // 0xE9: fstp [esp+0x18] -> pos_acc = term_a
    pos_acc = term_a;

    // Step 6: Devil surface term 2 (negative accumulator)
    // 0xED: fld x_sq
    // 0xF1: fmul [edi] = K_CONST0 (0.36) -> x_sq * 0.36
    // 0xF4: fld y_sq
    // 0xF8: fmul st(0) -> y^4
    // 0xFA: faddp -> x_sq*0.36 + y^4
    // 0xFC: fstp [esp+0x20] -> neg_acc
    neg_acc = x_sq * K_CONST0 + y_sq * y_sq;

    // Step 7: Fourth root difference
    // 0x100: fld pos_acc, fsqrt, fsqrt -> fourth_root(pos_acc)
    // 0x108: fld neg_acc, fsqrt, fsqrt -> fourth_root(neg_acc)
    // 0x110: fchs -> negate
    // 0x112: faddp -> fourth_root(pos_acc) - fourth_root(neg_acc)
    double raw_de = sqrt(sqrt(pos_acc)) - sqrt(sqrt(neg_acc));

    // 0x114: fmul [edi+0x10] = K_CONST2 (2.0) -> scale the surface DE
    raw_de *= K_CONST2;

    // Step 8: Spherical cutoff
    // 0x117: fld x_sq; fld y_sq; fld z_sq; faddp; faddp -> x_sq+y_sq+z_sq
    // 0x126: fsqrt -> r = sqrt(x_sq+y_sq+z_sq)
    // 0x128: fsub [edi-0x10] = Devil_limit -> r - Devil_limit
    double r = sqrt(x_sq + y_sq + z_sq);
    double r_dist = r - Devil_limit;

    // 0x12B: fcom st(1) -> compare r_dist vs raw_de
    // Same max pattern: if r_dist > raw_de, use r_dist; else use raw_de
    // 0x132: jae 0x138 -> if r_dist >= raw_de
    //   0x138: fstp st(1) -> keep r_dist
    // else:
    //   0x134: fstp st(0) -> discard r_dist, keep raw_de
    // = max(r_dist, raw_de)

    // Actually wait, let me re-read more carefully:
    // 0x12B: fcom st(1) -> compare st0(r_dist) vs st1(raw_de)
    // If r_dist < raw_de (C0=1): jae not taken -> fstp st(0) = discard r_dist -> keep raw_de
    // If r_dist >= raw_de: jae taken -> fstp st(1) = discard raw_de -> keep r_dist
    // = max(r_dist, raw_de) ✓
    double shape_de = max(r_dist, raw_de);

    // Step 9: Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return Apply_scale_add != 0 ? transformed : p;
#endif
}

cpu_double3 DevilIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_DEVILIFS>(p, params, de);
}

constant FormulaInfo formula_info_DevilIFS = {
    .id = FORMULA_DEVILIFS,
    .name = "DevilIFS",
    .param_count = 19,
    .complexity = 330,
    .uses_transcendental = false
};

/*
Original M3 Description:
  Citrus surface (from MathWorld). Very similar to piriform.
  Added rotation and customizers...
  Equation: CiMul * (x*x + z*z) + y^3 * ((y - CiSub) ^ 3) = 0
  Luca GN 2012

  Note: The M3 is named "DevilIFS" but the description says "Citrus surface".
  The actual ASM implements a different variant:
    pos_acc = x^4 + 2*x^2*z^2 + 0.25*y^2 + z^4 = (x^2+z^2)^2 + 0.25*y^2
    neg_acc = 0.36*x^2 + y^4
    raw_de = fourth_root(pos) - fourth_root(neg)
  This is closer to a "Devil's curve" surface than the citrus equation in the description.
  Trust the ASM.

ASM Trace Summary:
  0x00-0x40: SSE2 IFS preamble
  0x4B-0x54: Init pos/neg accumulators
  0x55-0xA7: 3x3 rotation (3SingleAngles CitrusRot)
  0xAA-0xC7: x_sq, y_sq, z_sq stored
  0xC8-0xE9: pos_acc = x^4 + 2*x^2*z^2 + 0.25*y^2 + z^4
  0xED-0xFC: neg_acc = 0.36*x^2 + y^4
  0x100-0x114: raw_de = (fourth_root(pos) - fourth_root(neg)) * 2.0
  0x117-0x128: r = sqrt(r_sq), r_dist = r - Devil_limit
  0x12B-0x138: shape_de = max(r_dist, raw_de)
  0x13A-0x149: Scale correction + store

Parameters:
  [0] Devil_limit: double (default: 2.0) -- bounding sphere radius
  [1] Scale: double (default: 2) (.2Doubles)
  [2] Z_add: double (default: 0) (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] Citrus_sub: double (default: 8) -- naming from M3, used in rotation context
  [6] Apply_scale_add: int (default: 1)
  [7..15] CitrusRot: 3SingleAngles (9 floats)

  [CONSTANTS] 0.36, 0.25, 2.0
*/
