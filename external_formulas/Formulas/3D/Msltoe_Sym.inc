// Msltoe_Sym.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Msltoe_Sym
// Hand-ported from ASM: 338 bytes, DEoption=0
// Mandelstoe symmetry formula

constexpr unsigned int FORMULA_MSLTOE_SYM = 2758;

// ASM trace:
//   Phase 1: angle reduction
//     theta = atan2(z, y) + angle_offset
//     acc = 0
//     while (theta > half_range): theta -= half_step; acc -= half_step
//   Phase 2: rotate (y,z) by acc
//     y_rot = y*cos(acc) - z*sin(acc)
//     z_rot = y*sin(acc) + z*cos(acc)
//   Phase 3: z²-symmetric iteration (like Mandelbrot x²-y², 2xy)
//     r² = x² + y_rot² + z_rot²
//     factor = c - z_rot²/r²
//     term1 = (x² - y_rot²) * factor
//     term2 = d * x * y_rot * factor
//     term3 = d * z_rot * sqrt(x² + y_rot²)
//   Phase 4: output with inverse rotation
//     out_x = term1 + Cx
//     out_y = term2*cos(acc) + term3*sin(acc) + Cy
//     out_z = term3*cos(acc) - term2*sin(acc) + Cz
//
// C handling: Standard (DEoption=0), engine adds C after return.

template<>
cpu_double3 execute_formula<FORMULA_MSLTOE_SYM>(cpu_double3 p, constant double* params, double &de) {
    const double half_range  = params[0];  // angle range threshold
    const double half_step   = params[1];  // angle reduction step
    const double coeff_c     = params[2];  // factor coefficient
    const double coeff_d     = params[3];  // cross-term coefficient
    const double angle_offset = params[4]; // initial angle offset

    double x = p.x, y = p.y, z = p.z;

    // Phase 1: angle reduction
    double theta = atan2(z, y) + angle_offset;
    double acc = 0.0;
    while (theta > half_range) {
        theta -= half_step;
        acc -= half_step;
    }

    // Phase 2: rotate (y,z) by accumulated angle
    double cos_acc = cos(acc);
    double sin_acc = sin(acc);
    double y_rot = y * cos_acc - z * sin_acc;
    double z_rot = y * sin_acc + z * cos_acc;

    // Phase 3: compute fractal iteration
    double r2 = x * x + y_rot * y_rot + z_rot * z_rot;
    double factor = coeff_c - (z_rot * z_rot) / r2;

    double term1 = (x * x - y_rot * y_rot) * factor;
    double term2 = coeff_d * x * y_rot * factor;

    double rho = sqrt(x * x + y_rot * y_rot);
    double term3 = coeff_d * z_rot * rho;

    // Phase 4: output with inverse rotation of (term2, term3)
    double new_x = term1;
    double new_y = term2 * cos_acc + term3 * sin_acc;
    double new_z = term3 * cos_acc - term2 * sin_acc;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_Msltoe_Sym = {
    .id = FORMULA_MSLTOE_SYM,
    .name = "Msltoe_Sym",
    .param_count = 5,
    .complexity = 338,
    .uses_transcendental = true
};

/*
Original M3 Description:
Mandelstoe symmetry formula.

Reduces the azimuthal angle in the yz-plane to a fundamental domain
(by subtracting half_step until theta <= half_range), applies a
z²-symmetric iteration similar to the Mandelbrot (x²-y², 2xy) pattern,
then rotates back to restore the original angular position.

Parameters:
  [0] Double: double - half_range (angle threshold for reduction)
  [1] Double: double - half_step (angle step for reduction)
  [2] Double: double - factor coefficient (c - z²/r²)
  [3] Double: double - cross-term coefficient
  [4] Double: double - angle offset added before reduction
*/
