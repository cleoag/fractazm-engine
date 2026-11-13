// _BT3_Transform3.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _BT3_Transform3
// Hand-ported from ASM: 203 bytes, DEoption=-1
// NOTE: This is byte-identical to _BenesiT3 (alias)

constexpr unsigned int FORMULA__BT3_TRANSFORM3 = 7102;  // same ID as _BenesiT3

// See _BenesiT3.metal for full ASM trace.

template<>
cpu_double3 execute_formula<FORMULA__BT3_TRANSFORM3>(cpu_double3 p, constant double* params, double &de) {
    const double Scale  = params[0];
    const double Offset = params[1];

    const double c0 = 0.816496580927726;
    const double c1 = 0.577350269189626;
    const double c2 = 0.707106781186548;

    double x = p.x, y = p.y, z = p.z;

    double t = (x * c0 - z * c1) * c2;
    double y_s = y * c2;
    double R = x * c1 + z * c0;

    double A = abs(t + y_s);
    double B = abs(t - y_s);
    double C = abs(R);

    double x3 = abs(A + C - Offset) * Scale * c2;
    double y3 = abs(B + C - Offset) * Scale * c2;
    double z3 = abs(A + B - Offset) * Scale;

    double s = x3 + y3;   // x3,y3 already include *c2 from Phase 2
    double d = y3 - x3;
    double new_x = s * c0 + z3 * c1;
    double new_z = z3 * c0 - s * c1;
    double new_y = d;

    return cpu_double3(new_x, new_y, new_z);
}

constant FormulaInfo formula_info__BT3_Transform3 = {
    .id = FORMULA__BT3_TRANSFORM3,
    .name = "_BT3_Transform3",
    .param_count = 2,
    .complexity = 203,
    .uses_transcendental = false
};

/*
Alias for _BenesiT3. Byte-identical machine code.
See _BenesiT3.metal for full documentation.

Parameters:
  [0] Scale: double (default: 2.0)
  [1] Offset: double (default: 2.0)
*/
