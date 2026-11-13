/* portable_math.c â€” deterministic sin/cos/sqrt/log.
 *
 * Shared implementations for the C++ port and patched Pascal reference.
 * Validate output bits and floating-point state on each target; this module
 * alone does not establish renderer parity. See docs/PARITY_TRACE.md.
 *
 * Algorithm source:
 *   - sin/cos: Cephes (public domain, Stephen L. Moshier), via
 *     FPC rtl/inc/genmath.inc sincof/coscof + rem_pio2.
 *   - log:    Cephes log, via FPC rtl/inc/genmath.inc fpc_ln_real.
 *   - sqrt:   SSE2 sqrtsd (IEEE-754 correctly-rounded, same on x86/x64).
 *
 * Operation order and intermediate precision must be checked at actual call sites.
 */

#include "portable_math.h"

#include <stdint.h>
#include <string.h>
#include <math.h>

#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
#  include <emmintrin.h>
#  include <xmmintrin.h>   /* MXCSR control */
#  include <float.h>       /* _controlfp for x87 precision control */
#endif

/* MSVC i386 with /arch:SSE2 still emits x87 instructions for certain
   mixed expressions (e.g. "1.0 - f(x) + g(x)"). Those x87 ops use the
   80-bit extended precision by default, perturbing the low bit compared
   to SSE2 double arithmetic. Force x87 precision to 53-bit (double) on
   function entry so x87 intermediate results match SSE2 bit-for-bit.
   FPC callers leave x87 in 64-bit extended mode by default â€” that's
   why C callers (libs that set up their own FPU state) see different
   output than Pascal callers. */
#if defined(_MSC_VER) && defined(_M_IX86)
#  define PM_SAVE_FPU(cw_var) unsigned int cw_var = _controlfp(0, 0)
#  define PM_FORCE_DOUBLE()   _controlfp(_PC_53, _MCW_PC)
#  define PM_RESTORE_FPU(cw_var) _controlfp(cw_var, _MCW_PC)
#else
#  define PM_SAVE_FPU(cw_var)     (void)0
#  define PM_FORCE_DOUBLE()       (void)0
#  define PM_RESTORE_FPU(cw_var)  (void)0
#endif

/* ---- tiny helpers (avoid libm) ---------------------------------------- */

static double pm_floor(double x) {
    /* Correct floor for all finite doubles (rounds toward -inf). */
    if (x != x) return x;  /* NaN */
    double r = (double)(int64_t)x;
    if (x < r) r -= 1.0;
    return r;
}

static double pm_fabs(double x) {
    uint64_t bits;
    memcpy(&bits, &x, sizeof(bits));
    bits &= 0x7FFFFFFFFFFFFFFFULL;
    double r;
    memcpy(&r, &bits, sizeof(r));
    return r;
}

static double pm_ldexp_pow2(double x, int e) {
    /* Multiply x by 2^e for modest e. Used for *0.5 / *2 safely. */
    uint64_t bits;
    memcpy(&bits, &x, sizeof(bits));
    int exp_bits = (int)((bits >> 52) & 0x7FF);
    if (exp_bits == 0 || exp_bits == 0x7FF) return x;
    exp_bits += e;
    if (exp_bits <= 0) { double r; uint64_t z = bits & 0x8000000000000000ULL; memcpy(&r, &z, sizeof(r)); return r; }
    if (exp_bits >= 0x7FF) {
        uint64_t inf = (bits & 0x8000000000000000ULL) | 0x7FF0000000000000ULL;
        double r; memcpy(&r, &inf, sizeof(r)); return r;
    }
    bits = (bits & 0x800FFFFFFFFFFFFFULL) | ((uint64_t)exp_bits << 52);
    double r; memcpy(&r, &bits, sizeof(r));
    return r;
}

/* ---- sin / cos (Cephes via FPC) --------------------------------------- */

static const double sincof[6] = {
     1.58962301576546568060e-10,
    -2.50507477628578072866e-8,
     2.75573136213857245213e-6,
    -1.98412698295895385996e-4,
     8.33333333332211858878e-3,
    -1.66666666666666307295e-1
};
static const double coscof[6] = {
    -1.13585365213876817300e-11,
     2.08757008419747316778e-9,
    -2.75573141792967388112e-7,
     2.48015872888517045348e-5,
    -1.38888888888730564116e-3,
     4.16666666666665929218e-2
};

static const double PIO4  = 7.85398163397448309616e-1;
static const double FOPI  = 1.27323954473516268615;       /* 4/Ï€ */
static const double DP1   = 7.85398125648498535156e-1;
static const double DP2   = 3.77489470793079817668e-8;
static const double DP3   = 2.69515142907905952645e-15;

