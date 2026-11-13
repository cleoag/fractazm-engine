// _BenesiSpheric.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _BenesiSpheric
// Hand-ported from ASM: 145 bytes, DEoption=-1

constexpr unsigned int FORMULA__BENESISPHERIC = 4906;

// ASM trace:
//   rCyz = y²/z²
//   if rCyz < 1: rCyz = sqrt(rCyz + 1) else rCyz = sqrt(1/rCyz + 1)
//   rCxyz = (y² + z²) / x²
//   if rCxyz < 1: rCxyz = sqrt(rCxyz + 1) else rCxyz = sqrt(1/rCxyz + 1)
//   factor_yz = rCyz * rCxyz * Scale
//   factor_x = rCxyz * Scale
//   x' = x * factor_x
//   y' = y * factor_yz
//   z' = z * factor_yz
//
// Warps a cube to a sphere (M.Benesi and Luca).
// Inverse of "_BenesiUnSpheric".

template<>
cpu_double3 execute_formula<FORMULA__BENESISPHERIC>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0]; // default: sqrt(2/3) ≈ 0.81649658

    double x = p.x, y = p.y, z = p.z;

    // Preserve the reference IEEE division at axes, including 0/0 NaN.
    double y2 = y * y;
    double z2 = z * z;
    double x2 = x * x;

    // rCyz = y²/z²
    double rCyz = y2 / z2;
    if (rCyz < 1.0)
        rCyz = sqrt(rCyz + 1.0);
    else
        rCyz = sqrt(1.0 / rCyz + 1.0);

    // rCxyz = (y² + z²) / x²
    double rCxyz = (y2 + z2) / x2;
    if (rCxyz < 1.0)
        rCxyz = sqrt(rCxyz + 1.0);
    else
        rCxyz = sqrt(1.0 / rCxyz + 1.0);

    // Apply factors
    double factor_x = rCxyz * Scale;
    double factor_yz = rCyz * rCxyz * Scale;

    cpu_double3 result;
    result.x = x * factor_x;
    result.y = y * factor_yz;
    result.z = z * factor_yz;

    return result;
}

// Alternative direct function implementation
cpu_double3 _BenesiSpheric_formula(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    double x = p.x, y = p.y, z = p.z;
    double y2 = y * y, z2 = z * z, x2 = x * x;

    double rCyz = y2 / z2;
    rCyz = (rCyz < 1.0) ? sqrt(rCyz + 1.0) : sqrt(1.0 / rCyz + 1.0);

    double rCxyz = (y2 + z2) / x2;
    rCxyz = (rCxyz < 1.0) ? sqrt(rCxyz + 1.0) : sqrt(1.0 / rCxyz + 1.0);

    double fx = rCxyz * Scale;
    double fyz = rCyz * rCxyz * Scale;
    return cpu_double3(x * fx, y * fyz, z * fyz);
}

// Formula metadata
constant FormulaInfo formula_info__BenesiSpheric = {
    .id = FORMULA__BENESISPHERIC,
    .name = "_BenesiSpheric",
    .param_count = 1,
    .complexity = 145,
    .uses_transcendental = false
};

/*
Original M3 Description:
Warps a cube to a sphere; transform made by M.Benesi and Luca.
Inverse of "_BenesiUnSpheric".

Mathematical Formula:
rCyz = y²/z²
rCxyz = (y²+z²)/x²
if rCyz < 1: rCyz = sqrt(rCyz+1) else rCyz = sqrt(1/rCyz+1)
if rCxyz < 1: rCxyz = sqrt(rCxyz+1) else rCxyz = sqrt(1/rCxyz+1)
y *= rCyz; z *= rCyz
x *= rCxyz*Scale; y *= rCxyz*Scale; z *= rCxyz*Scale

Parameters:
  [0] Scale: double (default: 0.81649658092773 = sqrt(2/3))
*/
