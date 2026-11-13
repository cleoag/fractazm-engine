// _helispiral.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _helispiral
// Hand-ported from ASM: 252 bytes, DEoption=-1

constexpr unsigned int FORMULA__HELISPIRAL = 10010;

// ASM trace:
//   esi = context, ebx = &x, edi = PVar
//   Params (from .3SingleAngles → 9 rotation matrix elements, Single):
//     Forward matrix M stored at: [edi-0x0c..0x2c] (3x3, column-major Single)
//     [edi-0x34]=Helix, [edi-0x3c]=Spiral (doubles)
//     [edi-0x40]=RotByC_S (int), [edi-0x44]=RotByC_H (int)
//     [edi-0x4c]=FixRot (double, degrees)
//   Constants: [edi+0x00]=deg_to_rad=0.017453293
//
//   Step 1: Forward rotation M * [x,y,z]
//     new_x = x*M[1,0] + y*M[1,1] + z*M[1,2]   (offsets -0x0c,-0x10,-0x14)
//     new_y = x*M[2,0] + y*M[2,1] + z*M[2,2]   (offsets -0x18,-0x1c,-0x20)
//     new_z = x*M[0,0] + y*M[0,1] + z*M[0,2]   (offsets -0x24,-0x28,-0x2c)
//
//   Step 2: Compute rotation angle
//     if RotByC_H: phi = Cz  else: phi = z
//     if RotByC_S: rho = sqrt(Cx²+Cy²)  else: rho = sqrt(x²+y²)
//     angle = FixRot * deg2rad + rho * Spiral + phi * Helix
//
//   Step 3: XY plane rotation by angle
//     x' = x*cos + y*sin
//     y' = y*cos - x*sin
//
//   Step 4: Inverse rotation M^T * [x',y',z]
//     (transpose of forward matrix)
//
// Helix + spiral deformation. NOTE: Rotation matrix from .3SingleAngles
// is pre-computed by engine. In Metal, the 9 matrix elements are passed
// as parameters.

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void HelispiralOriginal_x87(double*, const double*, int helixC, int spiralC);
#endif

template<>
cpu_double3 execute_formula<FORMULA__HELISPIRAL>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z};
    HelispiralOriginal_x87(state,params,int(params[11]),int(params[12]));
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E=numeric::Extended80;
    const E px(p.x),py(p.y),pz(p.z);
    // Both transforms and the XY rotation have explicit Double stores.
    const E x(static_cast<double>(px*E(params[0])+py*E(params[1])+pz*E(params[2])));
    const E y(static_cast<double>(pz*E(params[5])+(py*E(params[4])+px*E(params[3]))));
    const double storedZ=static_cast<double>(px*E(params[6])+py*E(params[7])+pz*E(params[8]));
    const E z(storedZ);
    const E a=int(params[12])==1 ? E(params[15]) : x;
    const E b=int(params[12])==1 ? E(params[16]) : y;
    const E h=int(params[11])==1 ? E(params[17]) : z;
    const E angle=sqrt(a*a+b*b)*E(params[10])+h*E(params[9])+E(params[13])*E(params[14]);
    // Here live X/Y follow the angle on the x87 stack; missing the FSINCOS
    // push makes the fixed rotation accesses underflow, hence all-NaN XYZ.
    if(abs(angle)>=E(0x1p63)) return cpu_double3(static_cast<double>(numeric::extendedNaN()));
    const auto trig=numeric::extendedSinCos(angle);
    const E nx(static_cast<double>(trig.cos*x+trig.sin*y));
    const E ny(static_cast<double>(y*trig.cos-x*trig.sin));
    return cpu_double3(static_cast<double>(nx*E(params[0])+ny*E(params[3])+z*E(params[6])),
                       static_cast<double>(z*E(params[7])+(ny*E(params[4])+nx*E(params[1]))),
                       static_cast<double>(nx*E(params[2])+ny*E(params[5])+z*E(params[8])));
#else
    // PVar-expanded layout for .3SingleAngles:
    // params[0..8]  = 3x3 rotation matrix
    // params[9]     = Helix
    // params[10]    = Spiral
    // params[11]    = RotByC (H)
    // params[12]    = RotByC (S)
    // params[13]    = FixRot (deg)
    const double M00 = params[0], M01 = params[1], M02 = params[2];
    const double M10 = params[3], M11 = params[4], M12 = params[5];
    const double M20 = params[6], M21 = params[7], M22 = params[8];

    const double Helix      = params[9];    // default: 0.03
    const double Spiral     = params[10];   // default: 0.03
    const int   RotByC_H    = int(params[11]);  // default: 0
    const int   RotByC_S    = int(params[12]);  // default: 0
    const double FixRot_deg = params[13];   // default: 0

    const double deg2rad = params[14];

    // C values for conditional usage
    const double Cx = params[15];
    const double Cy = params[16];
    const double Cz = params[17];

    double x = p.x, y = p.y, z = p.z;

    // Step 1: Forward rotation
    double rx = x * M00 + y * M01 + z * M02;
    double ry = x * M10 + y * M11 + z * M12;
    double rz = x * M20 + y * M21 + z * M22;
    x = rx; y = ry; z = rz;

    // Step 2: Compute rotation angle
    double rho, phi;
    if (RotByC_S == 1) {
        rho = sqrt(Cx * Cx + Cy * Cy);
    } else {
        rho = sqrt(x * x + y * y);
    }
    if (RotByC_H == 1) {
        phi = Cz;
    } else {
        phi = z;
    }
    double angle = FixRot_deg * deg2rad + rho * Spiral + phi * Helix;

    // Step 3: XY plane rotation
    double s = sin(angle);
    double c = cos(angle);
    double nx = x * c + y * s;
    double ny = y * c - x * s;
    x = nx; y = ny;

    // Step 4: Inverse rotation (transpose of M)
    double ix = x * M00 + y * M10 + z * M20;
    double iy = x * M01 + y * M11 + z * M21;
    double iz = x * M02 + y * M12 + z * M22;

    return cpu_double3(ix, iy, iz);
#endif
}

// Formula metadata
constant FormulaInfo formula_info__helispiral = {
    .id = FORMULA__HELISPIRAL,
    .name = "_helispiral",
    .param_count = 14,
    .complexity = 90,
    .uses_transcendental = true
};

/*
Original M3 Description:
1) Rotates the reference axis by selected angles
   NOTE: this will NOT affect C!
2) Rotates the XY plane on an angle:
   if RotByC_H: phi = Cz  else: phi = z
   if RotByC_S: rho = sqrt(Cx²+Cy²)  else: rho = sqrt(x²+y²)
   angle = FixRot*deg2rad + rho*Spiral + phi*Helix
   SinCos(angle, s, c)
   x' = x*c + y*s
   y' = y*c - x*s
3) Rotates the reference axis back (transposed rotation matrix)
   NOTE: this will NOT affect C!

This gives a helix + spiral deformation (if used as pretransform,
or else it gives strange results, you choose).

New version, June 21 2011 - Luca GN

Parameters:
  [0..8] Rotation matrix (9 Single values from .3SingleAngles)
  [9]  Helix: double (default: 0.03)
  [10] Spiral: double (default: 0.03)
  [11] RotByC (H): integer (default: 0)
  [12] RotByC (S): integer (default: 0)
  [13] FixRot (deg): double (default: 0)
  [14..16] Cx, Cy, Cz (from iteration context)
*/
