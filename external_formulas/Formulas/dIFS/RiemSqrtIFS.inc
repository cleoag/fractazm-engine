#include "ifs_original_state.h"
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void RiemSqrtIFSOriginal_x87(double*, const void*);
#endif

// RiemSqrtIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: RiemSqrtIFS
// Hand-ported from ASM (222 bytes, Capstone-verified)
// DEoption: 20 (dIFS shape — SSE2 IFS transform + x87 Riemann sqrt surface DE)
// Riemann square root surface: z +/- sqrt(sqrt(x^2+y^2))*|cos(0.5*atan2(y,x))| = 0

constexpr unsigned int FORMULA_RIEMSQRTIFS = 10190;

template<>
cpu_double3 execute_formula<FORMULA_RIEMSQRTIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    OriginalApplyIFSState native(p,params);
    native.integer(-80,static_cast<int32_t>(params[7]));
    native.real(-88,params[8]);
    native.real(-96,params[9]);
    native.real(0,0.5);
    native.real(8,0.017453293000000002);
    RiemSqrtIFSOriginal_x87(native.state,native.var());
    return native.finish(params,de);
#elif defined(MB3D_SOFTWARE_EXT80)
    Arm64ApplyIFSState native(p,params);
    using E = Arm64ApplyIFSState::E;
    const E x=native.transformed.x,y=native.transformed.y,z=native.transformed.z;
    const E radius=sqrt(sqrt(x*x+y*y));
    const E theta=mb3d::numeric::extendedAtan2(y,x)*E(int32_t(params[7]));
    const E angle=(theta+E(params[5])*double(0.017453293000000002))*E(0.5)+radius*params[9];
    // FCOS retains its input at finite |angle| >= 2^63; the helper's cos
    // member has precisely that single-output behavior.
    const E surface=(radius*abs(mb3d::numeric::extendedSinCos(angle).cos))*params[8];
    const E minus=surface-z, plus=surface+z;
    E distance=-(minus >= plus ? plus : minus);
    const E bx=abs(x)-params[0], by=abs(y)-params[0];
    distance=bx >= distance ? bx : distance;
    distance=by >= distance ? by : distance;
    return native.finish(distance,de);
#else
    // Portable numerical parity remains unverified.

    const double CutBoxSz = params[0];    // [edi-0x10]
    const double Scale = params[1];       // [edi-0x20] (.2Doubles)
    const double Z_add = params[2];       // [edi-0x30] (.2Doubles)
    const double Y_add = params[3];       // [edi-0x38]
    const double X_add = params[4];       // [edi-0x40]
    const double Orientation = params[5]; // [edi-0x48]
    const int Apply_scale_add = int(params[6]); // [edi-0x4c]
    const int Symmetry_order = int(params[7]);  // [edi-0x50]
    const double zAmpl = params[8];       // [edi-0x58]
    const double Swirly = params[9];      // [edi-0x60]
    // Constants: [edi+0x00] = 0.5, [edi+0x08] = 0.017453293 (deg2rad)

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

    // Step 2: Riemann square root surface computation
    // ASM trace 0x4B-0x5A:
    //   fld x; fmul st(0) -> x^2
    //   fld y; fmul st(0) -> y^2
    //   faddp -> x^2 + y^2
    //   fsqrt -> sqrt(x^2+y^2)
    //   fsqrt -> sqrt(sqrt(x^2+y^2)) = (x^2+y^2)^(1/4)
    double r2 = x * x + y * y;
    double r_fourth = sqrt(sqrt(r2));

    // Step 3: Compute angle
    // ASM 0x5C-0x65:
    //   fld y; fld x; fpatan -> theta = atan2(y, x)
    //   fimul [edi-0x50]=Symmetry_order -> theta * Symmetry_order
    double theta = atan2(y, x);
    double scaled_theta = theta * double(Symmetry_order);

    // Step 4: Add orientation and swirly modifiers
    // ASM 0x68-0x70:
    //   fld Orientation([edi-0x48])
    //   fmul [edi+0x08]=0.017453293(deg2rad) -> Orientation_rad
    //   faddp -> scaled_theta + Orientation_rad
    //   fmul [edi+0x00]=0.5 -> angle = (scaled_theta + Orientation_rad) * 0.5
    double Orientation_rad = Orientation * 0.017453293;
    double angle = (scaled_theta + Orientation_rad) * 0.5;

    // Step 5: Add swirly (distance-dependent twist)
    // ASM 0x72-0x78:
    //   fld r_fourth(st(1)); fmul Swirly([edi-0x60]); faddp
    //   -> angle = angle + r_fourth * Swirly
    angle = angle + r_fourth * Swirly;

    // Step 6: cos, abs, multiply by r_fourth, scale by zAmpl
    // ASM 0x79-0x7:
    //   fcos -> cos(angle)
    //   fabs -> |cos(angle)|
    //   fmulp -> |cos(angle)| * r_fourth
    //   fmul zAmpl([edi-0x58]) -> surface_val = |cos(angle)| * r_fourth * zAmpl
    double surface_val = abs(cos(angle)) * r_fourth * zAmpl;

    // Step 7: Compute signed distances and take min(abs) for shape DE
    // ASM 0x82-0x9C:
    //   fld surface_val(st(0)); fadd z -> val_plus = surface_val + z
    //   fxch; fsub z -> val_minus = surface_val - z
    //
    //   fcom st(1): compare val_minus vs val_plus
    //   if val_minus < val_plus: keep val_minus (fstp st(1))
    //   else: keep val_plus (fstp st(0))
    //   -> closer = min(val_minus, val_plus)
    //   fchs -> shape_de = -closer
    double val_plus = surface_val + z;
    double val_minus = surface_val - z;

    // ASM comparison: fcom st(1) compares st(0)=val_minus with st(1)=val_plus
    //   if val_minus < val_plus (C0=1): fstp st(1) -> keep val_minus
    //   else: fstp st(0) -> keep val_plus
    // Then fchs negates the result
    double closer;
    if (val_minus < val_plus) {
        closer = val_minus;
    } else {
        closer = val_plus;
    }
    double shape_de = -closer;

    // Step 8: Box clipping on |x| and |y|
    // ASM 0x9-0xCD:
    //   fld x; fabs; fsub CutBoxSz([edi-0x10]) -> box_x = |x| - CutBoxSz
    //   if box_x < shape_de: discard box_x, keep shape_de
    //   else: discard shape_de, keep box_x
    //   -> shape_de = max(shape_de, |x| - CutBoxSz)
    double box_x = abs(x) - CutBoxSz;

    // ASM: fcom st(1) compares st(0)=box_x with st(1)=shape_de
    //   if box_x < shape_de (C0=1): fstp st(0) -> discard box_x, keep shape_de
    //   else: fstp st(1) -> discard shape_de, keep box_x
    shape_de = max(shape_de, box_x);

    //   fld y; fabs; fsub CutBoxSz -> box_y = |y| - CutBoxSz
    //   Same max pattern
    double box_y = abs(y) - CutBoxSz;
    shape_de = max(shape_de, box_y);

    // Step 9: Scale correction
    // ASM 0xCE-0xD4: if Apply_scale_add == 0: fdiv Scale
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    // Original 0025 skips XYZ stores when Apply is zero; distance still uses local XYZ.
    return Apply_scale_add != 0 ? cpu_double3(x, y, z) : p;
