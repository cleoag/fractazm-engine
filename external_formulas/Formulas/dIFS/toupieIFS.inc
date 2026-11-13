#include "ifs_original_state.h"
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void toupieIFSOriginal_x87(double*, const void*);
#endif

// toupieIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: toupieIFS
// Hand-ported from ASM: 274 bytes, DEoption=20
// Toupie (spinning top) isosurface with rotation
// DE = sqrt4(x²+y²) - (Rmul - |z|), intersected with cylinder clip

constexpr unsigned int FORMULA_TOUPIEIFS = 10080;

// ASM trace (x87 section after rotation, 0xA2 onward):
//   0xA2: fld [esp]       → x (rotated)
//   0xA5: fmul st(0)      → x²
//   0xA7: fld [esp+8]     → y; x²
//   0xAB: fmul st(0)      → y²; x²
//   0xAD: faddp            → x²+y²
//   0xAF: fst [esp]       → R² saved to [esp]
//   0xB2: fsqrt            → R = sqrt(x²+y²)
//   0xB4: fst [esp+0x18]  → R saved to [esp+0x18]
//   0xB8: fsqrt            → sqrt(R) = (x²+y²)^(1/4)
//   0xBA: fmul [edi-0x48] → sqrt(R) * Rmul   [Rmul param]
//   0xBD: fld [esp+0x10]  → z; sqrt(R)*Rmul
//   0xC1: fabs             → |z|; sqrt(R)*Rmul
//   0xC3: fsubr [edi-0x10]→ PseudoRadius - |z|; sqrt(R)*Rmul
//   0xC6: fsubp st(1)     → (PseudoRadius - |z|) - sqrt(R)*Rmul
//         Wait — FSUBP: st(1) = st(1) - st(0), pop
//         st(1) was sqrt(R)*Rmul, st(0) was (PseudoRadius - |z|)
//         Result: sqrt(R)*Rmul - (PseudoRadius - |z|)
//   0xC8: fld [esp+0x18]  → R; shape_de
//   0xCC: fsub [edi-0x48] → R - Rmul
//   0xCF: fsub [edi-0x48] → R - 2*Rmul
//   0xD2: fsub [edi-0x48] → R - 3*Rmul
//   0xD5: fcom st(1)      → compare (R-3*Rmul) vs shape_de
//   min pattern: keep the smaller one
//   0xE4: fld [esp+0x10]  → z
//   0xE8: fabs             → |z|
//   0xEA: fsub [edi-0x48] → |z| - Rmul
//   0xED: fsub [edi-0x48] → |z| - 2*Rmul
//   0xF0: fsub [edi-0x48] → |z| - 3*Rmul
//   0xF3: fcom st(1)      → compare (|z|-3*Rmul) vs current_min
//   min pattern again

template<>
cpu_double3 execute_formula<FORMULA_TOUPIEIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    OriginalApplyIFSState native(p,params);
    native.matrix(-80,params+7);
    native.real(0,0.5);
    toupieIFSOriginal_x87(native.state,native.var());
    return native.finish(params,de);
#elif defined(MB3D_SOFTWARE_EXT80)
    Arm64ApplyIFSState native(p,params);
    using E = Arm64ApplyIFSState::E;
    const auto rotated=native.rotate(params+7);
    const E x=rotated.x,y=rotated.y,z=rotated.z;
    const E radius=sqrt(x*x+y*y);
    E distance=sqrt(radius)*params[5]-(E(params[0])-abs(z));
    const E clipR=((E(double(radius))-params[5])-params[5])-params[5];
    distance=clipR >= distance ? clipR : distance;
    const E clipZ=((abs(z)-params[5])-params[5])-params[5];
    distance=clipZ >= distance ? clipZ : distance;
    return native.finish(distance,de);
#else
    // Portable numerical parity remains unverified.

    const double PseudoRadius = params[0];
    const double Scale        = params[1];
    const double Z_add        = params[2];
    const double Y_add        = params[3];
    const double X_add        = params[4];
    const double Rmul         = params[5];
    const int   Apply_scale_add = int(params[6]);
    // params[7..15] = 3x3 rotation matrix (9 singles from .3SingleAngles ToupieRot)
    // [CONSTANTS] embedded as compile-time values:
    const double const0       = 0.5;        // [edi+0x00]

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
    }

    // Step 2: Apply 3x3 rotation matrix (from .3SingleAngles ToupieRot)
    double rx = params[7]  * x + params[8]  * y + params[9]  * z;
    double ry = params[10] * x + params[11] * y + params[12] * z;
    double rz = params[13] * x + params[14] * y + params[15] * z;

    // Step 3: Toupie shape DE
    double R_sq = rx * rx + ry * ry;
    double R = sqrt(R_sq);
    double R4 = sqrt(R);               // (x²+y²)^(1/4) = sqrt(sqrt(x²+y²))

    // shape_de = R4 * Rmul - (PseudoRadius - |rz|)
    // Which rearranges to: R4 * Rmul - PseudoRadius + |rz|
    double shape_de = R4 * Rmul - (PseudoRadius - fabs(rz));

    // x87 fcom/fstp pattern keeps the larger value here.
    double clip_R = R - 3.0 * Rmul;
    shape_de = max(shape_de, clip_R);

    // Same x87 pattern again keeps the larger value.
    double clip_z = fabs(rz) - 3.0 * Rmul;
    shape_de = max(shape_de, clip_z);

    // Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    // Original 0025 skips XYZ stores when Apply is zero; distance still uses local XYZ.
    return Apply_scale_add != 0 ? cpu_double3(x, y, z) : p;
#endif
}

cpu_double3 toupieIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_TOUPIEIFS>(p, params, de);
}

constant FormulaInfo formula_info_toupieIFS = {
    .id = FORMULA_TOUPIEIFS,
    .name = "toupieIFS",
    .param_count = 16,
    .complexity = 480,
    .uses_transcendental = false
};

/*
Original M3 Description:
Toupie is an isosurface defined as follows;

DE = sqrt4(x*x + y*y) - (zsub - abs(z))

Luca GN 2012

Parameters:
  [0] Pseudo Radius: double (default: 1)
  [1] Scale: double (default: 2) (.2Doubles)
  [2] Z_add: double (default: 0) (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] R-mul: double (default: 1)
  [6] Apply scale+add: int (default: 1)
  [7..15] ToupieRot: 9 singles (rotation matrix from .3SingleAngles)
  [16] const0: double (default: 0.5) — unused in shape DE (clipping helper)

PVar layout:
  [PVar - 0x10] = Pseudo Radius (double)
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add
  [PVar - 0x40] = X_add
  [PVar - 0x48] = R-mul (double)
  [PVar - 0x4c] = Apply scale+add (int)
  [PVar - 0x50..-0x70] = ToupieRot (9 singles)
  [edi + 0x00] = const0 (0.5)

Note: The toupie (spinning top) shape is a surface of revolution.
  The core shape is sqrt4(R) * Rmul = PseudoRadius - |z|.
  Cylinder and plane clips at 3*Rmul bound the shape.
  The FSUBP at 0xC6 is st(1) = st(1) - st(0): sqrt(R)*Rmul - (PseudoRadius - |z|).
*/
