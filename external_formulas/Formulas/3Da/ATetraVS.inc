// ATetraVS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: ATetraVS
// Hand-ported from ASM (884 bytes, Capstone-verified)
// DEoption: 11 (analytic DE via w tracking)
// Extended API required: VaryScale (iteration state), J1/J2/J3 (C components)
//
// Amazing Box Vary Scale with tetrahedral/octahedral folding geometry.
// Uses rotated folding planes instead of axis-aligned folds.
// Four fold operations with rotation matrix manipulation per mode.

constexpr unsigned int FORMULA_ATETRAVS = 10187;

// ---------------------------------------------------------------
// Rotated fold helper (ASM subroutine at 0x0021)
// Takes [z, y, x] from FPU stack, applies:
//   1. Rotation: rotate [z,y,x] by matrix M
//   2. Fold: x-axis fold (either Tglad-style or one-sided abs)
//   3. Inverse rotation: rotate back by M^T (transposed = inverse for orthogonal)
//
// The fold is controlled by the "Tglad folding" parameter:
//   TgladFold == 1: x' = |x+Fold| - |x-Fold| - x  (symmetric TgladFold)
//   TgladFold == 0: x' = Fold - |x - Fold|          (one-sided fold)
    //     Equivalent form: if x > 0: x' = Fold - |x-Fold|
//     ASM: fsub Fold, fabs, fchs, fadd Fold -> Fold - |x - Fold|
// ---------------------------------------------------------------

static void rotated_fold(double &x, double &y, double &z,
                         double Fold, int tglad_mode,
                         // Forward rotation matrix (row-major)
                         double M00, double M01, double M02,
                         double M10, double M11, double M12,
                         double M20, double M21, double M22) {
    // Forward rotation: [rx, ry, rz] = M * [x, y, z]
    // ASM at 0x0021-0x005E:
    //   new_x = x*M[0,0] + y*M[0,1] + z*M[0,2]
    //   new_y = x*M[1,0] + y*M[1,1] + z*M[1,2]
    //   new_z = x*M[2,0] + y*M[2,1] + z*M[2,2]
    // But ASM uses stack [z, y, x] order, so from the actual offsets:
    //   The first computed component uses M entries from [ebp-0x24], [ebp-0x30], [ebp-0x3C]
    //   These map to column of the rotation matrix.
    // For simplicity, I'll use standard R*v multiplication.

    double rx = x * M00 + y * M01 + z * M02;
    double ry = x * M10 + y * M11 + z * M12;
    double rz = x * M20 + y * M21 + z * M22;

    // Fold on rx (first component after rotation)
    // ASM at 0x0060-0x0088:
    //   cmp TgladFold, 1
    //   if TgladFold != 1:
    //     rx' = Fold - |rx - Fold|   (one-sided fold, ASM: fsub, fabs, fchs, fadd)
    //   else (TgladFold == 1):
    //     rx' = |rx+Fold| - |rx-Fold| - rx  (TgladFold)
    if (tglad_mode == 1) {
        double new_rx = abs(rx + Fold) - abs(rx - Fold) - rx;
        rx = new_rx;
    } else {
        // One-sided: Fold - |rx - Fold|
        rx = Fold - abs(rx - Fold);
    }

    // Inverse rotation (using transposed matrix = M^T for orthogonal matrices)
    // ASM at 0x008A-0x00C3:
    //   The ASM reuses the same matrix entries but accesses them transposed.
    //   Specifically the second rotation block accesses:
    //     [ebp-0x24]=[M02], [ebp-0x30]=[M12], [ebp-0x3C]=[M22] for first output
    //   vs original:
    //     [ebp-0x34]=[M00], [ebp-0x38]=[M01], [ebp-0x3C]=[M02]
    //   So the inverse uses columns of M as rows = transpose.

    x = rx * M00 + ry * M10 + rz * M20;
    y = rx * M01 + ry * M11 + rz * M21;
    z = rx * M02 + ry * M12 + rz * M22;
}

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ATetraVSOriginal_x87(double* state, const double* params, const float* matrix, int* flags);
#endif

