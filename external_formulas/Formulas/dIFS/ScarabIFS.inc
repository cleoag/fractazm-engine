#include "ifs_original_state.h"
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ScarabIFSOriginal_x87(double*, const void*);
#endif

// ScarabIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: ScarabIFS
// Hand-ported from ASM: 191 bytes, Capstone-verified
// DEoption: 20 (dIFS shape — SSE2 IFS transform + x87 scarab sextic DE)
// Revolution surface of the Scarabaeus sextic curve

constexpr unsigned int FORMULA_SCARABIFS = 3371;

template<>
cpu_double3 execute_formula<FORMULA_SCARABIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    OriginalApplyIFSState native(p,params);

    ScarabIFSOriginal_x87(native.state,native.var());
    return native.finish(params,de);
#elif defined(MB3D_SOFTWARE_EXT80)
    Arm64ApplyIFSState native(p,params);
    using E=Arm64ApplyIFSState::E;
    const E x=native.transformed.x,y=native.transformed.y,z=native.transformed.z;
    const E xx=x*x,yz=y*y+z*z,radius=xx+yz,shifted=radius+E(params[0])*x;
    // FST keeps unrounded squares for radius, but the cross/normal tails reload Double.
    const E sx=double(xx),sy=double(yz),r2=abs(E(params[5]));
    const E surface=sqrt(sqrt(radius*(shifted*shifted)+(sx*sy)*(r2*r2)));
    const E normal=sqrt(sqrt(sx*sx+sy*sy)*r2);
    return native.finish(surface+(-normal),de);
#else
    // Portable numerical parity remains unverified.

    const double ScarabR1 = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double ScarabR2 = params[5];
    const int Apply_scale_add = int(params[6]);

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;    // w *= Scale
    }

    // Step 2: Squared terms
    // ASM: x², y²+z², r² = x²+y²+z²
    double x_sq = x * x;
    double yz_sq = y * y + z * z;
    double r_sq = x_sq + yz_sq;

    // Step 3: Sextic surface term (quartic root for DE scaling)
    // Surface: r²·(r²+R1·x)² + x²·(y²+z²)·R2²
    // ASM: r²*(r²+R1*x)² → double fsqrt (4th root)
    double surface_val = r_sq * (r_sq + ScarabR1 * x) * (r_sq + ScarabR1 * x);
    double R2_abs = abs(ScarabR2);
    double cross_val = x_sq * yz_sq * R2_abs * R2_abs;
    double surface_term = sqrt(sqrt(surface_val + cross_val));

    // Step 4: Gradient normalization term
    // ASM: sqrt(|R2| * sqrt(x⁴ + (y²+z²)²))
    double norm_term = sqrt(R2_abs * sqrt(x_sq * x_sq + yz_sq * yz_sq));

    // Step 5: Shape DE = surface_term - norm_term
    // ASM: fchs → faddp
    double shape_de = surface_term - norm_term;

    // Step 6: Scale correction
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
cpu_double3 ScarabIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_SCARABIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_ScarabIFS = {
    .id = FORMULA_SCARABIFS,
    .name = "ScarabIFS",
    .param_count = 7,
    .complexity = 400,
    .uses_transcendental = false
};

/*
Original M3 Description:
Revolution surface of the Scarabaeus sextic curve;
(x²+y²)(x²+y²+R1·x)² - R2²(x²-y²)² = 0
Looks more like a bell to me.
Source: Shikin, E.V. Handbook and Atlas of Curves

Luca GN 2012

Mathematical Formula:
  DEoption=20 (dIFS shape — SSE2 IFS + x87 scarab sextic DE):
    - Standard SSE2 IFS preamble
    - Revolution of 2D sextic curve (y → sqrt(y²+z²))
    - Quartic root of surface value for DE scaling
    - Gradient normalization via sqrt(|R2|*sqrt(norm))

  Step 1: IFS transform:
    x = p.x * Scale + X_add;  y = p.y * Scale + Y_add;  z = p.z * Scale + Z_add
    if Apply_scale_add: w *= Scale

  Step 2: Squared terms:
    x_sq = x², yz_sq = y²+z², r_sq = x²+y²+z²

  Step 3: Surface term (quartic root):
    surface_term = ⁴√(r²·(r²+R1·x)² + x²·(y²+z²)·R2²)

  Step 4: Gradient normalization:
    norm_term = √(|R2|·√(x⁴ + (y²+z²)²))

  Step 5: Shape DE:
    shape_de = surface_term - norm_term

Parameters:
  [0] ScarabR1: double (default: 0.5) — first shape parameter
  [1] Scale: double (default: 2) — IFS scale factor (.2Doubles)
  [2] Z_add: double (default: 0) — Z offset (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] ScarabR2: double (default: 1.5) — second shape parameter (thickness)
  [6] Apply_scale_add: int (default: 1)

PVar layout (verified against x87 ASM offsets):
  [PVar - 0x10] = ScarabR1 (double)
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add (double)
  [PVar - 0x40] = X_add (double)
  [PVar - 0x48] = ScarabR2 (double)
  [PVar - 0x4c] = Apply_scale_add (integer)

Code structure:
  0x00-0x40: Standard SSE2 IFS preamble
  0x40-0x45: Store to stack for x87
  0x4B-0x69: Compute x², y²+z², r²
  0x6B-0x7A: r²*(r²+R1*x)² — sextic surface term
  0x7C-0x90: + x²*(y²+z²)*R2² — cross term
  0x92-0x94: Double fsqrt → quartic root of surface
  0x96-0xA9: Normalization: sqrt(|R2|*sqrt(x⁴+(y²+z²)²))
  0xAB-0xAD: fchs + faddp → surface - norm
  0xAF-0xBE: Scale correction + store + return

Key notes:
  - Scarabaeus is a classical sextic curve from algebraic geometry
  - Revolution around x-axis: y → sqrt(y²+z²) in surface equation
  - Quartic root (double fsqrt) scales degree-6 implicit to distance-like units
  - |R2| used for normalization (fabs at 0x86), R2² for surface term
  - Stack allocation 0x26 (38 bytes) for extended intermediate storage
  - 191 bytes — moderate algebraic surface complexity
*/
