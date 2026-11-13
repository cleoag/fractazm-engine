// _BRotateToMag.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _BRotateToMag
// Hand-ported from ASM: 336 bytes, DEoption=-1, SIpower=2

constexpr unsigned int FORMULA__BROTATETOMAG = 7047;

// ASM trace:
//   Constants: [edi+0]=sqrt(2/3)=a, [edi+8]=sqrt(1/3)=b, [edi+16]=sqrt(1/2)=c
//
//   DD03/DC0           x*a
//   DD01/DC4F08/D9E0    z*b; negate
//   DEC1/DC4F10         (x*a - z*b) * c = tx → local
//   DD5DF8
//
//   DD02/DC4F10/DD1A    y' = y*c (stored temp to [edx])
//
//   DD03/DC4F08         x*b
//   DD01/DC0/DEC1      z*a; x*b + z*a
//   DD19                store z'
//
//   DD45F8/DC22/DD1B    x' = tx - ty (ty from [edx])
//   DD45F8/DC02/DD1A    y' = tx + ty
//
// Rotates x-axis to magnitude axis. Inverse of _BRotateFromMag.

template<>
cpu_double3 execute_formula<FORMULA__BROTATETOMAG>(cpu_double3 p, constant double* params, double &de) {
    const double a = 0.816496580927726;   // sqrt(2/3)
    const double b = 0.5773502691896258;   // sqrt(1/3)
    const double c = 0.7071067811865475;   // sqrt(1/2)

    double tx = (p.x * a - p.z * b) * c;
    double ty = p.y * c;

    double new_x = tx - ty;
    double new_y = tx + ty;
    double new_z = p.x * b + p.z * a;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__BRotateToMag = {
    .id = FORMULA__BROTATETOMAG,
    .name = "_BRotateToMag",
    .param_count = 0,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
Rotates so that x-axis (-1,0,0)→(1,0,0) lies on magnitude axis (-1,-1,-1)→(1,1,1).
This is the inverse of _BRotateFromMag.

tx = (x*sqrt(2/3) - z*sqrt(1/3)) * sqrt(1/2)
ty = y * sqrt(1/2)
z' = x*sqrt(1/3) + z*sqrt(2/3)
x' = tx - ty
y' = tx + ty

By M. Benesi.
Parameters: none (uses pre-stored constants)
*/
