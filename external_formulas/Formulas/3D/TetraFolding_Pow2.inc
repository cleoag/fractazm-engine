// TetraFolding Pow2: the immutable M3F machine code is the parity reference.

// Metal Template for Formula: TetraFolding Pow2
// Hand-ported from ASM: 1062 bytes, DEoption=0 (Version=2, SIpower=2)
// Combination of ATetra's folding and Quadratic3D formulas
// Based on Mandelbulb3D formula system

constexpr unsigned int FORMULA_TETRAFOLDINGPOW2 = 10319;

// Native path preserves original x87 order, Single matrix spills and internal C.
// Portable fallback is not certified by the native oracle evidence.

// Helper: apply fold to a value
inline double tetra_fold(double val, double fold, int tglad) {
    if (tglad != 1) {
        // Non-Tglad: box fold
        // ASM: fsub fold, fabs, fchs, fadd fold
        // = Fold - |val - Fold|
        return fold - fabs(val - fold);
    } else {
        // Tglad fold:
        // ASM: |val+fold| - |val-fold| - val
        return fabs(val + fold) - fabs(val - fold) - val;
    }
}

// Helper: forward rotation (x,y,z) -> (R0, R1, R2)
// Using matrix [a,b,c; d,e,f; g,h,i]
inline cpu_double3 forward_rotate(double x, double y, double z,
                             double a, double b, double c,
                             double d, double e, double f,
                             double g, double h, double i) {
    double R0 = a*x + b*y + c*z;
    double R1 = d*x + e*y + f*z;
    double R2 = g*x + h*y + i*z;
    return cpu_double3(R0, R1, R2);
}

// Helper: inverse rotation (transpose) (R0, R1, R2) -> (x', y', z')
inline cpu_double3 inverse_rotate(double R0, double R1, double R2,
                              double a, double b, double c,
                              double d, double e, double f,
                              double g, double h, double i) {
    double xp = a*R0 + d*R1 + g*R2;
    double yp = b*R0 + e*R1 + h*R2;
    double zp = c*R0 + f*R1 + i*R2;
    return cpu_double3(xp, yp, zp);
}

// Subroutine equivalent of ASM 0x21:
// Rotate, fold R0, inverse rotate
inline cpu_double3 fold_rotate(double x, double y, double z,
                          double a, double b, double c,
                          double d, double e, double f,
                          double g, double h, double i,
                          double fold_val, int tglad) {
    cpu_double3 R = forward_rotate(x, y, z, a, b, c, d, e, f, g, h, i);
    R.x = tetra_fold(R.x, fold_val, tglad);
    return inverse_rotate(R.x, R.y, R.z, a, b, c, d, e, f, g, h, i);
}

// Subroutine equivalent of ASM 0x37C:
// Rotate, fold R0, multiply R0 by zmul2, inverse rotate
inline cpu_double3 fold_rotate_scale(double x, double y, double z,
                                double a, double b, double c,
                                double d, double e, double f,
                                double g, double h, double i,
                                double fold_val, int tglad, double zmul2) {
    cpu_double3 R = forward_rotate(x, y, z, a, b, c, d, e, f, g, h, i);
    R.x = tetra_fold(R.x, fold_val, tglad);
    R.x *= zmul2;
    return inverse_rotate(R.x, R.y, R.z, a, b, c, d, e, f, g, h, i);
}

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void TetraFoldingPow2Original_x87(double*, const double*, int, int);
#endif

