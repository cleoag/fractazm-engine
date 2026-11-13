// _BRotateFromMag.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _BRotateFromMag
// Hand-ported from ASM: 581 bytes, DEoption=-1, SIpower=2

constexpr unsigned int FORMULA__BROTATEFFROMMAG = 7041;

// ASM trace:
//   Constants: [edi+0]=sqrt(2/3), [edi+8]=sqrt(1/3), [edi+16]=sqrt(1/2)
//
//   DD03/DC4F10/DD5DF8    T1 = x * sqrt(1/2) → local
//   DD02/DC4F10/DD5DF0    T2 = y * sqrt(1/2) → local
//   DD45F8/DD45F0/DEC1    tx = T1 + T2 = (x+y)*sqrt(1/2)
//   DD5DE8                store tx → local
//   DD45F0/DC65F8/DD1A    y' = T2 - T1 = (y-x)*sqrt(1/2)
//
//   DD45E8/DC0           tx * sqrt(2/3)
//   DD01/DC4F08/DEC1      + z * sqrt(1/3)
//   DD1B                  x' = tx*sqrt(2/3) + z*sqrt(1/3)
//
//   DD01/DC0             z * sqrt(2/3)
//   DD45E8/DC4F08/D9E0    tx * sqrt(1/3); negate
//   DEC1                  z' = z*sqrt(2/3) - tx*sqrt(1/3)
//   DD19                  store z'
//
// Rotates from the magnitude axis (-1,-1,-1)→(1,1,1) to the x-axis.
// M. Benesi. No user parameters.

template<>
cpu_double3 execute_formula<FORMULA__BROTATEFFROMMAG>(cpu_double3 p, constant double* params, double &de) {
    // Pre-stored constants
    const double c_sqrt_2_3 = 0.816496580927726;   // sqrt(2/3)
    const double c_sqrt_1_3 = 0.5773502691896258;   // sqrt(1/3)
    const double c_sqrt_1_2 = 0.7071067811865475;   // sqrt(1/2)

    double tx = (p.x + p.y) * c_sqrt_1_2;
    double new_y = (p.y - p.x) * c_sqrt_1_2;

    double new_x = tx * c_sqrt_2_3 + p.z * c_sqrt_1_3;
    double new_z = p.z * c_sqrt_2_3 - tx * c_sqrt_1_3;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__BRotateFromMag = {
    .id = FORMULA__BROTATEFFROMMAG,
    .name = "_BRotateFromMag",
    .param_count = 0,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
Rotates from the magnitude axis (-1,-1,-1)→(1,1,1) to x-axis (-1,0,0)→(1,0,0).

tx = (x+y) * sqrt(1/2)
y' = (y-x) * sqrt(1/2)
x' = z * sqrt(1/3) + tx * sqrt(2/3)
z' = z * sqrt(2/3) - tx * sqrt(1/3)

Constants stored in [CONSTANTS] section:
  sqrt(2/3) = 0.816496580927726
  sqrt(1/3) = 0.5773502691896258
  sqrt(1/2) = 0.7071067811865475

By M. Benesi.
Parameters: none (uses pre-stored constants)
*/
