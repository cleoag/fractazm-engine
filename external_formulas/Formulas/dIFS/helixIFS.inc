// helixIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: helixIFS
// Hand-ported from ASM: 231 bytes, Capstone-verified
// DEoption: 20 (dIFS — formula computes shape DE directly)
// Multi-strand elliptical helix with configurable axis

constexpr unsigned int FORMULA_HELIXIFS = 3785;

template<>
cpu_double3 execute_formula<FORMULA_HELIXIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Helix_stretch = params[0];
    const double Diameter_1 = params[1];
    const double Diameter_2 = params[2];
    const double Scale = params[3];
    const double Z_add = params[4];
    const double Y_add = params[5];
    const double X_add = params[6];
    const double String_radius = params[7];
    const int apply_scale_add = int(params[8]);
    const int Coord = int(params[9]);        // 0=x axis, 1=y axis, 2=z axis
    const int Helix_count = int(params[10]);  // 1..16

    // Step 1: IFS transformation — scale + translate
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    // Step 2: Conditional position/w update
    cpu_double3 pos;
    if (apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;  // w tracking
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
        pos = cpu_double3(x, y, z);
    } else {
        pos = p;  // position unchanged
    }

    // Step 3: Select helix axis and radial coordinates
    // axis_coord = coordinate along helix axis
    // rad1, rad2 = coordinates in the radial plane
    double axis_coord;
    double rad1, rad2;
    if (Coord == 0) {
        // Helix along x-axis
        axis_coord = x;
        rad1 = y;
        rad2 = z;
    } else if (Coord == 1) {
        // Helix along y-axis
        axis_coord = y;
        rad1 = x;
        rad2 = z;
    } else {
        // Helix along z-axis (Coord == 2)
        axis_coord = z;
        rad1 = y;
        rad2 = x;
    }

    // Step 4: Compute helix angle step (2*pi / Helix_count)
    double angle_step;
    int count = (Helix_count - 1) & 0xF;
    if (count <= 1) {
        angle_step = M_PI;  // pi (optimization for count 1 or 2)
    } else {
        angle_step = 2.0 * M_PI / double(count + 1);
    }

    // Step 5: Loop over helix strands, find minimum distance
    double helix_angle = axis_coord * Helix_stretch;
    double min_dist = 10000.0;  // initial large value (from CONSTANT)

    for (int k = count; k >= 0; k--) {
        // Helix strand center on ellipse in radial plane
        double c = cos(helix_angle);
        double s = sin(helix_angle);
        double center1 = c * Diameter_2;
        double center2 = s * Diameter_1;

        // Distance from current point to strand center in radial plane
        double d1 = rad1 - center1;
        double d2 = rad2 - center2;
        double dist = sqrt(d1 * d1 + d2 * d2);

        min_dist = min(min_dist, dist);

        // Advance angle to next strand
        helix_angle += angle_step;
    }

    // Step 6: Shape DE = min distance to any strand - string radius
    double shape_de = min_dist - String_radius;

    // Step 7: DE compensation when not applying scale+add
    if (apply_scale_add == 0) {
        shape_de = shape_de / Scale;
    }

    // dIFS shape_de stored to Rout ([esi-0x20] in ASM).
    de = shape_de;

    return pos;
}

// Shape DE computation (standalone, for use by dIFS engine)
double helixIFS_shape_de(cpu_double3 p, constant double* params) {
    const double Helix_stretch = params[0];
    const double Diameter_1 = params[1];
    const double Diameter_2 = params[2];
    const double Scale = params[3];
    const double X_add = params[6];
    const double Y_add = params[5];
    const double Z_add = params[4];
    const double String_radius = params[7];
    const int apply_scale_add = int(params[8]);
    const int Coord = int(params[9]);
    const int Helix_count = int(params[10]);

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    double axis_coord, rad1, rad2;
    if (Coord == 0) { axis_coord = x; rad1 = y; rad2 = z; }
    else if (Coord == 1) { axis_coord = y; rad1 = x; rad2 = z; }
    else { axis_coord = z; rad1 = y; rad2 = x; }

    int count = (Helix_count - 1) & 0xF;
    double angle_step = (count <= 1) ? M_PI : 2.0 * M_PI / double(count + 1);

    double helix_angle = axis_coord * Helix_stretch;
    double min_dist = 10000.0;

    for (int k = count; k >= 0; k--) {
        double c = cos(helix_angle);
        double s = sin(helix_angle);
        double d1 = rad1 - c * Diameter_2;
        double d2 = rad2 - s * Diameter_1;
        min_dist = min(min_dist, sqrt(d1 * d1 + d2 * d2));
        helix_angle += angle_step;
    }

    double shape_de = min_dist - String_radius;
    if (apply_scale_add == 0) { shape_de /= Scale; }
    return shape_de;
}

