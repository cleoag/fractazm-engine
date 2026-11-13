// _kamtor.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _kamtor
// Hand-ported from ASM: 352 bytes, DEoption=-1
// KAM torus - customizable dynamic system using polynomial functions
// instead of sin/cos. z acts as the angle parameter; each z-slice
// shows a different 2D "KAM torus" variation.

constexpr unsigned int FORMULA__KAMTOR = 268;

// ASM trace (352 bytes):
//   Step 1: Update z as angle parameter
//     z' = Soft1*z + Soft2*sqrt(x² + y²)
//
//   Step 2: Compute cos/sin analogs (t1, t2) from z'
//     if |z'| < 1:  t1 = z',     t2 = sqrt(1 - z'²)
//     if |z'| >= 1: t1 = 1/z',   t2 = sqrt(1 - (1/z')²)
//     (ensures t1² + t2² = 1 when |z'| < 1; approximate otherwise)
//
//   Step 3: Evaluate two generalized polynomials in x, y
//     poly1 = A1*x + B1*y + C1*x² + D1*y² + E1*x*y + F1*sqrt(x²+y²)
//     poly2 = A2*x + B2*y + C2*x² + D2*y² + E2*x*y + F2*sqrt(x²+y²)
//
//   Step 4: Rotation-like combination
//     x' = poly1*t1 + poly2*t2
//     y' = poly1*t2 + poly2*t1
//
//   With defaults (A1=1, B2=-1, C2=1, others=0):
//     poly1 = x, poly2 = x²-y → standard KAM torus
//
// C handling: DEoption=-1 (transform, no C addition)

template<>
cpu_double3 execute_formula<FORMULA__KAMTOR>(cpu_double3 p, constant double* params, double &de) {
    const double Soft1 = params[0];   // [PVar-0x10] (default: 1.05)
    const double Soft2 = params[1];   // [PVar-0x18] (default: -0.05)
    const double A1    = params[2];   // [PVar-0x20] (default: 1)
    const double B1    = params[3];   // [PVar-0x28] (default: 0)
    const double C1    = params[4];   // [PVar-0x30] (default: 0)
    const double D1    = params[5];   // [PVar-0x38] (default: 0)
    const double E1    = params[6];   // [PVar-0x40] (default: 0)
    const double F1    = params[7];   // [PVar-0x48] (default: 0)
    const double A2    = params[8];   // [PVar-0x50] (default: 0)
    const double B2    = params[9];   // [PVar-0x58] (default: -1)
    const double C2    = params[10];  // [PVar-0x60] (default: 1)
    const double D2    = params[11];  // [PVar-0x68] (default: 0)
    const double E2    = params[12];  // [PVar-0x70] (default: 0)
    const double F2    = params[13];  // [PVar-0x78] (default: 0)

    double x = p.x, y = p.y, z = p.z;

    // Step 1: Update z (angle parameter)
    double r = sqrt(x * x + y * y);
    z = Soft1 * z + Soft2 * r;

    // Step 2: Compute cos/sin analogs
    double t1, t2;
    double az = fabs(z);
    if (az < 1.0) {
        t1 = z;
        t2 = sqrt(1.0 - z * z);
    } else {
        t1 = 1.0 / z;
        t2 = sqrt(1.0 - t1 * t1);
    }

    // Step 3: Evaluate generalized polynomials (using ORIGINAL x, y)
    double poly1 = A1 * x + B1 * y + C1 * x * x + D1 * y * y
                + E1 * x * y + F1 * r;
    double poly2 = A2 * x + B2 * y + C2 * x * x + D2 * y * y
                + E2 * x * y + F2 * r;

    // Step 4: Rotation-like combination
    double new_x = poly1 * t1 + poly2 * t2;
    double new_y = poly1 * t2 + poly2 * t1;

    return cpu_double3(new_x, new_y, z);
}

// Formula metadata
constant FormulaInfo formula_info__kamtor = {
    .id = FORMULA__KAMTOR,
    .name = "_kamtor",
    .param_count = 14,
    .complexity = 352,
    .uses_transcendental = false
};

/*
Original M3 Description:
KAM torus system customizable "dynamic" variation in 3D.
From UF database, modified version. Very odd shape, an interesting
"Henon-like" attractor. Instead of sine and cosine, uses only poly
functions. z is the old angle parameter. Each section at z=constant
shows a different "KAM torus" 2D variation.
Soft params control system dynamism. Formula by Luca GN 2011.

Mathematical Formula:
z' = Soft1*z + Soft2*sqrt(x²+y²)

if |z'| < 1:
  t1 = z', t2 = sqrt(1 - z'²)     (exact unit-circle projection)
else:
  t1 = 1/z', t2 = sqrt(1 - 1/z'²) (reciprocal mapping)

poly1 = A1*x + B1*y + C1*x² + D1*y² + E1*xy + F1*sqrt(x²+y²)
poly2 = A2*x + B2*y + C2*x² + D2*y² + E2*xy + F2*sqrt(x²+y²)

x' = poly1*t1 + poly2*t2  (cos-like rotation)
y' = poly1*t2 + poly2*t1  (sin-like rotation)

With defaults: poly1=x, poly2=x²-y → standard KAM torus:
  x' = x*cos(θ) + (x²-y)*sin(θ)
  y' = x*sin(θ) + (x²-y)*cos(θ)

Parameters:
  [0] Soft1: double (default: 1.05) - z feedback
  [1] Soft2: double (default: -0.05) - radial feedback to z
  [2] A1: double (default: 1) - linear x in poly1
  [3] B1: double (default: 0) - linear y in poly1
  [4] C1: double (default: 0) - x² in poly1
  [5] D1: double (default: 0) - y² in poly1
  [6] E1: double (default: 0) - xy in poly1
  [7] F1: double (default: 0) - r in poly1
  [8] A2: double (default: 0) - linear x in poly2
  [9] B2: double (default: -1) - linear y in poly2
  [10] C2: double (default: 1) - x² in poly2
  [11] D2: double (default: 0) - y² in poly2
  [12] E2: double (default: 0) - xy in poly2
  [13] F2: double (default: 0) - r in poly2
*/
