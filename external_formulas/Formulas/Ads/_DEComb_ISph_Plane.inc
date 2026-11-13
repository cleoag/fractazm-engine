// _DEComb_ISph_Plane.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _DEComb_ISph_Plane
// Hand-ported from ASM: 141 bytes, DEoption=-1 (transform)
// This is a DeCombinate transform: sphere inversion on 1st iter, sqrt(r2) scaling on all others.

constexpr unsigned int FORMULA_DECOMB_ISPH_PLANE = 10259;
template<> struct ExternalFormulaReadsRout<FORMULA_DECOMB_ISPH_PLANE> { static constexpr bool value = true; };

// ASM trace summary:
//   Parameters (PVar negative offsets):
//     [PVar-0x10] = Move X          (param 4)
//     [PVar-0x18] = Move Y          (param 5)
//     [PVar-0x20] = Move Z          (param 6)
//     [PVar-0x28] = Radius          (param 3)
//     [PVar-0x30] = Inv. X center   (param 0)
//     [PVar-0x38] = Inv. Y center   (param 1)
//     [PVar-0x40] = Inv. Z center   (param 2)
//
//   First iteration path (esi+0xd0 == 0):
//     Load z, y, x
//     x' = x - Inv_X_center
//     y' = y - Inv_Y_center
//     z' = z - Inv_Z_center
//     r2 = x'^2 + y'^2 + z'^2
//     scale = Radius / r2           (fdivrp: Radius / r2)
//     x_out = x' * scale - Move_X
//     y_out = y' * scale - Move_Y
//     z_out = z' * scale - Move_Z
//
//   Subsequent iterations path (esi+0xd0 != 0):
//     Load z, y, x
//     r2 = [esi+0x38]              (pre-computed r2 = x^2+y^2+z^2)
//     s = sqrt(r2)
//     x_out = x * s
//     y_out = y * s
//     z_out = z * s

template<>
cpu_double3 execute_formula<FORMULA_DECOMB_ISPH_PLANE>(cpu_double3 p, constant double* params, double &de) {
    const double inv_x_center = params[4];  // PVar -0x30
    const double inv_y_center = params[5];  // PVar -0x38
    const double inv_z_center = params[6];  // PVar -0x40
    const double radius       = params[3];  // default: 16
    const double move_x       = params[0];  // PVar -0x10
    const double move_y       = params[1];  // PVar -0x18
    const double move_z       = params[2];  // PVar -0x20
    // Pascal ASM uses [esi+0xd0] = bFirstIt (0 on first call, then incremented)
    const int iteration = (int)params[BFIRSTIT_SLOT];

    double x = p.x, y = p.y, z = p.z;

    if (iteration == 0) {
        params[BFIRSTIT_SLOT] = 1.0; // INC occurs only in the first-call branch.
        // Sphere inversion path
        // Shift by inversion center
        double dx = x - inv_x_center;
        double dy = y - inv_y_center;
        double dz = z - inv_z_center;

        // r2 = dx^2 + dy^2 + dz^2
        double r2 = dx * dx + dy * dy + dz * dz;

        // scale = Radius / r2
        double scale = radius / r2;

        // Scale and shift by Move
        x = dx * scale - move_x;
        y = dy * scale - move_y;
        z = dz * scale - move_z;
    } else {
        // sqrt(r2) scaling path
        // r2 is pre-computed as x^2+y^2+z^2 by the engine at [esi+0x38]
        const double r2 = params[FORMULA_ROUT_SLOT];
        double s = sqrt(r2);

        x = x * s;
        y = y * s;
        z = z * s;
    }

    return cpu_double3(x, y, z);
}

// Formula metadata
constant FormulaInfo formula_info_DEComb_ISph_Plane = {
    .id = FORMULA_DECOMB_ISPH_PLANE,
    .name = "_DEComb_ISph_Plane",
    .param_count = 8,
    .complexity = 141,
    .uses_transcendental = false
};

/*
Original M3 Description:
As the name states, this should be used only as the 1st transform on DeCombinates.
It's a combination of a _SphereInv (1st iter) and a _SquareR (all other iters).

Much more flexible than the normal _SquareR to get bubbles, planes or even inverse bubbles!

Use Move controls to move the position of the object and Inv Center to tweak the effect.

The effect will be gorgeous (a kind of deforming mirror) if the reflections are enabled.

Parameters:
  [0] Inv. X center: double (default: 0)
  [1] Inv. Y center: double (default: 0)
  [2] Inv. Z center: double (default: 0)
  [3] Radius: double (default: 16)
  [4] Move X: double (default: 0)
  [5] Move Y: double (default: 0)
  [6] Move Z: double (default: 0)
  [7] iteration: int (0=first, >0=subsequent - managed by engine)

PVar layout:
  [PVar-0x10] = Move X
  [PVar-0x18] = Move Y
  [PVar-0x20] = Move Z
  [PVar-0x28] = Radius
  [PVar-0x30] = Inv. X center
  [PVar-0x38] = Inv. Y center
  [PVar-0x40] = Inv. Z center

Context access:
  [esi+0xd0] = iteration counter (0 for first call)
  [esi+0x38] = pre-computed r^2

NOTE: This is a transform (DEoption=-1). DE is not modified.
The iteration counter at [esi+0xd0] determines which path is taken.
In Metal, this needs to be passed as a parameter by the engine.
*/
