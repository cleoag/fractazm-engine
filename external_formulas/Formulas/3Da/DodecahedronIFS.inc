// DodecahedronIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: DodecahedronIFS
// Hand-ported from ASM: 387 bytes, DEoption=2
// Dodecahedral IFS: R1 rotation with abs -> 3x plane reflection -> R2 rotation -> Scale*DE -> offsets

constexpr unsigned int FORMULA_DODECAHEDRONIFS = 10041;

// ASM trace summary:
//   000D-0054: load z,y,x -> R1 rotation with fabs after each row
//              row_i = x*R1[i,0] + y*R1[i,1] + z*R1[i,2]; row_i = |row_i|
//              Result: st0=|row0|, st1=|row1|, st2=|row2| (reordered via fxch)
//   0056-0092: 1st plane reflection: dot=v.n; if dot<0: v -= 2*dot*n; then abs(v0,v2)
//   0096-00D2: 2nd plane reflection (identical)
//   00D6-0112: 3rd plane reflection (identical)
//   0114-014B: R2 rotation on (mid, max, min) from reflected values
//   014D-0173: Scale, de*=Scale, xyz*=Scale; k=Scale-1; uniform offsets (all fsubp)
//
// Constants define the dodecahedral symmetry plane normal:
//   C0 = 0.309016994374947  = (sqrt(5)-1)/4
//   C1 = -0.5
//   C2 = -0.809016994374947 = -(1+sqrt(5))/4

template<>
cpu_double3 execute_formula<FORMULA_DODECAHEDRONIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Scale   = params[0];
    const double CScaleX = params[1];
    const double CScaleY = params[2];
    const double CScaleZ = params[3];

    // Constants (dodecahedral symmetry plane normal)
    const double C0 = 0.309016994374947;   // (sqrt(5)-1)/4
    const double C1 = -0.5;
    const double C2 = -0.809016994374947;  // -(1+sqrt(5))/4

    // Rotation1 from 3 Euler angles (degrees)
    RotMatrix3 R1 = rotMatrix3FromFloats(params, 4);

    // Rotation2 from 3 Euler angles (degrees)
    RotMatrix3 R2 = rotMatrix3FromFloats(params, 13);

    // Step 1: R1 rotation with abs after each row
    cpu_double3 r1v = R1 * p;
    double row0 = abs(r1v.x);
    double row1 = abs(r1v.y);
    double row2 = abs(r1v.z);

    double x = row0;
    double y = row1;
    double z = row2;

    // 3 iterations of: plane reflection + abs(y,z)
    for (int i = 0; i < 3; i++) {
        double dot = x * C0 + y * C1 + z * C2;
        if (dot < 0.0) {
            double d2 = 2.0 * dot;
            x -= d2 * C0;
            y -= d2 * C1;
            z -= d2 * C2;
        }
        y = abs(y);
        z = abs(z);
    }

    cpu_double3 r2v = R2 * cpu_double3(x, y, z);
    double rot0 = r2v.x;
    double rot1 = r2v.y;
    double rot2 = r2v.z;

    // Step 4: Scale + DE tracking
    de *= Scale;
    rot0 *= Scale;
    rot1 *= Scale;
    rot2 *= Scale;

    // Step 5: Uniform offsets
    // k = Scale - 1
    double k = Scale - 1.0;
    rot0 -= k * CScaleX;
    rot1 -= k * CScaleY;
    rot2 -= k * CScaleZ;

    return cpu_double3(rot0, rot1, rot2);
}

// Formula metadata
constant FormulaInfo formula_info_DodecahedronIFS = {
    .id = FORMULA_DODECAHEDRONIFS,
    .name = "DodecahedronIFS",
    .param_count = 10,
    .complexity = 1300,
    .uses_transcendental = true
};

/*
Original M3 Description:
Fast implementation - Hack by blob.

Dodecahedral IFS using plane reflections with golden-ratio normal vectors.

Mathematical Formula:
  1. R1 rotation with abs() after each row product
  2. 3x plane reflection: if (v.n < 0) v -= 2*(v.n)*n, then abs(v0,v2)
     where n = (0.309017, -0.5, -0.809017) is the dodecahedral normal
  3. R2 rotation
  4. Scale + DE tracking
  5. Uniform offsets: xyz -= (Scale-1)*CScale

Parameters:
  [0] Scale: double (default: 2.63)
  [1] CScale_X: double (default: 0.809016994374947)
  [2] CScale_Y: double (default: 0)
  [3] CScale_Z: double (default: 0.309016994374947)
  [4..6] Rotation1: 3 angles in degrees (default: 0)
  [7..9] Rotation2: 3 angles in degrees (default: 0)

Constants (embedded in code):
  [PVar+0x00] = 0.309016994374947  (C0)
  [PVar+0x08] = -0.5               (C1)
  [PVar+0x10] = -0.809016994374947 (C2)

PVar layout:
  [PVar-0x10] = Scale
  [PVar-0x18] = CScaleX
  [PVar-0x20] = CScaleY
  [PVar-0x28] = CScaleZ
  [PVar-0x2C..0x4C] = R1 matrix (9 floats)
  [PVar-0x50..0x70] = R2 matrix (9 floats)
*/
