// _LinCombineCxyz.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _LinCombineCxyz
// Hand-ported from ASM: 315 bytes, DEoption=-1

constexpr unsigned int FORMULA__LINCOMBINECXYZ = 7048;

template<>
struct ExternalFormulaAlwaysWritesJuliaXYZ<FORMULA__LINCOMBINECXYZ> { static constexpr bool value = true; };

// ASM trace:
//   esi=context, edi=PVar
//   DD4628/DD4620/DD4618    load Cz, Cy, Cx from [esi+0x28/0x20/0x18]
//   D9C0/DC4FF0/D9C2/DC4FE8/DEC1/D9C3/DC4FE0/DEC1  Cx' = CXx*Cx + CXy*Cy + CXz*Cz
//   DD5E18                  store Cx'
//   (same pattern for Cy' and Cz')
//   DD5E20, DD5E28          store Cy', Cz'
//
// 3x3 matrix multiplication on Julia constants (Cx,Cy,Cz).
// Coordinates (x,y,z) are unchanged.

template<>
cpu_double3 execute_formula<FORMULA__LINCOMBINECXYZ>(cpu_double3 p, constant double* params, double &de) {
    // This formula modifies Julia constants Cx, Cy, Cz via a 3x3 matrix.
    // The coordinate p is returned unchanged.
    // WRITE_J slots carry the updated constants through the formula wrapper.
    //
    // In the original ASM:
    //   Cx' = CXx*Cx + CXy*Cy + CXz*Cz
    //   Cy' = CYx*Cx + CYy*Cy + CYz*Cz
    //   Cz' = CZx*Cx + CZy*Cy + CZz*Cz
    //
    // Parameters (9-element matrix, default = identity):
    //   [0] CXx=1  [1] CXy=0  [2] CXz=0
    //   [3] CYx=0  [4] CYy=1  [5] CYz=0
    //   [6] CZx=0  [7] CZy=0  [8] CZz=1

    // All rows use the original C values retained by the x87 stack.
    const double cx = params[9], cy = params[10], cz = params[11];
    params[WRITE_J1_SLOT] = (params[0]*cx + params[1]*cy) + params[2]*cz;
    params[WRITE_J2_SLOT] = (params[3]*cx + params[4]*cy) + params[5]*cz;
    params[WRITE_J3_SLOT] = (params[6]*cx + params[7]*cy) + params[8]*cz;
    return p;
}

// Formula metadata
constant FormulaInfo formula_info__LinCombineCxyz = {
    .id = FORMULA__LINCOMBINECXYZ,
    .name = "_LinCombineCxyz",
    .param_count = 9,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
A general 3x3 matrix conversion for C:

Cx' = CXx*Cx + CXy*Cy + CXz*Cz
Cy' = CYx*Cx + CYy*Cy + CYz*Cz
Cz' = CZx*Cx + CZy*Cy + CZz*Cz

Use as a pretransform to rotate/scale the C (Julia seed) vector.
Default: identity matrix.

NOTE: This formula modifies Julia constants, not coordinates.
The formula wrapper propagates the WRITE_J slots to the iteration state.

Parameters:
  [0] CXx mul (default: 1)  [1] CXy mul (default: 0)  [2] CXz mul (default: 0)
  [3] CYx mul (default: 0)  [4] CYy mul (default: 1)  [5] CYz mul (default: 0)
  [6] CZx mul (default: 0)  [7] CZy mul (default: 0)  [8] CZz mul (default: 1)
*/
