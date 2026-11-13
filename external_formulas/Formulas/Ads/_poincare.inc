// Preserve original x87 arithmetic order and Double stores.
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void PoincareOriginal_x87(double*, const double*);
#endif
constexpr unsigned int FORMULA__POINCARE = 7066;


template<>
cpu_double3 execute_formula<FORMULA__POINCARE>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z};
    PoincareOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#else
    // Portable numerical parity remains unverified.

    const double Xctr     = params[0];  // default: 0
    const double Yctr     = params[1];  // default: 0
    const double Zctr     = params[2];  // default: 0
    const double Radius   = params[3];  // default: 1
    const double P_Radius = params[4];  // default: 1

    double x = p.x + Xctr;
    double y = p.y + Yctr;
    double z = p.z + Zctr;

    double r2 = x*x + y*y + z*z;
    double rho = abs(P_Radius - r2);

    double new_x = (abs(x) / rho) * Radius - Xctr;
    double new_y = (abs(y) / rho) * Radius - Yctr;
    double new_z = (abs(z) / rho) * Radius - Zctr;  // same pattern as x and y

    return cpu_double3(new_x, new_y, new_z);
#endif
}

constant FormulaInfo formula_info__poincare = {
    .id = FORMULA__POINCARE,
    .name = "_poincare",
    .param_count = 5,
    .complexity = 111,
    .uses_transcendental = false
};
