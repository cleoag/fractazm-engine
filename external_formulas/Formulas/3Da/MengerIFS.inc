// MengerIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MengerIFS
// Hand-ported from ASM: 178 bytes, Capstone-verified
// DEoption: 2 (analytic DE via w tracking)
// Menger sponge with abs + sort + rotation + scale + fold

constexpr unsigned int FORMULA_MENGERIFS = 3778;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void MengerIFSOriginal_x87(double* xyzw, const double* params);
#endif

template<>
cpu_double3 execute_formula<FORMULA_MENGERIFS>(cpu_double3 p, double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z,de};
    MengerIFSOriginal_x87(state,params);
    de = state[3];
    return cpu_double3(state[0],state[1],state[2]);
#else
    const double Scale = params[0];
    const double CScaleX = params[1];
    const double CScaleY = params[2];
    const double CScaleZ = params[3];

    // Pre-computed rotation matrix from .3SingleAngles (9 floats stored by Pascal engine)
    // param_map: params[4..6] = [esi-0x2C..-0x34], params[7..9] = [esi-0x38..-0x40],
    //            params[10..12] = [esi-0x44..-0x4C]
    // ASM uses: r0 = max*[esi-0x44] + mid*[esi-0x48] + min*[esi-0x4C] = max*p[10] + mid*p[11] + min*p[12]
    //           r1 = max*[esi-0x2C] + mid*[esi-0x30] + min*[esi-0x34] = max*p[4]  + mid*p[5]  + min*p[6]
    //           r2 = max*[esi-0x38] + mid*[esi-0x3C] + min*[esi-0x40] = max*p[7]  + mid*p[8]  + min*p[9]

    // Step 1: Absolute values (ASM clears sign bit via AND 0x7FFFFFFF)
    double ax = abs(p.x);
    double ay = abs(p.y);
    double az = abs(p.z);

    // Step 2: Sort network (3-element comparator)
    // Result: max_val >= mid_val >= min_val
    double max_val = ax;
    double mid_val = ay;
    double min_val = az;
    if (max_val < mid_val) { double t = max_val; max_val = mid_val; mid_val = t; }
    if (max_val < min_val) { double t = max_val; max_val = min_val; min_val = t; }
    if (mid_val < min_val) { double t = mid_val; mid_val = min_val; min_val = t; }

    // The stack is (mid,max,min), but FLD ST(1) reads max first.
    double r0 = max_val * params[10] + mid_val * params[11] + min_val * params[12];
    double r1 = max_val * params[4]  + mid_val * params[5]  + min_val * params[6];
    double r2 = max_val * params[7]  + mid_val * params[8]  + min_val * params[9];

    // Step 4: Scale + DE tracking
    // ASM: de *= Scale, then r1 *= Scale, r2 *= Scale (r0 is NOT scaled here)
    de = de * Scale;

    // Step 5: Menger offsets
    double k = Scale - 1.0;
    // ASM stores: x=[ebx]=r1*Scale-k*CScaleX, y=[edx]=r2*Scale-k*CScaleY
    //             z=[ecx]=(kz-|r0-kz|)*Scale where kz=(k/Scale)*CScaleZ
    double x_out = r1 * Scale - k * CScaleX;
    double y_out = r2 * Scale - k * CScaleY;

    // Special z-fold (Menger hole) uses r0 (NOT r2):
    double kz = (k / Scale) * CScaleZ;
    double z_out = (kz - abs(r0 - kz)) * Scale;

    cpu_double3 result;
    result.x = x_out;
    result.y = y_out;
    result.z = z_out;
    return result;
#endif
}

// Alternative direct function implementation
cpu_double3 MengerIFS_formula(cpu_double3 p, double* params, double &de) {
    return execute_formula<FORMULA_MENGERIFS>(p,params,de);
}

// Formula metadata
constant FormulaInfo formula_info_MengerIFS = {
    .id = FORMULA_MENGERIFS,
    .name = "MengerIFS",
    .param_count = 7,
    .complexity = 650,
    .uses_transcendental = true
};

/*
Original M3 Description:
A 3D menger version that harmonizes more in hybrids than 'Menger3'.

Mathematical Formula:
  1. Absolute values: x=|x|, y=|y|, z=|z|
  2. Sort: arrange into (max, mid, min)
  3. Rotation: R * [max, mid, min]^T
  4. Scale + DE: de *= Scale
  5. Offsets:
     x_out = row1_rot * Scale - (Scale-1) * CScaleX
     y_out = row0_rot * Scale - (Scale-1) * CScaleY
  6. Z-fold (Menger cross):
     kz = (Scale-1)/Scale * CScaleZ
     z_out = (kz - |row2_rot - kz|) * Scale

Parameters:
  [0] Scale: double (default: 3)
  [1] CScale_X: double (default: 1)
  [2] CScale_Y: double (default: 1)
  [3] CScale_Z: double (default: 0.5)
  [4] Rotation_X: double (default: 0) - angle in degrees
  [5] Rotation_Y: double (default: 0) - angle in degrees
  [6] Rotation_Z: double (default: 0) - angle in degrees

PVar layout (verified against ASM offsets):
  [PVar - 0x08] = 0.5 (marker, used in Menger fold comparison)
  [PVar - 0x10] = Scale (double)
  [PVar - 0x18] = CScale_X (double)
  [PVar - 0x20] = CScale_Y (double)
  [PVar - 0x28] = CScale_Z (double)
  [PVar - 0x2C..0x4C] = Rotation matrix (9 singles)

ASM register mapping:
  eax/@x -> ebx (saved), edx/@y, ecx/@z
  [ecx + 8] = w (DE tracking)
  [esi] = PVar (via [PIteration3D + 0x30])
*/
