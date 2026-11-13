// _updateC.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _updateC
// Hand-ported from ASM: 70 bytes, DEoption=-1

constexpr unsigned int FORMULA__UPDATEC = 7055;

// ASM trace:
//   Cx' = x * xyz_mul + Cx * C_mul
//   Cy' = y * xyz_mul + Cy * C_mul
//   Cz' = z * xyz_mul + Cz * C_mul
//
// Transform: updates C as a linear combination of position and current C.
// Does not modify x, y, z.

template<>
cpu_double3 execute_formula<FORMULA__UPDATEC>(cpu_double3 p, constant double* params, double &de) {
    const double xyz_mul = params[0];
    const double C_mul   = params[1];
    // C values appended by wrapper after PVar entries
    const double Cx = params[2];
    const double Cy = params[3];
    const double Cz = params[4];

    // Cx' = x * xyz_mul + Cx * C_mul
    params[WRITE_J1_SLOT] = p.x * xyz_mul + Cx * C_mul;
    params[WRITE_J2_SLOT] = p.y * xyz_mul + Cy * C_mul;
    params[WRITE_J3_SLOT] = p.z * xyz_mul + Cz * C_mul;

    return p;  // position unchanged
}

// Formula metadata
constant FormulaInfo formula_info__updateC = {
    .id = FORMULA__UPDATEC,
    .name = "_updateC",
    .param_count = 2,
    .complexity = 70,
    .uses_transcendental = false
};

/*
Original M3 Description:
Updates C values as a linear combination of current position and C.

Cx' = x * xyz_mul + Cx * C_mul
Cy' = y * xyz_mul + Cy * C_mul
Cz' = z * xyz_mul + Cz * C_mul

This is a side-effect transform: position is unchanged, but the engine
must apply the C update using the formula parameters.

Parameters:
  [0] xyz mul: double (default: 1)
  [1] C-xyz mul: double (default: 0)
*/
