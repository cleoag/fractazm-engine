#include "iq_original_arm64.h"
// IQ_NormBulb.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: IQ_NormBulb
// Hand-ported from ASM: 176 bytes, DEoption=0

constexpr unsigned int FORMULA_IQ_NORMBULB = 7087;

// ASM trace:
//   r = sqrt(x² + y² + z²)
//   r_P = pow(|r|, Power)
//   theta = atan2(sqrt(y²+z²), x)        (x-axis as pole)
//   phi = atan2(y, z)                     (azimuthal in yz-plane)
//   new_x = r_P * sin(Power * theta) + Cx
//   new_y = r_P * cos(Power * theta) * sin(zAnglePow * phi) + Cy
//   new_z = r_P * cos(Power * theta) * cos(zAnglePow * phi) * zMul + Cz
//
// "Normal Bulb" variant with x-axis as pole.
// All three C components are added (standard C handling).
// Differs from IQ_NormBulb_NoYZ which only adds Cx.
//
// C addition is internal, before original Double stores.

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void IQ_NormBulbOriginal_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_IQ_NORMBULB>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[10] = {p.x,p.y,p.z};
    IQ_NormBulbOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    return arm64IQ(p,params,Arm64IQKind::Normal,true);
#else
    // Portable fallback remains uncertified.
    const double Power     = params[0];  // default: 8
    const double zAnglePow = params[1];  // default: 8
    const double zMul      = params[2];  // default: 1

    double x = p.x, y = p.y, z = p.z;

    if (x == 0.0 && y == 0.0 && z == 0.0) {
        return cpu_double3(NAN, NAN, NAN);
    }

    // Radius
    double r = sqrt(x * x + y * y + z * z);

    // r^Power
    double r_P = pow(abs(r), Power);

    // Polar angle from x-axis: theta = atan2(sqrt(y²+z²), x)
    double rho_yz = sqrt(y * y + z * z);
    double theta = atan2(rho_yz, x);

    // Azimuthal angle in yz-plane: phi = atan2(y, z)
    double phi = atan2(y, z);

    // Compute trig of scaled angles
    double sin_Pt = sin(Power * theta);
    double cos_Pt = cos(Power * theta);
    double sin_Zp = sin(zAnglePow * phi);
    double cos_Zp = cos(zAnglePow * phi);

    // Spherical to Cartesian (x-axis pole)
    double new_x = r_P * sin_Pt + params[6];
    double new_y = r_P * cos_Pt * sin_Zp + params[7];
    double new_z = r_P * cos_Pt * cos_Zp * zMul + params[8];

    return cpu_double3(new_x, new_y, new_z);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_IQ_NormBulb = {
    .id = FORMULA_IQ_NORMBULB,
    .name = "IQ_NormBulb",
    .param_count = 3,
    .complexity = 176,
    .uses_transcendental = true
};

/*
Original M3 Description:
IQ Normal Bulb formula with x-axis as pole and full C addition.

Uses spherical coordinates with x as the pole axis:
r = sqrt(x² + y² + z²)
r_P = r^Power
theta = atan2(sqrt(y²+z²), x)
phi = atan2(y, z)

new_x = r_P * sin(Power*theta)
new_y = r_P * cos(Power*theta) * sin(zAnglePow*phi)
new_z = r_P * cos(Power*theta) * cos(zAnglePow*phi) * zMul

Differs from IQ_NormBulb_NoYZ: all three C components added.
Same structure, different C handling.

C handling: internal original addition before rounding.

Parameters:
  [0] Power: double (default: 8)
  [1] z-AnglePow: double (default: 8)
  [2] z-Mul: double (default: 1)
*/
