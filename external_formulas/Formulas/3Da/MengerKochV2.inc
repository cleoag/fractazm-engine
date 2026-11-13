// MengerKochV2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MengerKochV2
// Hand-ported from ASM: 307 bytes, DEoption=2
// Menger modified formula similar to Koch snowflake
// abs(xyz) -> R1 rotation -> sort descending -> Edge1 fold -> Edge2 fold ->
// R2 rotation -> Scale*DE -> Menger offsets with conditional z-fold

constexpr unsigned int FORMULA_MENGERKOCHV2 = 10037;

// ASM trace summary:
//   000D-0018: abs(x,y,z) via AND 0x7FFFFFFF on sign bits
//   001B-0058: load z,y,x -> R1 rotation (9 singles at PVar-0x2C..0x4C)
//              R1 applied to (x, y, z): row_i = x*R1[i,0] + y*R1[i,1] + z*R1[i,2]
//   005A-0078: sort descending (3 compare-swap with shr ah,1 / jae)
//   007A-0092: Edge1 fold on max: if Edge1>0, max = Edge1 - |Edge1 - max|
//   0096-00AE: Edge2 fold on mid: if Edge2>0, mid = Edge2 - |Edge2 - mid|
//   00B4-00EB: R2 rotation (9 singles at PVar-0x50..0x70)
//              R2 applied to (mid', max', min): row_i = mid'*R2[i,0] + max'*R2[i,1] + min*R2[i,2]
//   00ED-00FC: de *= Scale; row0,row1,row2 *= Scale
//   00FE-0129: k = Scale-1; y = row0*S - k*CScaleY; x = row1*S - k*CScaleX;
//              z = row2*S; if (k*CScaleZ*factor <= row2*S) z -= k*CScaleZ

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void MengerKochV2Original_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_MENGERKOCHV2>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={fabs(p.x),fabs(p.y),fabs(p.z),de};
    MengerKochV2Original_x87(state,params);
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E = numeric::Extended80;
    const auto dot = [&](E x, E y, E z, int first) {
        return (x*E(params[first])+y*E(params[first+1]))+z*E(params[first+2]);
    };
    const E x = abs(E(p.x)), y = abs(E(p.y)), z = abs(E(p.z));
    E maximum = dot(x,y,z,4), middle = dot(x,y,z,7), minimum = dot(x,y,z,10);
    if (!(maximum >= middle)) std::swap(maximum,middle);
    if (!(maximum >= minimum)) std::swap(maximum,minimum);
    if (!(middle >= minimum)) std::swap(middle,minimum);
    if (!(params[22] <= 0)) maximum = E(params[22])-abs(E(params[22])-maximum);
    if (!(params[23] <= 0)) middle = E(params[23])-abs(E(params[23])-middle);
    // 00B4..00EB consumes max/mid/min. The stack ends as rows 1/0/2;
    // 0125..0129 stores those to y/x/z, respectively. Keep every intermediate
    // extended until these final stores, as in the original CODE.
    const E scale(params[0]), k = scale-E(1);
    const E outX = dot(maximum,middle,minimum,13)*scale-k*E(params[1]);
    const E outY = dot(maximum,middle,minimum,16)*scale-k*E(params[2]);
    E outZ = dot(maximum,middle,minimum,19)*scale;
    const E kz = k*E(params[3]);
    if (!(kz*E(params[24]) > outZ)) outZ -= kz;
    de = double(E(de)*scale);
    return cpu_double3(double(outX),double(outY),double(outZ));
