#include "ifs_original_state.h"
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void gumdropIFSOriginal_x87(double*, const void*);
#endif

// gumdropIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: gumdropIFS
// Hand-ported from ASM: 293 bytes, Capstone-verified
// DEoption: 20 (dIFS shape -- SSE2 IFS + 3x3 rotation + gumdrop torus DE)
// Gumdrop torus: algebraic isosurface with sqrt4 normalization

constexpr unsigned int FORMULA_GUMDROPIFS = 10204;

template<>
cpu_double3 execute_formula<FORMULA_GUMDROPIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    OriginalApplyIFSState native(p,params);
    native.matrix(-80,params+7);
    native.real(0,5);
    gumdropIFSOriginal_x87(native.state,native.var());
    return native.finish(params,de);
#elif defined(MB3D_SOFTWARE_EXT80)
    Arm64ApplyIFSState native(p,params);
    using E=Arm64ApplyIFSState::E;
    const E x=native.transformed.x,y=native.transformed.y,z=native.transformed.z;
    E squared[3];
    for(int row=0;row<3;++row) {
        const double* m=params+7+row*3;
        const E value=(x*float(m[0])+y*float(m[1]))+z*float(m[2]);
        squared[row]=double(value*value);
    }
    const E a=squared[0],b=squared[1],c=squared[2],gum=abs(E(params[0])),drop=abs(E(params[5]));
    const E norm=double(E(1)/((gum+drop)+E(1))),sum=c+b;
    const E positive=sqrt(sqrt(((sum*sum+a*a)+((sum*a)*gum+gum))*norm));
    const E negative=sqrt(sqrt((((c+b)+a)*drop)*norm));
    return native.finish((positive+(-negative))*E(params[16]),de);
