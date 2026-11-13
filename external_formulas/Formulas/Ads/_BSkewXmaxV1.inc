// _BSkewXmaxV1.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _BSkewXmaxV1
// Hand-ported from ASM: 123 bytes, DEoption=-1
// X-axis skew transform (Benesi alpha version)

constexpr unsigned int FORMULA__BSKEWXMAXV1 = 3912;

// ASM trace:
//   ss1 = (|x| - xMin) * (xMax - |x|) * xScale
//   ss2 = |Cx| * (|Cx| + xPixelAdd) * xPixelScale
//   r_term = sqrt(y² + z²) * yzScale
//   x' = x - r_term - ss1 - ss2
//   y' = y (unchanged)
//   z' = z (unchanged)
//
// Note: The M3 description lists 9 params but only 6 are used by the ASM code.
// Unused params: yzMin [PVar-0x28], yzMax [PVar-0x38], xPixelMax [PVar-0x50].
// Author noted this is an "alpha version" with an unintentional assembly mistake.
// The ss2 computation uses |Cx|*(|Cx|+param) instead of the intended
// (xpixelmax-|xpixel|)*(|xpixel|-xpixelmin) form.
//
// C handling: DEoption=-1 (transform), reads Cx. No C addition.

template<>
cpu_double3 execute_formula<FORMULA__BSKEWXMAXV1>(cpu_double3 p, constant double* params, double &de) {
    const double xMin        = params[0];  // [PVar-0x10] (default: 0.707)
    const double xScale      = params[1];  // [PVar-0x18] (default: 0.0)
    const double xMax        = params[2];  // [PVar-0x20] (default: 1.414)
    const double yzScale     = params[4];  // [PVar-0x30] (default: 0.0)
    const double xPixelAdd   = params[6];  // [PVar-0x40] (default: 0.707, labeled xPixelMin in M3)
    const double xPixelScale = params[7];  // [PVar-0x48] (default: 0.0)
    const double Cx          = params[9];  // from engine context [esi+0x18]

    double x = p.x, y = p.y, z = p.z;

    // ss1: quadratic bell curve in |x| between xMin and xMax
    double ax = fabs(x);
    double ss1 = (ax - xMin) * (xMax - ax) * xScale;

    // ss2: Cx-dependent term (accidental formula per author)
    double acx = fabs(Cx);
    double ss2 = acx * (acx + xPixelAdd) * xPixelScale;

    // r_term: radial distance in y-z plane
    double r_term = sqrt(y * y + z * z) * yzScale;

    // Only x is modified
    double new_x = -(r_term + ss1 + ss2) + x;

    return cpu_double3(new_x, y, z);
}

// Formula metadata
constant FormulaInfo formula_info__BSkewXmaxV1 = {
    .id = FORMULA__BSKEWXMAXV1,
    .name = "_BSkewXmaxV1",
    .param_count = 9,
    .complexity = 123,
    .uses_transcendental = true
};

