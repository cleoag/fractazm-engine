#include "iq_original_arm64.h"
// IQ_NormBulb_NoYZ.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: IQ_NormBulb_NoYZ
// Hand-ported from ASM: 170 bytes, DEoption=0

constexpr unsigned int FORMULA_IQ_NORMBULB_NOYZ = 7086;

// ASM trace:
//   r = sqrt(x² + y² + z²)
//   r_P = pow(|r|, Power)
//   theta = atan2(sqrt(y²+z²), x)        (x-axis as pole)
//   phi = atan2(y, z)                     (azimuthal in yz-plane)
//   new_x = r_P * sin(Power * theta) + Cx
//   new_y = r_P * cos(Power * theta) * sin(zAnglePow * phi)
//   new_z = r_P * cos(Power * theta) * cos(zAnglePow * phi) * zMul
//
// "Normal Bulb" variant with x-axis as pole.
// ONLY Cx is added; y and z get NO C addition.
// This is the "NoYZ" variant of IQ-bulb.
//
// C handling: NON-STANDARD — only Cx added, not Cy/Cz.

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void IQ_NormBulb_NoYZOriginal_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_IQ_NORMBULB_NOYZ>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[10] = {p.x,p.y,p.z};
    IQ_NormBulb_NoYZOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    return arm64IQ(p,params,Arm64IQKind::Normal,false);
#else
    // Portable fallback remains uncertified.
    const double Power     = params[0];  // default: 8
    const double zAnglePow = params[1];  // default: 8
    const double zMul      = params[2];  // default: 1
    // ABI layout matches IQ_PineTree_NoYZ:
    //   params[0..4]  = mapped PVar values
    //   params[5]     = [CONSTANTS] Double = 1
    //   params[6..9]  = appended C values (J1..J4)
    // Only Cx is consumed by the formula body.
    const double Cx        = params[6];

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

    // Spherical to Cartesian (x-axis pole, NO Cy/Cz)
    double new_x = r_P * sin_Pt + Cx;               // only Cx added!
    double new_y = r_P * cos_Pt * sin_Zp;            // no Cy
    double new_z = r_P * cos_Pt * cos_Zp * zMul;     // no Cz

    return cpu_double3(new_x, new_y, new_z);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_IQ_NormBulb_NoYZ = {
    .id = FORMULA_IQ_NORMBULB_NOYZ,
    .name = "IQ_NormBulb_NoYZ",
    .param_count = 4,
    .complexity = 170,
    .uses_transcendental = true
};

/*
Original M3 Description:
IQ Normal Bulb formula with x-axis as pole and no C on y/z.

Uses spherical coordinates with x as the pole axis:
r = sqrt(x² + y² + z²)
r_P = r^Power
theta = atan2(sqrt(y²+z²), x)
phi = atan2(y, z)

new_x = r_P * sin(Power*theta) + Cx
new_y = r_P * cos(Power*theta) * sin(zAnglePow*phi)
new_z = r_P * cos(Power*theta) * cos(zAnglePow*phi) * zMul

Key difference from IQ-bulb:
1. x-axis is the pole (not y-axis)
2. Only Cx is added; Cy and Cz are NOT added
3. sin/cos roles reversed for x vs y due to different pole

C handling: NON-STANDARD (only Cx).
Engine should NOT add C to this formula; template handles Cx.

Parameters:
  [0] Power: double (default: 8)
  [1] z-AnglePow: double (default: 8)
  [2] z-Mul: double (default: 1)
  [3] Cx: double (from engine C)
*/
