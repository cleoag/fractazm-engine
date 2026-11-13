// Original x87 instruction/store order on Win64; portable fallback is not certified.
constexpr unsigned int FORMULA__KAMTORCUT = 634;

// ASM trace (352 bytes):
//   Same as _kamtor except:
//   - Uses Cutoff parameter instead of 1.0 for the |z'| threshold
//   - When |z'| >= Cutoff: sets x=Bailout, y=Bailout to force iteration escape
//     (instead of computing 1/z' reciprocal mapping)
//
//   z' = Soft1*z + Soft2*sqrt(x² + y²)
//   if |z'| >= Cutoff: x = Bailout, y = Bailout, return (force bailout)
//   if |z'| < Cutoff: t1 = z', t2 = sqrt(1 - z'²)
//
//   poly1/poly2 and rotation same as _kamtor.
//
// IMPORTANT: Cutoff must be > 0 and <= 1. Bailout should be < 100.
//
// C handling: DEoption=-1 (transform, no C addition)

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void KamtorcutOriginal_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA__KAMTORCUT>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z};
    KamtorcutOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#else
    const double Soft1   = params[0];   // [PVar-0x10] (default: 0.3)
    const double Soft2   = params[1];   // [PVar-0x18] (default: 0)
    const double A1      = params[2];   // [PVar-0x20] (default: 1)
    const double B1      = params[3];   // [PVar-0x28] (default: 0)
    const double C1      = params[4];   // [PVar-0x30] (default: 0)
    const double D1      = params[5];   // [PVar-0x38] (default: 0)
    const double E1      = params[6];   // [PVar-0x40] (default: 0)
    const double F1      = params[7];   // [PVar-0x48] (default: 0)
    const double A2      = params[8];   // [PVar-0x50] (default: 0)
    const double B2      = params[9];   // [PVar-0x58] (default: -1)
    const double C2      = params[10];  // [PVar-0x60] (default: 1)
    const double D2      = params[11];  // [PVar-0x68] (default: 0)
    const double E2      = params[12];  // [PVar-0x70] (default: 0)
    const double F2      = params[13];  // [PVar-0x78] (default: 0)
    // [CONSTANTS] embedded as compile-time values:
    const double Bailout = params[14];      // [PVar+0x00]
    const double Cutoff  = params[15];       // [PVar+0x08]

    double x = p.x, y = p.y, z = p.z;

    // Step 1: Update z (angle parameter)
    double r = sqrt(x * x + y * y);
    z = Soft1 * z + Soft2 * r;

    // x87 takes the bailout branch for out-of-range and NaN z-values here,
    // leaving z untouched but forcing x/y to Bailout.
    if (!isfinite(z) || fabs(z) >= Cutoff) {
        return cpu_double3(Bailout, Bailout, z);
    }

    // Step 3: Compute cos/sin analogs (|z| < Cutoff, so |z| < 1)
    double t1 = z;
    double t2 = sqrt(1.0 - z * z);

    // Step 4: Evaluate generalized polynomials (using ORIGINAL x, y)
    double poly1 = A1 * x + B1 * y + C1 * x * x + D1 * y * y
                + E1 * x * y + F1 * r;
    double poly2 = A2 * x + B2 * y + C2 * x * x + D2 * y * y
                + E2 * x * y + F2 * r;

    // Step 5: Rotation-like combination
    double new_x = poly1 * t1 + poly2 * t2;
    double new_y = poly1 * t2 + poly2 * t1;

    return cpu_double3(new_x, new_y, z);
#endif
}

// Formula metadata
constant FormulaInfo formula_info__kamtorcut = {
    .id = FORMULA__KAMTORCUT,
    .name = "_kamtorcut",
    .param_count = 14,
    .complexity = 352,
    .uses_transcendental = false
};

/*
Original M3 Description:
KAM torus system customizable "dynamic" variation in 3D with cutoff.
Same as _kamtor but less noisy and faster because when |z'| > cutoff
the iteration is discarded (forces bailout by setting x=Bailout, y=Bailout).
Formula by Luca GN 2011.

CUTOFF MUST BE GREATER THAN 0 AND LESS THAN OR EQUAL TO 1.
BAILOUT SHOULD BE LESS THAN 100.

Parameters:
  [0] Soft1: double (default: 0.3) - z feedback
  [1] Soft2: double (default: 0) - radial feedback to z
  [2..7] A1..F1: polynomial coefficients for poly1 (defaults: 1,0,0,0,0,0)
  [8..13] A2..F2: polynomial coefficients for poly2 (defaults: 0,-1,1,0,0,0)
  [14] Bailout: double (default: 100) - value to force bailout
  [15] Cutoff: double (default: 0.85) - |z| threshold for cutoff
*/
