// _ngon.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _ngon
// Hand-ported from ASM: 151 bytes, DEoption=-1

constexpr unsigned int FORMULA__NGON = 7081;

// ASM trace:
//   x' = x + Shift_x,  y' = y + Shift_y
//   angle = atan2(y', x')
//   combined = angle + (Skew_deg + Shift_deg) * DEG2RAD
//   sector_value = combined / TWOPI * Order
//   sector = round(sector_value)   (fistp/fild)
//   rot = -sector / Order * TWOPI + Shift_deg * DEG2RAD
//   (c, s) = (cos(rot), sin(rot))
//   rot_x = y'*s - x'*c           (negated standard x-rotation)
//   If No_yfold != 0:
//     y_out = sector_value
//   Else:
//     y_out = sector_value - sector  (fractional part)
//     if sector is even: y_out = -y_out
//   new_y = y_out / Order * TWOPI - Shift_y
//   new_x = rot_x - Shift_x
//   z unchanged
//
// Polygon angular unwrap by Luca GN. Converts angular sector
// position to y output. Related to _PolyFolding but outputs
// the angular fraction rather than the rotated coordinate.

template<>
cpu_double3 execute_formula<FORMULA__NGON>(cpu_double3 p, constant double* params, double &de) {
    const double Order     = params[0];  // default: 4
    const double Shift_deg = params[1];  // default: 0
    const double Shift_x   = params[2];  // default: 0
    const double Shift_y   = params[3];  // default: 0
    const double Skew_deg  = params[4];  // default: 0
    const int   No_yfold  = int(params[5]);  // integer, default: 0

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

    // Rotate shifted coordinates (negated x-rotation from ASM)
    double c = cos(rot);
    double s = sin(rot);
    double rot_x = ys * s - xs * c;

    // Compute y output based on No_yfold mode
    double y_out;
    if (No_yfold != 0) {
        // No folding: y = raw sector value
        y_out = sector_value;
    } else {
        // Folded: fractional part with even-sector negation
        y_out = sector_value - sector;
        if ((sector_i & 1) == 0) {
            y_out = -y_out;
        }
    }

    // Scale y output and un-shift
    double new_y = y_out / Order * TWOPI - Shift_y;
    double new_x = rot_x - Shift_x;

    return cpu_double3(new_x, new_y, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__ngon = {
    .id = FORMULA__NGON,
    .name = "_ngon",
    .param_count = 6,
    .complexity = 151,
    .uses_transcendental = true
};

/*
Original M3 Description:
Polygon angular unwrap transform by Luca GN.
Converts angular position within polygon sectors to y coordinate.

Shifts, computes angle, snaps to sector (like _PolyFolding),
then outputs:
  x = rotated x (y'*sin - x'*cos) minus shift
  y = angular fraction within sector (or raw sector_value if No_yfold)

With No_yfold=0 (default), the y output is the fractional
angular position within the sector, negated for even sectors
to create continuous output.

Constants (from m3 [CONSTANTS]):
  2*pi = 6.283185307 (at [PVar+0x00])
  deg_to_rad = 0.017453293 (at [PVar+0x08])

Parameters:
  [0] Order: double (default: 4)
  [1] Shift (deg): double (default: 0)
  [2] Shift x: double (default: 0)
  [3] Shift y: double (default: 0)
  [4] Skew (deg): double (default: 0)
  [5] No y-fold: integer (default: 0)
*/
