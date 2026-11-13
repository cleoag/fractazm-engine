#include "ifs_original_state.h"
// Original x87 instruction/store order on Win64; portable fallback is not certified.
constexpr unsigned int FORMULA_GYROIDIFS = 3154;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void GyroidDistance_x87(double*, const double*, int apply);
#endif

template<>
cpu_double3 execute_formula<FORMULA_GYROIDIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    const int apply=int(params[6]);
    const cpu_double3 local(p.x*params[1]+params[4],p.y*params[1]+params[3],p.z*params[1]+params[2]);
    if (apply) params[DIFS_VARYSCALE_SLOT]*=params[1];
    double state[]={local.x,local.y,local.z,0.,0.};
    GyroidDistance_x87(state,params,apply);
    de=state[3]; params[MAX_FORMULA_PARAMS-1]=state[4];
    return apply ? local : p;
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    Arm64ApplyIFSState native(p,params);
    using E=Arm64ApplyIFSState::E;
    const auto rotated=native.rotate(params+7);
    const E x=rotated.x,y=rotated.y,z=rotated.z;
    const E ax=x*params[5],ay=y*params[5],az=z*params[5];
    // Any missing FSINCOS push leaves too few operands for the three cross
    // products and their sum. Both outputs then consume an empty stack slot.
    const E range=E::fromBits(16383+63,0x8000000000000000ULL);
    if(abs(ax)>=range || abs(ay)>=range || abs(az)>=range) {
        params[MAX_FORMULA_PARAMS-1]=double(abs(extendedNaN()));
        return native.finish(extendedNaN(),de);
    }
    const auto sx=extendedSinCos(ax),sy=extendedSinCos(ay),sz=extendedSinCos(az);
    const E t1=sy.sin*sx.cos,t2=sz.cos*sx.sin,t3=sz.sin*sy.cos;
    params[MAX_FORMULA_PARAMS-1]=double(abs(((t1-t2)-t3)*params[16]));
    E distance=t3+(t2+t1),clip=abs(z);
    if(abs(y)>=clip) clip=abs(y);
    if(abs(x)>=clip) clip=abs(x);
    clip-=params[0];
    if(clip>=distance) distance=clip;
    return native.finish(distance/params[5],de);
#else
    const double Size = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Density = params[5];
    const int Apply_scale_add = int(params[6]);
    // params[7..15] = 3x3 rotation matrix (9 singles from .3SingleAngles GyRot)
    const double TrapColor_Density = params[16];

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;    // w *= Scale
    }

    // Step 2: Apply 3x3 rotation matrix (from .3SingleAngles GyRot)
    // ASM: add edi, -0x2c → rotation at standard offsets → add edi, 0x2c
    // The 9 singles in PVar order map to rotation rows (interleaved for x87 efficiency)
    double rx = params[7]  * x + params[8]  * y + params[9]  * z;
    double ry = params[10] * x + params[11] * y + params[12] * z;
    double rz = params[13] * x + params[14] * y + params[15] * z;

    // Step 3: Gyroid surface equation
    // ASM: three fsincos (rz*D, ry*D, rx*D) → three fmulp to form cross-products
    // Gyroid: cos(x*D)*sin(y*D) + cos(y*D)*sin(z*D) + cos(z*D)*sin(x*D)
    double D = Density;
    double sx, cx, sy, cy, sz, cz;
    sx = sincos(rx * D, cx);
    sy = sincos(ry * D, cy);
    sz = sincos(rz * D, cz);

    double term1 = cx * sy;    // cos(rx*D) * sin(ry*D)
    double term2 = cy * sz;    // cos(ry*D) * sin(rz*D)
    double term3 = cz * sx;    // cos(rz*D) * sin(rx*D)

    double gyroid_value = term1 + term2 + term3;

    // Step 4: OTrap writeback (orbit trap output)
    // ASM: fld st(0); fsub st(2); fsub st(3); fmul TrapColor_Density; fabs → [esi+0x80]
    // OTrap = |TrapColor_Density * (term1 - term3 - term2)|
    params[MAX_FORMULA_PARAMS - 1] = fabs(TrapColor_Density * (term1 - term3 - term2));

    // Step 5: Cube intersection distance
    // ASM: max(|rz|, |ry|) → max(that, |rx|) → fsub Size
    double cube_dist = max(abs(rx), max(abs(ry), abs(rz))) - Size;

    // Step 6: Shape DE = max(cube_dist, gyroid_value) / Density
    // ASM: fcom; jae → max; fdiv Density
    double shape_de = max(cube_dist, gyroid_value) / Density;

    // Step 7: Scale correction
    // ASM: if Apply_scale_add == 0: fdiv Scale
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    return cpu_double3(x, y, z);
#endif
}

