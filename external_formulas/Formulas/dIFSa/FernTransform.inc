// FernTransform: literal affine branches from the shipped 223-byte x86 code.
// SingleAngle values have already been expanded to their stored Single pairs.
constexpr unsigned int FORMULA_FERNTRANSFORM = 10251;

template<>
cpu_double3 execute_formula<FORMULA_FERNTRANSFORM>(cpu_double3 p, constant double* params, double &de) {
    // PVar -0x0C..-0x44, consecutive Singles:
    // offset, scale, angle pair, Z-angle pair, branch1 scale/pair,
    // branch2 scale/pair, branch1 offset, branch2 offset, clip slope.
    double x = p.x, y = p.y, z = p.z;
    const double test = -(abs(y) * params[14] + x);
    if (test >= params[0]) {
        // 0x1D..0x61: translate, scale, XY rotation, XZ rotation.
        x = (x + params[0]) * params[1];
        y *= params[1];
        z *= params[1];
        params[DIFS_VARYSCALE_SLOT] *= params[1];
        const double rx = x * params[3] + y * params[2];
        const double ry = y * params[3] - x * params[2];
        x = rx * params[5] + z * params[4];
        z = z * params[5] - rx * params[4];
        y = ry;
    } else if (y >= 0.0) {
        // 0x6D..0x9B: the positive-Y arm uses branch2 parameters.
        x += params[13];
        const double rx = x * params[11] + y * params[10];
        const double ry = y * params[11] - x * params[10];
        x = rx * params[9];
        y = ry * params[9];
        z *= params[9];
        params[DIFS_VARYSCALE_SLOT] *= params[9];
    } else {
        // 0x9D..0xD2: negate Y before the branch1 transform. If FTST
        // remains unordered, the final stores retain the exchanged stack.
        y = -y;
        if (y >= 0.0) {
            x += params[12];
            const double rx = x * params[8] + y * params[7];
            const double ry = y * params[8] - x * params[7];
            x = rx * params[6];
            y = ry * params[6];
            z *= params[6];
            params[DIFS_VARYSCALE_SLOT] *= params[6];
        } else {
            const double t = x; x = y; y = t;
        }
    }
    return cpu_double3(x, y, z);
}

cpu_double3 FernTransform_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_FERNTRANSFORM>(p, params, de);
}

constant FormulaInfo formula_info_FernTransform = {
    .id = FORMULA_FERNTRANSFORM,
    .name = "FernTransform",
    .param_count = 11,
    .complexity = 223,
    .uses_transcendental = false
};
