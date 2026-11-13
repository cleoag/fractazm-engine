#include "ifs_original_state.h"
// Numerical reference: immutable original M3F CODE.

// Metal Template for Formula: CitrusIFS
// Hand-ported from ASM: 345 bytes, Capstone-verified
// DEoption: 20 (dIFS shape -- SSE2 IFS transform + x87 citrus surface DE)
// Citrus surface (from MathWorld) with rotation and cylindrical scissor

constexpr unsigned int FORMULA_CITRUSIFS = 10099;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void CitrusIFSOriginal_x87(double*,const double*,const float*,int);
#endif

template<>
cpu_double3 execute_formula<FORMULA_CITRUSIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[10] = {p.x,p.y,p.z,params[DIFS_VARYSCALE_SLOT]};
    float rotation[9];
    for (int i=0;i<9;++i) rotation[i]=float(params[7+i]);
    CitrusIFSOriginal_x87(state,params,rotation,int(params[6]));
    params[DIFS_VARYSCALE_SLOT]=state[3];
    de=state[9];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    Arm64ApplyIFSState native(p,params);
    using E = Arm64ApplyIFSState::E;
    const auto rotated=native.rotate(params+7);
    const E x=rotated.x,y=rotated.y,z=rotated.z, sub=params[5];
    double positive=double((z*z+y*y)*sub), negative=0;
    const E shifted=x-params[0];
    const E term=((((shifted*shifted)*shifted)*x)*x)*x;
    // FTST/JAE takes the negative-accumulator branch on unordered too.
    if (!(term >= E(0))) negative=double(E(negative)-term);
    else positive=double(term+E(positive));
    E distance=sqrt(sqrt(E(positive)))-sqrt(sqrt(E(negative)));
    const E clipR=((sqrt(x*x+y*y)-sub)-sub)-sub;
    if (clipR >= distance) distance=clipR;
    const E clipZ=((abs(z)-sub)-sub)-sub;
    if (clipZ >= distance) distance=clipZ;
    return native.finish(distance,de);
#else

    // Raw param layout (extract_default_params_from_m3 gives 1 value per option entry):
    // [0] Citrus_add, [1] Scale (.2Doubles raw=1), [2] Z_add (.2Doubles raw=1),
    // [3] Y_add, [4] X_add, [5] Citrus_sub, [6] Apply_scale_add,
    // [7..9] 3 angles (.3SingleAngles raw=3) → buildRotMatrix3
    const double Citrus_add      = params[0];    // default 3.3
    const double Scale           = params[1];    // .2Doubles — 1 raw value
    const double Z_add           = params[2];    // .2Doubles — 1 raw value
    const double Y_add           = params[3];
    const double X_add           = params[4];
    const double Citrus_sub      = params[5];    // default 8
    const int Apply_scale_add   = int(params[6]);
    // .3SingleAngles — 3 raw angles → build 3x3 rotation matrix
    RotMatrix3 Rot = rotMatrix3FromFloats(params, 7);
    // [CONSTANTS] Double = 1  -> at [edi+0x00] after original edi = [PVar]
    // After add edi, -0x2c: constants shift to [edi+0x2c]
    // But code restores edi += 0x2c at 0x00A7, so constant at [edi] = [PVar+0x00]
    // Not used in the formula math after rotation -- the constant=1 is at [PVar+0x00]
    // Actually [edi-0x48] = Citrus_sub (used for spherical subtract)

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    const cpu_double3 transformed(x,y,z);
    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

    // Step 2: Apply 3x3 rotation matrix (built from raw angles via buildRotMatrix3)
    double rx = x * Rot.col0.x + y * Rot.col1.x + z * Rot.col2.x;
    double ry = x * Rot.col0.y + y * Rot.col1.y + z * Rot.col2.y;
    double rz = x * Rot.col0.z + y * Rot.col1.z + z * Rot.col2.z;

    // After rotation, store back
    x = rx; y = ry; z = rz;

    // Step 3: Citrus surface DE
    // ASM at 0x00AA-0x00F0:
    //   z_sq_y_sq = z*z + y*y
    //   shape_pos = z_sq_y_sq * Citrus_add  -> [esp+0x18]
    //   [esp+0x18] = pos accumulator, [esp+0x20] = neg accumulator (both init 0)

    double pos_acc = 0.0;  // [esp+0x18]
    double neg_acc = 0.0;  // [esp+0x20]

    // 0x00AA: z*z + y*y -> * [edi-0x48]=Citrus_sub -> store to [esp+0x18] (pos_acc)
    double z_sq_y_sq = z * z + y * y;
    pos_acc = z_sq_y_sq * Citrus_sub;

    // 0x00BF-0x00D4: x - [edi-0x10]=Citrus_add -> dup -> square -> multiply -> * x * x * x
    // fld x, fsub [edi-0x10] -> st0 = (x - Citrus_add)
    // fld st(0), fmul st(0) -> st0 = (x-Citrus_add)^2, st1 = (x-Citrus_add)
    // fmulp -> st0 = (x-Citrus_add)^3
    // fmul x, fmul x, fmul x -> (x-Citrus_add)^3 * x^3
    double xms = x - Citrus_add;
    double val = xms * xms * xms * x * x * x;

    // 0x00D5-0x00EC: ftst -> if val < 0: neg_acc = neg_acc - val (fsubr); else pos_acc += val
    // ftst: compare st0 with 0
    // shr ah, 1 -> jae: if st0 >= 0 jump to 0xE8
    // if val < 0 (carry set): fsubr [esp+0x20] -> st0 = [esp+0x20] - val = 0 - val = -val
    //   store to neg_acc [esp+0x20]
    // if val >= 0: fadd [esp+0x18] -> pos_acc += val
    if (val < 0.0) {
        neg_acc = neg_acc - val;  // neg_acc = -val (since neg_acc starts at 0)
    } else {
        pos_acc = pos_acc + val;
    }

    // 0x00F0-0x0102: shape_de = sqrt(sqrt(pos_acc)) - sqrt(sqrt(neg_acc))
    // fld pos_acc, fsqrt, fsqrt -> fourth root of pos_acc
    // fld neg_acc, fsqrt, fsqrt -> fourth root of neg_acc
    // fchs -> negate
    // faddp -> fourth_root(pos_acc) - fourth_root(neg_acc)
    double shape_de = sqrt(sqrt(pos_acc)) - sqrt(sqrt(neg_acc));

    // 0x0104-0x0119: Cylindrical scissor: sqrt(x^2 + y^2) - 3*Citrus_sub
    // fld x, fmul st(0) -> x^2
    // fld y, fmul st(0) -> y^2
    // faddp -> x^2+y^2
    // fsqrt -> sqrt(x^2+y^2)
    // fsub Citrus_sub * 3 (three fsub instructions)
    double cyl = sqrt(x * x + y * y) - 3.0 * Citrus_sub;

    // 0x011C-0x0129: min(cyl, shape_de) -> keep smaller
    // fcom st(1): compare cyl with shape_de
    // if cyl < shape_de: fstp st(0) (discard cyl, keep shape_de) -- wait, check logic
    // shr ah,1 -> jae 0x129: if cyl >= shape_de -> fstp st(1) (keep cyl)
    // if cyl < shape_de: fstp st(0), jmp -> keep shape_de as result
    // Actually: st0=cyl, st1=shape_de
    // fcom st(1): compare st0 (cyl) with st1 (shape_de)
    // if cyl < shape_de (C0=1): shr ah,1 -> CF, jae NOT taken -> fstp st(0) discard cyl -> shape_de
    // if cyl >= shape_de: jae taken -> fstp st(1) discard shape_de -> cyl
    // So result = max(cyl, shape_de)?
    // Wait: re-read. At 0x125: fstp st(0) = discard top (cyl), keep st(1)=shape_de
    //        At 0x129: fstp st(1) = discard shape_de, keep cyl
    // When cyl < shape_de -> discard cyl -> result = shape_de
    // When cyl >= shape_de -> discard shape_de -> result = cyl
    // That's max(cyl, shape_de)
    double result = max(cyl, shape_de);

    // 0x012B-0x0137: |z| - 3*Citrus_sub
    // fld z, fabs -> |z|
    // fsub Citrus_sub, fsub Citrus_sub, fsub Citrus_sub -> |z| - 3*Citrus_sub
    double z_clip = fabs(z) - 3.0 * Citrus_sub;

    // 0x013A-0x0147: max(z_clip, result)
    // Same pattern: fcom st(1) + shr/jae -> max
    result = max(z_clip, result);

    // Step 4: Scale correction
    if (Apply_scale_add == 0) {
        result /= Scale;
    }

    de = result;
    return Apply_scale_add != 0 ? transformed : p;
