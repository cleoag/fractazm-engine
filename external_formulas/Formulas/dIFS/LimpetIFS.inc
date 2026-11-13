#include "ifs_original_state.h"
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void LimpetIFSOriginal_x87(double*, const void*);
#endif

// LimpetIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: LimpetIFS
// Hand-ported from ASM: 292 bytes, Capstone-verified
// DEoption: 20 (dIFS shape -- SSE2 IFS + 3x3 rotation + limpet torus DE)
// Limpet torus: an "ufoid" shape with cylinder clipping

constexpr unsigned int FORMULA_LIMPETIFS = 10203;

template<>
cpu_double3 execute_formula<FORMULA_LIMPETIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    OriginalApplyIFSState native(p,params);
    native.real(-84,params[7]);
    native.matrix(-88,params+8);
    LimpetIFSOriginal_x87(native.state,native.var());
    return native.finish(params,de);
#elif defined(MB3D_SOFTWARE_EXT80)
    Arm64ApplyIFSState native(p,params);
    using E = Arm64ApplyIFSState::E;
    const auto rotated=native.rotate(params+8);
    const E x=rotated.x,y=rotated.y,z=rotated.z;
    const E radius=double(sqrt(x*x+y*y));
    const E inner=(E(params[0])+radius*z)+radius*params[7];
    E distance=sqrt(sqrt(inner*inner+z*z))-params[5];
    const E clipR=((radius-params[5])-params[5])-params[5];
    distance=clipR >= distance ? clipR : distance;
    const E clipZ=((abs(z)-params[5])-params[5])-params[5];
    distance=clipZ >= distance ? clipZ : distance;
    return native.finish(distance,de);
#else
    // Portable numerical parity remains unverified.

    const double R1         = params[0];   // [edi - 0x10]  default: 1.2
    const double Scale      = params[1];   // [edi - 0x20]  .2Doubles
    const double Z_add      = params[2];   // [edi - 0x30]  .2Doubles
    const double Y_add      = params[3];   // [edi - 0x38]
    const double X_add      = params[4];   // [edi - 0x40]
    const double R2         = params[5];   // [edi - 0x48]  default: 1.0
    const int Apply_scale_add = int(params[6]); // [edi - 0x4c]
    const double Rmul       = params[7];   // [edi - 0x54]  default: -1.6
    // params[8..16] = 3x3 rotation matrix (9 singles from .3SingleAngles LimpetRot)
    // Note: edi offset for rotation is -0x34 (add edi, -0x34) instead of -0x2c
    // This accounts for the extra parameter Rmul between Apply_scale_add and rotation

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

    // Step 2: Apply 3x3 rotation matrix (from .3SingleAngles LimpetRot)
    // ASM: add edi,-0x34 then matrix mul then add edi,+0x34
    double rx = params[8]  * x + params[9]  * y + params[10] * z;
    double ry = params[11] * x + params[12] * y + params[13] * z;
    double rz = params[14] * x + params[15] * y + params[16] * z;

    // Step 3: Limpet torus computation
    // ASM trace (0xA1-0x114):
    //
    // 0xA1-0xB0: Compute R = sqrt(rx^2 + ry^2) -- cylindrical radius
    //   fld rx; fmul st(0) -> rx^2
    //   fld ry; fmul st(0) -> ry^2
    //   faddp -> rx^2 + ry^2
    //   fsqrt -> R = sqrt(rx^2 + ry^2)
    //   fstp [esp] -> store R
    double R = sqrt(rx * rx + ry * ry);

    // 0xB4-0xC8: Compute limpet_de = sqrt(sqrt( (R1 + rz*R + Rmul*R)^2 + rz^2 ))
    //   fld R1                          ; R1
    //   fld R                           ; R, R1
    //   fmul rz                         ; R*rz, R1
    //   faddp st(1)                     ; R1 + R*rz
    //   (nop x2)
    //   fld R                           ; R, (R1 + R*rz)
    //   fmul Rmul                       ; R*Rmul, (R1 + R*rz)
    //   faddp st(1)                     ; R1 + R*rz + R*Rmul
    //   fmul st(0)                      ; (R1 + R*rz + R*Rmul)^2
    //   fld rz                          ; rz, (...)^2
    //   fmul st(0)                      ; rz^2, (...)^2
    //   faddp st(1)                     ; (R1 + R*rz + R*Rmul)^2 + rz^2
    //   fsqrt                           ; sqrt(...)
    //   fsqrt                           ; sqrt(sqrt(...)) = 4th root
    //   fsub R2                         ; limpet_de = 4th_root(...) - R2
    double inner = R1 + R * rz + Rmul * R;
    double limpet_de = sqrt(sqrt(inner * inner + rz * rz)) - R2;

    // 0xDB-0xE4: Compute cylinder clipping DE = R - 3*R2
    //   fld R                           ; R, limpet_de
    //   fsub R2                         ; R - R2
    //   fsub R2                         ; R - 2*R2
    //   fsub R2                         ; R - 3*R2
    double cyl_de = R - 3.0 * R2;

    // 0xE7-0xF4: max(cyl_de, limpet_de)
    //   fcom st(1): compare cyl_de vs limpet_de
    //   shr ah,1; jae -> if cyl_de >= limpet_de: keep cyl_de
    //   else: keep limpet_de
    //   Actually: st0=cyl_de, st1=limpet_de
    //   C0=1 if cyl_de < limpet_de -> CF after shr
    //   jae (CF=0): cyl_de >= limpet_de -> fstp st(1) = discard limpet_de, keep cyl_de
    //   fall-through (CF=1): cyl_de < limpet_de -> fstp st(0) = discard cyl_de, keep limpet_de
    //   Result = max(cyl_de, limpet_de)
    //
    //   But wait -- the description says DE = min(DE, cyl). Let me re-check.
    //   Actually looking at the comparison sense more carefully:
    //   fcom st(1): C0 set if st0 < st1 i.e. cyl_de < limpet_de
    //   shr ah,1: CF = C0
    //   jae 0xF4: jump if NOT CF, i.e. cyl_de >= limpet_de
    //     -> 0xF4: fstp st(1) -> discard st1=limpet_de -> keep cyl_de
    //   fall-through 0xF0:
    //     -> fstp st(0) -> discard st0=cyl_de -> keep limpet_de
    //   So result = max(cyl_de, limpet_de). That clips the limpet to inside the cylinder.
    //   Wait, cylinder DE is R - 3*R2. Inside cylinder: R < 3*R2, so cyl_de < 0.
    //   The limpet_de is typically positive outside the torus.
    //   max(negative_cyl, positive_limpet) = limpet_de inside the cylinder.
    //   But at cylinder boundary: cyl_de=0, so it clips the torus shape.
    //   This is actually an intersection (max of two SDF = intersection).
    double de1 = max(cyl_de, limpet_de);

    // 0xF6-0x105: Compute |rz| - 3*R2 and take max with de1
    //   fld rz                          ; rz, de1
    //   fabs                            ; |rz|
    //   fsub R2                         ; |rz| - R2
    //   fsub R2                         ; |rz| - 2*R2
    //   fsub R2                         ; |rz| - 3*R2
    //   fcom st(1): compare with de1
    //   -> max(|rz|-3*R2, de1)
    double z_clip = fabs(rz) - 3.0 * R2;
    double shape_de = max(z_clip, de1);

    // Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    // Original 0025 skips XYZ stores when Apply is zero; distance still uses local XYZ.
    return Apply_scale_add != 0 ? cpu_double3(x, y, z) : p;