// Alternative direct function implementation
cpu_double3 helixIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HELIXIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_helixIFS = {
    .id = FORMULA_HELIXIFS,
    .name = "helixIFS",
    .param_count = 11,
    .complexity = 520,
    .uses_transcendental = true
};

/*
Original M3 Description:
A helix shape.
Coord defines the direction.
High values of Helix stretch and the Diameters makes the distance estimate bad,
meaning it needs lowering the Raystep multiplier to not overstep.

Mathematical Formula:
  dIFS execution model (DEoption=20):
    - Formula computes shape DE directly
    - IFS engine accumulates: min_de = min(shape_de_i / w_i)

  Step 1: IFS transformation (if apply_scale_add != 0):
    (x, y, z) = (x, y, z) * Scale + (X_add, Y_add, Z_add)
    w *= Scale

  Step 2: Coordinate selection (Coord parameter):
    Coord 0: axis=x, radial=(y, z)  — helix along x-axis
    Coord 1: axis=y, radial=(x, z)  — helix along y-axis
    Coord 2: axis=z, radial=(y, x)  — helix along z-axis

  Step 3: Angle step:
    if Helix_count <= 2: angle_step = pi
    else: angle_step = 2*pi / Helix_count

  Step 4: Loop over Helix_count strands:
    helix_angle = axis_coord * Helix_stretch
    for k = 0 to Helix_count-1:
      center = (Diameter_2 * cos(helix_angle), Diameter_1 * sin(helix_angle))
      dist = sqrt((rad1 - center.x)^2 + (rad2 - center.y)^2)
      min_dist = min(min_dist, dist)
      helix_angle += angle_step

  Step 5: shape_de = min_dist - String_radius

  Step 6: DE compensation (if apply_scale_add == 0):
    shape_de /= Scale

Parameters:
  [0] Helix_stretch: double (default: 1)
  [1] Diameter_1: double (default: 1) — ellipse semi-axis for sin component
  [2] Diameter_2: double (default: 1) — ellipse semi-axis for cos component
  [3] Scale: double (default: 1.6) — from .2Doubles (replicated for SIMD)
  [4] Z_add: double (default: 0)
  [5] Y_add: double (default: 0)
  [6] X_add: double (default: 0)
  [7] String_radius: double (default: 0.1) — radius of helix wire
  [8] apply_scale_add: int (default: 1)
  [9] Coord: int (default: 0) — 0=x, 1=y, 2=z axis
  [10] Helix_count: int (default: 2) — number of strands, 1..16

PVar layout (verified against SSE2 ASM offsets):
  [PVar + 0x00..0x07] = CONSTANT: 10000.0 (initial min_dist)
  [PVar - 0x10] = Helix_stretch (double)
  [PVar - 0x18] = Diameter_1 (double)
  [PVar - 0x20] = Diameter_2 (double)
  [PVar - 0x28] = Scale high (double, from .2Doubles)
  [PVar - 0x30] = Scale low (double, from .2Doubles — both = Scale)
  [PVar - 0x38] = Z_add (double)
  [PVar - 0x40] = Y_add (double)
  [PVar - 0x48] = X_add (double)
  [PVar - 0x50] = String_radius (double)
  [PVar - 0x54] = apply_scale_add (integer)
  [PVar - 0x58] = Coord (integer, 0..2)
  [PVar - 0x5C] = Helix_count (integer, 1..16)

dIFS state structure (via esi):
  [esi - 0x78] = x position (double)
  [esi - 0x70] = y position (double, packed low)
  [esi - 0x68] = z position (double, packed high)
  [esi + 0x70] = w (DE scale tracking, double)
  [esi - 0x20] = shape DE output (double)

SSE2 + x87 hybrid execution:
  - SSE2 for position transform and distance computation
  - x87 FPU for fsincos and angle stepping (more efficient for scalar trig)
  - Stack-allocated local storage aligned to 16 bytes

Key notes:
  - Multi-strand helix: Helix_count evenly-spaced spiral strands
  - Elliptical cross-section: Diameter_1 (sin) and Diameter_2 (cos) can differ
  - Helix_stretch controls spiral tightness (turns per unit along axis)
  - String_radius is subtracted from min distance (gives the wire thickness)
  - The shape DE is a torus-like distance: distance_to_center - wire_radius
  - High Helix_stretch or Diameters degrade DE accuracy (need lower Raystep)
  - No OTrap coloring (unlike cylinderIFS)
*/
