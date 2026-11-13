// _Exp4D.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _Exp4D
// Hand-ported from ASM: 120 bytes, DEoption=4
// Quaternion exponential: exp(q) = exp(Re) * (cos|v| + sin|v|/|v| * v)

constexpr unsigned int FORMULA__EXP4D = 124;

// ASM trace (120 bytes, 1 branch):
//
// r = sqrt(w² + y² + z²)       ; magnitude of imaginary part
// sincos(r) → sin_r, cos_r
// e = exp(x)                    ; via log2(e)*x → fprem → f2xm1 → fscale
//
// x' = e * cos_r
// factor = e * sin_r
// if (r > threshold):           ; je at 0x4: skip division when r < threshold
//     factor = factor / r
// y' = factor * y
// z' = factor * z
// w' = factor * w
//
// Standard quaternion exponential formula.
// Branch guards against division by zero when imaginary part is tiny.
//
// For 3D (w=0): r = sqrt(y² + z²)
//
// C handling: Formula does not add C. Engine handles C externally.

template<>
cpu_double3 execute_formula<FORMULA__EXP4D>(cpu_double3 p, constant double* params, double &de) {
    const double threshold = params[0];  // [PVar+0x00] division guard, default 1e-200

    double x = p.x, y = p.y, z = p.z, w = de;

    // Magnitude of imaginary part
    double r = sqrt(y * y + z * z + w * w);

    // Quaternion exponential: exp(x+v) = exp(x) * (cos|v| + sin|v|/|v| * v)
    double e = exp(x);

    double x_out = e * cos(r);

    // ASM contract:
    //   factor = e * sin(r)
    //   if (r > threshold) factor /= r
    double factor = e * sin(r);
    if (r > threshold) {
        factor /= r;
    }

    double y_out = factor * y;
    double z_out = factor * z;
    double w_out = factor * w;

    de = w_out;

    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info__Exp4D = {
    .id = FORMULA__EXP4D,
    .name = "_Exp4D",
    .param_count = 0,
    .complexity = 120,
    .uses_transcendental = true
};

/*
Original M3 Description:
Quaternion exponential: exp(Q) following Hamiltonian theory.
exp(q) = exp(Re) * (cos|v| + v/|v| * sin|v|)

Full 4D:
  r = sqrt(y² + z² + w²)
  e = exp(x)
  x' = e * cos(r)
  factor = e * sin(r) / r  (with guard for r≈0)
  y' = factor * y
  z' = factor * z
  w' = factor * w

Parameters:
  [0] threshold: double (default: ~1e-200) — division guard
*/
