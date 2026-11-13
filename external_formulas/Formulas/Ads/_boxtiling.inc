// _boxtiling.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _boxtiling
// Hand-ported from ASM: 159 bytes, DEoption=-1
// Periodic box tiling — maps each coordinate to [-tile/2, tile/2].

constexpr unsigned int FORMULA__BOXTILING = 1143;

// ASM trace (159 bytes, 4 branches):
//
// For each axis (w, z, y, x):
//   if tile_size != 0:
//     coord' = coord - round(coord / tile_size) * tile_size
//
// Uses frndint (round to nearest integer) for periodic wrapping.
// Result maps coord to range [-tile_size/2, tile_size/2].
// If tile_size == 0, axis is left unchanged (infinite tile = no tiling).
//
// For 3D: w tiling skipped (w=0).
// C handling: Transform (DEoption=-1), no C.

template<>
cpu_double3 execute_formula<FORMULA__BOXTILING>(cpu_double3 p, constant double* params, double &de) {
    const double x_tile = params[0];   // [esi-0x10] (default: ?)
    const double y_tile = params[1];   // [esi-0x18] (default: ?)
    const double z_tile = params[2];   // [esi-0x20] (default: ?)

    double x = p.x, y = p.y, z = p.z;

    // Tile each axis: coord' = coord - round(coord/tile) * tile
    if (x_tile != 0.0) {
        x = x - rint(x / x_tile) * x_tile;
    }
    if (y_tile != 0.0) {
        y = y - rint(y / y_tile) * y_tile;
    }
    if (z_tile != 0.0) {
        z = z - rint(z / z_tile) * z_tile;
    }

    return cpu_double3(x, y, z);
}

// Formula metadata
constant FormulaInfo formula_info__boxtiling = {
    .id = FORMULA__BOXTILING,
    .name = "_boxtiling",
    .param_count = 3,
    .complexity = 159,
    .uses_transcendental = false
};

/*
Original M3 Description:
Periodic box tiling transform. Maps each coordinate into a periodic
cell centered at the origin.

For each axis:
  coord' = coord - round(coord / tile_size) * tile_size
Result lies in [-tile_size/2, tile_size/2].
Set tile_size = 0 to disable tiling on that axis.

Full 4D:
  Same pattern for all 4 axes (x, y, z, w).

3D projection:
  w tiling has no effect (w=0).

Parameters:
  [0] x_tile: double - x-axis tile size [esi-0x10]
  [1] y_tile: double - y-axis tile size [esi-0x18]
  [2] z_tile: double - z-axis tile size [esi-0x20]
  4D-only: w_tile [esi-0x28]
*/
