// PseudoXDB.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: PseudoXDB
// Hand-ported from ASM: 379 bytes, DEscale=1, SIpower=2

constexpr unsigned int FORMULA_PSEUDOXDB = 7025;

// ASM trace (AbsZ + CosinePow2 variant):
//   DD02        fld [edx]           → y
//   DC0A        fmul [edx]          → y²
//   DD00        fld [eax]           → x
//   DC08        fmul [eax]          → x²   → ST: x², y²
//   D9C0        fld st(0)           → dup x²
//   D8C2        fadd st0,st2        → x²+y² (D8C2 = FADD st0,st(2))
//   DD01        fld [ecx]           → z
//   D9E1        fabs                → |z|
//   D9C0        fld st(0)           → dup |z|
//   D8C8        fmul st0,st0        → z²  (= |z|²)
//   D8E2        fsub st0,st2        → z² - (x²+y²) (D8E2 = FSUB st0,st(2))
//   DC4FF0      fmul [edi-0x10]     → * Zmul
//   DC4628      fadd [esi+0x28]     → + Cz
//   DD19        fstp [ecx]          → z' = (z²-x²-y²)*Zmul + Cz
//   D8C0        fadd st0,st0        → 2|z|  (D8C0 = FADD st0,st0)
//   D9C9/D9FA   fxch, fsqrt         → R = sqrt(x²+y²)
//   DEF9        fdivp st1,st0       → K = 2|z|/R
//   D9C0        fld st(0)           → dup K
//   DC08        fmul [eax]          → K*x
//   DC0A        fmul [edx]          → K*x*y
//   D8C0        fadd st0,st0        → 2Kxy
//   DC4620      fadd [esi+0x20]     → + Cy
//   DD1A        fstp [edx]          → y' = 4|z|xy/R + Cy
//   D9C9        fxch st1
//   DEE2        fsubrp st2,st0      → x²-y²
//   DEC9        fmulp st1,st0       → K*(x²-y²)
//   DC4618      fadd [esi+0x18]     → + Cx
//   DD18        fstp [eax]          → x' = 2|z|(x²-y²)/R + Cx
//
// R = sqrt(x²+y²), K = 2*|z|/R
// x' = K*(x²-y²) + Cx = 2|z|(x²-y²)/R + Cx
// y' = 2K*x*y + Cy = 4|z|*x*y/R + Cy
// z' = (z²-x²-y²)*Zmul + Cz

template<>
cpu_double3 execute_formula<FORMULA_PSEUDOXDB>(cpu_double3 p, constant double* params, double &de) {
    const double Zmul = params[0];

    double x = p.x, y = p.y, z = p.z;
    double az = abs(z);
    double x2 = x * x;
    double y2 = y * y;
    double R = sqrt(x2 + y2);
    double K = 2.0 * az / R;

    double new_x = K * (x2 - y2);
    double new_y = 2.0 * K * x * y;
    double new_z = (az * az - x2 - y2) * Zmul;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_PseudoXDB = {
    .id = FORMULA_PSEUDOXDB,
    .name = "PseudoXDB",
    .param_count = 1,
    .complexity = 90,
    .uses_transcendental = false
};

/*
Original M3 Description:
AbsZ + CosinePow2 — a set very similar to XenoDream's variation.
Beautiful julia sets in some spots!

R = sqrt(x²+y²), K = 2|z|/R
x' = K(x²-y²) + Cx
y' = 2Kxy + Cy
z' = (z²-x²-y²)*Zmul + Cz

DEscale=1, SIpower=2.
Parameters:
  [0] Z multiplier: double (default: 1.0)
*/
