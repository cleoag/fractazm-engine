// _sphereinv.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _sphereinv
// Hand-ported from ASM: 146 bytes, DEoption=1

constexpr unsigned int FORMULA__SPHEREINV = 7092;

// ASM trace:
//   x' = x + ShiftX,  y' = y + ShiftY,  z' = z + ShiftZ
//   r_scaled = (x'² + y'² + z'²) * RadiusScale
//   if Mode != 0 and r_scaled >= 1.0: return (x,y,z) unchanged
//   if r_scaled > MinRadius:
//       factor = 1 / r_scaled
//   elif Mode == 2:
//       factor = 1 / (2 * MinRadius - r_scaled)    (reflect)
//   else:
//       factor = FixedFactor                        (replacement)
//   new_x = x' * factor - ShiftX
//   new_y = y' * factor - ShiftY
//   new_z = z' * factor - ShiftZ
//
// Sphere inversion transform with shift, scale, and inner-radius handling.
// Mode 0: no skip guard; Mode 1: skip if outside unit sphere; Mode 2: reflect inner.
// DEoption=1 (sphere inversion type).

template<>
cpu_double3 execute_formula<FORMULA__SPHEREINV>(cpu_double3 p, constant double* params, double &de) {
    const double ShiftX      = params[0];  // x shift (default: 0)
    const double ShiftY      = params[1];  // y shift (default: 0)
    const double ShiftZ      = params[2];  // z shift (default: 0)
    const double RadiusScale = params[3];  // 1/Radius from .DRecipro (default: 1)
    const double MinRadius   = params[4];  // .DReci2 Rad2 first slot (default: -1)
    const double FixedFactor = params[5];  // .DReci2 Rad2 second slot = 1/Rad2 (default: -1)
    const int   Mode        = int(params[6]);  // 0=normal, 1=skip if r>=1, 2=reflect (default: 1)

    double x = p.x, y = p.y, z = p.z;

    // Shift coordinates
    double xs = x + ShiftX;
    double ys = y + ShiftY;
    double zs = z + ShiftZ;

    // Compute scaled radius
    double r_scaled = (xs * xs + ys * ys + zs * zs) * RadiusScale;

    // ASM returns unchanged when Mode != 0 and the scaled radius is outside
    // or exactly on the unit sphere.
    if (Mode != 0 && r_scaled >= 1.0) {
        return cpu_double3(x, y, z);
    }

    // ASM is fallback-first around the inner radius: when r_scaled <= Rad2 it
    // takes either the fixed reciprocal or the reflected reciprocal path.
    double factor;
    if (r_scaled <= MinRadius) {
        if (Mode == 2) {
            factor = 1.0 / (2.0 * MinRadius - r_scaled);
        } else {
            factor = FixedFactor;
        }
    } else {
        factor = 1.0 / r_scaled;
    }

    // Apply inversion and unshift
    double new_x = xs * factor - ShiftX;
    double new_y = ys * factor - ShiftY;
    double new_z = zs * factor - ShiftZ;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__sphereinv = {
    .id = FORMULA__SPHEREINV,
    .name = "_sphereinv",
    .param_count = 7,
    .complexity = 146,
    .uses_transcendental = false
};

/*
Original M3 Description:
Sphere inversion transform with configurable inner-radius handling.

x' = x + ShiftX, y' = y + ShiftY, z' = z + ShiftZ
r_scaled = (x'² + y'² + z'²) * RadiusScale

Mode handling:
  Mode 0: No guard. Always applies inversion.
  Mode 1: If r_scaled >= 1.0, return unchanged (skip large radii).
  Mode 2: Same skip as mode 1, plus reflect inner radius.

Inversion:
  r_scaled > MinRadius: factor = 1/r_scaled (standard)
  r_scaled <= MinRadius, mode 2: factor = 1/(2*MinRadius - r_scaled) (reflect)
  r_scaled <= MinRadius, other: factor = FixedFactor (constant replacement)

Output: (x'*factor - ShiftX, y'*factor - ShiftY, z'*factor - ShiftZ)

Parameters:
  [0] ShiftX: double (default: 0)
  [1] ShiftY: double (default: 0)
  [2] ShiftZ: double (default: 0)
  [3] RadiusScale: double (default: 1)
  [4] MinRadius: double (threshold for inner handling)
  [5] FixedFactor: double (replacement factor)
  [6] Mode: integer (0/1/2)
*/
