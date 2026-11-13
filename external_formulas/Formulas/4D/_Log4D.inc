// _Log4D.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _Log4D
// Hand-ported from ASM: 171 bytes, DEoption=4
// Quaternion logarithm: log(q) = ln|q| + (v/|v|)*atan2(|v|, Re(q))

constexpr unsigned int FORMULA__LOG4D = 5340;

// ASM trace (171 bytes, 3 branches):
//
// r = sqrt(w² + y² + z²)       ; magnitude of imaginary part
//
// if r != 0:
//   theta = atan2(r, x)         ; fpatan: atan2(st1=r, st0=x)
//   factor = theta / r
//   |q|² = x² + r²
//   if |q|² > threshold:
//     x' = ln(|q|²) * Scale     ; via fyl2x: ln(2)*log2(|q|²)
//   else:
//     x' = default_val
//   y' = y * factor
//   z' = z * factor
//   w' = w * factor
// else (r == 0, pure real):
//   if x > threshold:
//     x' = ln(x)                ; natural log of x
//   else:
//     x' = default_val
//   y, z, w unchanged (all zero)
//
// With default Scale=0.5: ln(|q|²)*0.5 = ln|q|, giving standard quaternion log.
//
// For 3D (w=0): r = sqrt(y² + z²)
//
// C handling: Formula does NOT add C. Engine handles C externally.

template<>
cpu_double3 execute_formula<FORMULA__LOG4D>(cpu_double3 p, constant double* params, double &de) {
    const double Threshold  = 1e-200;      // [esi+0x00]
    const double Scale      = 0.5;         // [esi+0x08]
    const double DefaultVal = 460.0;       // [esi+0x10] semantic fallback value

    double x = p.x, y = p.y, z = p.z, w = de;

    // Magnitude of imaginary part
    double r = sqrt(y * y + z * z + w * w);

    double x_out, y_out, z_out, w_out;

    if (r != 0.0) {
        // General case: imaginary part nonzero
        double theta = atan2(r, x);
        double factor = theta / r;

        double q2 = x * x + r * r;  // |q|²

        // x' = ln(|q|²) * Scale
        x_out = (q2 > Threshold) ? (log(q2) * Scale) : DefaultVal;
        y_out = y * factor;
        z_out = z * factor;
        w_out = w * factor;
    } else {
        // Pure real case
        x_out = (x > Threshold) ? log(x) : DefaultVal;
        y_out = y;
        z_out = z;
        w_out = w;
    }

    de = w_out;
    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info__Log4D = {
    .id = FORMULA__LOG4D,
    .name = "_Log4D",
    .param_count = 0,
    .complexity = 171,
    .uses_transcendental = true
};

/*
Original M3 Description:
Quaternion logarithm: log(Q) following Hamiltonian theory.
log(q) = ln|q| + (v/|v|)*atan2(|v|, Re(q))

Full 4D:
  r = sqrt(y² + z² + w²)
  theta = atan2(r, x)
  factor = theta / r
  x' = ln(x² + r²) * Scale
  y' = y * factor
  z' = z * factor
  w' = w * factor

  With Scale=0.5 (default): x' = ln|q| (standard log).

3D projection (w=0):
  r = sqrt(y² + z²)
  Same formulas, w' = 0.

Parameters:
  [0] Scale: double (default: 0.5) - log scale factor [esi+8]
  Internal: threshold [esi]=~1e-200, default_val [esi+0x10]
*/
