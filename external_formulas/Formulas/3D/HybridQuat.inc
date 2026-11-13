// HybridQuat.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.
// Hand-ported from Pascal source (formulas.pas)
// DEoption: 4 in legacy hybrid runtime. This helper returns XYZ and stores W in `de`.

// HybridQuat — 4D quaternion Mandelbrot step (formulas.pas line 4081)
// Pascal source (clean, not ASM):
//   xt := x; yt := y; zt := z;   (save originals)
//   x  := x*x - y*y - z*z - w*w + J1;
//   y  := 2*(y*xt + z*w) + J2;
//   z  := 2*(z*xt + p1*yt*w) + J3;
//   w  := 2*(w*xt + yt*zt) + p2 + J4;
//
// Legacy hybrid runtime supplies J1..J4 externally.
// macOS CPU/GPU runtime therefore computes the pure polynomial here,
// returns XYZ, and stores W in `de`; caller injects the current hybrid C.

constexpr unsigned int FORMULA_HYBRIDQUAT = 90013;

template<>
cpu_double3 execute_formula<FORMULA_HYBRIDQUAT>(cpu_double3 p, constant double* params, double &de) {
    const double x = p.x;
    const double y = p.y;
    const double z = p.z;
    const double w = de;
    const double ywMultiplier = params[0];
    const double wAdd = params[1];

    const double new_x = x * x - y * y - z * z - w * w;
    const double new_y = 2.0 * (y * x + z * w);
    const double new_z = 2.0 * (z * x + ywMultiplier * y * w);
    const double new_w = 2.0 * (w * x + y * z) + wAdd;

    de = new_w;
    return cpu_double3(new_x, new_y, new_z);
}

cpu_double3 HybridQuat_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HYBRIDQUAT>(p, params, de);
}

constant FormulaInfo formula_info_HybridQuat = {
    .id = FORMULA_HYBRIDQUAT,
    .name = "HybridQuat",
    .param_count = 1,
    .complexity = 80,
    .uses_transcendental = false
};

/*
Original Pascal source: formulas.pas, procedure HybridQuat (line 4081)
4D quaternion Mandelbrot iteration. Legacy hybrid runtime injects J1..J4 externally.

Pure polynomial step:
  xt = x; yt = y;
  new_x = x²-y²-z²-w²
  new_y = 2*(y*x + z*w)
  new_z = 2*(z*x + p1*y*w)
  new_w = 2*(w*x + y*z) + p2

Caller injects J1/J2/J3/J4 after return.

Parameters:
  [0] p1: YW multiplier
  [1] p2: W add
*/