template<>
cpu_double3 execute_formula<FORMULA_ATETRAVS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x, p.y, p.z, de, params[DIFS_VARYSCALE_SLOT], params[16], params[17], params[18]};
    float matrix[9];
    for (int i = 0; i < 9; ++i) matrix[i] = float(params[5+i]);
    int flags[] = {int(params[14]), int(params[15]), int(params[BFIRSTIT_SLOT])};
    ATetraVSOriginal_x87(state, params, matrix, flags);
    de = state[3];
    params[DIFS_VARYSCALE_SLOT] = state[4];
    params[BFIRSTIT_SLOT] = flags[2];
    return cpu_double3(state[0], state[1], state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E = numeric::Extended80;
    double m[9];
    for (int i=0;i<9;++i) m[i]=static_cast<float>(params[5+i]);
    const E fold(params[3]);
    auto applyFold = [&] {
        const E x(p.x), y(p.y), z(p.z);
        // CODE 0021..00C7: the last matrix row is evaluated first, but
        // the first row is folded. Keep the original reverse-product sum
        // for Y and spill all XYZ only after the inverse transform.
        const E rz=x*E(m[6])+y*E(m[7])+z*E(m[8]);
        E rx=x*E(m[0])+y*E(m[1])+z*E(m[2]);
        const E ry=z*E(m[5])+(y*E(m[4])+x*E(m[3]));
        if (int(params[15])==1) rx=-(rx-(abs(rx+fold)-abs(rx-fold)));
        else rx=-abs(rx-fold)+fold;
        const E nz=rx*E(m[2])+ry*E(m[5])+rz*E(m[8]);
        const E nx=rx*E(m[0])+ry*E(m[3])+rz*E(m[6]);
        const E ny=rz*E(m[7])+(ry*E(m[4])+rx*E(m[1]));
        p=cpu_double3(static_cast<double>(nx),static_cast<double>(ny),static_cast<double>(nz));
    };
    auto flip = [&](int a,int b,int c,int d) {
        m[a]=-m[a];m[b]=-m[b];m[c]=-m[c];m[d]=-m[d];applyFold();
    };
    applyFold();
    const int mode=int(params[14]);
    if (mode==0) {flip(2,5,6,7);flip(1,5,3,7);flip(2,5,6,7);}
    else if (mode==1) {flip(1,5,3,7);flip(2,5,6,7);flip(1,5,3,7);}
    else if (mode==2) {flip(2,1,6,3);flip(1,5,3,7);flip(2,1,6,3);}
    else {flip(1,5,3,7);flip(2,1,6,3);flip(1,5,3,7);}
    E previous(params[DIFS_VARYSCALE_SLOT]);
    if(params[BFIRSTIT_SLOT]<=0) {previous=E(params[0]);params[BFIRSTIT_SLOT]+=1;}
    const double vary=static_cast<double>(previous+(abs(previous)-E(1))*E(params[4]));
    params[DIFS_VARYSCALE_SLOT]=vary;
    const E x(p.x),y(p.y),z(p.z),rr=z*z+y*y+x*x;
    const E scale=!(rr>=E(params[2])) ? E(params[1]) :
        (rr<=E(1) ? E(vary)/rr : E(vary));
    de=static_cast<double>(E(de)*scale);
    return cpu_double3(static_cast<double>(x*scale+E(params[16])),
                       static_cast<double>(y*scale+E(params[17])),
                       static_cast<double>(z*scale+E(params[18])));
#else
    const double Scale = params[0];           // [PVar - 0x10] = Scale
    const double ScaleOverMinR2 = params[1];  // BOXSCALE pre-computed: Scale/sqr(MinR)
    const double minR2 = params[2];            // BOXSCALE pre-computed: sqr(MinR)
    const double Fold = params[3];            // [PVar - 0x28] = Fold
    const double Scale_vary = params[4];      // [PVar - 0x30] = Scale vary

    // Rotation matrix from 9 pre-computed float elements (from .3SingleAngles)
    RotMatrix3 RM = rotMatrix3FromFloats(params, 5);
    const double M00 = RM.col0.x, M01 = RM.col1.x, M02 = RM.col2.x;  // row 0
    const double M10 = RM.col0.y, M11 = RM.col1.y, M12 = RM.col2.y;  // row 1
    const double M20 = RM.col0.z, M21 = RM.col1.z, M22 = RM.col2.z;  // row 2

    const int FoldMode = int(params[14]);     // [edi - 0x58] = Modes (0 to 3)
    const int TgladFold = int(params[15]);    // [edi - 0x5C] = Tglad folding

    // C components

    double x = p.x;
    double y = p.y;
    double z = p.z;

    // ---------------------------------------------------------------
    // ASM at 0x00CB-0x0101: Copy rotation matrix from PVar to local stack vars
    // Then call the first rotated fold with the original matrix.
    //
    // At 0x0101: call rotated_fold (initial fold with base matrix)
    //
    // Then based on FoldMode (0-3), apply 3 more rotated folds with
    // modified matrices (sign flips on certain elements).
    //
    // Mode 0 (default): fold sequence ABDC
    //   A: original matrix
    //   B: negate M02, M12, M20, M21 (corresponding to -45,-45,+45)
    //   D: negate M01, M12, M10, M21 from the B state
    //   C: negate M02, M12, M20, M21 from the D state
    //
    // Mode 1: fold sequence ACDB
    //   A: original
    //   C: negate M01, M12, M10, M21
    //   D: negate M02, M12, M20, M21 from the C state
    //   B: negate M01, M12, M10, M21 from the D state
    //
    // Mode 2: fold sequence ADBC
    //   A: original
    //   D: negate M02, M01, M20, M10
    //   B: negate M01, M12, M10, M21 from the D state
    //   C: negate M02, M01, M20, M10 from the B state
    //
    // Mode 3 (default else): fold sequence ACBD
    //   A: original
    //   C: negate M01, M12, M10, M21
    //   B: negate M02, M01, M20, M10 from the C state
    //   D: negate M01, M12, M10, M21 from the B state
    //
    // Each negation corresponds to changing reflection plane orientations,
    // creating tetrahedral symmetry from the combined 4 folds.
    // ---------------------------------------------------------------

    // Apply first fold with base matrix
    rotated_fold(x, y, z, Fold, TgladFold, M00, M01, M02, M10, M11, M12, M20, M21, M22);

    // The remaining 3 folds use negated matrix elements.
    // Pattern B: negate M02, M12, M20, M21
    // Pattern C: negate M01, M12, M10, M21

    if (FoldMode == 0) {
        // B fold: negate M02, M12, M20, M21
        rotated_fold(x, y, z, Fold, TgladFold,
                     M00, M01, -M02, M10, M11, -M12, -M20, -M21, M22);
        // D fold: from the B state, negate M01, M12, M10, M21
        rotated_fold(x, y, z, Fold, TgladFold,
                     M00, -M01, -M02, -M10, M11, M12, -M20, M21, M22);
        // C fold: from the D state, negate M02, M12, M20, M21
        rotated_fold(x, y, z, Fold, TgladFold,
                     M00, -M01, M02, -M10, M11, -M12, M20, -M21, M22);
    } else if (FoldMode == 1) {
        // C fold first
        rotated_fold(x, y, z, Fold, TgladFold,
                     M00, -M01, M02, -M10, M11, -M12, M20, -M21, M22);
        // D fold: from the C state, negate M02, M12, M20, M21
        rotated_fold(x, y, z, Fold, TgladFold,
                     M00, -M01, -M02, -M10, M11, M12, -M20, M21, M22);
        // B fold: from the D state, negate M01, M12, M10, M21
        rotated_fold(x, y, z, Fold, TgladFold,
                     M00, M01, -M02, M10, M11, -M12, -M20, -M21, M22);
    } else if (FoldMode == 2) {
        // D fold: negate M02, M01, M20, M10
        rotated_fold(x, y, z, Fold, TgladFold,
                     M00, -M01, -M02, -M10, M11, M12, -M20, M21, M22);
        // B fold: from the D state, negate M01, M12, M10, M21
        rotated_fold(x, y, z, Fold, TgladFold,
                     M00, M01, -M02, M10, M11, -M12, -M20, -M21, M22);
        // C fold: from the B state, negate M02, M01, M20, M10
        rotated_fold(x, y, z, Fold, TgladFold,
                     M00, -M01, M02, -M10, M11, -M12, M20, -M21, M22);
    } else {
        // Mode 3 (default else): ACBD order
        // C fold
        rotated_fold(x, y, z, Fold, TgladFold,
                     M00, -M01, M02, -M10, M11, -M12, M20, -M21, M22);
        // B fold: from the C state, negate M02, M01, M20, M10
        rotated_fold(x, y, z, Fold, TgladFold,
                     M00, M01, -M02, M10, M11, -M12, -M20, -M21, M22);
        // D fold: from the B state, negate M01, M12, M10, M21
        rotated_fold(x, y, z, Fold, TgladFold,
                     M00, -M01, -M02, -M10, M11, M12, -M20, M21, M22);
    }

    // ---------------------------------------------------------------
    // VaryScale tracking (ASM at 0x02E8-0x0307)
    // ---------------------------------------------------------------
        double prevVaryScale = params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0.0) {
        prevVaryScale = Scale;
        params[BFIRSTIT_SLOT] += 1.0;
    }
    double VaryScale = prevVaryScale + Scale_vary * (abs(prevVaryScale) - 1.0);
    params[DIFS_VARYSCALE_SLOT] = VaryScale;  // write back for wrapper

    // ---------------------------------------------------------------
    // Sphere fold (ASM at 0x030C-0x034A)
    // rr = x*x + y*y + z*z
    // Standard BOXSCALE sphere fold
    // ---------------------------------------------------------------
    double rr = x * x + y * y + z * z;

    double m;
    if (rr < minR2) {
        m = ScaleOverMinR2;
    } else if (rr < 1.0) {
        m = VaryScale / rr;
    } else {
        m = VaryScale;
    }

    // DE tracking
    de = de * m;

    // Apply scaling + C addition
    // ASM at 0x0358-0x0365:
    //   z*m + J3 -> [ecx]
    //   y*m + J2 -> [edx]
    //   x*m + J1 -> [ebx]
    // Note: esi-0x58 = J3, esi-0x60 = J2, esi-0x68 = J1
    cpu_double3 result;
    result.z = z * m + params[18];
    result.y = y * m + params[17];
    result.x = x * m + params[16];
    return result;
