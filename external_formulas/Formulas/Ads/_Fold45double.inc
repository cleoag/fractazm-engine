// _Fold45double.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _Fold45double
// Hand-ported from ASM: 314 bytes, DEoption=-1

constexpr unsigned int FORMULA__FOLD45DOUBLE = 10017;

// ASM trace:
//   Parameters (edi = PVar):
//     [edi-0x10] = k (+) (double, default 1.0)
//     [edi-0x14] = Enable pre-rotation (integer, default 0)
//     [edi-0x18] = Opposite x (integer, default 0)
//     [edi-0x1c] = Opposite y (integer, default 0)
//     [edi-0x20] = Opposite z (integer, default 0)
//     [edi-0x28] = 2nd k fix (double, default 0.0)
//   Constants:
//     [edi+0x00] = 2/3 (0.66666666...)
//
//   Sign convention: xsig = (Opposite_x ? -1 : 1), similarly for y, z
//
//   Pre-rotation (subroutine at 0xDD, if enabled):
//     Note: pre-rotation uses INVERTED signs vs main fold
//     dd_pre = (-x*xsig_inv + -y*ysig_inv + -z*zsig_inv) * h   [where xsig_inv = -xsig]
//     Wait, ASM uses: if Opposite==0, negate (i.e. sign is -1 when NOT opposite)
//     So pre-rotation sign: sig_pre = (Opposite ? 1 : -1) = -xsig
//     sum_pre = (Opp_x ? x : -x) + (Opp_y ? y : -y) + (Opp_z ? z : -z)
//     dd_pre = sum_pre * (2/3)
//     Then: x' = -(x + (Opp_x==0 ? dd_pre : -dd_pre))  →  -(x +/- dd_pre)
//     ASM detail: if Opp==0 → fadd; if Opp!=0 → fsub; then fchs
//
//   Main fold:
//     sum = (Opp_x ? -x : x) + (Opp_y ? -y : y) + (Opp_z ? -z : z)
//     dd = sum - |k|
//     First half: if dd >= 0:
//       dd *= 2/3
//       For each axis: if Opposite: coord += dd; else: coord -= dd
//     Second half: if dd < 0:
//       dd2 = (sum - |k|) + 2*|k| + k2fix = sum + |k| + k2fix
//       if dd2 <= 0:
//         dd2 *= 2/3
//         Same adjustment as first half
//
//   Comparison note (0x004D jb):
//     ftst compares st0 with 0. jb after sahf = jump if st0 < 0.
//     So jb 0x86 means: if (sum - |k|) < 0, go to second half.
//   Comparison note (0x009B ja):
//     ja after sahf = jump if st0 > 0.
//     So ja 0xd0 means: if dd2 > 0, skip (no fold).

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Fold45doubleOriginal_x87(double*,const double*,const int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA__FOLD45DOUBLE>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[9] = {p.x,p.y,p.z};
    const int flags[] = {cpu_bitcast_to_int(params[1]),cpu_bitcast_to_int(params[2]),cpu_bitcast_to_int(params[3]),cpu_bitcast_to_int(params[4])};
    Fold45doubleOriginal_x87(state,params,flags);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E = numeric::Extended80;
    const bool opposite[] = {cpu_bitcast_to_int(params[2]) != 0,
                             cpu_bitcast_to_int(params[3]) != 0,
                             cpu_bitcast_to_int(params[4]) != 0};
    double v[] = {p.x,p.y,p.z};
    const E factor(params[6]);
    if (cpu_bitcast_to_int(params[1])) {
        const E shift = ((opposite[0] ? E(v[0]) : -E(v[0]))
                       + (opposite[1] ? E(v[1]) : -E(v[1]))
                       + (opposite[2] ? E(v[2]) : -E(v[2]))) * factor;
        for (int i=0;i<3;++i)
            v[i] = double(-(opposite[i] ? E(v[i])-shift : E(v[i])+shift));
    }
    const E k = abs(E(params[0]));
    E delta = ((opposite[0] ? -E(v[0]) : E(v[0]))
             + (opposite[1] ? -E(v[1]) : E(v[1]))
             + (opposite[2] ? -E(v[2]) : E(v[2]))) - k;
    if (!(delta >= E(0))) {
        // Preserve 008B..0090 cancellation/grouping and the unordered branch.
        delta = (delta+k) + (k+E(params[5]));
        if (delta > E(0)) return cpu_double3(v[0],v[1],v[2]);
    }
    delta *= factor;
    for (int i=0;i<3;++i) v[i] = double(opposite[i] ? E(v[i])+delta : E(v[i])-delta);
    return cpu_double3(v[0],v[1],v[2]);
#else

    double k_param     = params[0];  // [PVar-0x10], double, default 1.0
    int   preRotation = cpu_bitcast_to_int(params[1]);  // [PVar-0x14], integer, default 0
    int   oppX        = cpu_bitcast_to_int(params[2]);  // [PVar-0x18], integer, default 0
    int   oppY        = cpu_bitcast_to_int(params[3]);  // [PVar-0x1c], integer, default 0
    int   oppZ        = cpu_bitcast_to_int(params[4]);  // [PVar-0x20], integer, default 0
    double k2fix       = params[5];  // [PVar-0x28], double, default 0.0
    double h           = 0.6666666666666667;  // constant 2/3

    double k = fabs(k_param);  // k is forced positive (fabs at 0x0040)

    double x = p.x;
    double y = p.y;
    double z = p.z;

    // Sign multipliers: +1 normally, -1 if Opposite
    double sx = (oppX != 0) ? -1.0 : 1.0;
    double sy = (oppY != 0) ? -1.0 : 1.0;
    double sz = (oppZ != 0) ? -1.0 : 1.0;

    // Pre-rotation (optional)
    // ASM uses inverted sign convention: negate when Opposite==0
    if (preRotation != 0) {
        double sum_pre = (-sx * x) + (-sy * y) + (-sz * z);
        double dd_pre = sum_pre * h;
        // For each axis: if Opposite==0: coord = -(coord + dd_pre)
        //                if Opposite!=0: coord = -(coord - dd_pre)
        x = -(x - sx * dd_pre);
        y = -(y - sy * dd_pre);
        z = -(z - sz * dd_pre);
    }

    // Main fold computation
    double sum = sx * x + sy * y + sz * z;
    double dd = sum - k;

    if (dd >= 0.0) {
        // First half: fold when sum >= k
        dd *= h;
        x -= sx * dd;
        y -= sy * dd;
        z -= sz * dd;
    } else {
        // Second half: dd2 = sum + k + k2fix
        double dd2 = sum + k + k2fix;
        if (dd2 <= 0.0) {
            dd2 *= h;
            x -= sx * dd2;
            y -= sy * dd2;
            z -= sz * dd2;
        }
    }

    return cpu_double3(x, y, z);
#endif
}

