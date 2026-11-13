// _PolyFolding.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _PolyFolding
// Hand-ported from ASM: 102 bytes, DEoption=-1

constexpr unsigned int FORMULA__POLYFOLDING = 7065;

// ASM trace:
//   angle = atan2(y + Shift_y, x + Shift_x)
//   combined = angle + (Skew_deg + Shift_deg) * DEG2RAD
//   sector = round(combined / TWOPI * Order)
//   rot = -sector / Order * TWOPI + Shift_deg * DEG2RAD
//
//   Apply rotation (with x negation):
//     x' = -(x*cos(rot) - y*sin(rot))    = y*sin(rot) - x*cos(rot)
//     y' =   x*sin(rot) + y*cos(rot)
//   z unchanged
//
// Polygon folding: snaps angle to nearest polygon edge, then rotates.
// Constants: TWOPI=6.283185307, DEG2RAD=0.017453293

template<>
cpu_double3 execute_formula<FORMULA__POLYFOLDING>(cpu_double3 p, constant double* params, double &de) {
    const double Order     = params[0];  // default: 5
    const double Shift_deg = params[1];  // default: 0
    const double Shift_x   = params[2];  // default: 0
    const double Shift_y   = params[3];  // default: 0
    const double Skew_deg  = params[4];  // default: 0

    // Constants (from [CONSTANTS] section of m3)
    const double TWOPI   = 6.283185307;
    const double DEG2RAD = 0.017453293;

    double x = p.x, y = p.y;

    // Compute angle and snap to polygon sector
    double angle = atan2(y + Shift_y, x + Shift_x);
    double combined = angle + (Skew_deg + Shift_deg) * DEG2RAD;
    double sector = round(combined / TWOPI * Order);
    double rot = -sector / Order * TWOPI + Shift_deg * DEG2RAD;

    // Rotate (with x negation from ASM)
    double c = cos(rot);
    double s = sin(rot);
    double new_x = y * s - x * c;      // -(x*cos - y*sin)
    double new_y = x * s + y * c;      //  x*sin + y*cos

    return cpu_double3(new_x, new_y, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__PolyFolding = {
    .id = FORMULA__POLYFOLDING,
    .name = "_PolyFolding",
    .param_count = 5,
    .complexity = 102,
    .uses_transcendental = true
};

/*
Original M3 Description:
Polygon folding transform. Snaps the angular position to the nearest
polygon edge and applies a rotation. Used for creating polygon-symmetric
patterns.

angle = atan2(y+Shift_y, x+Shift_x)
combined = angle + (Skew_deg + Shift_deg) * DEG2RAD
sector = round(combined / (2*pi) * Order)
rot = -sector/Order * 2*pi + Shift_deg * DEG2RAD
x' = y*sin(rot) - x*cos(rot)
y' = x*sin(rot) + y*cos(rot)
z unchanged

Constants (from m3 [CONSTANTS]):
  2*pi = 6.283185307 (at [PVar+0x00])
  deg_to_rad = 0.017453293 (at [PVar+0x08])

Parameters:
  [0] Order: double (default: 5)
  [1] Shift (deg): double (default: 0)
  [2] Shift x: double (default: 0)
  [3] Shift y: double (default: 0)
  [4] Skew (deg): double (default: 0)
*/