#endif
}

cpu_double3 LimpetIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_LIMPETIFS>(p, params, de);
}

constant FormulaInfo formula_info_LimpetIFS = {
    .id = FORMULA_LIMPETIFS,
    .name = "LimpetIFS",
    .param_count = 17,
    .complexity = 500,
    .uses_transcendental = false
};

/*
Original M3 Description:
Limpet torus. An "ufoid"...

Note! This surface is cutted off with a big cylinder, so for some critical parameters
you won't see it entirely (and in some cases it's impossible to see it "entirely" ...
because it degenerates in a sort of gaussian bell).

Equation;
  R = sqrt(x*x+y*y)
  DE = sqrt4(sqr(R1 + z*R + Rmul*R) + z*z) - R2
  DE = min (DE, cyl(x,y,z, 3 * R2) )

Luca GN 2012

Mathematical Formula (from ASM trace):
  DEoption=20 (dIFS shape -- SSE2 IFS + rotation + limpet torus):
    Step 1: IFS transform
    Step 2: 3x3 rotation from .3SingleAngles LimpetRot
    Step 3: Limpet torus:
      R = sqrt(rx^2 + ry^2)
      limpet_de = sqrt(sqrt((R1 + R*rz + Rmul*R)^2 + rz^2)) - R2
    Step 4: Cylinder clipping:
      cyl_de = R - 3*R2
      z_clip = |rz| - 3*R2
      shape_de = max(z_clip, max(cyl_de, limpet_de))

Parameters:
  [0]  R1: double (default: 1.2)
  [1]  Scale: double (default: 2) -- IFS scale (.2Doubles)
  [2]  Z_add: double (default: 0) -- Z offset (.2Doubles)
  [3]  Y_add: double (default: 0)
  [4]  X_add: double (default: 0)
  [5]  R2: double (default: 1.0)
  [6]  Apply_scale_add: int (default: 1)
  [7]  Rmul: double (default: -1.6)
  [8..16] LimpetRot: 9 singles (rotation matrix from .3SingleAngles)

PVar layout:
  [PVar - 0x10] = R1 (double)
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add (double)
  [PVar - 0x40] = X_add (double)
  [PVar - 0x48] = R2 (double)
  [PVar - 0x4c] = Apply_scale_add (integer)
  [PVar - 0x54] = Rmul (double)
  [PVar - 0x58..-0x78] = Rotation matrix (9 singles, .3SingleAngles)

Code structure:
  0x00-0x40: Standard SSE2 IFS preamble
  0x40-0x4B: Store to stack
  0x4B-0x9D: 3x3 rotation (add edi,-0x34; matrix mul; add edi,+0x34)
  0xA1-0xB0: R = sqrt(rx^2 + ry^2)
  0xB4-0xD8: limpet_de = sqrt(sqrt((R1+R*rz+Rmul*R)^2 + rz^2)) - R2
  0xDB-0xE4: cyl_de = R - 3*R2
  0xE7-0xF4: max(cyl_de, limpet_de)
  0xF6-0x112: max(|rz|-3*R2, result)
  0x114-0x123: Scale correction + store + return
*/
