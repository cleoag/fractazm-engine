// _Fold45single.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _Fold45single
// Hand-ported from ASM: 241 bytes, DEoption=-1

constexpr unsigned int FORMULA__FOLD45SINGLE = 10009;

// ASM trace:
//   esi = context, edi = PVar, ebx = &x
//   Params: [edi-0x10]=k (double), [edi-0x14]=Mode (int),
//           [edi-0x18]=Opposite_x (int), [edi-0x1c]=Opposite_y (int),
//           [edi-0x20]=Opposite_z (int), [edi-0x24]=Enable_pre_rotation (int)
//   Constants: [edi+0x00]=h=2/3
//
//   Define xsig=+1 if Opposite_x==0, -1 otherwise (same for y,z)
//
//   --- Pre-rotation subroutine (0x94) if Enable_pre_rotation != 0 ---
//   sum_pre = (-x if Opp_x==0 else x) + (-y if Opp_y==0 else y)
//             + (-z if Opp_z==0 else z)
//   dd_pre = sum_pre * h
//   For each axis: val' = -(val ± dd_pre)
//     If Opp_n==0: val' = -(val + dd_pre)
//     If Opp_n!=0: val' = -(val - dd_pre)
//
//   --- Main fold ---
//   signed_n = n * xsig (negate if Opp_n != 0)
//   sum = signed_x + signed_y + signed_z
//   dd = sum - k
//   test = dd * q (q=+1 if Mode==0, q=-1 if Mode==1)
//   if test >= 0:
//     dd = dd * h
//     For each axis:
//       If Opp_n==0: n' = n - dd    (normal fold)
//       If Opp_n!=0: n' = n + dd    (opposite fold)
//
// 45-degree asymmetric fold on plane ±x±y±z-k=0 by Luca GN 2011.

template<>
cpu_double3 execute_formula<FORMULA__FOLD45SINGLE>(cpu_double3 p, constant double* params, double &de) {
    const double k    = params[0];  // default: 1.0
    const int Mode   = int(params[1]);  // default: 0 (0 or 1)
    const int Opp_x  = int(params[2]);  // default: 0 (Opposite x)
    const int Opp_y  = int(params[3]);  // default: 0 (Opposite y)
    const int Opp_z  = int(params[4]);  // default: 0 (Opposite z)
    const int PreRot = int(params[5]);  // default: 0 (Enable pre-rotation)

    const double h = 0.66666666666666667;  // 2/3

    double x = p.x, y = p.y, z = p.z;

    // Sign multipliers: +1 if Opp==0, -1 if Opp!=0
    double xsig = (Opp_x == 0) ? 1.0 : -1.0;
    double ysig = (Opp_y == 0) ? 1.0 : -1.0;
    double zsig = (Opp_z == 0) ? 1.0 : -1.0;

    // Pre-rotation (subroutine at 0x94)
    // ASM: negate if Opp_n==0 (opposite of main fold!)
    if (PreRot != 0) {
        double sx = (Opp_x != 0) ? x : -x;
        double sy = (Opp_y != 0) ? y : -y;
        double sz = (Opp_z != 0) ? z : -z;
        double dd_pre = (sx + sy + sz) * h;
        // ASM: if Opp_n==0: val'=-(val+dd), if Opp_n!=0: val'=-(val-dd)
        x = (Opp_x == 0) ? -(x + dd_pre) : -(x - dd_pre);
        y = (Opp_y == 0) ? -(y + dd_pre) : -(y - dd_pre);
        z = (Opp_z == 0) ? -(z + dd_pre) : -(z - dd_pre);
    }

    // Main fold
    double dd = x * xsig + y * ysig + z * zsig - k;

    // Mode determines fold condition:
    // Mode==0: fold if dd >= 0
    // Mode==1: fold if dd <= 0  (ASM negates dd before test)
    bool do_fold = (Mode == 0) ? (dd >= 0.0) : (dd <= 0.0);

    if (do_fold) {
        dd = dd * h;
        x = x - dd * xsig;
        y = y - dd * ysig;
        z = z - dd * zsig;
    }

    return cpu_double3(x, y, z);
}

// Formula metadata
constant FormulaInfo formula_info__fold45single = {
    .id = FORMULA__FOLD45SINGLE,
    .name = "_Fold45single",
    .param_count = 6,
    .complexity = 60,
    .uses_transcendental = false
};

/*
Original M3 Description:
This folding introduces asymmetry, and can give unexpected results.
Basically, it's like "_Abs" modifier but it introduces a rotation in some areas.
Use itercount>1 to see cool multi-folds!
Works with any plane inclined like the OctahedronIFS folding planes
(eight in total). The planes are defined by +-x+-y+-z-k=0 (45 deg for each axis)
Two modes available.
Formula by Luca GN 2011

if (PreRot == 1):
  dd = -(x*xsig + y*ysig + z*zsig) * h   // pre-rotation
  x' = -(x - dd*xsig)
  y' = -(y - dd*ysig)
  z' = -(z - dd*zsig)

dd = (x*xsig + y*ysig + z*zsig - k)
if dd*q > 0:    // q = +1 for Mode 0, -1 for Mode 1
  dd = dd * h   // h = 2/3
  x' = x - dd * xsig
  y' = y - dd * ysig
  z' = z - dd * zsig

Note: xsig is +1 if Opposite_x is 0, -1 otherwise.

Parameters:
  [0] k: double (default: 1.0)
  [1] Mode: integer (default: 0, values 0-1)
  [2] Opposite x: integer (default: 0)
  [3] Opposite y: integer (default: 0)
  [4] Opposite z: integer (default: 0)
  [5] Enable pre-rotation: integer (default: 0)
*/