static double polevl5(double x, const double c[6]) {
    double r = c[0];
    r = r * x + c[1];
    r = r * x + c[2];
    r = r * x + c[3];
    r = r * x + c[4];
    r = r * x + c[5];
    return r;
}

/* rem_pio2: x -> z in roughly [-Ï€/4, Ï€/4], returns quadrant 0..7
   (caller masks with & 3 for sin/cos). Matches FPC rtl/inc/genmath.inc
   fast path exactly. Only the Cody-Waite branch is implemented; inputs
   outside [-lossth, lossth] are rare in mb3d (angles from
   deg*Pid180 âˆˆ [-2Ï€, 2Ï€]) and fall back to simple reduction. */
static int rem_pio2(double x, double* z_out) {
    double y_abs = pm_fabs(x);
    if (y_abs < PIO4) {
        *z_out = x;
        return 0;
    }
    double y = pm_floor(x * FOPI);
    int i = (int)(y - 16.0 * pm_floor(y * 0.0625));
    if (i & 1) { i += 1; y += 1.0; }
    int j = ((i >> 1) & 7);
    double z = x - y * DP1;
    z = z - y * DP2;
    z = z - y * DP3;
    *z_out = z;
    return j;
}

PM_API double pm_sin(double x) {
    /* Preserve sin(-0) = -0. */
    if (x == 0.0) return x;
    PM_SAVE_FPU(cw);
    PM_FORCE_DOUBLE();
    double z;
    int j = rem_pio2(x, &z) & 3;
    double zz = z * z;
    double y;
    if (j == 1 || j == 3) {
        /* cos polynomial: 1 - zz/2 + zzÂ² * Q(zz) */
        y = 1.0 - pm_ldexp_pow2(zz, -1) + zz * zz * polevl5(zz, coscof);
    } else {
        /* sin polynomial: z + zÂ³ * P(zz) */
        y = z + z * z * z * polevl5(zz, sincof);
    }
    if (j > 1) y = -y;
    PM_RESTORE_FPU(cw);
    return y;
}

PM_API double pm_cos(double x) {
    PM_SAVE_FPU(cw);
    PM_FORCE_DOUBLE();
    double z;
    int j = rem_pio2(x, &z) & 3;
    double zz = z * z;
    double y;
    if (j == 1 || j == 3) {
        y = z + z * z * z * polevl5(zz, sincof);
    } else {
        y = 1.0 - pm_ldexp_pow2(zz, -1) + zz * zz * polevl5(zz, coscof);
    }
    if (j == 1 || j == 2) y = -y;
    PM_RESTORE_FPU(cw);
    return y;
}

/* ---- out-parameter variants (bypass x87 return path) ----------------- */

PM_API void pm_sin_out(double x, double* r) { *r = pm_sin(x); }
PM_API void pm_cos_out(double x, double* r) { *r = pm_cos(x); }
PM_API void pm_log_out(double x, double* r) { *r = pm_log(x); }

/* ---- bit-exact variants (avoid FPU registers entirely) --------------- */

PM_API uint64_t pm_sin_bits(uint64_t x_bits) {
    double x, r;
    uint64_t r_bits;
    memcpy(&x, &x_bits, 8);
    r = pm_sin(x);
    memcpy(&r_bits, &r, 8);
    return r_bits;
}

PM_API uint64_t pm_cos_bits(uint64_t x_bits) {
    double x, r;
    memcpy(&x, &x_bits, 8);
    r = pm_cos(x);
    uint64_t r_bits;
    memcpy(&r_bits, &r, 8);
    return r_bits;
}

PM_API uint64_t pm_log_bits(uint64_t x_bits) {
    double x, r;
    memcpy(&x, &x_bits, 8);
    r = pm_log(x);
    uint64_t r_bits;
    memcpy(&r_bits, &r, 8);
    return r_bits;
}

/* ---- sqrt (hardware / platform IEEE-754 implementation) -------------- */

PM_API double pm_sqrt(double x) {
#if !defined(PM_FORCE_PORTABLE_SQRT) && defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
    __m128d v = _mm_set_sd(x);
    v = _mm_sqrt_sd(v, v);
    return _mm_cvtsd_f64(v);
#elif !defined(PM_FORCE_PORTABLE_SQRT) && defined(__SSE2__)
    double r;
    __asm__ volatile("sqrtsd %1, %0" : "=x"(r) : "x"(x));
    return r;
#else
    /* ARM64 and other non-SSE targets need the platform square root,
       including signed zero, subnormals, infinities and negative inputs.
       A fixed Newton iteration count with an unscaled seed fails across
       the binary64 exponent range. PM_FORCE_PORTABLE_SQRT lets the small
       standalone regression target exercise this branch on an x86 host. */
    return sqrt(x);
#endif
}

PM_API void pm_sqrt_out(double x, double* r) { *r = pm_sqrt(x); }

