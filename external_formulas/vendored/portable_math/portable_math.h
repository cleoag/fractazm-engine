/* portable_math.h — deterministic sin/cos/log/sqrt for cross-language parity.
 *
 * Goal: bit-identical output on both MSVC x64 (used by the C++ port) and
 * MSVC i386 / MinGW i386 (loaded as DLL by the FPC Pascal reference).
 *
 * sin/cos/log avoid platform libm argument reduction and use C double
 * polynomial evaluation. sqrt uses SSE2 on x86 and the platform sqrt on
 * other targets. Validate output bits and floating-point state on each
 * target; this interface alone does not certify cross-platform parity.
 * Compile with /fp:strict (MSVC) or, for x86 GCC/MinGW,
 * -ffloat-store -msse2 -mfpmath=sse.
 *
 * See docs/PARITY_TRACE.md for validation rules and current limits.
 */
#ifndef MB3D_VENDORED_PORTABLE_MATH_H
#define MB3D_VENDORED_PORTABLE_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) && defined(MB3D_PORTABLE_MATH_DLL)
#  define PM_API __declspec(dllexport)
#elif defined(_WIN32) && defined(MB3D_PORTABLE_MATH_USE_DLL)
#  define PM_API __declspec(dllimport)
#else
#  define PM_API
#endif

PM_API double pm_sin(double x);
PM_API double pm_cos(double x);
PM_API double pm_sqrt(double x);
PM_API double pm_log(double x);

/* Out-parameter variants to bypass the Win32 x86 __cdecl double-return
   convention, which transfers doubles through the x87 ST(0) register
   and can perturb the low bit on FPC callers. Cross-platform callers
   (Pascal via cdecl DLL, C++ via direct link) get identical bits when
   they use these. */
PM_API void   pm_sin_out(double x, double* r);
PM_API void   pm_cos_out(double x, double* r);
PM_API void   pm_sqrt_out(double x, double* r);
PM_API void   pm_log_out(double x, double* r);

/* Bit-exact variants that avoid FPU registers entirely — inputs and
   outputs move through integer ALU as uint64 bit patterns. Use these
   from FPC which otherwise rounds 1 ULP on ST(0) double returns even
   when the DLL was compiled with /arch:SSE2 /fp:strict. Callers
   memcpy/Move between double <-> uint64 on both ends. */
#include <stdint.h>
PM_API uint64_t pm_sin_bits(uint64_t x_bits);
PM_API uint64_t pm_cos_bits(uint64_t x_bits);
PM_API uint64_t pm_sqrt_bits(uint64_t x_bits);
PM_API uint64_t pm_log_bits(uint64_t x_bits);

/* ====================================================================
 * SSE2 re-implementations of .m3f [CODE] formulas.
 *
 * Pascal's .m3f files ship pre-compiled i386 x87 machine code in
 * [CODE]. That bytecode runs in 80-bit extended precision and produces
 * results that differ by 1+ ULP from the C++ port's SSE2 execution
 * path for the same formula. At image level this drifts 4-37% of
 * pixels on affected scenes even after portable sin/cos/log/sqrt.
 *
 * These exports provide SSE2 re-implementations of the same formulas.
 * They are exported with **Pascal i386 register calling convention**
 * (EAX=var x, EDX=var y, ECX=var z; stack: [ebp+12]=var w,
 * [ebp+8]=PIteration3D; callee cleans 8 bytes on return).  That lets
 * Pascal plug a DLL function pointer directly into its existing
 * ThybridIteration dispatch without any shim code.
 * ================================================================== */

PM_API void mb3d_formula__Rotate_Pascal(void);  /* see note above */

#ifdef __cplusplus
}
#endif

#endif /* MB3D_VENDORED_PORTABLE_MATH_H */
