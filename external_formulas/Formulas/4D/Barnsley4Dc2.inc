// Barnsley4Dc2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Barnsley4Dc2
// Hand-ported from ASM: 420 bytes, DEoption=4
// 4D formula: uses de parameter as w coordinate input/output
// Barnsley with commutative quat product (Barnett variant)

constexpr unsigned int FORMULA_BARNSLEY4DC2 = 10332;

// ASM trace:
//   esi = context, edi = PVar, ebx = &x
//   eax/ebx = &x, edx = &y, ecx = &z, [ecx+8] = w
//   Parameters:
//     [edi-0x10] = dx (double, default: 1)
//     [edi-0x18] = dy (double, default: 0)
//     [edi-0x20] = dz (double, default: 1)
//     [edi-0x28] = dw (double, default: 0)
//     [edi-0x30] = w_add (double, default: 0)
//     [edi-0x38] = Threshold 1 (double, default: 0)
//     [edi-0x40] = Threshold 2 (double, default: 0)
//     [edi-0x44] = ConjMode (integer, default: 0)
//     [edi-0x48] = FlipMode (integer, default: 0)
//   Constants:
//     [edi+0x00] = 0.5
//     [edi+0x08] = 2.0  (not used in main computation, but for final step)
//   C values: [esi+0x18]=Cx, [esi+0x20]=Cy, [esi+0x28]=Cz, [esi-0x38]=Cw
//
//   Step 1: Coordinate transform
//     x' = x - w;  y' = y + z;  z' = x + w;  w' = y - z
//
//   Step 2: Conditional for x vs Threshold1
//     if x' > T1:  x'' = x' + dx, y'' = y' + dy
//     else:        x'' = x' - dx, y'' = y' - dy
//
//   Step 3: Complex multiply (x,y) with (Cx', Cy')
//     where Cx', Cy' may be modified by ConjMode and FlipMode
//     if ConjMode: Cy' = -Cy
//     if FlipMode: swap Cx' and Cy'
//     x''' = x''*Cx' - y''*Cy'
//     y''' = x''*Cy' + y''*Cx'
//
//   Step 4: Conditional for z vs Threshold2
//     if z' > T2:  z'' = z' + dz, w'' = w' + dw
//     else:        z'' = z' - dz, w'' = w' - dw
//
//   Step 5: Complex multiply (z,w) with (Cz', Cw')
//     Same ConjMode/FlipMode applied to (Cz, Cw)
//     z''' = z''*Cz' - w''*Cw'
//     w''' = x''*Cw' + w''*Cz'  (wait, should be z''*Cw' + w''*Cz')
//     Actually ASM at 0x13C: fld [ebx]; fmul [ebp-0x28] = x_current * Cw'
//     Hmm... let me check. At 0x12D [ecx] has z'', [ecx+8] has w''
//     But [ebx] has x''' from step 3! So it's: x'''*Cw'?
//     0x12D: fld [ecx]; fmul [ebp-0x20] = z'' * Cz'
//     0x132: fld [ecx+8]; fmul [ebp-0x28] = w'' * Cw'; fchs; faddp → z''*Cz' - w''*Cw'
//     0x13C: fld [ebx]; fmul [ebp-0x28] = x''' * Cw'? That doesn't match...
//     Wait at this point ebx still points to &x which was overwritten in step 3.
//     So [ebx] = x''' (result from step 3).
//     Hmm but that means the z,w complex multiply uses x''' instead of z''.
//     Let me re-check...
//     0x13E: [ebx] = the current x value (which was updated in step 3 to x''')
//     But the formula says z' = z*Cz - w*Cw. The fact that x is used seems wrong.
//
//     Actually looking more carefully, ebx at this point... let me trace ebx:
//     After step 3 at 0xF3: fstp [ebx] → x''' stored to [ebx]
//     Then step 4 at 0x12D uses [ecx] and [ecx+8] for z''/w''
//     At 0x13C: fld [ebx] = x'''; fmul [ebp-0x28] = Cw'
//     0x141: fld [ecx+8]; fmul [ebp-0x20] = w''*Cz'; faddp
//     So we get: x'''*Cw' + w''*Cz' ... then + w_add
//     This is intentional - it's a cross-coupling!
//     Actually wait, let me re-read the description more carefully:
//     "z' = z*Cz - w*Cw; w' = z*Cw + w*Cz;" - this is for the z,w pair
//     But the ASM uses ebx (which is x''') for the w' cross term.
//     This might be intentional for this specific Barnsley variant.
//
//   Step 6: Final coordinate transform (inverse of step 1 with scaling)
//     x_final = (x''' + z''') * 0.5
//     y_final = (y''' + w''') * 0.5
//     z_final = (y''' - w''') * 0.5
//     w_final = (-x''' + z''') * 0.5

