#include "iq_original_arm64.h"
// Numerical reference: immutable original M3F CODE.
constexpr unsigned int FORMULA_IQ_PINETREE_NOYZ = 7089;


#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void IQ_PineTree_NoYZOriginal_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_IQ_PINETREE_NOYZ>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[10] = {p.x,p.y,p.z};
    IQ_PineTree_NoYZOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    return arm64IQ(p,params,Arm64IQKind::Pine,false);
#else
    // Portable fallback remains uncertified.
    const double Power     = params[0];  // default: 8
    const double zAnglePow = params[1];  // default: 8
    const double zMul      = params[2];  // default: 1
    const double Cx        = params[6];

    double x = p.x, y = p.y, z = p.z;

    if (x == 0.0 && y == 0.0 && z == 0.0) {
        return cpu_double3(NAN, NAN, NAN);
    }

    double r = sqrt(x * x + y * y + z * z);

    double r_P = pow(abs(r), Power);

    double rho_yz = sqrt(y * y + z * z);
    double theta = atan2(rho_yz, x);

    double phi = atan2(y, z);

    double sin_Pt = sin(Power * theta);
    double cos_Pt = cos(Power * theta);
    double sin_Zp = sin(zAnglePow * phi);
    double cos_Zp = cos(zAnglePow * phi);

    double new_x = r_P * cos_Pt + Cx;               // only Cx added!
    double new_y = r_P * sin_Pt * cos_Zp;            // no Cy
    double new_z = r_P * sin_Pt * sin_Zp * zMul;     // no Cz

    return cpu_double3(new_x, new_y, new_z);
#endif
}

constant FormulaInfo formula_info_IQ_PineTree_NoYZ = {
    .id = FORMULA_IQ_PINETREE_NOYZ,
    .name = "IQ_PineTree_NoYZ",
    .param_count = 4,
    .complexity = 170,
    .uses_transcendental = true
};