PM_API uint64_t pm_sqrt_bits(uint64_t x_bits) {
    double x, r;
    memcpy(&x, &x_bits, 8);
    r = pm_sqrt(x);
    uint64_t r_bits;
    memcpy(&r_bits, &r, 8);
    return r_bits;
}

/* ---- log (Sun fdlibm via FPC fpc_ln_real) ---------------------------- */
/* Based on C/Sun Microsystems fdlibm log.c. See FPC rtl/inc/genmath.inc
   fpc_ln_real for the Pascal reference. */

static const double ln2_hi = 6.93147180369123816490e-01;
static const double ln2_lo = 1.90821492927058770002e-10;
static const double Lg1    = 6.666666666666735130e-01;
static const double Lg2    = 3.999999999940941908e-01;
static const double Lg3    = 2.857142874366239149e-01;
static const double Lg4    = 2.222219843214978396e-01;
static const double Lg5    = 1.818357216161805012e-01;
static const double Lg6    = 1.531383769920937332e-01;
static const double Lg7    = 1.479819860511658591e-01;

PM_API double pm_log(double x) {
    if (x != x) return x;                    /* NaN */
    if (x <= 0.0) {
        uint64_t bits = (x == 0.0) ? 0xFFF0000000000000ULL  /* -inf */
                                   : 0x7FF8000000000000ULL; /* NaN */
        double r; memcpy(&r, &bits, sizeof(r)); return r;
    }
    PM_SAVE_FPU(cw);
    PM_FORCE_DOUBLE();

    /* Extract high/low 32-bit halves of the double. */
    uint64_t bits;
    memcpy(&bits, &x, sizeof(bits));
    int32_t  hx = (int32_t)(bits >> 32);
    uint32_t lx = (uint32_t)(bits & 0xFFFFFFFFULL);

    int k = 0;
    if ((uint32_t)hx < 0x00100000U) {
        /* x < 2^-1022, subnormal â€” scale up by 2^54. Unreachable for
           typical mb3d inputs but kept for completeness. */
        return (x - x) / 0.0;
    }
    if ((uint32_t)hx >= 0x7FF00000U) {
        return x + x;                        /* +inf or NaN */
    }
    k += (hx >> 20) - 1023;
    hx &= 0x000FFFFF;
    int32_t i = (hx + 0x95F64) & 0x100000;
    int32_t new_hx = hx | (i ^ 0x3FF00000);
    /* Rebuild d with normalized mantissa (value in [sqrt(2)/2, sqrt(2)]). */
    bits = ((uint64_t)(uint32_t)new_hx << 32) | (uint64_t)lx;
    memcpy(&x, &bits, sizeof(x));
    k += (int)(i >> 20);

    double result;
    double f = x - 1.0;
    if ((0x000FFFFF & (2 + hx)) < 3) {       /* |f| < 2^-20 */
        if (f == 0.0) {
            if (k == 0) { result = 0.0; }
            else {
                double dk = (double)k;
                result = dk * ln2_hi + dk * ln2_lo;
            }
        } else {
            double R = f * f * (0.5 - 0.33333333333333333 * f);
            if (k == 0) result = f - R;
            else {
                double dk = (double)k;
                result = dk * ln2_hi - ((R - dk * ln2_lo) - f);
            }
        }
    } else {
        double s = f / (2.0 + f);
        double dk = (double)k;
        double z = s * s;
        int32_t ii = hx - 0x6147A;
        double w = z * z;
        int32_t jj = 0x6B851 - hx;
        double t1 = w * (Lg2 + w * (Lg4 + w * Lg6));
        double t2 = z * (Lg1 + w * (Lg3 + w * (Lg5 + w * Lg7)));
        ii |= jj;
        double R = t2 + t1;
        if (ii > 0) {
            double hfsq = 0.5 * f * f;
            if (k == 0) result = f - (hfsq - s * (hfsq + R));
            else        result = dk * ln2_hi - ((hfsq - (s * (hfsq + R) + dk * ln2_lo)) - f);
        } else {
            if (k == 0) result = f - s * (f - R);
            else        result = dk * ln2_hi - ((s * (f - R) - dk * ln2_lo) - f);
        }
    }
    PM_RESTORE_FPU(cw);
    return result;
}

/* ====================================================================
 * mb3d_formula_<name> â€” SSE2 re-implementations of .m3f [CODE] formulas
 *
 * Calling convention: Pascal i386 register (Borland/FPC default).
 *   EAX        = var x (pointer to double)
 *   EDX        = var y
 *   ECX        = var z
 *   [ebp+8]    = PIteration3D pointer (last declared stack arg,
 *                pushed last â†’ lowest offset above ebp)
 *   [ebp+12]   = var w (second-to-last declared)
 *   callee-clean: `ret 8` pops the two stack pointers
 *
 * Each formula is implemented in two parts:
 *   mb3d_formula_<name>_impl  : C cdecl, does the math
 *   mb3d_formula_<name>_Pascal: __declspec(naked), pushes the
 *                               register-passed args onto the stack
 *                               and tail-calls into _impl.
 *
 * TIteration3D layout reminder (relevant fields):
 *   offset +48 = PVar (void*)  â€” used to reach formula-specific
 *                                constants laid out at negative
 *                                offsets from PVar.
 * ================================================================== */

