// _QuadrayTransform.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _QuadrayTransform
// Hand-ported from ASM: 92 bytes, DEoption=-1

constexpr unsigned int FORMULA__QUADRAYTRANSFORM = 7059;

template<>
struct ExternalFormulaWritesWState<FORMULA__QUADRAYTRANSFORM> {
    static constexpr bool value = true;
};

template<>
struct ExternalFormulaUsesWrittenCForSameIterationAdd<FORMULA__QUADRAYTRANSFORM> {
    static constexpr bool value = true;
};

// ASM trace:
//   Quadray coordinate transform on both (x,y,z) and (Cx,Cy,Cz):
//   a = x + y + z
//   b = z - x - y
//   c = y - x - z
//   d = x - y - z    (stored to w / [ecx+8] in ASM)
//
//   Same transform applied to C:
//   Ca = Cx + Cy + Cz → new Cx
//   Cb = Cz - Cx - Cy → new Cy
//   Cc = Cy - Cx - Cz → new Cz
//   Cd = Cx - Cy - Cz → stored to context [esi-0x38]
//
// The same quadray remap is also applied to the hidden w lane and Julia lanes.
// No parameters.

template<>
cpu_double3 execute_formula<FORMULA__QUADRAYTRANSFORM>(cpu_double3 p, constant double* params, double &de) {
    (void)de;

    double x = p.x, y = p.y, z = p.z;
    const double Cx = params[0];
    const double Cy = params[1];
    const double Cz = params[2];

    double new_x = x + y + z;       // a
    double new_y = z - x - y;       // b
    double new_z = y - x - z;       // c
    params[FORMULA_STATE_W_SLOT] = x - y - z;  // d

    params[WRITE_J1_SLOT] = Cx + Cy + Cz;
    params[WRITE_J2_SLOT] = Cz - Cx - Cy;
    params[WRITE_J3_SLOT] = Cy - Cx - Cz;
    params[WRITE_J4_SLOT] = Cx - Cy - Cz;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__QuadrayTransform = {
    .id = FORMULA__QUADRAYTRANSFORM,
    .name = "_QuadrayTransform",
    .param_count = 0,
    .complexity = 92,
    .uses_transcendental = false
};

/*
Original M3 Description:
Quadray coordinate transform. Converts Cartesian (x,y,z) to quadray
coordinates (a,b,c,d) where a+b+c+d=0.

a = x + y + z
b = z - x - y
c = y - x - z
d = x - y - z   (pseudo-4D, stored as w)

Same transform is applied to C values (Cx,Cy,Cz → Ca,Cb,Cc).

No parameters.
*/
