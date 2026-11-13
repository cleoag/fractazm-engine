// Benesi3Pow2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Benesi3Pow2
// Hand-ported from ASM: 101 bytes, DEoption=0

constexpr unsigned int FORMULA_BENESI3POW2 = 7085;

// ASM trace:
//   ρ = sqrt(y² + z²)
//   base_x = x² - (y² + z²)
//   Sign correction: if (ρ < x AND Cx >= 0) then base_x = -base_x
//   new_x = base_x + Cx
//   factor = -2 * |x| / ρ
//   new_y = factor * (y² - z²) + Cy
//   new_z = factor * y * z * 2 + Cz   (= -4*|x|*y*z/ρ + Cz)
//
// Benesi Type 3 Power-2 formula.
// Similar to Type 1 but with signed outputs (not abs) for y,z
// and conditional sign correction on x based on Cx sign.
// C handling: NON-STANDARD — x has conditional sign flip based on Cx.

template<>
cpu_double3 execute_formula<FORMULA_BENESI3POW2>(cpu_double3 p, constant double* params, double &de) {
    // C passed as params (non-standard C handling)
    const double Cx = params[0];
    const double Cy = params[1];
    const double Cz = params[2];

    double x = p.x, y = p.y, z = p.z;

    double y2 = y * y;
    double z2 = z * z;
    double x2 = x * x;
    double rho2 = y2 + z2;
    double rho = sqrt(rho2);

    // x: power-2 with conditional sign correction
    double base_x = x2 - rho2;
    // ASM: if sqrt(y²+z²) < x AND sign bit of Cx is 0 (Cx >= 0), negate
    if (rho < x && Cx >= 0.0) {
        base_x = -base_x;
    }
    double new_x = base_x + Cx;

    // y, z: use -2|x|/ρ factor (standard + C addition)
    double factor;
    if (rho == 0.0 && (y != 0.0 || z != 0.0)) {
        // x87 keeps tiny yz tails alive longer than ARM double. When rho only
        // vanished due to underflow, the reference still collapses finitely.
        factor = 0.0;
    } else {
        factor = -2.0 * fabs(x) / rho;
    }
    double new_y = factor * (y2 - z2) + Cy;
    double new_z = factor * y * z * 2.0 + Cz;  // = -4|x|yz/ρ + Cz

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_Benesi3Pow2 = {
    .id = FORMULA_BENESI3POW2,
    .name = "Benesi3Pow2",
    .param_count = 3,
    .complexity = 101,
    .uses_transcendental = false
};

/*
Original M3 Description:
Benesi Type 3 Power-2 formula by Ben Benesi.
Uses yz-plane orientation like Type 1 but with different
sign handling.

ρ = sqrt(y²+z²)
base_x = x² - (y²+z²)
if (ρ < x AND Cx >= 0): base_x = -base_x
new_x = base_x + Cx
new_y = -2*|x|*(y²-z²)/ρ + Cy
new_z = -4*|x|*y*z/ρ + Cz

Differs from Type 1:
- y and z use standard +C addition (not |C|-|formula|)
- Conditional sign flip on x depends on sign of Cx
- Uses |x| (absolute x) in y,z terms

C handling: NON-STANDARD (x sign correction depends on Cx).
Engine should NOT add C to this formula; template handles it.

Parameters (C passed as params):
  [0] Cx: double
  [1] Cy: double
  [2] Cz: double
*/
