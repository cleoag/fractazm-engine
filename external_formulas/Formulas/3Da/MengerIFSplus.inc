// MengerIFSplus.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MengerIFSplus
// Hand-ported from ASM: 223 bytes, DEoption=2
// Menger sponge with rotation center offset (extended MengerIFS)

constexpr unsigned int FORMULA_MENGERIFS_PLUS = 10025;

// ASM trace summary:
//   0x0D-0x17: Load z, y, x; abs each -> ST0=|x|, ST1=|y|, ST2=|z|
//   0x19-0x37: Sort network (3 comparisons with jae) -> ST0=max, ST1=mid, ST2=min
//   0x3A-0x4D: Add rotation center offsets: mid+=dX(esi-0x54), max+=dY(esi-0x5c)...
//              Wait: DC C2 adds to st(2)=max. So: max+=dX, mid+=dY, min+=dZ
//              Actually traced precisely: st(2)+=dX, st(1)+=dY, st(3)+=dZ
//              After loads, ST0=dX pushes, so st(2)=max. Result:
//              ST0=mid+dY, ST1=max+dX, ST2=min+dZ (after all three add-pop cycles)
//   0x4-0x86: Apply rotation matrix (same offsets as MengerIFS)
//              r0 = (max+dX)*M00 + (mid+dY)*M01 + (min+dZ)*M02
//              r1 = (max+dX)*M10 + (mid+dY)*M11 + (min+dZ)*M12
//              r2 = (max+dX)*M20 + (mid+dY)*M21 + (min+dZ)*M22
//   0x89-0x9C: Subtract offsets: Row0(ST0)-=dY, Row1(ST1)-=dX, Row2(ST2)-=dZ
//   0x9-0xAC: Scale + DE: de*=Scale, r1*=Scale (->x path), r2*=Scale (->y path)
//   0xAE-0xCF: Menger fold on z:
//              k = Scale-1, x = r1*S - k*CScaleX, y = r2*S - k*CScaleY
//              kz = (k/Scale)*CScaleZ, z = (kz - |r0 - kz|) * Scale
//   0xD1-0xD5: Store y=[edx], x=[ebx], z=[ecx]

template<>
cpu_double3 execute_formula<FORMULA_MENGERIFS_PLUS>(cpu_double3 p, constant double* params, double &de) {
    const double Scale   = params[0];   // [PVar-0x10] default: 3
    const double CScaleX = params[1];   // [PVar-0x18] default: 1
    const double CScaleY = params[2];   // [PVar-0x20] default: 1
    const double CScaleZ = params[3];   // [PVar-0x28] default: 0.5

    // Rotation matrix: 9 pre-computed float elements (from .3SingleAngles)
    // params[4..12] map to [esi-0x2C..-0x4C] — same as MengerIFS
    const double dX = params[13];   // [PVar-0x54] default: 0.5
    const double dY = params[14];   // [PVar-0x5C] default: 0.5
    const double dZ = params[15];   // [PVar-0x64] default: 0.5

    // Step 1: Absolute values
    double ax = abs(p.x);
    double ay = abs(p.y);
    double az = abs(p.z);

    // Step 2: Sort into max >= mid >= min
    double max_val = ax, mid_val = ay, min_val = az;
    if (max_val < mid_val) { double t = max_val; max_val = mid_val; mid_val = t; }
    if (max_val < min_val) { double t = max_val; max_val = min_val; min_val = t; }
    if (mid_val < min_val) { double t = mid_val; mid_val = min_val; min_val = t; }

    // Step 3: Add rotation center offsets: max+=dX, mid+=dY, min+=dZ
    double pmax = max_val + dX;
    double pmid = mid_val + dY;
    double pmin = min_val + dZ;

    // Step 4: Apply rotation. This family is sensitive to the sorted-triple slot
    // order after the x87 compare network; use the natural (max, mid, min)
    // ordering here and validate against the Windows reference.
    // r1 uses params[4..6]   ([esi-0x2C..-0x34])
    // r2 uses params[7..9]   ([esi-0x38..-0x40])
    double r0 = pmax * params[10] + pmid * params[11] + pmin * params[12];
    double r1 = pmax * params[4]  + pmid * params[5]  + pmin * params[6];
    double r2 = pmax * params[7]  + pmid * params[8]  + pmin * params[9];

    // Step 5: Subtract offsets — ASM: r1 -= dX, r2 -= dY, r0 -= dZ
    r1 -= dX;
    r2 -= dY;
    r0 -= dZ;

    // Step 6: Scale + DE tracking + Menger fold
    // ASM output: x = r1*Scale - k*CScaleX, y = r2*Scale - k*CScaleY,
    //             z = (kz - |r0 - kz|) * Scale
    de *= Scale;
    double k = Scale - 1.0;
    double kz = (k / Scale) * CScaleZ;

    return cpu_double3(
        r1 * Scale - k * CScaleX,
        r2 * Scale - k * CScaleY,
        (kz - abs(r0 - kz)) * Scale
    );
}

// Alternative direct function implementation
cpu_double3 MengerIFSplus_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_MENGERIFS_PLUS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_MengerIFSplus = {
    .id = FORMULA_MENGERIFS_PLUS,
    .name = "MengerIFSplus",
    .param_count = 10,
    .complexity = 750,
    .uses_transcendental = true
};

/*
Original M3 Description:
Same as normal MengerIFS, but you can move the rotation center arbitrarily for many more variations.
It should be obvious that since you leave all rotations zero you will never notice any effect.
Plus, you will see that dX and rotation X are independent; only dY and dZ will modify it.
Same for all other axis.

"Hacked" from Jesse's formula (Luca GN 2011). Idea taken from the original KIFS thread.

Mathematical Formula:
  1. Absolute values: x=|x|, y=|y|, z=|z|
  2. Sort: arrange into (max, mid, min)
  3. Add rotation center offsets: max+=dX, mid+=dY, min+=dZ
  4. Rotation: R * [max+dX, mid+dY, min+dZ]^T
  5. Subtract rotation center offsets: r0-=dY (y-path), r1-=dX (x-path), r2-=dZ (z-fold)
  6. Scale + DE: de *= Scale
  7. Offsets:
     x_out = r1 * Scale - (Scale-1) * CScaleX
     y_out = r2 * Scale - (Scale-1) * CScaleY
  8. Z-fold (Menger cross):
     kz = (Scale-1)/Scale * CScaleZ
     z_out = (kz - |r0 - kz|) * Scale

Parameters:
  [0] Scale: double (default: 3)
  [1] CScale_X: double (default: 1)
  [2] CScale_Y: double (default: 1)
  [3] CScale_Z: double (default: 0.5)
  [4..6] Rotation angles (degrees) - .3SingleAngles
  [7] Rot_dX: double (default: 0.5) - rotation center X
  [8] Rot_dY: double (default: 0.5) - rotation center Y
  [9] Rot_dZ: double (default: 0.5) - rotation center Z

PVar layout:
  [PVar-0x10] = Scale
  [PVar-0x18] = CScale_X
  [PVar-0x20] = CScale_Y
  [PVar-0x28] = CScale_Z
  [PVar-0x2C..0x4C] = Rotation matrix (9 singles)
  [PVar-0x54] = Rot_dX
  [PVar-0x5C] = Rot_dY
  [PVar-0x64] = Rot_dZ
*/
