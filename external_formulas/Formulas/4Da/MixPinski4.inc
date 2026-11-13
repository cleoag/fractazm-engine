// MixPinski4: port of the shipped x86 formula in mb3d/M3Formulas/MixPinski4.m3f.
// DEoption=6: de is W; Deriv1 is multiplied by the signed scale.

constexpr unsigned int FORMULA_MIXPINSKI4 = 10343;
template<> struct ExternalFormulaUsesXYZState<FORMULA_MIXPINSKI4> { static constexpr bool value = true; };

template<>
cpu_double3 execute_formula<FORMULA_MIXPINSKI4>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    // Original x87 multiplies Deriv1 by the signed scale, without FABS.
    params[FORMULA_STATE_X_SLOT] *= Scale;
    const double CX = params[1];
    const double CY = params[2];
    const double CZ = params[3];
    const double CW = params[4];

    // Expanded .6SingleAngles matrix occupies params[5..20].
    double x = p.x, y = p.y, z = p.z, w = de;

    // Sierpinski-type fold (conditional swap with negation)
    if (x + y < 0.0) { double t = x; x = -y; y = -t; }
    if (x + z < 0.0) { double t = x; x = -z; z = -t; }
    if (y + z < 0.0) { double t = y; y = -z; z = -t; }
    if (x + w < 0.0) { double t = x; x = -w; w = -t; }
    if (y + w < 0.0) { double t = y; y = -w; w = -t; }
    if (z + w < 0.0) { double t = z; z = -w; w = -t; }

    // Matrix rows follow the original x87 stack order.
    double r0 = x*params[5]  + y*params[6]  + z*params[7]  + w*params[8];
    double r1 = x*params[9]  + y*params[10] + z*params[11] + w*params[12];
    double r2 = x*params[13] + y*params[14] + z*params[15] + w*params[16];
    double r3 = x*params[17] + y*params[18] + z*params[19] + w*params[20];

    double k = Scale - 1.0;

    // w output
    double w_out = Scale * r3 - k * CW;
    de = w_out;

    // The shared Menger postfix subtracts CX from scaled row1 and CY
    // from scaled row0, then writes row0 to Y and row1 to X.
    double x_out = Scale * r1 - k * CX;
    double y_out = Scale * r0 - k * CY;

    // z output: Menger cross fold
    double kz = (k / Scale) * CZ;
    double z_out = (kz - abs(r2 - kz)) * Scale;

    return cpu_double3(x_out, y_out, z_out);
}

constant FormulaInfo formula_info_MixPinski4 = {
    .id = FORMULA_MIXPINSKI4,
    .name = "MixPinski4",
    .param_count = 11,
    .complexity = 331,
    .uses_transcendental = false
};
