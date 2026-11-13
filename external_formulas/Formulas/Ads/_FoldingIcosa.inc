// _FoldingIcosa.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _FoldingIcosa
// Hand-ported from ASM: 290 bytes, DEoption=-1
// Icosahedral folding transform

constexpr unsigned int FORMULA__FOLDINGICOSA = 4734;

// ASM trace:
//   Phase 1: 3x3 rotation matrix applied to (x,y,z), then abs of each component
//     A = |x*m00 + y*m01 + z*m02|   (row 0: [PVar-0x24,-0x28,-0x2c])
//     B = |x*m10 + y*m11 + z*m12|   (row 1: [PVar-0x0c,-0x10,-0x14])
//     C = |x*m20 + y*m21 + z*m22|   (row 2: [PVar-0x18,-0x1c,-0x20])
//   Phase 2: rearrange stack: val0=B, val1=C, val2=A
//   Phase 3: three iterations of:
//     dot = val0*n0 + val1*n1 + val2*n2
//     if (dot < 0): reflect: val_i -= 2*dot*n_i
//     val1 = |val1|, val2 = |val2|   (val0 intentionally NOT abs'd)
//   Phase 4: output x=val0, y=val1, z=val2
//
// C handling: DEoption=-1 (transform), no C addition.

template<>
cpu_double3 execute_formula<FORMULA__FOLDINGICOSA>(cpu_double3 p, constant double* params, double &de) {
    // Rotation matrix built from 3 Euler angles (degrees)
    RotMatrix3 R = rotMatrix3FromFloats(params, 0);
    const double m00 = R.col0.x, m01 = R.col1.x, m02 = R.col2.x;  // row 0
    const double m10 = R.col0.y, m11 = R.col1.y, m12 = R.col2.y;  // row 1
    const double m20 = R.col0.z, m21 = R.col1.z, m22 = R.col2.z;  // row 2

    // [CONSTANTS] embedded as compile-time values:
    const double n0 = 0.309016994374947;   // [PVar+0x00]
    const double n1 = -0.5;               // [PVar+0x08]
    const double n2 = -0.809016994374947;  // [PVar+0x10]

    double x = p.x, y = p.y, z = p.z;

    // Phase 1: Apply rotation matrix with abs
    double A = fabs(x * m00 + y * m01 + z * m02);
    double B = fabs(x * m10 + y * m11 + z * m12);
    double C = fabs(x * m20 + y * m21 + z * m22);

    // Phase 2: Rearrange (from ASM stack ordering)
    double val0 = B;   // maps to output x
    double val1 = C;   // maps to output y
    double val2 = A;   // maps to output z

    // Phase 3: Three iterations of conditional plane reflection
    for (int i = 0; i < 3; i++) {
        double dot = val0 * n0 + val1 * n1 + val2 * n2;
        if (dot < 0.0) {
            double t = 2.0 * dot;
            val0 -= t * n0;
            val1 -= t * n1;
            val2 -= t * n2;
        }
        val1 = fabs(val1);
        val2 = fabs(val2);
        // val0 intentionally NOT abs'd (icosahedral symmetry)
    }

    return cpu_double3(val0, val1, val2);
}

// Formula metadata
constant FormulaInfo formula_info__FoldingIcosa = {
    .id = FORMULA__FOLDINGICOSA,
    .name = "_FoldingIcosa",
    .param_count = 3,
    .complexity = 290,
    .uses_transcendental = false
};

/*
Original M3 Description:
Icosahedral folding transform.

Applies a 3x3 rotation matrix (derived from 3 Euler angles), takes the
absolute value of each rotated component, then performs 3 iterations of
conditional reflection against a normal plane (with golden-ratio-related
defaults characteristic of icosahedral symmetry). After each reflection,
two of the three components are abs'd while the first is left signed.

Parameters:
  [0..2] Rotation angles (3 Euler angles in degrees, from .3SingleAngles)
  [3]  Double: double (default: 0.309016994374947) - normal x  [COMPILE-TIME CONSTANT]
  [4] Double: double (default: -0.5) - normal y  [COMPILE-TIME CONSTANT]
  [5] Double: double (default: -0.809016994374947) - normal z  [COMPILE-TIME CONSTANT]
*/