#endif
}

// Alternative direct function implementation
cpu_double3 ATetraVS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_ATETRAVS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_ATetraVS = {
    .id = FORMULA_ATETRAVS,
    .name = "ATetraVS",
    .param_count = 9,
    .complexity = 1200,
    .uses_transcendental = false
};

/*
Original M3 Description:
Variation of the Amazing Box Vary Scale, based on some
inclined folding planes. The shapes resemble a tetra/octahedral
geometry.

Formula (when Tglad folding is ON):
 RotatedFolding(+45,+45,+45,Fold)
 if (New_mode != 1)
   RotatedFolding(-45,-45,+45,Fold)
   RotatedFolding(-45,+45,-45,Fold)
   RotatedFolding(+45,-45,-45,Fold)
 else
   RotatedFolding(+45,-45,-45,Fold)
   RotatedFolding(-45,+45,-45,Fold)
   RotatedFolding(-45,-45,+45,Fold)
 endif

The non-Tglad folding rule:
 x'=1-abs(x-1) // folds only the positive side (tetrahedral shapes)

Parameters:
  [0] Scale: double (default: sqrt(3) ~ 1.732)
  [1] ScaleOverMinR2: BOXSCALE precomputed (MinR default: 2/3)
  [2] minR2: BOXSCALE precomputed
  [3] Fold: double (default: sqrt(3)/2 ~ 0.866)
  [4] Scale_vary: double (default: 0)
  [5-7] Rotation angles: 3 Euler angles in degrees (from .3SingleAngles, default: 45)
  [8] FoldMode: integer (default: 0) - controls fold order (0-3)
  [9] TgladFold: integer (default: 0) - 0=one-sided, 1=TgladFold (octahedral)
  [10] Cx (J1): from TIteration3D
  [11] Cy (J2): from TIteration3D
  [12] Cz (J3): from TIteration3D

NOTES:
  - 4 rotated folds create tetrahedral/octahedral symmetry
  - Default angles (45) produce standard tetra geometry
  - FoldMode changes the ORDER of the 3 additional folds
  - TgladFold=0 gives tetrahedral shapes, TgladFold=1 gives octahedral
  - Thanks to Aexion for innovative ideas; Luca GN '11
*/