template<>
cpu_double3 execute_formula<FORMULA_BARNSLEY4DC2>(cpu_double3 p, constant double* params, double &de) {
    const double dx    = params[0];     // [PVar-0x10]
    const double dy    = params[1];     // [PVar-0x18]
    const double dz    = params[2];     // [PVar-0x20]
    const double dw    = params[3];     // [PVar-0x28]
    const double w_add = params[4];     // [PVar-0x30]
    const double T1    = params[5];     // [PVar-0x38] Threshold 1
    const double T2    = params[6];     // [PVar-0x40] Threshold 2
    const int conjMode= int(params[7]);// [PVar-0x44]
    const int flipMode= int(params[8]);// [PVar-0x48]
    // [CONSTANTS] embedded as compile-time values:
    const double half_val  = 0.5;          // [const+0x00]

    // C values passed by engine from context
    // Generated metadata maps 9 negative options first, then 2 positive constants
    // (0.5 and 2.0), then wrapper-appended C values.
    const double Cx = params[11];
    const double Cy = params[12];
    const double Cz_val = params[13];
    const double Cw = params[14];

    double x = p.x, y = p.y, z = p.z, w = de;

    // Step 1: Coordinate transform
    double x1 = x - w;
    double y1 = y + z;
    double z1 = x + w;
    double w1 = y - z;
    x = x1; y = y1; z = z1; w = w1;

    // Step 2: x-conditional from .m3f description:
    //   if x > T1 then +dx/+dy else -dx/-dy
    if (x >= T1) {
        x -= dx;
        y -= dy;
    } else {
        x += dx;
        y += dy;
    }

    // Step 3: z-conditional from .m3f description:
    //   if z > T2 then +dz/+dw else -dz/-dw
    if (z >= T2) {
        z -= dz;
        w -= dw;
    } else {
        z += dz;
        w += dw;
    }

    // Step 4: Complex multiply (x,y) * (Cx', Cy')
    double cx1 = Cx, cy1 = Cy;
    if (conjMode != 0) cy1 = -cy1;
    if (flipMode != 0) { double tmp = cx1; cx1 = cy1; cy1 = tmp; }

    double nx = x * cx1 - y * cy1;
    double ny = x * cy1 + y * cx1;
    x = nx; y = ny;

    // Step 5: Complex multiply (z,w) * (Cz', Cw') following the .m3f definition.
    double cz1 = Cz_val, cw1 = Cw;
    if (conjMode != 0) cw1 = -cw1;
    if (flipMode != 0) { double tmp = cz1; cz1 = cw1; cw1 = tmp; }

    double nz = z * cz1 - w * cw1;
    // ASM uses the already-updated x result from the first complex multiply
    // in the cross term here, not z.
    double nw = x * cw1 + w * cz1 + w_add;
    z = nz; w = nw;

    // Step 6: Final coordinate transform with 0.5 scaling
    double fx = (x + z) * half_val;
    double fy = (y + w) * half_val;
    double fz = (y - w) * half_val;
    double fw = (-x + z) * half_val;

    de = fw;
    return cpu_double3(fx, fy, fz);
}

// Formula metadata
constant FormulaInfo formula_info_Barnsley4Dc2 = {
    .id = FORMULA_BARNSLEY4DC2,
    .name = "Barnsley4Dc2",
    .param_count = 13,
    .complexity = 420,
    .uses_transcendental = false
};

/*
Original M3 Description:
4-D Barnsley conditional formula using commutative quat product (Barnett).
Uses two conditional checks for x and z with two thresholds.

x' = x-w; y' = y+z; z' = x+w; w' = y-z;
if x > T1: x' += dx, y' += dy  else: x' -= dx, y' -= dy
(x,y) complex multiply with (Cx',Cy') [with optional conj/flip]
if z > T2: z' += dz, w' += dw  else: z' -= dz, w' -= dw
(z,w) complex multiply with (Cz',Cw') [with optional conj/flip]
Final: x = (x+z)/2, y = (y+w)/2, z = (y-w)/2, w = (-x+z)/2

Parameters:
  [0] dx: double (default: 1)
  [1] dy: double (default: 0)
  [2] dz: double (default: 1)
  [3] dw: double (default: 0)
  [4] w add: double (default: 0)
  [5] Threshold 1: double (default: 0)
  [6] Threshold 2: double (default: 0)
  [7] ConjMode: integer (default: 0)
  [8] FlipMode: integer (default: 0)
  [9] const 0.5
  [10] const 2.0
  [11..14] Cx, Cy, Cz, Cw (engine-provided)
*/
