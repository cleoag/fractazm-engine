// _BT1_Transform1.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _BT1_Transform1
// Hand-ported from ASM: 184 bytes, DEoption=-1
// NOTE: This is byte-identical to _BenesiT1 (alias)

constexpr unsigned int FORMULA__BT1_TRANSFORM1 = 7097;  // same ID as _BenesiT1

// See _BenesiT1.metal for full ASM trace.
// _BT1_Transform1 is an alias with identical machine code.

template<>
cpu_double3 execute_formula<FORMULA__BT1_TRANSFORM1>(cpu_double3 p, constant double* params, double &de) {
    const double Scale  = params[0];  // scale factor (default: 2.0)
    const double Offset = params[1];  // x offset (default: 2.0)

    const double c0 = 0.816496580927726;  // sqrt(2/3)
    const double c1 = 0.577350269189626;  // sqrt(1/3)
    const double c2 = 0.707106781186548;  // sqrt(1/2)

    double x = p.x, y = p.y, z = p.z;

    // Phase 1: rotation + abs folding
    double t = (x * c0 - z * c1) * c2;
    double z1 = abs(x * c1 + z * c0);
    double x1 = abs(t - y * c2);
    double y1 = abs(t + y * c2);

    // Phase 2: second rotation
    double s = (x1 + y1) * c2;
    double d = (y1 - x1) * c2;
    double fx = s * c0 + z1 * c1;
    double fz = z1 * c0 - s * c1;
    double fy = d;

    // Phase 3: scale and offset
    double new_x = fx * Scale - Offset;
    double new_y = fy * Scale;
    double new_z = fz * Scale;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__BT1_Transform1 = {
    .id = FORMULA__BT1_TRANSFORM1,
    .name = "_BT1_Transform1",
    .param_count = 2,
    .complexity = 184,
    .uses_transcendental = false
};

/*
Original M3 Description:
Alias for _BenesiT1. Byte-identical machine code.
See _BenesiT1.metal for full documentation.

Parameters:
  [0] Scale: double (default: 2.0)
  [1] xOff: double (default: 2.0)
*/