#endif
}

cpu_double3 CitrusIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_CITRUSIFS>(p, params, de);
}

constant FormulaInfo formula_info_CitrusIFS = {
    .id = FORMULA_CITRUSIFS,
    .name = "CitrusIFS",
    .param_count = 17,
    .complexity = 345,
    .uses_transcendental = false
};

/*
Original M3 Description:
  Citrus surface (from MathWorld). Very similar to piriform.
  Added rotation and customizers...
  Equation: CiMul * (x*x + z*z) + y^3 * ((y - CiSub) ^ 3) = 0
  EDIT - Added a defix (cylindrical scissor)
  Luca GN 2012

ASM Trace Summary:
  0x00-0x40: Standard SSE2 IFS preamble (Scale, adds, de*=Scale)
  0x4B-0x9C: 3x3 rotation matrix multiply (3SingleAngles)
  0xAA-0xB8: z^2+y^2 * Citrus_add -> pos accumulator
  0xBF-0xD4: (x - Citrus_sub)^3 * x^3
  0xD5-0xEC: Split pos/neg into two accumulators
  0xF0-0x102: 4th_root(pos) - 4th_root(neg) = citrus shape DE
  0x104-0x119: Cylindrical scissor: sqrt(x^2+y^2) - 3*Citrus_sub
  0x11C-0x129: max(cyl, shape_de)
  0x12B-0x147: max(|z| - 3*Citrus_sub, result)
  0x149-0x155: Scale correction if !Apply_scale_add

Parameters:
  [0] Citrus_add: double (default: 3.3) -- citrus surface multiplier
  [1] Scale: double (default: 2) -- IFS scale factor (.2Doubles)
  [2] Z_add: double (default: 0)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] Citrus_sub: double (default: 8) -- citrus subtraction
  [6] Apply_scale_add: int (default: 1)
  [7..15] CitrusRot: 3SingleAngles (9 floats, 3x3 rotation matrix)
  [CONSTANTS] Double = 1

PVar layout:
  [PVar - 0x10] = Citrus_add
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add
  [PVar - 0x40] = X_add
  [PVar - 0x48] = Citrus_sub
  [PVar - 0x4c] = Apply_scale_add (integer)
  [PVar - 0x50..0x74] = CitrusRot 3SingleAngles (9 floats)
  [PVar + 0x00] = constant 1.0
*/