template<>
cpu_double3 execute_formula<FORMULA_TETRAFOLDINGPOW2>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    // Original subroutines round only at XYZ stores and Single matrix spills.
    double state[8] = {p.x,p.y,p.z};
    TetraFoldingPow2Original_x87(state,params,int(params[15]),int(params[16]));
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E = numeric::Extended80;
    double matrix[9];
    for (int i=0;i<9;++i) matrix[i]=static_cast<float>(params[6+i]);
    const E fold(params[5]);
    auto applyFold = [&](bool scaleFold) {
        const E x(p.x), y(p.y), z(p.z);
        // CODE 0021..00C7 and 037C..0425: keep the forward and inverse
        // products extended, including the reversed Y sums. Only the return
        // from each fold subroutine stores XYZ as Double.
        const E rz=x*E(matrix[6])+y*E(matrix[7])+z*E(matrix[8]);
        E rx=x*E(matrix[0])+y*E(matrix[1])+z*E(matrix[2]);
        const E ry=z*E(matrix[5])+(y*E(matrix[4])+x*E(matrix[3]));
        if (int(params[16])==1) rx=-(rx-(abs(rx+fold)-abs(rx-fold)));
        else rx=-abs(rx-fold)+fold;
        if (scaleFold) rx*=E(params[17]);
        const E nz=rx*E(matrix[2])+ry*E(matrix[5])+rz*E(matrix[8]);
        const E nx=rx*E(matrix[0])+ry*E(matrix[3])+rz*E(matrix[6]);
        const E ny=rz*E(matrix[7])+(ry*E(matrix[4])+rx*E(matrix[1]));
        p=cpu_double3(static_cast<double>(nx),static_cast<double>(ny),
                      static_cast<double>(nz));
    };
    auto flip = [&](int a,int b,int c,int d) {
        matrix[a]=-matrix[a]; matrix[b]=-matrix[b];
        matrix[c]=-matrix[c]; matrix[d]=-matrix[d];
        applyFold(false);
    };
    applyFold(true);
    const int mode=int(params[15]);
    if (mode==0) {flip(2,5,6,7);flip(1,5,3,7);flip(2,5,6,7);}
    else if (mode==1) {flip(1,5,3,7);flip(2,5,6,7);flip(1,5,3,7);}
    else if (mode==2) {flip(2,1,6,3);flip(1,5,3,7);flip(2,1,6,3);}
    else {flip(1,5,3,7);flip(2,1,6,3);flip(1,5,3,7);}

    const E x(p.x), y(p.y), z(p.z);
    // CODE 02EA..0358 adds HRadius to Y squared before adding X squared.
    // The zero/unordered branch returns C; a negative radius returns zero.
    const E radius=x*x+(y*y+E(params[3]));
    if (radius<E(0)) return cpu_double3(0,0,0);
    if (!(radius>E(0))) return cpu_double3(params[18],params[19],params[20]);
    const E factor=E(params[4]) + -(z*z/radius);
    const E nx=(x*x+-(y*y))*E(params[0])*factor+E(params[18]);
    const E ny=x*y*E(params[1])*factor+E(params[19]);
    const E nz=z*sqrt(radius)*E(params[2])+E(params[20]);
    return cpu_double3(static_cast<double>(nx),static_cast<double>(ny),
                      static_cast<double>(nz));
