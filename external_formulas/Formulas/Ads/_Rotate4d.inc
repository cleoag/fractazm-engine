// _Rotate4d.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _Rotate4d
// Hand-ported from ASM: 135 bytes, DEoption=-1
// 4x4 rotation matrix applied to (x,y,z,w).
// The loader expands .6SingleAngles into sixteen Single matrix coefficients.

constexpr unsigned int FORMULA__ROTATE4D = 6518;
template<> struct ExternalFormulaWritesWState<FORMULA__ROTATE4D> { static constexpr bool value = true; };

// ASM trace (135 bytes, 0 branches):
//
// esi = PVar (overwrite convention: mov esi, [esi+0x30])
// Matrix elements stored as SINGLE PRECISION floats (dword), not doubles!
//
// 4x4 matrix multiply:
// x' = m00*x + m01*y + m02*z + m03*w
// y' = m10*x + m11*y + m12*z + m13*w
// z' = m20*x + m21*y + m22*z + m23*w
// w' = m30*x + m31*y + m32*z + m33*w
//
// Scene angles are expanded by the loader using this axis order:
//   (1,2)=yz, (0,2)=xz, (0,1)=xy, (0,3)=xw, (1,3)=yw, (2,3)=zw
// The fourth coordinate uses the wrapper W-state slot.
//
// C handling: Transform (DEoption=-1), no C.

template<>
cpu_double3 execute_formula<FORMULA__ROTATE4D>(cpu_double3 p, constant double* params, double &de) {
    RotMatrix4 rot = rotMatrix4FromFloats(params, 0);
    cpu_double4 v = {p.x, p.y, p.z, params[FORMULA_STATE_W_SLOT]};
    cpu_double4 out = rot * v;
    params[FORMULA_STATE_W_SLOT] = out.w;
    return cpu_double3(out.x, out.y, out.z);
}

// Formula metadata
constant FormulaInfo formula_info__Rotate4d = {
    .id = FORMULA__ROTATE4D,
    .name = "_Rotate4d",
    .param_count = 16,
    .complexity = 135,
    .uses_transcendental = false
};

/*
Original M3 Description:
4D rotation transform. Applies a 4x4 rotation matrix built from 6 raw angles
(one for each pair of axes: YZ, XZ, XY, XW, YW, ZW), matching MB3D's
BuildRotMatrix4d contract.

Full 4D: standard 4x4 matrix multiply with input/output W in its state slot.

Parameters:
  Scene: six angles in degrees. Execution: [0..15] expanded Single matrix.
*/
