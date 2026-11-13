// _updateC2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _updateC2
// Hand-ported from ASM: 265 bytes, DEoption=-1
// C-modifying transform: rotates, scales, offsets, conditionally swaps with C.

constexpr unsigned int FORMULA__UPDATEC2 = 10014;
template<> struct ExternalFormulaWritesWState<FORMULA__UPDATEC2> { static constexpr bool value = true; };
template<> struct ExternalFormulaAlwaysWritesJuliaXYZ<FORMULA__UPDATEC2> { static constexpr bool value = true; };

// ASM trace:
//   esi = context, ebx = &x, edi = PVar
//   Params (from .3SingleAngles -> 9 rotation matrix elements, Single):
//     m[0]=[edi-0x0c], m[1]=[edi-0x10], m[2]=[edi-0x14]   (row 0)
//     m[3]=[edi-0x18], m[4]=[edi-0x1c], m[5]=[edi-0x20]   (row 1)
//     m[6]=[edi-0x24], m[7]=[edi-0x28], m[8]=[edi-0x2c]   (row 2)
//     [edi-0x34] = Scale (double, default: 1)
//     [edi-0x3c] = AddX (double, default: 0)
//     [edi-0x44] = AddY (double, default: 0)
//     [edi-0x4c] = AddZ (double, default: 0)
//   Context:
//     [esi+0x18] = Cx, [esi+0x20] = Cy, [esi+0x28] = Cz
//     [ecx+8] = DE value
//
//   Phase 1: Forward rotation M * [x,y,z]
//     rx = x*m[0] + y*m[1] + z*m[2]
//     ry = x*m[3] + y*m[4] + z*m[5]
//     rz = x*m[6] + y*m[7] + z*m[8]
//
//   Phase 2: Scale and offset
//     x = rx * Scale + AddX
//     y = ry * Scale + AddY
//     z = rz * Scale + AddZ
//     DE = DE * Scale
//
//   Phase 3: Conditional swap (per axis: keep max in position, min in C)
//     ASM comparison pattern: fcom + fnstsw + shr ah,1 + jbe
//     When Cx >= x: swap -> x=max(old_x, old_Cx), Cx=min(old_x, old_Cx)
//     When Cx < x: no swap -> x stays as x, Cx stays as Cx
//     Net effect per axis: position = max(pos, C), C = min(pos, C)
//
//     NOTE: Description says "if (x>Cx) swap(x,Cx)" which would give the
//     opposite result. The ASM is trusted: it puts max into position, min into C.
//
//   Phase 4: Inverse rotation M^T * [x,y,z] - Add
//     ox = (x*m[0] + y*m[3] + z*m[6]) - AddX
//     oy = (x*m[1] + y*m[4] + z*m[7]) - AddY
//     oz = (x*m[2] + y*m[5] + z*m[8]) - AddZ

template<>
cpu_double3 execute_formula<FORMULA__UPDATEC2>(cpu_double3 p, constant double* params, double &de) {
    // Rotation matrix built from 3 Euler angles (degrees)
    RotMatrix3 R = rotMatrix3FromFloats(params, 0);

    const double Scale = params[9];   // [edi-0x34] (default: 1)
    const double AddX  = params[10];   // [edi-0x3c] (default: 0)
    const double AddY  = params[11];   // [edi-0x44] (default: 0)
    const double AddZ  = params[12];   // [edi-0x4c] (default: 0)

    // C values passed by engine (read/write: engine must update C after call)
    double Cx = params[13];  // [esi+0x18]
    double Cy = params[14];  // [esi+0x20]
    double Cz = params[15];  // [esi+0x28]

    double x = p.x, y = p.y, z = p.z;

    // Phase 1: Forward rotation M * p
    cpu_double3 rotated = R * cpu_double3(x, y, z);
    double rx = rotated.x;
    double ry = rotated.y;
    double rz = rotated.z;

    // Phase 2: Scale and offset
    x = rx * Scale + AddX;
    y = ry * Scale + AddY;
    z = rz * Scale + AddZ;

    // DE scaling: ASM does DE = DE * Scale via [ecx+8]
    params[FORMULA_STATE_W_SLOT] *= Scale;

    // Phase 3: Conditional swap per axis
    // ASM: fcom Cx vs x, if Cx >= x then swap (fxch)
    // Result: position gets max, C gets min
    if (Cx >= x) {
        double tmp = x;
        x = Cx;
        Cx = tmp;
    }
    if (Cy >= y) {
        double tmp = y;
        y = Cy;
        Cy = tmp;
    }
    if (Cz >= z) {
        double tmp = z;
        z = Cz;
        Cz = tmp;
    }
    // Equivalent to: x = max(x, Cx), Cx = min(x, Cx)  (and same for y,z)

    // Phase 4: Inverse rotation M^T * [x,y,z] minus offset
    cpu_double3 v = cpu_double3(x, y, z);
    double ox = dot(R.col0, v) - AddX;
    double oy = dot(R.col1, v) - AddY;
    double oz = dot(R.col2, v) - AddZ;

    // Write updated C values back through J writeback slots.
    // Pascal ASM writes directly to [esi+0x18/0x20/0x28] (J1/J2/J3).
    params[WRITE_J1_SLOT] = Cx;
    params[WRITE_J2_SLOT] = Cy;
    params[WRITE_J3_SLOT] = Cz;

    return cpu_double3(ox, oy, oz);
}

// Formula metadata
constant FormulaInfo formula_info__updateC2 = {
    .id = FORMULA__UPDATEC2,
    .name = "_updateC2",
    .param_count = 13,
    .complexity = 265,
    .uses_transcendental = false
};

/*
Original M3 Description:
Mixes conditionally x,y,z and C in a strange way;

if (x>Cx) swap(x,Cx);
if (y>Cy) swap(y,Cy);
if (z>Cz) swap(z,Cz);

NOTE: The description's swap condition is REVERSED from what the ASM actually
implements. The ASM does: swap when C >= position (not when position > C).
Result per axis: position = max(pos, C), C = min(pos, C).
Trusting the ASM over the description per project conventions.

Full algorithm:
1) Forward rotation: r = M * p   (using .3SingleAngles rotation matrix)
2) Scale + offset:   r = r * Scale + Add
3) Scale DE:         DE = DE * Scale
4) Per-axis swap:    pos = max(pos, C), C = min(pos, C)
5) Inverse rotation: out = M^T * pos
6) Subtract offset:  out = out - Add

WARNING: This is a C-MODIFYING TRANSFORM. The formula writes new values back
to Cx, Cy, Cz in the iteration context ([esi+0x18], [esi+0x20], [esi+0x28]).
The Metal engine must provide C values as input params[13..15] and retrieve
the modified C values after execution.

Parameters:
  [0..8] Rotation matrix (9 Single values from .3SingleAngles "Rotate")
  [9]  Scale: double (default: 1)
  [10] AddX: double (default: 0)
  [11] AddY: double (default: 0)
  [12] AddZ: double (default: 0)
  [13] Cx: from engine context (read/write)
  [14] Cy: from engine context (read/write)
  [15] Cz: from engine context (read/write)
*/