#endif
}

// Alternative direct function implementation
cpu_double3 RiemSqrtIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_RIEMSQRTIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_RiemSqrtIFS = {
    .id = FORMULA_RIEMSQRTIFS,
    .name = "RiemSqrtIFS",
    .param_count = 10,
    .complexity = 450,
    .uses_transcendental = true
};

/*
Original M3 Description:
This plots the solid limited by the "Riemann square root surface" and the unit cube
(no z limits though).
Idea came from: http://en.wikipedia.org/wiki/File:Riemann_surface_sqrt.jpg

After some reading, I came up with this implicit expression:
  z +/- sqrt(sqrt(x^2 + y^2)) * abs(cos(0.5*atan2(y,x))) = 0

Added also three extra modifiers just for fun :)
Luca GN 2012

Mathematical Formula:
  DEoption=20 (dIFS shape — SSE2 IFS + x87 Riemann sqrt surface DE):
    - Standard SSE2 IFS preamble
    - Compute r_fourth = (x^2+y^2)^(1/4)
    - theta = atan2(y,x) * Symmetry_order
    - angle = (theta + Orientation*deg2rad) * 0.5 + r_fourth * Swirly
    - surface_val = |cos(angle)| * r_fourth * zAmpl
    - shape_de = -min(surface_val + z, surface_val - z)
    - Clip with box: shape_de = max(shape_de, |x|-CutBoxSz, |y|-CutBoxSz)

Parameters:
  [0] CutBoxSz: double (default: 2) — box clipping size
  [1] Scale: double (default: 2) — IFS scale (.2Doubles)
  [2] Z_add: double (default: 0) — Z offset (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] Orientation: double (default: 0) — rotation in degrees
  [6] Apply_scale_add: int (default: 1)
  [7] Symmetry_order: int (default: 1) — angular symmetry fold
  [8] zAmpl: double (default: 1) — z amplitude scale
  [9] Swirly: double (default: 0) — distance-dependent twist

Constants:
  [edi+0x00] = 0.5
  [edi+0x08] = 0.017453293 (pi/180, degree to radian)

ASM trace summary:
  0x00-0x3: SSE2 IFS preamble (standard)
  0x40-0x4A: Store transformed coords to stack
  0x4B-0x5A: r_fourth = (x^2+y^2)^(1/4) via double sqrt
  0x5C-0x65: theta = atan2(y,x) * Symmetry_order (fimul)
  0x68-0x70: angle = (theta + Orientation_rad) * 0.5
  0x72-0x78: angle += r_fourth * Swirly
  0x79-0x7: surface_val = |cos(angle)| * r_fourth * zAmpl
  0x82-0x9C: shape_de = -min(surface_val+z, surface_val-z)
  0x9-0xCD: Box clip: max with (|x|-CutBoxSz) and (|y|-CutBoxSz)
  0xCE-0xD7: Scale correction + store DE + return

Key notes:
  - r_fourth = double sqrt gives the characteristic Riemann surface shape
  - Symmetry_order folds the angular component (higher = more petals)
  - Swirly adds distance-dependent twist for artistic effect
  - Box clipping limits the surface to a cube of side 2*CutBoxSz
  - The min/negate gives the signed distance to the closer sheet
  - 222 bytes, moderate complexity with trig
*/
