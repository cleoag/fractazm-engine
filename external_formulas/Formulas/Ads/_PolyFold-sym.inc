// _PolyFold-sym.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _PolyFold-sym
// Hand-ported from ASM: 131 bytes, DEoption=-1

constexpr unsigned int FORMULA__POLYFOLDSYM = 7076;

// ASM trace:
//   x' = x + Shift_x,  y' = y + Shift_y
//   angle = atan2(y', x')
//   combined = angle + (Skew_deg + Shift_deg) * DEG2RAD
//   sector = round(combined / TWOPI * Order)   (fistp/fild = round to nearest)
//   rot = -sector / Order * TWOPI + Shift_deg * DEG2RAD
//   (c, s) = (cos(rot), sin(rot))
//   rot_x = y'*s - x'*c        (same x-negation as _PolyFolding)
//   rot_y = x'*s + y'*c
//   if (sector is even): rot_y = -rot_y   (symmetry flip!)
//   new_x = rot_x - Shift_x
//   new_y = rot_y - Shift_y
//   z unchanged
//
// Symmetric polygon fold by Luca GN (2012). Differs from _PolyFolding:
// 1. Shifts coords, rotates shifted, then un-shifts
// 2. Even sectors get y-negation for star/polygon symmetry
// Continuous symmetry on even orders.

template<>
cpu_double3 execute_formula<FORMULA__POLYFOLDSYM>(cpu_double3 p, constant double* params, double &de) {
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
    double sector = rint(sector_value);  // round to nearest (matches fistp/fild)
    int sector_i = int(sector);

    double rot = -sector / Order * TWOPI + Shift_deg * DEG2RAD;

    // Rotate shifted coordinates (with x negation from ASM)
    double c = cos(rot);
    double s = sin(rot);
    double rot_x = ys * s - xs * c;     // -(x'*cos - y'*sin)
    double rot_y = xs * s + ys * c;     //  x'*sin + y'*cos

    // Symmetry: negate y for even sectors
    if ((sector_i & 1) == 0) {
        rot_y = -rot_y;
    }

    // Un-shift
    double new_x = rot_x - Shift_x;
    double new_y = rot_y - Shift_y;

    return cpu_double3(new_x, new_y, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__PolyFold_sym = {
    .id = FORMULA__POLYFOLDSYM,
    .name = "_PolyFold-sym",
    .param_count = 5,
    .complexity = 131,
    .uses_transcendental = true
};

/*
Original M3 Description:
Symmetric polygon/star fold by Luca GN (2012).
Continuous symmetry on even orders.
Shift X/Y and angular shift parameters do not cause discontinuity.

Differs from _PolyFolding:
1. Shifts coordinates before rotation, un-shifts after
2. Even sectors get y-negation (star/polygon symmetry)

x' = x + Shift_x, y' = y + Shift_y
angle = atan2(y', x')
combined = angle + (Skew_deg + Shift_deg) * DEG2RAD
sector = round(combined / (2*pi) * Order)
rot = -sector/Order * 2*pi + Shift_deg * DEG2RAD
(c, s) = (cos(rot), sin(rot))
rot_x = y'*s - x'*c
rot_y = x'*s + y'*c
if sector even: rot_y = -rot_y
new_x = rot_x - Shift_x
new_y = rot_y - Shift_y
z unchanged

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
