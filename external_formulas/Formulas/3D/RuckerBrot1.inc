// Preserve original x87 branches, arithmetic and Double stores.
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void RuckerBrot1Original_x87(double*, const double*);
#endif
constexpr unsigned int FORMULA_RUCKERBROT1 = 8081;


template<>
cpu_double3 execute_formula<FORMULA_RUCKERBROT1>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z,de};
    RuckerBrot1Original_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#else
    // Portable numerical parity remains unverified.

    const double XMul  = params[0];  // (default: 1)
    const double YMul  = params[1];  // (default: 2)
    const double ZMul  = params[2];  // (default: -2)
    const double CzMul = params[3];  // (default: -1)
    const double eps   = params[4];  // (default: 1e-10)

    double x = p.x, y = p.y, z = p.z;

    const double Cx = params[5];
    const double Cy = params[6];
    const double Cz = params[7];

    double x2 = x * x;
    double y2 = y * y;
    double z2 = z * z;
    double r2 = x2 + (y2 + z2);

    double T = r2 / (sqrt(x2 + z2) + eps);

    double U = (x2 - y2) / (x2 + y2 + eps);

    double new_x = (U * T) * fabs(x) * XMul + Cx;

    double V = x * y * fabs(x) / (x2 + y2 + eps);
    double new_y = V * T * YMul + Cy;

    double new_z = z * sqrt(r2) * ZMul + fabs(Cz) * CzMul;

    return cpu_double3(new_x, new_y, new_z);
#endif
}

constant FormulaInfo formula_info_RuckerBrot1 = {
    .id = FORMULA_RUCKERBROT1,
    .name = "RuckerBrot1",
    .param_count = 8,
    .complexity = 190,
    .uses_transcendental = true
};
