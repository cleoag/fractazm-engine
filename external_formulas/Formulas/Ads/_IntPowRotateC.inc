// _IntPowRotateC: Julia-state transform, transcribed from the shipped SSE2
// code (724 bytes). All divisions, including zero denominators, are retained.
constexpr unsigned int FORMULA__INTPOWROTATEC = 10020;
// Original SSE output is observable by a later uncounted Manowar callback.
template<> struct ExternalFormulaUsesSse01<FORMULA__INTPOWROTATEC> { static constexpr bool value = true; };
template<> struct ExternalFormulaAlwaysWritesJuliaXYZ<FORMULA__INTPOWROTATEC> { static constexpr bool value = true; };


template<>
cpu_double3 execute_formula<FORMULA__INTPOWROTATEC>(cpu_double3 p, constant double* params, double &de) {
    double x = params[13], y = params[14], z = params[15];
    const double r = sqrt(z*z + (x*x + y*y));
    const double inv = params[12] / r;
    x *= inv; y *= inv; z *= inv;
    if (params[0] != 0.0) { const double t = x; x = y; y = t; }
    complex_power_rotate(x,y,z,(int)params[1],(int)params[2],params[12],true);
    if (params[3] != 0.0) complex_power_rotate(x,y,z,(int)params[4],(int)params[5],params[12],true);
    if (params[6] != 0.0) complex_power_rotate(x,y,z,(int)params[7],(int)params[8],params[12],true);
    // The final rotation leaves an unscaled lane in XMM1.high. Scalar MULSD
    // at 0x94/0x9c changes only XMM1.low; the six return paths are 0xf2,
    // 0x136, 0x17f, 0x1bb, 0x208 and 0x250 in the immutable original CODE.
    int lastType = static_cast<int>(params[6] != 0.0 ? params[8] :
                                   params[3] != 0.0 ? params[5] : params[2]);
    if (lastType == (std::numeric_limits<int>::min)()) lastType = 5;
    if (lastType >= 6) lastType -= 6;
    params[FORMULA_STATE_X_SLOT] = x*r;
    params[FORMULA_STATE_Y_SLOT] = y*r;
    params[FORMULA_STATE_Z_SLOT] = (z*r)*params[9];
    params[FORMULA_STATE_W_SLOT] = lastType <= 0 ? 0.0 :
                                 lastType == 1 ? x : lastType == 2 ? y : z;
    params[WRITE_J1_SLOT] = x*r;
    params[WRITE_J2_SLOT] = y*r;
    params[WRITE_J3_SLOT] = (z*r)*params[9];
    return p;
}

// Formula metadata
constant FormulaInfo formula_info__IntPowRotateC = {
    .id = FORMULA__INTPOWROTATEC,
    .name = "_IntPowRotateC",
    .param_count = 10,
    .complexity = 724,
    .uses_transcendental = true
};

/*
Original M3 Description:
Upon an idea from Aexion to also rotate the adding constant in addition to a bulb formula.
So put this in front or after a bulb formula in an alternating hybrid.

The vector length of C is not changed, only complex rotated.
Rotation power can be 2..11 (integer).

The vector components for the complex rotation types are:
  0:  x|y       6:  y|x
  1:  x|z       7:  z|x
  2:  y|z       8:  z|y
  3:  Rxy|z     9:  z|Rxy
  4:  Rxz|y     10: y|Rxz
  5:  Ryz|x     11: x|Ryz

Flip Cxy = 1 is exchanging Cx with Cy at the beginning.

SPECIAL: C-ONLY transform. Does NOT modify coordinates.
The wrapper writes the transformed Julia constant after each call.

Algorithm:
  1. Normalize C to unit vector, save |C|
  2. Optionally flip Cx/Cy
  3. For each enabled rotation (1-3):
     - Select 2D plane from C components based on type
     - Apply complex integer power (complex multiply repeated)
  4. Restore |C| magnitude
  5. Apply Cz_multiplier to Cz

Parameters:
  [0] Flip_Cxy: integer (0 or 1, default: 0)
  [1] Rot1_power: integer (2-11, default: 8)
  [2] Rot1_type: integer (0-11, default: 0)
  [3] Enable_rot2: integer (0 or 1, default: 1)
  [4] Rot2_power: integer (2-11, default: 8)
  [5] Rot2_type: integer (0-11, default: 3)
  [6] Enable_rot3: integer (0 or 1, default: 0)
  [7] Rot3_power: integer (2-11, default: 8)
  [8] Rot3_type: integer (0-11, default: 0)
  [9] Cz_multiplier: double (default: 1.0)
*/
