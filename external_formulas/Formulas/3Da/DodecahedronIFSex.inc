// DodecahedronIFSex.inc — Canonical double-precision formula
// The immutable M3F machine code is the parity reference.

// Metal Template for Formula: DodecahedronIFSex
// Hand-ported from ASM: 530 bytes, DEoption=2
// Extended Dodecahedron IFS with two rotation matrices and selectable rotation style.
// Based on Fragmentarium's DodecahedronIFS with Knighty's algorithm.

constexpr unsigned int FORMULA_DODECAHEDRONIFS_EX = 10048;

// ASM trace summary:
//   Internal subroutines:
//     init_normal (0x5A): Compute normalized n1 = (Phi-1, 1/(Phi-1), -1)/|n|
//     reflect_fold (0x1C): If dot(p, n1) < 0 then p -= 2*dot*n1
//     flip3D_and_reflect (0x107): Cycle n1 components (x,y,z)->(z,x,y) then reflect
//     rotation2 (0x11): Apply second 3x3 rotation matrix
//   Main flow:
//     1. Apply Rotation1 (3x3 matrix)
//     2. Init normal vector from Phi parameter
//     3. reflect_fold (first fold)
//     4. If Rot2_style == 0: apply Rotation2
//     5. For i = 0..5: flip3D + reflect; if Rot2_style == i+1: apply Rotation2
//     6. Scale all by Scale, DE *= Scale
//     7. Subtract offset * (Scale-1)
//     8. If Rot2_style == 7: apply Rotation2

// Helper: reflect across plane with normal n (if dot product negative)
// ASM at 0x1C: compute d = dot(p, n); ftst; jbe 0x3B (fold if d <= 0)
// Fold: fadd st(0) = 2*d; fchs = -2*d (positive since d<=0)
//   x += -2*d * nx; y += -2*d * ny; z += -2*d * nz
static inline void reflect_fold(double &x, double &y, double &z,
                                 double nx, double ny, double nz) {
    double d = x * nx + y * ny + z * nz;
    if (d < 0.0) {
        double t = -2.0 * d;  // t > 0 since d < 0
        x += t * nx;
        y += t * ny;
        z += t * nz;
    }
}

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void DodecahedronIFSexOriginal_x87(double*, const double*, int);
#endif

