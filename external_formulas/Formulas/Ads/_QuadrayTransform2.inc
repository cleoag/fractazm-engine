// _QuadrayTransform2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _QuadrayTransform2
// Hand-ported from ASM: 156 bytes, DEoption=-1

constexpr unsigned int FORMULA__QUADRAYTRANSFORM2 = 3272;

template<>
struct ExternalFormulaWritesWState<FORMULA__QUADRAYTRANSFORM2> { static constexpr bool value = true; };

// ASM trace (from M3 description + hex verification):
//   For position (x,y,z):
//     x' = Scale * abs( x + y + z) + Offset
//     y' = Scale * abs(-x - y + z) + Offset
//     z' = Scale * abs(-x + y - z) + Offset
//     w' = Scale * abs( x - y - z) + Offset  (4D, dropped in 3D)
//   Same transform applied to Julia constants (Cx,Cy,Cz):
//     Cx' = Scale * abs( Cx + Cy + Cz) + Offset
//     Cy' = Scale * abs(-Cx - Cy + Cz) + Offset
//     Cz' = Scale * abs(-Cx + Cy - Cz) + Offset
//     Cw' = Scale * abs( Cx - Cy - Cz) + Offset
//
// Based on Aexion's ideas. 3D→4D quadray pre-transform.
//
// XYZ/W and Julia writes are checked against the original x86 code.
// Full-orbit and image parity require separate validation.

template<>
cpu_double3 execute_formula<FORMULA__QUADRAYTRANSFORM2>(cpu_double3 p, constant double* params, double &de) {
    const double Offset = params[0];
    const double Scale = params[1];

    double x = p.x, y = p.y, z = p.z;

    // Quadray transform on position
    cpu_double3 result;
    result.x = Scale * abs( x + y + z) + Offset;
    result.y = Scale * abs(-x - y + z) + Offset;
    result.z = Scale * abs(-x + y - z) + Offset;
    params[FORMULA_STATE_W_SLOT] = Scale * abs(x - y - z) + Offset;
    const double cx = params[2], cy = params[3], cz = params[4];
    params[WRITE_J1_SLOT] = Scale * abs(cx + cy + cz) + Offset;
    params[WRITE_J2_SLOT] = Scale * abs(cz - cx - cy) + Offset;
    params[WRITE_J3_SLOT] = Scale * abs(cy - cx - cz) + Offset;
    params[WRITE_J4_SLOT] = Scale * abs(cx - cy - cz) + Offset;

    return result;
}

// Alternative direct function implementation
cpu_double3 _QuadrayTransform2_formula(cpu_double3 p, constant double* params, double &de) {
    const double O = params[0], S = params[1];
    double x = p.x, y = p.y, z = p.z;
    de = S * abs( x - y - z) + O;
    return cpu_double3(S * abs( x + y + z) + O,
                   S * abs(-x - y + z) + O,
                   S * abs(-x + y - z) + O);
}

// Formula metadata
constant FormulaInfo formula_info__QuadrayTransform2 = {
    .id = FORMULA__QUADRAYTRANSFORM2,
    .name = "_QuadrayTransform2",
    .param_count = 2,
    .complexity = 156,
    .uses_transcendental = false
};

/*
Original M3 Description:
Based on Aexion's ideas, modified quadray transform as pre-transform.
Result is 4D, best combined with 4D formulas.

Mathematical Formula:
x' = Scale * abs( x + y + z) + Offset
y' = Scale * abs(-x - y + z) + Offset
z' = Scale * abs(-x + y - z) + Offset
w' = Scale * abs( x - y - z) + Offset
(Same applied to Cx,Cy,Cz,Cw)

Parameters:
  [0] Offset: double (default: -1.35)
  [1] Scale: double (default: +1.0)
*/
