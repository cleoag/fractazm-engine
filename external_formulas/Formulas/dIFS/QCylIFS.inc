#include "ifs_original_state.h"
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void QCylIFSOriginal_x87(double*, const void*);
#endif

// QCylIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: QCylIFS
// Hand-ported from ASM: 156 bytes, Capstone-verified
// DEoption: 20 (dIFS shape — SSE2 IFS transform + x87 quartic cylinder DE)
// Quartic curve in cylindrical coords with pinnacle limiter

constexpr unsigned int FORMULA_QCYLIFS = 3604;

template<>
cpu_double3 execute_formula<FORMULA_QCYLIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    OriginalApplyIFSState native(p,params);
    native.real(-84,params[7]);
    QCylIFSOriginal_x87(native.state,native.var());
    return native.finish(params,de);
#else
    // Portable numerical parity remains unverified.

    const double DivergMul = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Distance = params[5];
    const int Apply_scale_add = int(params[6]);
    const double Pinnacle_Limiter = params[7];

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;    // w *= Scale
    }

    // Step 2: Absolute z and cylindrical radius
    // ASM: fabs z → r_xy = sqrt(x² + y²)
    double z_abs = abs(z);
    double r_xy = sqrt(x * x + y * y);

    // Step 3: Quartic surface term
    // ASM: r_xy * z_abs → fxch → fld1 → fchs → faddp → (r_xy-1)*DivergMul → faddp
    double quartic_term = r_xy * z_abs;
    double diverge_term = (r_xy - 1.0) * DivergMul;
    double shape_de = quartic_term + diverge_term - Distance;

    // Step 4: Pinnacle limiter — clips z-extent with ramp function
    // ASM: (z_abs - PL) + |z_abs - PL| = 2*max(0, z_abs - PL)
    double pinnacle = z_abs - Pinnacle_Limiter;
    shape_de += pinnacle + abs(pinnacle);    // = shape_de + 2*max(0, z_abs - PL)

    // Step 5: Scale correction
    // ASM: if Apply_scale_add == 0: fdiv Scale
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    // Original 0025 skips XYZ stores when Apply is zero; distance still uses local XYZ.
    return Apply_scale_add != 0 ? cpu_double3(x, y, z) : p;
#endif
}

// Alternative direct function implementation
cpu_double3 QCylIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_QCYLIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_QCylIFS = {
    .id = FORMULA_QCYLIFS,
    .name = "QCylIFS",
    .param_count = 8,
    .complexity = 270,
    .uses_transcendental = false
};

/*
Original M3 Description:
A quartic curve modified.
The pinnacle can cause some DE trouble, decrease raystep/stepwidth or the limiter.

Luca GN 2012

Mathematical Formula:
  DEoption=20 (dIFS shape — SSE2 IFS + x87 quartic cylinder DE):
    - Standard SSE2 IFS preamble
    - Quartic surface in cylindrical coordinates: r_xy * |z|
    - Divergence adjustment: (r_xy - 1) * DivergMul
    - Pinnacle limiter: ramp penalty for |z| > PL

  Step 1: IFS transform:
    x = p.x * Scale + X_add;  y = p.y * Scale + Y_add;  z = p.z * Scale + Z_add
    if Apply_scale_add: w *= Scale, store position

  Step 2: Cylinder radius and |z|:
    z_abs = |z|
    r_xy = sqrt(x² + y²)

  Step 3: Quartic surface:
    shape_de = r_xy * z_abs + (r_xy - 1) * DivergMul - Distance

  Step 4: Pinnacle limiter:
    shape_de += 2 * max(0, z_abs - Pinnacle_Limiter)

Parameters:
  [0] DivergMul: double (default: 0.3) — divergence multiplier
  [1] Scale: double (default: 2) — IFS scale factor (.2Doubles)
  [2] Z_add: double (default: 0) — Z offset (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] Distance: double (default: 0.2) — surface offset (like radius)
  [6] Apply_scale_add: int (default: 1)
  [7] Pinnacle_Limiter: double (default: 3) — z-extent clamp

PVar layout (verified against x87 ASM offsets):
  Options (negative offsets):
  [PVar - 0x10] = DivergMul (double)
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add (double)
  [PVar - 0x40] = X_add (double)
  [PVar - 0x48] = Distance (double)
  [PVar - 0x4c] = Apply_scale_add (integer)
  [PVar - 0x54] = Pinnacle_Limiter (double)

Code structure:
  0x00-0x40: Standard SSE2 IFS preamble
  0x40-0x45: Store to stack for x87
  0x4B-0x51: z_abs = |z|
  0x55-0x62: r_xy = sqrt(x² + y²)
  0x64-0x78: quartic + divergence: r_xy*z_abs + (r_xy-1)*DivergMul
  0x7A: subtract Distance
  0x7D-0x8A: Pinnacle limiter: 2*max(0, z_abs - PL)
  0x8C-0x9B: Scale correction + store + return

Key notes:
  - Quartic surface r*|z| = const defines a hyperbola-like shape in cylindrical coords
  - DivergMul adjusts how quickly surface diverges from unit cylinder
  - Pinnacle limiter uses ramp function (a + |a|) = 2*max(0, a) trick
  - Stack allocation 0x1c (28 bytes) — minimal
  - 156 bytes — simplest dIFS formula
*/
