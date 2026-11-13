#include "ifs_original_state.h"
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void borgIFSOriginal_x87(double*, const void*);
#endif

// borgIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: borgIFS
// Hand-ported from ASM: 304 bytes, Capstone-verified
// DEoption: 20 (dIFS shape -- SSE2 IFS + 3x3 rotation + Borg cube / gyroid variant DE)
// The Borg Cube: sin(xy) + sin(yz) + sin(zx) = 0, intersected with a cube

constexpr unsigned int FORMULA_BORGIFS = 10207;

template<>
cpu_double3 execute_formula<FORMULA_BORGIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    OriginalApplyIFSState native(p,params);
    native.matrix(-80,params+7);
    native.real(-120,params[16]);
    borgIFSOriginal_x87(native.state,native.var());
    return native.finish(params,de);
#elif defined(MB3D_SOFTWARE_EXT80)
    Arm64ApplyIFSState native(p,params);
    using E = Arm64ApplyIFSState::E;
    const auto rotated=native.rotate(params+7);
    const E x=rotated.x,y=rotated.y,z=rotated.z;
    auto sine = [](E a) {
        if ((a.signExponent()&0x7fff) != 0x7fff && abs(a) >= E(0x1p63)) return a;
        return mb3d::numeric::extendedSinCos(a).sin;
    };
    const E s1=sine((x*y)*params[5]), s2=sine((z*y)*params[5]), s3=sine((z*x)*params[5]);
    params[MAX_FORMULA_PARAMS-1]=double(abs(((s3-s2)-s1)*params[16]));
    const E sum=s1+(s2+s3);
    E cube=abs(y) >= abs(z) ? abs(y) : abs(z);
    cube=abs(x) >= cube ? abs(x) : cube;
    cube-=params[0];
    return native.finish((cube >= sum ? cube : sum)/params[5],de);
#else
    // Portable numerical parity remains unverified.

    const double Size       = params[0];   // [edi - 0x10]  default: 1.5
    const double Scale      = params[1];   // [edi - 0x20]  .2Doubles
    const double Z_add      = params[2];   // [edi - 0x30]  .2Doubles
    const double Y_add      = params[3];   // [edi - 0x38]
    const double X_add      = params[4];   // [edi - 0x40]
    const double Density    = params[5];   // [edi - 0x48]  default: 6.28
    const int Apply_scale_add = int(params[6]); // [edi - 0x4c]
    // params[7..15] = 3x3 rotation matrix (9 singles from .3SingleAngles BoRot)
    const double TrapColor_Density = params[16]; // [edi - 0x78]  default: 1
    // Note: the M3 has ".Double TrapColor Density = 1" which maps to [edi-0x78]
    // because after Apply_scale_add(-0x4c) there are 9 singles (-0x50..-0x70) for rotation
    // then TrapColor_Density at -0x78
    // Actually: [edi - 0x78] -> 0x78 = 0x100 - 0x88 -> in ASM: DC 4 88 at 0xCC
    // Wait: 0xCC: DC 4 88. 0x88 = -0x78 in signed byte. So [edi + (-0x78)] = [edi-0x78].
    // But 0x88 as unsigned is 136, and as signed 8-bit is... wait, this is a dword displacement.
    // DC 4 88 = fmul qword ptr [edi - 0x78]? Let me check:
    // Actually ModR/M byte 4 = mod=01, reg=001 (for DC prefix this is fmul), rm=111 (edi)
    // Displacement = 0x88 (signed 8-bit) = -0x78. Yes, [edi - 0x78].

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

    // Step 2: Apply 3x3 rotation matrix (from .3SingleAngles BoRot)
    double rx = params[7]  * x + params[8]  * y + params[9]  * z;
    double ry = params[10] * x + params[11] * y + params[12] * z;
    double rz = params[13] * x + params[14] * y + params[15] * z;

    // Step 3: Borg cube / gyroid surface
    // ASM trace (0xA1-0x12):
    //
    // 0xA1-0xC4: Compute three sin terms
    //   fld rx; fmul ry; fmul Density; fsin -> sin(rx*ry*Density) = s1
    //   fld rz; fmul ry; fmul Density; fsin -> sin(rz*ry*Density) = s2
    //   fld rz; fmul rx; fmul Density; fsin -> sin(rz*rx*Density) = s3
    //   Stack: [s3, s2, s1]
    double s1 = sin(rx * ry * Density);
    double s2 = sin(rz * ry * Density);
    double s3 = sin(rz * rx * Density);

    // 0xC6-0xD1: Orbit trap coloring
    //   fld st(0)              ; [s3, s3, s2, s1]
    //   fsub st(2)             ; [s3-s2, s3, s2, s1]
    //     D8 E2 = fsub st(0), st(2): st(0) -= st(2)
    //   fsub st(3)             ; [s3-s2-s1, s3, s2, s1]
    //     D8 E3 = fsub st(0), st(3)
    //   fmul TrapColor_Density ; * TrapClr
    //   fabs                   ; |...|
    //   fstp [esi+0x80]        ; store orbit trap
    params[MAX_FORMULA_PARAMS - 1] = fabs((s3 - s2 - s1) * TrapColor_Density);

    // 0xD7-0xDB: Sum the three sin values
    //   faddp st(1)            ; [s3+s2, s1]
    //   faddp st(1)            ; [s3+s2+s1]
    double gyroid_sum = s1 + s2 + s3;

    // 0xDB-0x10B: Cube intersection distance
    //   fld rz; fabs -> |rz|
    //   fld ry; fabs -> |ry|
    //   fcom; max(|rz|, |ry|) -> max_yz
    //   fld rx; fabs -> |rx|
    //   fcom; max(|rx|, max_yz) -> max_xyz
    double max_xyz = max(fabs(rx), max(fabs(ry), fabs(rz)));

    // 0x10B: fsub Size -> cube_dist = max_xyz - Size
    double cube_dist = max_xyz - Size;

    // 0x10E-0x11B: max(cube_dist, gyroid_sum)
    //   fcom st(1): compare cube_dist vs gyroid_sum
    //   st0=cube_dist, st1=gyroid_sum
    //   if cube_dist < gyroid_sum: keep gyroid_sum
    //   else: keep cube_dist
    //   result = max(cube_dist, gyroid_sum)
    double shape_max = max(cube_dist, gyroid_sum);

    // 0x11D: fdiv Density -> shape_de = max_val / Density
    double shape_de = shape_max / Density;

    // Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    // Original 0025 skips XYZ stores when Apply is zero; distance still uses local XYZ.
    return Apply_scale_add != 0 ? cpu_double3(x, y, z) : p;
