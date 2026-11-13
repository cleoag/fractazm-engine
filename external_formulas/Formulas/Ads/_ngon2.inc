// _ngon2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _ngon2
// Hand-ported from ASM: 142 bytes, DEoption=-1

constexpr unsigned int FORMULA__NGON2 = 7082;

// ASM trace:
//   x' = x + Shift_x,  y' = y + Shift_y
//   angle = atan2(y', x')
//   combined = angle + (Skew_deg + Shift_deg) * DEG2RAD
//   sector_value = combined / TWOPI * Order
//   sector = round(sector_value)   (fistp/fild)
//   rot = -sector / Order * TWOPI + Shift_deg * DEG2RAD
//   (c, s) = (cos(rot), sin(rot))
//   rot_x = y'*c + x'*s           (standard rot_y component!)
//   if sector is even: rot_x = -rot_x
//   new_y = sqrt(x'² + y'²) / Order * TWOPI - Shift_y
//   new_x = rot_x - Shift_x
//   z unchanged
//
// Polygon fold variant with radial y output. Differs from _ngon:
// 1. x output uses y'*cos+x'*sin (not y'*sin-x'*cos)
// 2. Even-sector negation on x (not y)
// 3. y output is radial distance (not angular fraction)

template<>
cpu_double3 execute_formula<FORMULA__NGON2>(cpu_double3 p, constant double* params, double &de) {
    const double Order     = params[0];  // default: 4
    const double Shift_deg = params[1];  // default: 0
    const double Shift_x   = params[2];  // default: 0
    const double Shift_y   = params[3];  // default: 0
    const double Skew_deg  = params[4];  // default: 0

    // Constants (from [CONSTANTS] section of m3)
    const double TWOPI   = 6.283185307;
    const double DEG2RAD = 0.017453293;

    // Shift coordinates
    double xs = p.x + Shift_x;
    double ys = p.y + Shift_y;

    // Compute angle and snap to polygon sector
    double angle = atan2(ys, xs);
    double combined = angle + (Skew_deg + Shift_deg) * DEG2RAD;
    double sector_value = combined / TWOPI * Order;
    double sector = rint(sector_value);
    int sector_i = int(sector);

    // Compute rotation angle
    double rot = -sector / Order * TWOPI + Shift_deg * DEG2RAD;

    // Rotate: y'*cos + x'*sin (standard rot_y, NOT negated rot_x)
    double c = cos(rot);
    double s = sin(rot);
    double rot_x = ys * c + xs * s;

    // Even-sector negation on x
    if ((sector_i & 1) == 0) {
        rot_x = -rot_x;
    }

    // y output: radial distance (not angular fraction)
    double radial = sqrt(xs * xs + ys * ys);
    double new_y = radial / Order * TWOPI - Shift_y;
    double new_x = rot_x - Shift_x;

    return cpu_double3(new_x, new_y, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__ngon2 = {
    .id = FORMULA__NGON2,
    .name = "_ngon2",
    .param_count = 5,
    .complexity = 142,
    .uses_transcendental = true
};

/*
Original M3 Description:
Polygon fold with radial y output by Luca GN.
Similar sector snapping to _ngon, but:
1. x output = y'*cos(rot) + x'*sin(rot) (standard y-rotation component)
2. Even-sector negation applied to x (star symmetry)
3. y output = radial distance sqrt(x'²+y'²), not angular fraction

This creates a radial unwrap effect where y represents the
distance from origin, and x represents the angular position.

Constants (from m3 [CONSTANTS]):
  2*pi = 6.283185307 (at [PVar+0x00])
  deg_to_rad = 0.017453293 (at [PVar+0x08])

Parameters:
  [0] Order: double (default: 4)
  [1] Shift (deg): double (default: 0)
  [2] Shift x: double (default: 0)
  [3] Shift y: double (default: 0)
  [4] Skew (deg): double (default: 0)
*/
