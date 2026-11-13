// _BT2_Transform2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _BT2_Transform2
// Hand-ported from ASM: 209 bytes, DEoption=-1
// NOTE: This is byte-identical to _BenesiT2 (alias)

constexpr unsigned int FORMULA__BT2_TRANSFORM2 = 7101;  // same ID as _BenesiT2

// See _BenesiT2.metal for full ASM trace.

template<>
cpu_double3 execute_formula<FORMULA__BT2_TRANSFORM2>(cpu_double3 p, constant double* params, double &de) {
    const double Scale  = params[0];
    const double Offset = params[1];

    const double c0 = 0.816496580927726;
    const double c1 = 0.577350269189626;
    const double c2 = 0.707106781186548;

    double x = p.x, y = p.y, z = p.z;

    double t = (x * c0 - z * c1) * c2;
    double y_s = y * c2;
    double R = x * c1 + z * c0;

    double A = (t + y_s) * (t + y_s);
    double B = (t - y_s) * (t - y_s);
    double C = R * R;

    double x2 = abs(sqrt(A + C) - Offset) * Scale * c2;
    double y2 = abs(sqrt(B + C) - Offset) * Scale * c2;
    double z2 = abs(sqrt(A + B) - Offset) * Scale;

    double s = x2 + y2;   // x2,y2 already include *c2 from Phase 2
    double d = y2 - x2;
    double new_x = s * c0 + z2 * c1;
    double new_z = z2 * c0 - s * c1;
    double new_y = d;

    return cpu_double3(new_x, new_y, new_z);
}

constant FormulaInfo formula_info__BT2_Transform2 = {
    .id = FORMULA__BT2_TRANSFORM2,
    .name = "_BT2_Transform2",
    .param_count = 2,
    .complexity = 209,
    .uses_transcendental = false
};

/*
Alias for _BenesiT2. Byte-identical machine code.
See _BenesiT2.metal for full documentation.

Parameters:
  [0] Scale: double (default: 2.0)
  [1] Offset: double (default: 2.0)
*/