#else
    const double Xmul     = params[0];   // X mul, default 1
    const double Ymul     = params[1];   // Y mul, default 2
    const double Zmul     = params[2];   // Z mul, default -2
    const double HRadius  = params[3];   // HRadius, default 0
    const double ZRadius  = params[4];   // ZRadius, default 1
    const double Fold     = params[5];   // Fold, default 0.866025...
    // Rotation matrix from 9 pre-computed float elements (from .3SingleAngles)
    RotMatrix3 Rot = rotMatrix3FromFloats(params, 6);
    const int   Mode     = int(params[15]);   // Modes (0 to 3), default 0
    const int   Tglad    = int(params[16]);  // Tglad folding, default 0
    const double Zmul2    = params[17];  // Z mul 2, default 1

    // ASM copies the generated matrix into 9 single-precision stack slots and
    // then uses those singles for every fold/rotate multiply. Rounding here
    // materially reduces long-iteration drift against the x87 reference.
    double a = static_cast<float>(Rot.col0.x), b = static_cast<float>(Rot.col1.x), c = static_cast<float>(Rot.col2.x);
    double d = static_cast<float>(Rot.col0.y), e = static_cast<float>(Rot.col1.y), f = static_cast<float>(Rot.col2.y);
    double g = static_cast<float>(Rot.col0.z), h = static_cast<float>(Rot.col1.z), i_val = static_cast<float>(Rot.col2.z);

    double x = p.x;
    double y = p.y;
    double z = p.z;

    // --- Step 1: Initial fold+rotate with Z mul 2 scaling (call 0x37C) ---
    cpu_double3 pos = fold_rotate_scale(x, y, z, a, b, c, d, e, f, g, h, i_val,
                                    Fold, Tglad, Zmul2);
    x = pos.x; y = pos.y; z = pos.z;

    // --- Step 2: Mode-dependent fold sequences (3 calls to fold_rotate) ---
    // Each mode applies 3 fold operations with different matrix sign patterns.
    // The ASM negates specific matrix elements between calls.
    // We pass modified copies to each fold_rotate call.

    if (Mode == 0) {
        // Call 1: negate c, f, g, h
        pos = fold_rotate(x, y, z, a, b, -c, d, e, -f, -g, -h, i_val, Fold, Tglad);
        x = pos.x; y = pos.y; z = pos.z;

        // Call 2: from previous state {a,b,-c,d,e,-f,-g,-h,i}, negate b,f,d,h
        // → {a,-b,-c,-d,e,f,-g,h,i}
        pos = fold_rotate(x, y, z, a, -b, -c, -d, e, f, -g, h, i_val, Fold, Tglad);
        x = pos.x; y = pos.y; z = pos.z;

        // Call 3: from previous state, negate c,f,g,h
        // → {a,-b,c,-d,e,-f,g,-h,i}
        pos = fold_rotate(x, y, z, a, -b, c, -d, e, -f, g, -h, i_val, Fold, Tglad);
        x = pos.x; y = pos.y; z = pos.z;

    } else if (Mode == 1) {
        // Call 1: negate b, f, d, h
        pos = fold_rotate(x, y, z, a, -b, c, -d, e, -f, g, -h, i_val, Fold, Tglad);
        x = pos.x; y = pos.y; z = pos.z;

        // Call 2: from {a,-b,c,-d,e,-f,g,-h,i}, negate c,f,g,h
        // → {a,-b,-c,-d,e,f,-g,h,i}
        pos = fold_rotate(x, y, z, a, -b, -c, -d, e, f, -g, h, i_val, Fold, Tglad);
        x = pos.x; y = pos.y; z = pos.z;

        // Call 3: from previous, negate b,f,d,h
        // → {a,b,-c,d,e,-f,-g,-h,i}
        pos = fold_rotate(x, y, z, a, b, -c, d, e, -f, -g, -h, i_val, Fold, Tglad);
        x = pos.x; y = pos.y; z = pos.z;

    } else if (Mode == 2) {
        // Call 1: negate c, b, g, d
        pos = fold_rotate(x, y, z, a, -b, -c, -d, e, f, -g, h, i_val, Fold, Tglad);
        x = pos.x; y = pos.y; z = pos.z;

        // Call 2: from {a,-b,-c,-d,e,f,-g,h,i}, negate b,f,d,h
        // → {a,b,-c,d,e,-f,-g,-h,i}
        pos = fold_rotate(x, y, z, a, b, -c, d, e, -f, -g, -h, i_val, Fold, Tglad);
        x = pos.x; y = pos.y; z = pos.z;

        // Call 3: from previous, negate c,b,g,d
        // → {a,-b,c,-d,e,-f,g,-h,i} (wait, let me recalculate)
        // Previous: {a,b,-c,d,e,-f,-g,-h,i}
        // Negate c: -c → +c
        // Negate b: b → -b
        // Negate g: -g → +g
        // Negate d: d → -d
        // → {a,-b,c,-d,e,-f,g,-h,i}
        pos = fold_rotate(x, y, z, a, -b, c, -d, e, -f, g, -h, i_val, Fold, Tglad);
        x = pos.x; y = pos.y; z = pos.z;

    } else {
        // Mode 3 (default): negate b,f,d,h then negate c,b,g,d then negate b,f,d,h
        // Call 1: negate b, f, d, h
        pos = fold_rotate(x, y, z, a, -b, c, -d, e, -f, g, -h, i_val, Fold, Tglad);
        x = pos.x; y = pos.y; z = pos.z;

        // Call 2: from {a,-b,c,-d,e,-f,g,-h,i}, negate c,b,g,d
        // → {a,b,-c,d,e,-f,-g,-h,i}
        pos = fold_rotate(x, y, z, a, b, -c, d, e, -f, -g, -h, i_val, Fold, Tglad);
        x = pos.x; y = pos.y; z = pos.z;

        // Call 3: from previous, negate b,f,d,h
        // Previous: {a,b,-c,d,e,-f,-g,-h,i}
        // Negate b: b → -b
        // Negate f: -f → +f
        // Negate d: d → -d
        // Negate h: -h → +h
        // → {a,-b,-c,-d,e,f,-g,h,i}
        pos = fold_rotate(x, y, z, a, -b, -c, -d, e, f, -g, h, i_val, Fold, Tglad);
        x = pos.x; y = pos.y; z = pos.z;
    }

    // --- Step 3: Quadratic3D formula ---
    // ASM 0x2E8-0x367:
    // r2 = x^2 + y^2 + HRadius
    double r2 = x * x + y * y + HRadius;

    // Original unordered compare takes the equality branch for NaN as well.
    if (r2 < 0.0) return cpu_double3(0.0,0.0,0.0);
    if (!(r2 > 0.0)) return cpu_double3(params[18],params[19],params[20]);

    // Normal case (r2 > 0):
    // z_factor = ZRadius - z^2/r2
    double z_factor = ZRadius - (z * z) / r2;

    // new_x = (x^2 - y^2) * Xmul * z_factor
    // ASM: x^2, y^2, fchs, faddp → x^2-y^2, fmul Xmul, then * z_factor, + Cx
    double new_x = (x * x - y * y) * Xmul * z_factor;

    // new_y = x * y * Ymul * z_factor
    // ASM: x*y, fmul Ymul, then * z_factor, + Cy
    double new_y = x * y * Ymul * z_factor;

    // new_z = z * sqrt(r2) * Zmul
    // ASM: z, fld r2, fsqrt, fmulp, fmul Zmul, + Cz
    double new_z = z * sqrt(r2) * Zmul;

    return cpu_double3(new_x+params[18], new_y+params[19], new_z+params[20]);
#endif
}

// Alternative direct function
cpu_double3 TetraFoldingPow2_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_TETRAFOLDINGPOW2>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_TetraFoldingPow2 = {
    .id = FORMULA_TETRAFOLDINGPOW2,
    .name = "TetraFolding Pow2",
    .param_count = 12,
    .complexity = 1062,
    .uses_transcendental = true
};