template<>
cpu_double3 execute_formula<FORMULA_DODECAHEDRONIFS_EX>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    // Preserve x87 dot products, unordered folds, matrix stores and the
    // original crossed CScale X/Y offsets. Expanded Singles are exact doubles.
    double state[7] = {p.x,p.y,p.z,de};
    DodecahedronIFSexOriginal_x87(state,params,int(params[23]));
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#else
    // Portable fallback: not certified by native original-code fixtures.
    // Parameters from M3:
    //   .Double Scale = 2.63             -> params[0]
    //   .Double CScale X = 1             -> params[1]
    //   .Double CScale Y = 1             -> params[2]
    //   .Double CScale Z = 1             -> params[3]
    //   .3SingleAngles Rotation1 = 0     -> params[4..6] (3 angles in degrees)
    //   .3SingleAngles Rotation2 = 0     -> params[7..9] (3 angles in degrees)
    //   .Double Phi = 1.618              -> params[10]
    //   .Integer Rot2 style (0-7) = 7    -> params[11]

    const double Scale = params[0];
    const double CScaleX = params[1];
    const double CScaleY = params[2];
    const double CScaleZ = params[3];

    // Rotation1 from 9 pre-computed float elements (from .3SingleAngles)
    RotMatrix3 R1 = rotMatrix3FromFloats(params, 4);

    // Rotation2 from 9 pre-computed float elements (from .3SingleAngles)
    RotMatrix3 R2 = rotMatrix3FromFloats(params, 13);

    const double Phi = params[22];
    const int Rot2Style = (int)params[23];

    double x = p.x;
    double y = p.y;
    double z = p.z;

    // Step 1: Apply Rotation1 matrix
    {
        cpu_double3 r1v = R1 * cpu_double3(x, y, z);
        x = r1v.x;
        y = r1v.y;
        z = r1v.z;
    }

    // Step 2: Compute normalized normal vector n1 from Phi
    // n1 = normalize(Phi-1, 1/(Phi-1), -1)
    double n1x = -(1.0 - Phi);  // = Phi - 1
    double n1y = 1.0 / n1x;     // = 1 / (Phi - 1)
    double n1z = -1.0;
    double nlen = sqrt(n1x * n1x + n1y * n1y + n1z * n1z);
    double inv_nlen = 1.0 / nlen;
    n1x *= inv_nlen;
    n1y *= inv_nlen;
    n1z *= inv_nlen;

    // Step 3: First reflect fold
    reflect_fold(x, y, z, n1x, n1y, n1z);

    // Step 4: If Rot2_style == 0, apply Rotation2
    if (Rot2Style == 0) {
        cpu_double3 t = R2 * cpu_double3(y, x, z);
        x = t.y;
        y = t.x;
        z = t.z;
    }

    // Step 5: fold loop - 6 iterations of flip3D + reflect, with optional Rotation2
    for (int i = 0; i < 6; i++) {
        // flip3D (0x107): cycle normal components (x,y,z) -> (z,x,y)
        // ASM stores: n1.x=n1.z, n1.y=n1.x, n1.z=n1.y (old values)
        double tmp = n1x;
        n1x = n1z;   // new_x = old_z
        n1z = n1y;   // new_z = old_y
        n1y = tmp;   // new_y = old_x

        // reflect_fold
        reflect_fold(x, y, z, n1x, n1y, n1z);

        // If Rot2_style == i+1, apply Rotation2
        if (Rot2Style == i + 1) {
            cpu_double3 t = R2 * cpu_double3(y, x, z);
            x = t.y;
            y = t.x;
            z = t.z;
        }
    }

    // Step 6: Scale and DE tracking
    double k = Scale - 1.0;
    de *= Scale;
    double new_x = x * Scale - k * CScaleY;
    double new_y = y * Scale - k * CScaleX;
    double new_z = z * Scale - k * CScaleZ;
    x = new_x; y = new_y; z = new_z;

    // Step 7: If Rot2_style == 7, apply Rotation2
    if (Rot2Style == 7) {
        cpu_double3 t = R2 * cpu_double3(y, x, z);
        x = t.y;
        y = t.x;
        z = t.z;
    }

    // NOTE: This formula does NOT add C internally. Engine adds it.
    return cpu_double3(x, y, z);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_DodecahedronIFSex = {
    .id = FORMULA_DODECAHEDRONIFS_EX,
    .name = "DodecahedronIFSex",
    .param_count = 12,
    .complexity = 530,
    .uses_transcendental = true  // sqrt in normal vector computation
};

/*
Original M3 Description:
This formula comes from Fragmentarium's default examples. Again thanks to
Syntopia and of course Knighty.
Btw, this is a lot more flexible, because you can choose between 8 different
rotation modes (for rot2), and choose the offsets (CScales).

 // Snippet from Frag's script with my tweaks; (z is a vector with x,y,z)
 z *= Rot1;
 for (n2=0; n2<7; n2++) {
   z -= 2.0 * min(0.0, dot(z, n1)) * n1;
   if style==n2 { z *= Rot2; }
   flip3D(n1);
 }
 z = +z*Scale - offset*(Scale-1.0);
 if style==7 { z *= Rot2; }

Parameters:
  [0] Scale: double (default: 2.63)
  [1] CScale X: double (default: 1)
  [2] CScale Y: double (default: 1)
  [3] CScale Z: double (default: 1)
  [4..6] Rotation1 angles (degrees, from .3SingleAngles)
  [7..9] Rotation2 angles (degrees, from .3SingleAngles)
  [10] Phi: double (default: 1.618)
  [11] Rot2 style: integer (default: 7, range 0-7)

PVar offset layout (edi = PVar):
  [edi-0x10] = Scale
  [edi-0x18] = CScale X
  [edi-0x20] = CScale Y
  [edi-0x28] = CScale Z
  [edi-0x2c..-0x4c] = Rotation1 matrix (9 singles)
  [edi-0x50..-0x70] = Rotation2 matrix (9 singles)
  [edi-0x78] = Phi
  [edi-0x7c] = Rot2 style (integer)
*/