#else
    // Portable numerical parity remains unverified.

    const double Gum        = params[0];   // [edi - 0x10]  default: 4.25
    const double Scale      = params[1];   // [edi - 0x20]  .2Doubles
    const double Z_add      = params[2];   // [edi - 0x30]  .2Doubles
    const double Y_add      = params[3];   // [edi - 0x38]
    const double X_add      = params[4];   // [edi - 0x40]
    const double Drop       = params[5];   // [edi - 0x48]  default: 5
    const int Apply_scale_add = int(params[6]); // [edi - 0x4c]
    // params[7..15] = 3x3 rotation matrix (9 singles from .3SingleAngles TorusRotate)
    // Constant: [edi+0x00] = 5

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

    // Step 2: Apply 3x3 rotation matrix (from .3SingleAngles TorusRotate)
    double rx = params[7]  * x + params[8]  * y + params[9]  * z;
    double ry = params[10] * x + params[11] * y + params[12] * z;
    double rz = params[13] * x + params[14] * y + params[15] * z;

    // Step 3: Square the rotated coordinates
    // ASM: after rotation, the three results are squared before storing:
    //   0x93: fmul st(0) -> rx^2; fstp [esp]
    //   0x98: fmul st(0) -> ry^2; fstp [esp+8]
    //   0x9E: fmul st(0) -> rz^2; fstp [esp+0x10]
    double a = rx * rx;   // [esp]     = rx^2
    double b = ry * ry;   // [esp+8]   = ry^2
    double c = rz * rz;   // [esp+0x10]= rz^2

    // Step 4: Compute normalization factor
    // ASM (0xA8-0xC4):
    //   fld1                   ; 1.0
    //   fld |Gum|              ; |Gum|, 1.0
    //   fst [esp+0x18]         ; save |Gum| = i
    //   fld |Drop|             ; |Drop|, |Gum|, 1.0
    //   fst [esp+0x20]         ; save |Drop| = j
    //   faddp st(1)            ; |Drop| + |Gum|, 1.0
    //   faddp st(1)            ; |Drop| + |Gum| + 1.0
    //   fld1                   ; 1.0, sum
    //   fdivrp st(1)           ; 1.0 / (|Gum| + |Drop| + 1.0)
    //   fstp [esp+0x28]        ; save as norm = 1/(|Gum|+|Drop|+1)
    double i_val = fabs(Gum);    // |Gum|, stored as i
    double j_val = fabs(Drop);   // |Drop|, stored as j
    double norm = 1.0 / (i_val + j_val + 1.0);

    // Step 5: Compute first term (gumdrop part 1)
    // ASM (0xC8-0xF5):
    //   fld c                  ; c
    //   fadd b                 ; c + b
    //   fmul st(0)             ; (b + c)^2
    //   fld a                  ; a, (b+c)^2
    //   fmul st(0)             ; a^2, (b+c)^2
    //   faddp st(1)            ; a^2 + (b+c)^2
    //
    //   fld c                  ; c, above
    //   fadd b                 ; c+b, above
    //   fmul a                 ; a*(b+c), above
    //   fld i_val              ; i, a*(b+c), above
    //   fmul st(1), st(0)     ; i, i*a*(b+c), above    (DC C9)
    //   faddp st(1)            ; i*a*(b+c) + i, above... wait
    //     Actually: DC C9 = fmul st(1), st(0) -> st(1) *= st(0)
    //     st(1) was a*(b+c), now = i * a*(b+c)
    //     DE C1 = faddp st(1) -> st(1) += st(0), pop
    //     st(0) was i, st(1) was i*a*(b+c)
    //     After: st(0) = i*a*(b+c) + i = i*(a*(b+c) + 1)
    //     Hmm wait, that doesn't seem right. Let me re-trace:
    //
    //     Before fld i_val: stack = [a*(b+c), a^2+(b+c)^2]
    //     fld i_val: stack = [i, a*(b+c), a^2+(b+c)^2]
    //     DC C9 = fmul st(1), st(0): st(1) = st(1)*st(0) = a*(b+c)*i
    //       stack = [i, i*a*(b+c), a^2+(b+c)^2]
    //     DE C1 = faddp st(1): st(1) = st(1) + st(0) = i*a*(b+c) + i, pop
    //       stack = [i*a*(b+c) + i, a^2+(b+c)^2]
    //     Hmm that gives i*(a*(b+c) + 1). That doesn't match the description.
    //
    //     Wait, let me re-read the description: "DE = i(aa + (b+c)^2) + j*a*(b+c) + j - k(a+b+c)"
    //     Where i=4, j=17, k=20 (default). But Gum=4.25, Drop=5.
    //
    //     Actually the faddp adds i to i*a*(b+c), giving i + i*a*(b+c) = i*(1 + a*(b+c))
    //     Then DE C1 again: faddp st(1) -> adds this to a^2+(b+c)^2
    //     So: a^2 + (b+c)^2 + i*(1 + a*(b+c))
    //     Hmm, that still doesn't match. Let me re-trace more carefully.
    //
    //     After 0xE4 (fld i_val): [i, a*(b+c), a^2+(b+c)^2]
    //     0xE9 (DC C9 = fmul st(1), st(0)): st(1) *= st(0) -> st(1) = i * a*(b+c)
    //       [i, i*a*(b+c), a^2+(b+c)^2]
    //     0xEB (DE C1 = faddp st(1)): st(1) += st(0) = i*a*(b+c) + i; pop
    //       [i*a*(b+c) + i, a^2+(b+c)^2]
    //     0xED (DE C1 = faddp st(1)): st(1) += st(0) = a^2+(b+c)^2 + i*a*(b+c) + i; pop
    //       [a^2 + (b+c)^2 + i*(a*(b+c) + 1)]
    //     0xEF (fmul [esp+0x28] = norm): *= norm
    //       [(a^2 + (b+c)^2 + i*(a*(b+c) + 1)) * norm]
    //     0xF3 (fsqrt; fsqrt): 4th root
    //       [sqrt(sqrt(... * norm))]
    //
    // part1 = sqrt(sqrt((a^2 + (b+c)^2 + i*(a*(b+c) + 1)) * norm))
    double term1 = a * a + (b + c) * (b + c);
    double term2 = i_val * (a * (b + c) + 1.0);
    double part1 = sqrt(sqrt((term1 + term2) * norm));

    // Step 6: Compute second term (0xF7-0x10E)
    //   fld c                  ; c
    //   fadd b                 ; c + b
    //   fadd a                 ; c + b + a = a + b + c
    //   fmul j_val             ; j * (a+b+c)
    //   fmul norm              ; j * (a+b+c) * norm
    //   fsqrt; fsqrt           ; sqrt(sqrt(j*(a+b+c)*norm))
    //   fchs                   ; negate
    //   faddp st(1)            ; part1 - sqrt(sqrt(j*(a+b+c)*norm))
    double part2 = sqrt(sqrt(j_val * (a + b + c) * norm));

    // Step 7: Final multiplication
    //   fmul [edi] = constant 5.0
    double shape_de = (part1 - part2) * 5.0;

    // Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    // Original 0025 skips XYZ stores when Apply is zero; distance still uses local XYZ.
    return Apply_scale_add != 0 ? cpu_double3(x, y, z) : p;
