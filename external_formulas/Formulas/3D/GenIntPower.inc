// GenIntPower: shipped SSE3 code (772 bytes), DEoption=0.
// Radius power uses the original signed branch chain and low-bit odd correction.
// The wrapper adds C; rotation arithmetic is shared with _IntPowRotateC.
constexpr unsigned int FORMULA_GENINTPOWER = 10300;

template<>
cpu_double3 execute_formula<FORMULA_GENINTPOWER>(cpu_double3 p, constant double* params, double &de) {
    const int power = int(params[0]);
    const double r2 = p.z*p.z + (p.x*p.x + p.y*p.y);
    const double r = sqrt(r2);
    double rp = r2;
    if (power >= 4) {
        rp *= rp;
        if (power >= 8) {
            rp *= rp;
            if (power >= 10) rp *= r2;
        } else if (power >= 6) rp *= r2;
    }
    if (power & 1) rp *= r;
    const double inv = 1.0 / r;
    double x = p.x*inv, y = p.y*inv, z = p.z*inv;
    complex_power_rotate(x,y,z,int(params[1]),int(params[2]),1.0);
    if (params[3] != 0.0) complex_power_rotate(x,y,z,int(params[4]),int(params[5]),1.0);
    if (params[6] != 0.0) complex_power_rotate(x,y,z,int(params[7]),int(params[8]),1.0);
    return cpu_double3(x*rp, y*rp, (z*rp)*params[9]);
}

constant FormulaInfo formula_info_GenIntPower = {
    .id = FORMULA_GENINTPOWER,
    .name = "GenIntPower",
    .param_count = 10,
    .complexity = 772,
    .uses_transcendental = false
};
