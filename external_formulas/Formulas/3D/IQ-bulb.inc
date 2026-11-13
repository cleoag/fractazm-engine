#include "iq_original_arm64.h"
// Numerical reference: immutable original M3F CODE.
constexpr unsigned int FORMULA_IQBULB = 7080;


#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void IQ_bulbOriginal_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_IQBULB>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[10] = {p.x,p.y,p.z};
    IQ_bulbOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    return arm64IQ(p,params,Arm64IQKind::Bulb,true);
#else
    // Portable fallback remains uncertified.
    const double Power     = params[0];  // default: 8
    const double zAnglePow = params[1];  // default: 8
    const double zMul      = params[2];  // default: 1

    double x = p.x, y = p.y, z = p.z;

    if (x == 0.0 && y == 0.0 && z == 0.0) {
        return cpu_double3(NAN, NAN, NAN);
    }

    double r = sqrt(x * x + y * y + z * z);

    double r_P = pow(abs(r), Power);

    double rho_xz = sqrt(x * x + z * z);
    double theta = atan2(rho_xz, y);

    double phi = atan2(x, z);

    double sin_Pt = sin(Power * theta);
    double cos_Pt = cos(Power * theta);
    double sin_Zp = sin(zAnglePow * phi);
    double cos_Zp = cos(zAnglePow * phi);

    double new_x = r_P * sin_Pt * sin_Zp + params[6];
    double new_y = r_P * cos_Pt + params[7];
    double new_z = r_P * sin_Pt * cos_Zp * zMul + params[8];

    return cpu_double3(new_x, new_y, new_z);
#endif
}

constant FormulaInfo formula_info_IQbulb = {
    .id = FORMULA_IQBULB,
    .name = "IQ-bulb",
    .param_count = 3,
    .complexity = 176,
    .uses_transcendental = true
};