#endif
}

cpu_double3 borgIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_BORGIFS>(p, params, de);
}

constant FormulaInfo formula_info_borgIFS = {
    .id = FORMULA_BORGIFS,
    .name = "borgIFS",
    .param_count = 17,
    .complexity = 650,
    .uses_transcendental = true
};

/*
Original M3 Description:
The Borg Cube, a cool variant of the gyroid.
Enable "Orbit trap" mode to see a marble coloring.

Equation;
      sin xy + sin yz + sin zx =0

(Intersected with a cube... it's infinite)

Luca GN 2012

Mathematical Formula (from ASM trace):
  DEoption=20 (dIFS shape -- SSE2 IFS + rotation + borg cube):
    Step 1: IFS transform
    Step 2: 3x3 rotation from .3SingleAngles BoRot
    Step 3: Borg/gyroid variant surface:
      s1 = sin(rx*ry*Density)
      s2 = sin(rz*ry*Density)
      s3 = sin(rz*rx*Density)
      gyroid_sum = s1 + s2 + s3
    Step 4: Cube intersection:
      cube_dist = max(|rx|, |ry|, |rz|) - Size
    Step 5: shape_de = max(cube_dist, gyroid_sum) / Density

    Note: This differs from the standard gyroid (cos*sin cross products)
    by using sin of PRODUCTS of coordinates (sin(x*y) etc). The Density
    parameter controls the spatial frequency.

    OTrap = |TrapColor_Density * (s3 - s2 - s1)|

Parameters:
  [0]  Size: double (default: 1.5) -- cube half-size
  [1]  Scale: double (default: 2) -- IFS scale (.2Doubles)
  [2]  Z_add: double (default: 0) -- Z offset (.2Doubles)
  [3]  Y_add: double (default: 0)
  [4]  X_add: double (default: 0)
  [5]  Density: double (default: 6.28) -- surface frequency
  [6]  Apply_scale_add: int (default: 1)
  [7..15] BoRot: 9 singles (rotation matrix from .3SingleAngles)
  [16] TrapColor_Density: double (default: 1)

PVar layout:
  [PVar - 0x10] = Size
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add
  [PVar - 0x40] = X_add
  [PVar - 0x48] = Density
  [PVar - 0x4c] = Apply_scale_add (integer)
  [PVar - 0x50..-0x70] = Rotation matrix (9 singles)
  [PVar - 0x78] = TrapColor_Density

Code structure:
  0x00-0x40: Standard SSE2 IFS preamble
  0x40-0x4B: Store to stack
  0x4C-0x9E: 3x3 rotation
  0xA1-0xC4: Three sin terms: sin(xy*D), sin(zy*D), sin(zx*D)
  0xC6-0xD1: OTrap = |TrapClr * (s3-s2-s1)| -> [esi+0x80]
  0xD7-0xDB: gyroid_sum = s1 + s2 + s3
  0xDB-0x10B: cube_dist = max(|rx|,|ry|,|rz|) - Size
  0x10E-0x11D: max(cube_dist, gyroid_sum) / Density
  0x120-0x12: Scale correction + store + return
*/
