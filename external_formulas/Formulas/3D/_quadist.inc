// _quadist.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _quadist
// Hand-ported from ASM: 168 bytes, DEoption=0

constexpr unsigned int FORMULA__QUADIST = 7078;

// ASM trace:
//   new_x = x*Linear + (d-a)*x² + (e-b)*y² + (f-c)*z²
//   new_y = y*Linear + (f-a)*x² + (d-b)*y² + (e-c)*z²
//   new_z = z*Linear + (e-a)*x² + (f-b)*y² + (d-c)*z²
//
// Quadratic distortion with cyclic coefficient pattern.
// Coefficients (d,e,f) rotate: x uses (d,e,f), y uses (f,d,e), z uses (e,f,d).
// Coefficients (a,b,c) stay fixed for each squared term.
//
// C handling: NOT added by formula. Engine adds Cx,Cy,Cz after return.

template<>
cpu_double3 execute_formula<FORMULA__QUADIST>(cpu_double3 p, constant double* params, double &de) {
    const double Linear = params[0];  // default: 1.0
    const double a      = params[1];  // default: 0
    const double b      = params[2];  // default: 0
    const double c      = params[3];  // default: 0
    const double d      = params[4];  // default: 0
    const double e      = params[5];  // default: 0
    const double f      = params[6];  // default: 0

    double x = p.x, y = p.y, z = p.z;
    double x2 = x * x, y2 = y * y, z2 = z * z;

    // Cyclic coefficient pattern: (d,e,f) rotate, (a,b,c) fixed per axis
    double new_x = x * Linear + (d - a) * x2 + (e - b) * y2 + (f - c) * z2;
    double new_y = y * Linear + (f - a) * x2 + (d - b) * y2 + (e - c) * z2;
    double new_z = z * Linear + (e - a) * x2 + (f - b) * y2 + (d - c) * z2;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__quadist = {
    .id = FORMULA__QUADIST,
    .name = "_quadist",
    .param_count = 7,
    .complexity = 168,
    .uses_transcendental = false
};

/*
Original M3 Description:
Quadratic distortion formula. Each output coordinate is:
  coord' = coord * Linear + A*x² + B*y² + C*z²

The quadratic coefficients follow a cyclic pattern:
  x: (d-a), (e-b), (f-c)
  y: (f-a), (d-b), (e-c)
  z: (e-a), (f-b), (d-c)

The (d,e,f) coefficients rotate cyclically across axes while
(a,b,c) remain fixed, creating a symmetric deformation.

C handling: Engine adds C externally (formula does not access C).

Parameters:
  [0] Linear: double (default: 1.0)
  [1] a: double (default: 0)
  [2] b: double (default: 0)
  [3] c: double (default: 0)
  [4] d: double (default: 0)
  [5] e: double (default: 0)
  [6] f: double (default: 0)
*/
