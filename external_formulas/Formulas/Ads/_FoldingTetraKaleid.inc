// _FoldingTetraKaleid.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _FoldingTetraKaleid
// Hand-ported from ASM: 308 bytes, DEoption=-1

constexpr unsigned int FORMULA__FOLDINGTETRAKALEID = 10016;

// ASM trace:
//   Parameters (edi = PVar):
//     [edi-0x10] = k (double, default 1.0)
//     [edi-0x14] = Enable pre-rotation (integer, default 0)
//     [edi-0x18] = Undo Tetra3D (integer, default 0)
//   Constants:
//     [edi+0x00] = 2/3 (0.66666666...)
//
//   Local flags (stored as double on stack):
//     flag_xy = 0 initially; set to 1 if tetra3d swapped x,y
//     flag_xz = 0 initially; set to 1 if tetra3d swapped x,z
//     flag_yz = 0 initially; set to 1 if tetra3d swapped y,z
//
//   Phase 1: Tetra3D fold (1st half - always done):
//     if (x + y <= 0) { x,y = -y,-x; flag_xy = 1 }
//     if (x + z <= 0) { x,z = -z,-x; flag_xz = 1 }
//     if (z + y <= 0) { z,y = -y,-z; flag_yz = 1 }
//
//   Phase 2: Pre-rotation (if enabled):
//     dd = -(x+y+z) * (2/3)
//     x = -(x + dd), y = -(y + dd), z = -(z + dd)
//
//   Phase 3: Fold45-style fold:
//     sum = x + y + z
//     if (sum >= k) {
//       dd = (sum - k) * (2/3)
//       x -= dd; y -= dd; z -= dd
//     }
//
//   Phase 4: Undo Tetra3D (if enabled):
//     if (!flag_yz) { z,y = -y,-z }  // undo swap for axes NOT folded
//     if (!flag_xz) { x,z = -z,-x }
//     if (!flag_xy) { x,y = -y,-x }

template<>
cpu_double3 execute_formula<FORMULA__FOLDINGTETRAKALEID>(cpu_double3 p, constant double* params, double &de) {
    double k           = params[0];  // [PVar-0x10], double, default 1.0
    int   preRotation = cpu_bitcast_to_int(params[1]);  // [PVar-0x14], integer, default 0
    int   undoTetra   = cpu_bitcast_to_int(params[2]);  // [PVar-0x18], integer, default 0
    double h           = 0.6666666666666667;      // constant 2/3

    double x = p.x;
    double y = p.y;
    double z = p.z;

    // Phase 1: Tetra3D fold (1st half symmetry planes)
    bool swapped_xy = false;
    bool swapped_xz = false;
    bool swapped_yz = false;

    // if (x + y <= 0) { tmp = x; x = -y; y = -tmp; }
    if (x + y <= 0.0) {
        double tmp = x;
        x = -y;
        y = -tmp;
        swapped_xy = true;
    }

    // if (x + z <= 0) { tmp = x; x = -z; z = -tmp; }
    if (x + z <= 0.0) {
        double tmp = x;
        x = -z;
        z = -tmp;
        swapped_xz = true;
    }

    // if (z + y <= 0) { tmp = z; z = -y; y = -tmp; }
    if (z + y <= 0.0) {
        double tmp = z;
        z = -y;
        y = -tmp;
        swapped_yz = true;
    }

    // Phase 2: Pre-rotation (optional)
    if (preRotation != 0) {
        double dd = -(x + y + z) * h;
        x = -(x + dd);
        y = -(y + dd);
        z = -(z + dd);
    }

    // Phase 3: Fold45-style fold
    double sum = x + y + z;
    if (sum >= k) {
        double dd = (sum - k) * h;
        x -= dd;
        y -= dd;
        z -= dd;
    }

    // Phase 4: Undo Tetra3D (optional)
    // For each axis pair that was NOT swapped, apply reverse swap-negate
    if (undoTetra != 0) {
        if (!swapped_yz) {
            double tmp = z;
            z = -y;
            y = -tmp;
        }
        if (!swapped_xz) {
            double tmp = x;
            x = -z;
            z = -tmp;
        }
        if (!swapped_xy) {
            double tmp = x;
            x = -y;
            y = -tmp;
        }
    }

    return cpu_double3(x, y, z);
}

// Formula metadata
constant FormulaInfo formula_info__FoldingTetraKaleid = {
    .id = FORMULA__FOLDINGTETRAKALEID,
    .name = "_FoldingTetraKaleid",
    .param_count = 3,
    .complexity = 308,
    .uses_transcendental = false
};

/*
Original M3 Description:

Combination of;

1. Tetra3D folding, 1st half only
2. Fold45single with default inclination
3. Eventually, undo the sign changes done by 1st folding

The effect is "visually perfect" for symmetric cubic 3D fractals, like;
Amazing Box, ABoxVaryScale, MengerIFS, Menger3 ...
when used as a pre-transform. Everything else can have "strange" effects.
Undo Tetra3D can be good for non-cubic fractals (else not noticeable),
and for all "adding c" fractals. Be aware! It introduces some cuts (DE trouble...)

Parameters:
  params[0] = k (double, default 1.0) - fold threshold
  params[1] = Enable pre-rotation (integer, default 0)
  params[2] = Undo Tetra3D (integer, default 0)

Constant: 2/3 (0.66666666...) used as fold factor
*/
