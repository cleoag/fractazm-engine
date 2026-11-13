// Quadrat3D.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Quadrat3D
// Hand-ported from ASM: 166 bytes, DEoption=0

constexpr unsigned int FORMULA_QUADRAT3D = 9813;

// ASM trace:
//   temp = x² + y² + HRadius
//   if temp > 0:
//     factor = ZRadius - z²/temp
//     x' = X_mul * (x² - y²) * factor + Cx
//     y' = Y_mul * x*y * factor + Cy
//     z' = Z_mul * z * sqrt(temp) + Cz
//   if temp == 0: x'=Cx, y'=Cy, z'=Cz
//   if temp < 0:  x'=0, y'=0, z'=0
//
// Quadratic 3D Mandelbulb (D. White's triplex squaring, modified by Luca GN).

template<>
cpu_double3 execute_formula<FORMULA_QUADRAT3D>(cpu_double3 p, constant double* params, double &de) {
    const double X_mul = params[0];
    const double Y_mul = params[1];
    const double Z_mul = params[2];
    const double HRadius = params[3];
    const double ZRadius = params[4];

    // Julia constants from iteration context
    const double Cx = params[5];
    const double Cy = params[6];
    const double Cz = params[7];

    double x = p.x, y = p.y, z = p.z;
    double temp = x * x + y * y + HRadius;

    if (temp > 0.0) {
        double factor = ZRadius - z * z / temp;

        cpu_double3 result;
        result.x = X_mul * (x * x - y * y) * factor + Cx;
        result.y = Y_mul * x * y * factor + Cy;
        result.z = Z_mul * z * sqrt(temp) + Cz;
        return result;
    } else if (temp == 0.0) {
        return cpu_double3(Cx, Cy, Cz);
    } else {
        return cpu_double3(0.0, 0.0, 0.0);
    }
}

// Alternative direct function implementation
cpu_double3 Quadrat3D_formula(cpu_double3 p, constant double* params, double &de) {
    const double X_mul = params[0];
    const double Y_mul = params[1];
    const double Z_mul = params[2];
    const double HRadius = params[3];
    const double ZRadius = params[4];
    const double Cx = params[5];
    const double Cy = params[6];
    const double Cz = params[7];

    double x = p.x, y = p.y, z = p.z;
    double temp = x * x + y * y + HRadius;

    if (temp > 0.0) {
        double factor = ZRadius - z * z / temp;
        return cpu_double3(X_mul * (x * x - y * y) * factor + Cx,
                       Y_mul * x * y * factor + Cy,
                       Z_mul * z * sqrt(temp) + Cz);
    } else if (temp == 0.0) {
        return cpu_double3(Cx, Cy, Cz);
    } else {
        return cpu_double3(0.0);
    }
}

// Formula metadata
constant FormulaInfo formula_info_Quadrat3D = {
    .id = FORMULA_QUADRAT3D,
    .name = "Quadrat3D",
    .param_count = 5,
    .complexity = 166,
    .uses_transcendental = false
};

/*
Original M3 Description:
Found on http://bugman123.com/Hypercomplex/index.html
Quadratic 3D Mandelbulb set, based on D. White's triplex squaring formula.
Modified with two "hyperbolic radiuses". Standard formula has HR=0 & ZR=1.

Mathematical Formula:
temp = (HRadius + x² + y²)
if temp = 0: x'=Cx, y'=Cy, z'=Cz
if temp < 0: x'=0, y'=0, z'=0
if temp > 0:
  factor = ZRadius - z²/temp
  x' = X_mul * (x² - y²) * factor + Cx
  y' = Y_mul * x*y * factor + Cy
  z' = Z_mul * z * sqrt(temp) + Cz

Parameters:
  [0] X_mul: double (default: 1)
  [1] Y_mul: double (default: 2)
  [2] Z_mul: double (default: -2)
  [3] HRadius: double (default: 0)
  [4] ZRadius: double (default: 1)
  [5..7] Cx, Cy, Cz: Julia constants (from iteration context)
*/
