// _LinCombineXYZ.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _LinCombineXYZ
// Hand-ported from ASM: 510 bytes, DEoption=-1

constexpr unsigned int FORMULA__LINCOMBINEXYZ = 7032;

// ASM trace:
//   Load z, y, x onto FPU stack
//   For x': dup x, mul Xx, dup y, mul Xy, add, dup z, mul Xz, add → store x'
//   For y': dup x, mul Yx, dup y, mul Yy, add, dup z, mul Yz, add → store y'
//   For z': mul Zx (using x on stack), swap y, mul Zy, add, swap z, mul Zz, add → store z'
//
// Parameters at PVar offsets (8 bytes each):
//   [PVar-0x10] = Xx  [PVar-0x18] = Xy  [PVar-0x20] = Xz
//   [PVar-0x28] = Yx  [PVar-0x30] = Yy  [PVar-0x38] = Yz
//   [PVar-0x40] = Zx  [PVar-0x48] = Zy  [PVar-0x50] = Zz
//
// General 3x3 matrix multiplication:
// p' = M * p

template<>
cpu_double3 execute_formula<FORMULA__LINCOMBINEXYZ>(cpu_double3 p, constant double* params, double &de) {
    const double Xx = params[0], Xy = params[1], Xz = params[2];
    const double Yx = params[3], Yy = params[4], Yz = params[5];
    const double Zx = params[6], Zy = params[7], Zz = params[8];

    double new_x = Xx * p.x + Xy * p.y + Xz * p.z;
    double new_y = Yx * p.x + Yy * p.y + Yz * p.z;
    double new_z = Zx * p.x + Zy * p.y + Zz * p.z;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__LinCombineXYZ = {
    .id = FORMULA__LINCOMBINEXYZ,
    .name = "_LinCombineXYZ",
    .param_count = 9,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
A general 3x3 matrix conversion:
x' = Xx*x + Xy*y + Xz*z
y' = Yx*x + Yy*y + Yz*z
z' = Zx*x + Zy*y + Zz*z

Default: identity matrix.

Parameters:
  [0] Xx mul (default: 1)  [1] Xy mul (default: 0)  [2] Xz mul (default: 0)
  [3] Yx mul (default: 0)  [4] Yy mul (default: 1)  [5] Yz mul (default: 0)
  [6] Zx mul (default: 0)  [7] Zy mul (default: 0)  [8] Zz mul (default: 1)
*/
