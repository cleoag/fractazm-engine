// Benesi1Pow2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Benesi1Pow2
// Hand-ported from ASM: 85 bytes, DEoption=0

constexpr unsigned int FORMULA_BENESI1POW2 = 7083;

// ASM trace:
//   ρ = sqrt(y² + z²)
//   scale = 2*x/ρ
//   y_term = (y² - z²) * scale = 2*x*(y² - z²)/ρ
//   z_term = scale * y * z = 2*x*y*z/ρ
//   z_term2 = 2 * z_term = 4*x*y*z/ρ
//   new_x = x² - (y²+z²) + Cx
//   new_y = |Cy| - |y_term|
//   new_z = |Cz| - |z_term2|
//
// Benesi Type 1 Power-2 formula (Ben Benesi).
// Y-Z plane orientation (ρ = sqrt(y²+z²)).
// C handling: NON-STANDARD — x gets normal +Cx, but y and z
// use |C| - |formula| pattern. C must be passed as params.

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Benesi1Pow2Original_x87(double*,const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_BENESI1POW2>(cpu_double3 p, double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z};
    Benesi1Pow2Original_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    mb3d::numeric::ExtendedScope scope;
    const E x=p.x, y=p.y, z=p.z, yy=y*y, zz=z*z;
    const E sum=zz+yy, rootX=sqrt(sum)*x, scale=(rootX+rootX)/sum;
    const E yt=(yy-zz)*scale, zt=(y*scale)*z;
    return cpu_double3(double(x*x-sum+params[0]),double(abs(E(params[1]))-abs(yt)),
                       double(abs(E(params[2]))-abs(zt+zt)));
#else
    // C passed as params (non-standard C handling)
    const double Cx = params[0];
    const double Cy = params[1];
    const double Cz = params[2];

    double x = p.x, y = p.y, z = p.z;
    const double qnan = NAN;

    double y2 = y * y;
    double z2 = z * z;
    double rho = sqrt(y2 + z2);
    if (rho == 0.0 || !std::isfinite(rho)) {
        return cpu_double3(qnan, qnan, qnan);
    }

    double scale = 2.0 * x / rho;
    if (!std::isfinite(scale)) {
        return cpu_double3(qnan, qnan, qnan);
    }

    // x: standard power-2 (x² - ρ²) + Cx
    double new_x = x * x - (y2 + z2) + Cx;

    // y: |Cy| - |2*x*(y²-z²)/ρ|
    double y_term = (y2 - z2) * scale;
    double new_y = abs(Cy) - abs(y_term);

    // z: |Cz| - |4*x*y*z/ρ|
    double z_term = 2.0 * scale * y * z;  // 2 * (2x/ρ) * y * z = 4xyz/ρ
    double new_z = abs(Cz) - abs(z_term);
    if (!std::isfinite(new_x) || !std::isfinite(new_y) || !std::isfinite(new_z)) {
        return cpu_double3(qnan, qnan, qnan);
    }

    return cpu_double3(new_x, new_y, new_z);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_Benesi1Pow2 = {
    .id = FORMULA_BENESI1POW2,
    .name = "Benesi1Pow2",
    .param_count = 3,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
Benesi Type 1 Power-2 formula by Ben Benesi.
Uses yz-plane for the radial component (ρ = sqrt(y²+z²)).

ρ = sqrt(y²+z²)
new_x = x² - (y²+z²) + Cx
new_y = |Cy| - |2*x*(y²-z²)/ρ|
new_z = |Cz| - |4*x*y*z/ρ|

C handling: NON-STANDARD.
  - x: normal addition (+Cx)
  - y: |Cy| - |formula_y|
  - z: |Cz| - |formula_z|
Engine should NOT add C to this formula; template handles it.

Parameters (C passed as params):
  [0] Cx: double
  [1] Cy: double
  [2] Cz: double
*/
