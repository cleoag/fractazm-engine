// _hopalm1.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _hopalm1
// Hand-ported from ASM: 136 bytes, DEoption=-1

constexpr unsigned int FORMULA__HOPALM1 = 7091;

// ASM trace:
//   val = x * Scale_x - z * Scale_z + Offset
//   sq = sqrt(|val|)
//   if |val| < 1:
//       hop = sign(val) * sq
//   else:
//       hop = sign(val) / sq
//   new_x = y + hop
//   new_y = (Center - x) * Scale_xy
//   new_z = z + (Center * y - x * Scale_x) * Scale_yz
//
// Modified 3D Hopalong attractor transform.
// The hopalong function is bounded: uses sqrt for |val|<1, 1/sqrt for |val|>=1.
// No C access (PVar used directly, context pointer overwritten).
// z-axis couples x-y dynamics into 3D.

template<>
cpu_double3 execute_formula<FORMULA__HOPALM1>(cpu_double3 p, constant double* params, double &de) {
    const double Scale_xy = params[0];  // x-to-y scaling (default: 1)
    const double Center   = params[1];  // center/offset value (default: 0)
    const double Scale_x  = params[2];  // x coefficient for val (default: 1)
    const double Scale_z  = params[3];  // z coefficient for val (default: 1)
    const double Offset   = params[4];  // offset for val (default: 0)
    const double Scale_yz = params[5];  // z coupling scale (default: 1)

    double x = p.x, y = p.y, z = p.z;

    // Compute val for hopalong function
    double val = x * Scale_x - z * Scale_z + Offset;
    double abs_val = abs(val);
    double sq = sqrt(abs_val);

    // Bounded hopalong function: sign(val) * min(sqrt(|val|), 1/sqrt(|val|))
    double hop;
    if (abs_val < 1.0) {
        hop = sq;
    } else {
        hop = (sq > 1e-30) ? (1.0 / sq) : 0.0;
    }
    hop = copysign(hop, val);

    // 3D Hopalong map
    double new_x = y + hop;
    double new_y = (Center - x) * Scale_xy;
    double new_z = z + (Center * y - x * Scale_x) * Scale_yz;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__hopalm1 = {
    .id = FORMULA__HOPALM1,
    .name = "_hopalm1",
    .param_count = 6,
    .complexity = 136,
    .uses_transcendental = false
};

/*
Original M3 Description:
Modified 3D Hopalong attractor by Luca GN.

val = x * Scale_x - z * Scale_z + Offset
hop = sign(val) * (|val| < 1 ? sqrt(|val|) : 1/sqrt(|val|))
new_x = y + hop
new_y = (Center - x) * Scale_xy
new_z = z + (Center * y - x * Scale_x) * Scale_yz

The hopalong function is bounded to [-1, 1] by using
sqrt for small values and 1/sqrt for large values.
This creates a 3D chaotic attractor.

Note: Formula overwrites esi to PVar, so C is not accessible.
Engine adds C externally (standard addition).

Parameters:
  [0] Scale_xy: double (default: 1)
  [1] Center: double (default: 0)
  [2] Scale_x: double (default: 1)
  [3] Scale_z: double (default: 1)
  [4] Offset: double (default: 0)
  [5] Scale_yz: double (default: 1)
*/