#else
    const double Scale   = params[0];
    const double CScaleX = params[1];
    const double CScaleY = params[2];
    const double CScaleZ = params[3];
    const double Edge1   = params[22];
    const double Edge2   = params[23];

    // Rotation1 from 9 pre-computed float elements (from .3SingleAngles)
    RotMatrix3 R1 = rotMatrix3FromFloats(params, 4);

    // Rotation2 from 9 pre-computed float elements (from .3SingleAngles)
    RotMatrix3 R2 = rotMatrix3FromFloats(params, 13);

    // Step 1: abs(x,y,z)
    double ax = abs(p.x);
    double ay = abs(p.y);
    double az = abs(p.z);

    // Step 2: R1 rotation applied to (x, y, z)
    cpu_double3 r1v = R1 * cpu_double3(ax, ay, az);
    double r0 = r1v.x;
    double r1 = r1v.y;
    double r2 = r1v.z;

    // Step 3: Sort descending (result: max_val >= mid_val >= min_val)
    double max_val = r0;
    double mid_val = r1;
    double min_val = r2;
    if (!(max_val >= mid_val)) { double t = max_val; max_val = mid_val; mid_val = t; }
    if (!(max_val >= min_val)) { double t = max_val; max_val = min_val; min_val = t; }
    if (!(mid_val >= min_val)) { double t = mid_val; mid_val = min_val; min_val = t; }

    // Step 4: Edge1 fold on max_val
    // After the descending sort the x87 stack is [mid, max, min].
    // The Edge1 block subtracts ST(2), so it folds the current max slot.
    if (!(Edge1 <= 0.0)) {
        max_val = Edge1 - abs(Edge1 - max_val);
    }

    // Step 5: Edge2 fold on mid_val
    // The second fold block subtracts ST(1), which is the current mid slot.
    if (!(Edge2 <= 0.0)) {
        mid_val = Edge2 - abs(Edge2 - mid_val);
    }

    // Step 6: R2 rotation applied to (mid', max', min)
    // Shared Menger-family parity fixes confirm the second rotation consumes (mid,max,min),
    // not the natural (max,mid,min) tuple.
    // After R2: ST0=Row_2, ST1=Row_1, ST2=Row_0.
    cpu_double3 r2v = R2 * cpu_double3(mid_val, max_val, min_val);
    double Row_0 = r2v.x;
    double Row_1 = r2v.y;
    double Row_2 = r2v.z;

    // Step 7: Scale + DE tracking
    de *= Scale;
    Row_0 *= Scale;
    Row_1 *= Scale;
    Row_2 *= Scale;

    // Step 8: Menger offsets
    // After the second rotation the scaled x87 stack stores x from Row_0, y from Row_1,
    // and z from Row_2 before the conditional z-fold.
    double k = Scale - 1.0;
    double x_out = Row_1 - k * CScaleX;
    double y_out = Row_0 - k * CScaleY;
    double z_out = Row_2;  // before conditional fold

    // Conditional z-fold:
    // ASM at 0x113: fld k*CScaleZ; fld st(0); fmul [esi-0x8]=0.5; compare vs z_out
    // if (0.5*k*CScaleZ <= z_out): z_out -= k*CScaleZ (i.e., `fsub st(3)`)
    double kz = k * CScaleZ;
    double kz_test = params[24] * kz;
    if (!(kz_test > z_out)) {
        z_out -= kz;
    }

    return cpu_double3(x_out, y_out, z_out);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_MengerKochV2 = {
    .id = FORMULA_MENGERKOCHV2,
    .name = "MengerKochV2",
    .param_count = 12,
    .complexity = 1100,
    .uses_transcendental = true
};

/*
Original M3 Description:
Menger modified formula to get results similar to the Koch snowflake.
The result in my opinion is very interesting.
The main param is Edge1. Produces great results for a wide range of values.
You can get more variations setting a positive value for Edge2 too -
works good with values less than 1.

LGN 2011

Parameters:
  [0] Scale: double (default: 3)
  [1] CScale_X: double (default: 1)
  [2] CScale_Y: double (default: 0)
  [3] CScale_Z: double (default: 0.33333)
  [4..6] Rotation1: 3 angles in degrees (default: 0)
  [7..9] Rotation2: 3 angles in degrees (default: 0)
  [10] Edge1: double (default: 1)
  [11] Edge2: double (default: 0)

PVar layout:
  [PVar-0x10] = Scale
  [PVar-0x18] = CScaleX
  [PVar-0x20] = CScaleY
  [PVar-0x28] = CScaleZ
  [PVar-0x2C..0x4C] = R1 matrix (9 floats)
  [PVar-0x50..0x70] = R2 matrix (9 floats)
  [PVar-0x78] = Edge1
  [PVar-0x80] = Edge2
  [PVar-0x08] = engine-computed (1/Scale factor for z-fold test)
*/
