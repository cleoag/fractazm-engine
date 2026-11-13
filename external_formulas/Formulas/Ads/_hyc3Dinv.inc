// _hyc3Dinv.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _hyc3Dinv
// Hand-ported from ASM: 148 bytes, DEoption=-1
// Hypercomplex 3D inversion (tricomplex number theory)

constexpr unsigned int FORMULA__HYC3DINV = 1941;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void HycInv_x87(double* state, const double* params);
#endif

// The shipped x87 stack order differs from the ideal tricomplex inverse.
// FDIVRP ST(3) moves the third quotient below the first two results; the
// final radius and Z-center operations affect the previous Y result.
// No extra Julia addition. Preserve singular divisions, including det=0.

template<>
cpu_double3 execute_formula<FORMULA__HYC3DINV>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z};
    HycInv_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#else
    const numeric::X87Scope extendedScope;
    const double X_center = params[0];  // [PVar-0x10] (default: 0)
    const double Y_center = params[1];  // [PVar-0x18] (default: 0)
    const double Z_center = params[2];  // [PVar-0x20] (default: 0)
    const double Radius   = params[3];  // [PVar-0x28] (default: 1)

    // Translate to center
    double a = p.x + X_center;
    double b = p.y + Y_center;
    double c = p.z + Z_center;

    // Tricomplex modulus: det = a³ + b³ + c³ - 3abc
    const numeric::X87Real aa=a, bb=b, cc=c;
    const numeric::X87Real product=aa*cc*bb;
    const numeric::X87Real sum=aa*aa*aa+bb*bb*bb+cc*cc*cc;
    const numeric::X87Real det=-(product-((sum-product)-product));

    // Tricomplex inverse: (a²-bc, b²-ac, c²-ab) / det
    // Scale by Radius and translate back
    const numeric::X87Real new_x = (aa*aa-bb*cc)/det*Radius-X_center;
    const numeric::X87Real new_y = (bb*bb-aa*cc)/det*Radius-Y_center;
    const numeric::X87Real new_z = (cc*cc-aa*bb)/det;

    // FDIVRP ST(3) moves the quotient below both existing outputs. The final
    // scale/subtract therefore operate on the old Y result, exactly as shipped.
    return cpu_double3(double(new_z),double(new_x),double(new_y*Radius-Z_center));
#endif
}

// Formula metadata
constant FormulaInfo formula_info__hyc3Dinv = {
    .id = FORMULA__HYC3DINV,
    .name = "_hyc3Dinv",
    .param_count = 4,
    .complexity = 148,
    .uses_transcendental = false
};

// Parameters: X/Y/Z center, radius. The original description presents an
// ideal tricomplex inverse; executable behavior above is the parity target.