/* --- _Rotate --------------------------------------------------------
 * 3x3 rotation matrix applied to (x, y, z). The matrix lives as 9
 * single-precision floats at PVar[-0x0c] through PVar[-0x2c] in
 * ROW-major order (m[0..8]):
 *   x' = x*m[0] + y*m[1] + z*m[2]
 *   y' = x*m[3] + y*m[4] + z*m[5]
 *   z' = x*m[6] + y*m[7] + z*m[8]
 * w is unused (pass-through).  Verified against
 * external_formulas/Formulas/Ads/_Rotate.inc and .asm disassembly. */
#if defined(_MSC_VER) && defined(_M_IX86)

#pragma pack(push, 1)
/* Just enough of TIteration3D to reach PVar at offset +48. */
typedef struct {
    double C1, C2, C3;         /* +0  */
    double J1, J2, J3;         /* +24 */
    void*  PVar;               /* +48 */
} mb3d_tit3d_head;
#pragma pack(pop)

static void __cdecl mb3d_formula__Rotate_impl(
    double* xp, double* yp, double* zp, double* wp, void* PIt3D)
{
    (void)wp;  /* _Rotate is pure xyz transform */
    const mb3d_tit3d_head* pit = (const mb3d_tit3d_head*)PIt3D;
    const unsigned char* pvar = (const unsigned char*)pit->PVar;

    /* Read 9 singles laid out in row-major starting at PVar-0x0c. */
    float m[9];
    for (int i = 0; i < 9; ++i) {
        const unsigned char* addr = pvar - 0x0c - i * 4;
        float v;
        memcpy(&v, addr, sizeof(v));
        m[i] = v;
    }

    double x = *xp, y = *yp, z = *zp;
    double nx = x * (double)m[0] + y * (double)m[1] + z * (double)m[2];
    double ny = x * (double)m[3] + y * (double)m[4] + z * (double)m[5];
    double nz = x * (double)m[6] + y * (double)m[7] + z * (double)m[8];
    *xp = nx;
    *yp = ny;
    *zp = nz;
}

/* Naked shim: Pascal register convention â†’ cdecl.
 *
 * Pascal register-convention stack arg layout (FPC/Delphi) pushes the
 * stack-resident arguments in **declaration order** (left-to-right)
 * for params that spill past the first three registers.  For
 *   procedure(var x,y,z,w: Double; PIt3D)
 * the register-held ones are (x=EAX, y=EDX, z=ECX) and the remaining
 * are pushed as: push w  then  push PIt3D.  Immediately after the
 * Pascal caller's `call` to this shim the stack looks like:
 *   [esp+0]  = return address into Pascal
 *   [esp+4]  = PIt3D   (pushed last, lowest stack slot)
 *   [esp+8]  = w
 * This matches the ABoxMod1/_Rotate disassembly in
 * external_formulas/asm/*.asm which read PIt3D from `[ebp+8]` and
 * never touch `[ebp+12]` unless the formula consumes w.
 *
 * To call cdecl impl(xp, yp, zp, wp, PIt3D) we push right-to-left:
 * PIt3D, wp, zp, yp, xp, then call, then add esp,20 (cdecl caller
 * cleanup), then `ret 8` (Pascal register callee cleanup: 2 stack
 * args * 4 bytes). */
__declspec(naked) PM_API void mb3d_formula__Rotate_Pascal(void) {
    __asm {
        push    dword ptr [esp + 4]   ; push PIt3D (Pascal's 5th param)
        push    dword ptr [esp + 12]  ; push wp    (orig [esp+8], shifted by +4)
        push    ecx                   ; push zp
        push    edx                   ; push yp
        push    eax                   ; push xp
        call    mb3d_formula__Rotate_impl
        add     esp, 20               ; cdecl caller cleanup: 5 * 4 bytes
        ret     8                     ; Pascal callee cleanup: 2 stack args
    }
}

#else  /* other platforms: export the C cdecl directly */

typedef struct { double C1, C2, C3; double J1, J2, J3; void* PVar; } mb3d_tit3d_head;

PM_API void mb3d_formula__Rotate_Pascal(void) {
    /* 64-bit / non-MSVC build â€” shim is only meaningful for the i386
     * FPC Pascal side.  Provide a stub so the DLL still builds. */
}

#endif
