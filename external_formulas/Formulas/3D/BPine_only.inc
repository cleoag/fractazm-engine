// BPine_only.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: BPine_only
// Hand-ported from ASM: 138 bytes, DEoption=0

constexpr unsigned int FORMULA_BPINE_ONLY = 7090;

// ASM trace:
//   x² → local, y² → local, z² → local
//   ρ = sqrt(y² + z²)
//   factor = 2*x / ρ
//   new_x = x² - y² - z² + Cx * CxScale
//   new_y = factor * 2 * z * y + Cy * CyScale     (= 4xyz/ρ + Cy*CyScale)
//   new_z = factor * (y² - z²) + Cz * CzScale     (= 2x(y²-z²)/ρ + Cz*CzScale)
//
// Benesi Pine formula (standalone, without BenesiPine1/2 pre-rotation).
// Same structure as Benesi1Pow2 but C components are SCALED by parameters.
//
// C handling: NON-STANDARD — each C component scaled by a parameter.

template<>
cpu_double3 execute_formula<FORMULA_BPINE_ONLY>(cpu_double3 p, constant double* params, double &de) {
    // params[0..2] are C scale factors, C passed as params[3..5]
    const double CxScale = params[0];  // scale for Cx (default: 1)
    const double CzScale = params[1];  // scale for Cz (default: 1)
    const double CyScale = params[2];  // scale for Cy (default: 1)
    const double Cx      = params[3];
    const double Cy      = params[4];
    const double Cz      = params[5];

    double x = p.x, y = p.y, z = p.z;

    double x2 = x * x;
    double y2 = y * y;
    double z2 = z * z;
    double rho = sqrt(y2 + z2);

    // factor = 2*x / ρ
    double factor;
    if (rho == 0.0 && (y != 0.0 || z != 0.0)) {
        // x87 keeps tiny yz tails alive longer than ARM double. Preserve the
        // finite underflow collapse, but keep the true origin singularity.
        factor = 0.0;
    } else {
        factor = 2.0 * x / rho;
    }

    // x: standard power-2 with scaled Cx
    double new_x = x2 - y2 - z2 + Cx * CxScale;

    // y: 4xyz/ρ + Cy * CyScale
    double new_y = factor * 2.0 * z * y + Cy * CyScale;

    // z: 2x(y²-z²)/ρ + Cz * CzScale
    double new_z = factor * (y2 - z2) + Cz * CzScale;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_BPine_only = {
    .id = FORMULA_BPINE_ONLY,
    .name = "BPine_only",
    .param_count = 6,
    .complexity = 138,
    .uses_transcendental = false
};

/*
Original M3 Description:
Benesi Pine formula (standalone).
Uses yz-plane for the radial component (ρ = sqrt(y²+z²)).

ρ = sqrt(y²+z²)
factor = 2x/ρ
new_x = x² - y² - z² + Cx * CxScale
new_y = 4xyz/ρ + Cy * CyScale
new_z = 2x(y²-z²)/ρ + Cz * CzScale

Same geometry as Benesi1Pow2 but with C scaling factors.
This is the "bare" Pine formula without the rotation
pre-processing of BenesiPine1/BenesiPine2.

C handling: NON-STANDARD (C scaled by params).
Engine should NOT add C; template handles it.

Parameters:
  [0] CxScale: double (default: 1)
  [1] CzScale: double (default: 1)
  [2] CyScale: double (default: 1)
  [3] Cx: double (from engine C)
  [4] Cy: double (from engine C)
  [5] Cz: double (from engine C)
*/
