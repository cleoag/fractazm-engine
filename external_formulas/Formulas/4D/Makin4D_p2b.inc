// Makin4D p2b.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Makin4D p2b
// Hand-ported from ASM: 182 bytes, DEoption=4
// David Makin's 4D "p2b" formula — stereographic-like squaring.

constexpr unsigned int FORMULA_MAKIN4D_P2B = 7653;

// ASM trace (182 bytes, 0 branches):
//
// A = x² + y²
// r² = x² + y² + z² + w²
// scale = (2*A - r²) / (2*A) = (x²+y²-z²-w²) / (2*(x²+y²))
//
// x' = 2*(x²-y²) * scale + Cx
// y' = 4*x*y * scale + Cy
// z' = 2*(x*z - y*w) + Cz
// w' = 2*(x*w + y*z) + Cw + W_add
//
// The scale computation:
//   sqrt(R² + (4xy)²) = sqrt(4(x²-y²)² + 16x²y²) = 2(x²+y²)
//   scale = 1 - r²/(2*A)
//
// For 3D: w=0:
// scale = (x²+y²-z²) / (2*(x²+y²))
// x' = (x²-y²) * (x²+y²-z²) / (x²+y²) + Cx
// y' = 2*x*y * (x²+y²-z²) / (x²+y²) + Cy
// z' = 2*x*z + Cz
//
// C handling: Formula handles C internally (DEoption=4).

template<>
cpu_double3 execute_formula<FORMULA_MAKIN4D_P2B>(cpu_double3 p, constant double* params, double &de) {
    const double W_add = params[0];   // [edi-0x10] (default: 0.0)
    const double Cx = params[1];      // [esi+0x18]
    const double Cy = params[2];      // [esi+0x20]
    const double Cz = params[3];      // [esi+0x28]
    const double Cw = params[4];      // [esi-0x38]

    const double x = p.x;
    const double y = p.y;
    const double z = p.z;
    const double w = de;

    const double A = x * x + y * y;
    const double r2 = A + z * z + w * w;
    const double scale = (2.0 * A - r2) / (2.0 * A);

    const double x_out = 2.0 * (x * x - y * y) * scale + Cx;
    const double y_out = 4.0 * x * y * scale + Cy;
    const double z_out = 2.0 * (x * z - y * w) + Cz;
    const double w_out = 2.0 * (x * w + y * z) + Cw + W_add;

    de = w_out;
    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_Makin4D_p2b = {
    .id = FORMULA_MAKIN4D_P2B,
    .name = "Makin4D p2b",
    .param_count = 5,
    .complexity = 182,
    .uses_transcendental = false
};

/*
Original M3 Description:
David Makin's 4D formula "p2b" from fractalforums.com.
Stereographic-like squaring where (x,y) components are scaled
by a factor depending on the full 4D radius.

Full 4D:
  A = x² + y², r² = x² + y² + z² + w²
  scale = (2*A - r²) / (2*A)
  x' = 2*(x²-y²) * scale
  y' = 4*x*y * scale
  z' = 2*(x*z - y*w)
  w' = 2*(x*w + y*z) + Cw + W_add

3D projection (w=0):
  scale = (x²+y²-z²) / (2*(x²+y²))
  x' = 2*(x²-y²) * scale
  y' = 4*x*y * scale
  z' = 2*x*z

Parameters:
  [0] W_add: double (default: 0.0)
  [1..4] Cx, Cy, Cz, Cw: passed by engine/harness from context
*/
