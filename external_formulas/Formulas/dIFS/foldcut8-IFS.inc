// foldcut8-IFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: foldcut8-IFS
// Hand-ported from ASM: 249 bytes, DEoption=20
// Based on Knighty's algorithm: polyhedra with sphere limit/decoration
// Computes weighted sum of sorted absolute coordinates, minus a sphere

constexpr unsigned int FORMULA_FOLDCUT8IFS = 10072;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Foldcut8Distance_x87(double* state, const double* params, int apply);
#endif

// Original x87 fold keeps [mid,max,min] after the final FXCH.
// Weights are mid*N3 + max*N1 + min*N2; the sign bit of R selects min/max.

template<>
cpu_double3 execute_formula<FORMULA_FOLDCUT8IFS>(cpu_double3 p, constant double* params, double &de) {
    // IFS parameters (standard SSE2 preamble layout)
    const double Size = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const int Apply_scale_add = int(params[5]);
    const double R = params[6];          // sphere radius (pos or neg)
    const double Normal_1 = params[7];   // weight for largest component
    const double Normal_2 = params[8];   // weight for smallest component
    const double Normal_3 = params[9];   // weight for middle component

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {x, y, z, 0.0};
    Foldcut8Distance_x87(state, params, Apply_scale_add);
    de = state[3];
#else
    const numeric::X87Scope extendedScope;
    // Keep extended intermediates through cancellation and division by Scale.
    double ax = abs(x);
    double ay = abs(y);
    double az = abs(z);

    // Three compare-and-swap passes (bubble sort on 3 elements)
    // Pass 1: ensure largest is in position 0
    double v0 = az, v1 = ay, v2 = ax;  // stack order: st0=|z|, st1=|y|, st2=|x|
    // 0x5C: fcom st(1) -> if st0 < st1, swap st0/st1
    if (v0 < v1) { double t = v0; v0 = v1; v1 = t; }
    // 0x68: fcom st(2) -> if st0 < st2, swap st0/st2
    if (v0 < v2) { double t = v0; v0 = v2; v2 = t; }
    // 0x74: st1/st2 comparison: swap st1, fcom st(2), swap back
    if (v1 < v2) { double t = v1; v1 = v2; v2 = t; }
    // Now: v0=max, v1=mid, v2=min

    // Step 3: Weighted sum (polyhedron distance)
    // FXCH at 0x74 leaves [mid,max,min], with no swap back. The original
    // addresses are -0x64 (N3), -0x54 (N1), -0x5c (N2), respectively.
    numeric::X87Real weighted = (static_cast<numeric::X87Real>(v1) * Normal_3
                         + static_cast<numeric::X87Real>(v0) * Normal_1)
                         + static_cast<numeric::X87Real>(v2) * Normal_2;
    numeric::X87Real plane_de = weighted - Size;

    // Step 4: Sphere distance
    numeric::X87Real r = sqrt((static_cast<numeric::X87Real>(x) * x
                       + static_cast<numeric::X87Real>(y) * y)
                       + static_cast<numeric::X87Real>(z) * z);
    double abs_R = abs(R);
    numeric::X87Real sphere_de = r - abs_R;

    // Step 5: Combine based on sign of R
    // ASM: checks sign bit of R at [edi-0x48]
    // ASM: R >= 0: min(sphere_de, plane_de); R < 0: max(sphere_de, plane_de)
    numeric::X87Real shape_de;
    if (!std::signbit(R)) { // The original tests the sign bit, including -0.
        shape_de = !(sphere_de > plane_de) ? sphere_de : plane_de;
    } else {
        shape_de = !(sphere_de > plane_de) ? plane_de : sphere_de;
    }

    // Step 6: Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = double(shape_de);
#endif
    return Apply_scale_add ? cpu_double3(x, y, z) : p;
}

// Alternative direct function implementation
cpu_double3 foldcut8IFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_FOLDCUT8IFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_foldcut8IFS = {
    .id = FORMULA_FOLDCUT8IFS,
    .name = "foldcut8-IFS",
    .param_count = 10,
    .complexity = 350,
    .uses_transcendental = false
};