#endif
}

cpu_double3 gumdropIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_GUMDROPIFS>(p, params, de);
}

constant FormulaInfo formula_info_gumdropIFS = {
    .id = FORMULA_GUMDROPIFS,
    .name = "gumdropIFS",
    .param_count = 16,
    .complexity = 550,
    .uses_transcendental = false
};

/*
Original M3 Description:
Gumdrop torus, a simple but fancy isosurface.
As usual uncheck "Vary DEstop" for a more homogen object appearance.

a = x^2; b = y^2; c = z^2;
i = 4; j = 17; k = 20;
DE = i ( a a + (b + c) ^ 2 ) + j a ( b + c ) + j - k ( a + b + c )
// I used a "stabilized" version of this expression good for many values of i,j,k

Luca GN 2012

Mathematical Formula (from ASM trace):
  DEoption=20 (dIFS shape -- SSE2 IFS + rotation + gumdrop torus):
    Step 1: IFS transform
    Step 2: 3x3 rotation from .3SingleAngles TorusRotate
    Step 3: Square coordinates: a=rx^2, b=ry^2, c=rz^2
    Step 4: Normalize: i=|Gum|, j=|Drop|, norm = 1/(i+j+1)
    Step 5: Part1 = sqrt4((a^2 + (b+c)^2 + i*(a*(b+c)+1)) * norm)
    Step 6: Part2 = sqrt4(j * (a+b+c) * norm)
    Step 7: shape_de = (Part1 - Part2) * 5.0

    Note: The formula is a "stabilized" algebraic torus expression.
    The 4th root (sqrt(sqrt(...))) normalizes the quartic expression for DE.
    The constant 5.0 scales the result.

Parameters:
  [0]  Gum: double (default: 4.25) -- shape parameter i
  [1]  Scale: double (default: 2) -- IFS scale (.2Doubles)
  [2]  Z_add: double (default: 0) -- Z offset (.2Doubles)
  [3]  Y_add: double (default: 0)
  [4]  X_add: double (default: 0)
  [5]  Drop: double (default: 5) -- shape parameter j
  [6]  Apply_scale_add: int (default: 1)
  [7..15] TorusRotate: 9 singles (rotation matrix from .3SingleAngles)
  Constant: [edi+0x00] = 5.0

PVar layout:
  [PVar - 0x10] = Gum (double)
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add (double)
  [PVar - 0x40] = X_add (double)
  [PVar - 0x48] = Drop (double)
  [PVar - 0x4c] = Apply_scale_add (integer)
  [PVar - 0x50..-0x70] = Rotation matrix (9 singles, .3SingleAngles)
  [PVar + 0x00] = 5.0 (constant)

Code structure:
  0x00-0x40: Standard SSE2 IFS preamble
  0x40-0x4B: Store to stack
  0x4C-0x91: 3x3 rotation
  0x93-0xA0: Square rx, ry, rz -> a, b, c
  0xA4: restore edi
  0xA8-0xC4: norm = 1/(|Gum|+|Drop|+1)
  0xC8-0xF5: part1 = sqrt4((a^2+(b+c)^2+i*(a*(b+c)+1))*norm)
  0xF7-0x10E: part2 = sqrt4(j*(a+b+c)*norm)
  0x110-0x112: (part1-part2)*5.0
  0x115-0x124: Scale correction + store + return
*/
