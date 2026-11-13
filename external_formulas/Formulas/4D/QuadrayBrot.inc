// QuadrayBrot.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: QuadrayBrot
// Hand-ported from ASM: 169 bytes, DEoption=4 (4D formula)

constexpr unsigned int FORMULA_QUADRAYBROT = 1946;

// ASM trace (4D quadray Mandelbrot, Aexion/Luca GN):
//   ctx = |Cx + Cy + Cz|
//   cty = |-Cx - Cy + Cz|
//   ctz = |-Cx + Cy - Cz|
//   ctw = |Cx - Cy - Cz|
//   wz = 2*w*z
//   xy = 2*x*y
//   x' = x² - y² + wz + ctx
//   y' = y² - x² + wz + cty
//   z' = z² - w² + xy + ctz
//   w' = w² - z² + xy + ctw
//
// 4D formula: w at [ecx+8]. In 3D projection w=0 for first iteration only.
// Full 4D dynamics require w tracking across iterations.

template<>
cpu_double3 execute_formula<FORMULA_QUADRAYBROT>(cpu_double3 p, constant double* params, double &de) {
    // C constants come from iteration context in the original engine.
    // This formula has no user parameters (.param_count = 0); in standalone
    // validation these slots are zero-padded by the harness.
    const double Cx = params[0];
    const double Cy = params[1];
    const double Cz = params[2];

    double x = p.x, y = p.y, z = p.z;
    double w = de; // 4D component is carried via de across iterations

    // Quadray Julia constants
    double ctx = abs(Cx + Cy + Cz);
    double cty = abs(-Cx - Cy + Cz);
    double ctz = abs(-Cx + Cy - Cz);
    double ctw = abs(Cx - Cy - Cz);

    // Cross products
    double wz = 2.0 * w * z; // = 0 when w=0
    double xy = 2.0 * x * y;

    // Quadray squaring + Julia addition
    double new_x = x * x - y * y + wz + ctx;
    double new_y = y * y - x * x + wz + cty;
    double new_z = z * z - w * w + xy + ctz;
    double new_w = w * w - z * z + xy + ctw;

    de = new_w;

    return cpu_double3(new_x, new_y, new_z);
}

// Alternative direct function implementation
cpu_double3 QuadrayBrot_formula(cpu_double3 p, constant double* params, double &de) {
    const double Cx = params[0], Cy = params[1], Cz = params[2];
    double x = p.x, y = p.y, z = p.z, w = de;
    double x2 = x*x, y2 = y*y, z2 = z*z;
    double xy2 = 2.0*x*y;
    double w2 = w*w;
    de = w2 - z2 + xy2 + abs(Cx-Cy-Cz);
    return cpu_double3(x2 - y2 + 2.0*w*z + abs(Cx+Cy+Cz),
                   y2 - x2 + 2.0*w*z + abs(-Cx-Cy+Cz),
                   z2 - w2 + xy2 + abs(-Cx+Cy-Cz));
}

// Formula metadata
constant FormulaInfo formula_info_QuadrayBrot = {
    .id = FORMULA_QUADRAYBROT,
    .name = "QuadrayBrot",
    .param_count = 0,
    .complexity = 169,
    .uses_transcendental = false
};

/*
Original M3 Description:
This formula uses quadray numbers in 4D.
Invented and kindly shared by Aexion, the author of Incendia, 2011.
Implementation Luca GN.

Mathematical Formula (4D):
ctx = |Cx + Cy + Cz|
cty = |-Cx - Cy + Cz|
ctz = |-Cx + Cy - Cz|
ctw = |Cx - Cy - Cz|
wz = 2*w*z
xy = 2*x*y
x' = x² - y² + wz + ctx
y' = y² - x² + wz + cty
z' = z² - w² + xy + ctz
w' = w² - z² + xy + ctw

In 3D projection (w=0):
x' = x² - y² + |Cx+Cy+Cz|
y' = y² - x² + |-Cx-Cy+Cz|
z' = z² + 2*x*y + |-Cx+Cy-Cz|

Note: DEoption=4 (4D formula). Julia constants accessed from iteration context.
SIpower=2.
*/