// Formula metadata
constant FormulaInfo formula_info__Fold45double = {
    .id = FORMULA__FOLD45DOUBLE,
    .name = "_Fold45double",
    .param_count = 6,
    .complexity = 314,
    .uses_transcendental = false
};

/*
Original M3 Description:

This folding is center-symmetrical, but can give unexpected results.
You can modify this with the fixing factor (example, for a triple
Sierpinski pyramid or similar effects...)
Variation of _Fold45single; it's more like Tglad's folding
on a single chosen direction. The main difference is that
it introduces rotations in some areas. This can be interesting.
Use itercount>1 to see cool multi-folds!
See the basic version for more comments.

if (PR == 1)
 dd = -(x*xsig + y*ysig + z*zsig)*h  // pre-rotation; center rotated, sides normal
 x' = -(x - dd*xsig) // Switch both signs to keep the correct effect!
 y' = -(y - dd*ysig)
 z' = -(z - dd*zsig)
endif
// k is forced to be positive
dd = (x*xsig + y*ysig + z*zsig - k)  // first half
if dd*q > 0
 dd = dd*h
 x' = x - dd*xsig
 y' = y - dd*ysig
 z' = z - dd*zsig
else
 dd = dd + 2*k + k2fix // second half
 if dd*q < 0
  dd = dd*h
  x' = x - dd*xsig
  y' = y - dd*ysig
  z' = z - dd*zsig
 endif
endif

Parameters:
  params[0] = k (+) (double, default 1.0) - fold threshold, forced positive
  params[1] = Enable pre-rotation (integer, default 0)
  params[2] = Opposite x (integer, default 0) - flips sign of x contribution
  params[3] = Opposite y (integer, default 0) - flips sign of y contribution
  params[4] = Opposite z (integer, default 0) - flips sign of z contribution
  params[5] = 2nd k fix (double, default 0.0) - adjustment for second half fold

Constant: 2/3 (0.66666666...) used as fold factor

ASM verification notes:
  - Pre-rotation uses inverted signs: when Opposite==0, the axis value
    is negated in the sum (jne skips fchs, so if Opposite==0, fchs applies).
    Then the adjustment is: -(coord + dd_pre) when Opposite==0,
    -(coord - dd_pre) when Opposite!=0. Final fchs at 0x010,0x0121,0x0135.
  - Main fold: when Opposite==0, use fsub (subtract dd from coord);
    when Opposite!=0, use fadd (add dd to coord). This corresponds to
    coord -= sx * dd where sx is the sign multiplier.
  - Second half condition: ftst + ja means skip if dd2 > 0, so fold
    applies when dd2 <= 0.
*/