// Alternative direct function implementation
cpu_double3 gyroidIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_GYROIDIFS>(p,params,de);
}

// Formula metadata
constant FormulaInfo formula_info_gyroidIFS = {
    .id = FORMULA_GYROIDIFS,
    .name = "gyroidIFS",
    .param_count = 17,
    .complexity = 720,
    .uses_transcendental = true
};

/*
Original M3 Description:
Approximation of the gyroid minimal surface.
Enable "Orbit trap" mode to see a marble coloring.

Equation;
      cos x sin y + cos y sin z + cos z sin x = 0

(Intersected with a cube... it's infinite)

Luca GN 2012

Mathematical Formula:
  DEoption=20 (dIFS shape — SSE2 IFS + x87 gyroid surface DE):
    - Standard SSE2 IFS preamble
    - 3x3 rotation via .3SingleAngles (edi adjustment trick)
    - Gyroid surface: cos(x*D)*sin(y*D) + cos(y*D)*sin(z*D) + cos(z*D)*sin(x*D)
    - Cube intersection: max(|rx|, |ry|, |rz|) - Size
    - Shape DE = max(cube_dist, gyroid_value) / Density
    - OTrap = |TrapColor_Density * (term1 - term3 - term2)|

  Step 1: IFS transform:
    x = p.x * Scale + X_add;  y = p.y * Scale + Y_add;  z = p.z * Scale + Z_add
    if Apply_scale_add: w *= Scale, store

  Step 2: 3x3 rotation (from .3SingleAngles GyRot):
    [rx, ry, rz] = R * [x, y, z]

  Step 3: Gyroid surface (three fsincos for rx*D, ry*D, rz*D):
    term1 = cos(rx*D) * sin(ry*D)
    term2 = cos(ry*D) * sin(rz*D)
    term3 = cos(rz*D) * sin(rx*D)
    gyroid_value = term1 + term2 + term3

  Step 4: OTrap (orbit trap coloring):
    OTrap = |TrapColor_Density * (term1 - term3 - term2)|

  Step 5: Cube distance (using rotated coordinates):
    cube_dist = max(|rx|, |ry|, |rz|) - Size

  Step 6: Shape DE:
    shape_de = max(cube_dist, gyroid_value) / Density
    if !Apply_scale_add: shape_de /= Scale

Parameters:
  [0] Size: double (default: 1.5) — cube half-size
  [1] Scale: double (default: 2) — IFS scale (.2Doubles)
  [2] Z_add: double (default: 0) — Z offset (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] Density: double (default: 6.28) — surface frequency (≈ 2*pi)
  [6] Apply_scale_add: int (default: 1)
  [7..15] GyRot: 9 singles (from .3SingleAngles, rotation matrix)
  [16] TrapColor_Density: double (default: 1) — orbit trap coloring scale

PVar layout (verified against x87 ASM offsets):
  Options (negative offsets):
  [PVar - 0x10] = Size (double)
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add (double)
  [PVar - 0x40] = X_add (double)
  [PVar - 0x48] = Density (double)
  [PVar - 0x4c] = Apply_scale_add (integer)
  [PVar - 0x50..-0x70] = Rotation matrix (9 singles, .3SingleAngles)
  [PVar - 0x78] = TrapColor_Density (double)

Code structure:
  0x00-0x40: Standard SSE2 IFS preamble
  0x40-0x45: Store to stack for x87
  0x4B: nop (alignment)
  0x4C-0x9E: 3x3 rotation (add edi,-0x2c; matrix multiply; add edi,+0x2c)
  0xA1-0xB9: Three fsincos for rx*D, ry*D, rz*D
  0xBB-0xBF: Three fmulp to form gyroid cross-products
  0xC1-0xCC: OTrap = |TrapColor * (term1 - term3 - term2)| → [esi+0x80]
  0xD2-0xD4: Gyroid sum = term1 + term2 + term3
  0xD6-0x104: Cube distance = max(|rx|, |ry|, |rz|)
  0x106: Subtract Size from cube distance
  0x109-0x116: max(cube_dist, gyroid) → shape_de
  0x118: Divide by Density
  0x11B-0x124: Scale correction + store

Key notes:
  - .3SingleAngles edi trick: add edi,-0x2c before, add edi,+0x2c after
  - fsincos produces both sin and cos in one instruction (x87 optimization)
  - Three fmulp create all three gyroid cross-products from 6 trig values
  - OTrap uses term DIFFERENCES for coloring (not the sum)
  - Cube intersection uses ROTATED coordinates (aligned with gyroid)
  - Density default 6.28 ≈ 2*pi gives standard gyroid wavelength
  - Division by Density normalizes the DE for the surface frequency
*/
