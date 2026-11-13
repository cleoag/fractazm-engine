// _BenesiUnSpheric.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _BenesiUnSpheric
// Hand-ported from ASM: 160 bytes, DEoption=-1

constexpr unsigned int FORMULA__BENESIUNSPHERIC = 1260;

// ASM trace (inverse of _BenesiSpheric):
//   rCyz = y²/z²
//   if rCyz < 1: rCyz = 1/sqrt(rCyz + 1) else rCyz = 1/sqrt(1/rCyz + 1)
//   new_y = y * rCyz, new_z = z * rCyz
//   rCxyz = (new_y² + new_z²) / x²
//   if rCxyz < 1: rCxyz = 1/sqrt(rCxyz + 1) else rCxyz = 1/sqrt(1/rCxyz + 1)
//   combined = rCxyz * Scale
//   x' = x * combined, y' = new_y * combined, z' = new_z * combined
//
// Warps a sphere to a cube (M.Benesi / Luca). Inverse of _BenesiSpheric.

template<>
cpu_double3 execute_formula<FORMULA__BENESIUNSPHERIC>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0]; // default: sqrt(2/3) ≈ 0.81649658

    double x = p.x, y = p.y, z = p.z;
    double y2 = y * y, z2 = z * z, x2 = x * x;

    // rCyz = y²/z² with conditional inversion, then reciprocal sqrt
    double rCyz_ratio = y2 / z2;
    double rCyz;
    if (rCyz_ratio < 1.0)
        rCyz = 1.0 / sqrt(rCyz_ratio + 1.0);
    else
        rCyz = 1.0 / sqrt(1.0 / rCyz_ratio + 1.0);

    // Apply rCyz to y and z first
    double new_y = y * rCyz;
    double new_z = z * rCyz;

    // rCxyz uses MODIFIED y,z (unlike _BenesiSpheric which uses originals)
    double rCxyz_ratio = (new_y * new_y + new_z * new_z) / x2;
    double rCxyz;
    if (rCxyz_ratio < 1.0)
        rCxyz = 1.0 / sqrt(rCxyz_ratio + 1.0);
    else
        rCxyz = 1.0 / sqrt(1.0 / rCxyz_ratio + 1.0);

    // Final scaling
    double combined = rCxyz * Scale;

    cpu_double3 result;
    result.x = x * combined;
    result.y = new_y * combined;
    result.z = new_z * combined;

    return result;
}

// Alternative direct function implementation
cpu_double3 _BenesiUnSpheric_formula(cpu_double3 p, constant double* params, double &de) {
    const double S = params[0];
    double x = p.x, y = p.y, z = p.z;
    double y2 = y*y, z2 = z*z, x2 = x*x;

    double r1 = y2/z2;
    double rCyz = (r1 < 1.0) ? rsqrt(r1+1.0) : rsqrt(1.0/r1+1.0);

    double ny = y * rCyz, nz = z * rCyz;
    double r2 = (ny*ny + nz*nz)/x2;
    double rCxyz = (r2 < 1.0) ? rsqrt(r2+1.0) : rsqrt(1.0/r2+1.0);

    double c = rCxyz * S;
    return cpu_double3(x * c, ny * c, nz * c);
}

// Formula metadata
constant FormulaInfo formula_info__BenesiUnSpheric = {
    .id = FORMULA__BENESIUNSPHERIC,
    .name = "_BenesiUnSpheric",
    .param_count = 1,
    .complexity = 160,
    .uses_transcendental = false
};

/*
Original M3 Description:
Warps a sphere to a cube; transform made by M.Benesi, optimized by Luca.
Inverse of "_BenesiSpheric".

Mathematical Formula:
rCyz = y²/z²
if rCyz < 1: rCyz = 1/sqrt(rCyz+1) else rCyz = 1/sqrt(1/rCyz+1)
y *= rCyz; z *= rCyz
rCxyz = (y²+z²)/x²  (using modified y,z)
if rCxyz < 1: rCxyz = 1/sqrt(rCxyz+1) else rCxyz = 1/sqrt(1/rCxyz+1)
x *= rCxyz*Scale; y *= rCxyz*Scale; z *= rCxyz*Scale

Parameters:
  [0] Scale: double (default: 0.81649658092773 = sqrt(2/3))
*/
