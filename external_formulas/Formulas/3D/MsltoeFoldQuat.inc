// MsltoeFoldQuat.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MsltoeFoldQuat
// Hand-ported from ASM: 118 bytes, DEoption=0

constexpr unsigned int FORMULA_MSLTOEFOLDQUAT = 7075;

// ASM trace:
//   Compute x², y², z² and sort by squared value (bubble sort):
//     if x² < z²: swap(x,z)
//     if x² < y²: swap(x,y)
//     if y² < z²: swap(y,z)
//   After sort: |x| >= |y| >= |z|
//   Power-2 quaternion formula (no sign corrections):
//     new_x = x² - y² - z²   (+Cx in ASM)
//     new_y = 2*x*y           (+Cy in ASM)
//     new_z = 2*x*z           (+Cz in ASM)
//
// Quaternion fold formula by Msltoe (fractalforums.com).
// Similar to MsltoeSym4 but sorts by |value| without sign corrections.
// Nice julias with x seeds; leave y and z at zero to avoid noise.

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void MsltoeFoldQuatOriginal_x87(double*,const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_MSLTOEFOLDQUAT>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z};
    MsltoeFoldQuatOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    mb3d::numeric::ExtendedScope scope;
    E x=p.x, y=p.y, z=p.z, xx=x*x, yy=y*y, zz=z*z;
    // Equality and unordered comparisons exchange both the coordinate and
    // its squared value in the original six-register sorting network.
    if (!(xx > zz)) { std::swap(x,z); std::swap(xx,zz); }
    if (!(xx > yy)) { std::swap(x,y); std::swap(xx,yy); }
    if (!(yy > zz)) { std::swap(y,z); std::swap(yy,zz); }
    const E xy=x*y, xz=x*z;
    return cpu_double3(double((xx-yy)-zz+params[0]),double(xy+xy+params[1]),
                       double(xz+xz+params[2]));
#else
    // Portable arithmetic is not yet certified against the x87 reference.
    double x = p.x, y = p.y, z = p.z;

    // Sort by absolute value (largest first), using bubble sort
    if (abs(x) < abs(z)) { double t = x; x = z; z = t; }
    if (abs(x) < abs(y)) { double t = x; x = y; y = t; }
    if (abs(y) < abs(z)) { double t = y; y = z; z = t; }

    // Power-2 quaternion formula (no sign corrections unlike MsltoeSym4)
    double new_x = x*x - y*y - z*z;
    double new_y = 2.0 * x * y;
    double new_z = 2.0 * x * z;

    return cpu_double3(new_x + params[0], new_y + params[1], new_z + params[2]);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_MsltoeFoldQuat = {
    .id = FORMULA_MSLTOEFOLDQUAT,
    .name = "MsltoeFoldQuat",
    .param_count = 0,
    .complexity = 118,
    .uses_transcendental = false
};

/*
Original M3 Description:
Quaternion fold formula by Msltoe (fractalforums.com).
Sorts coordinates by absolute value (largest in x position),
then applies power-2 quaternion formula without sign corrections.

Sort: |x| >= |y| >= |z| (swap to achieve this)
x' = x² - y² - z²
y' = 2*x*y
z' = 2*x*z

Nice julias with x seeds; leave y and z at zero to avoid noise.

C handling: Cx, Cy, Cz all added (standard z→z²+c behavior).

Parameters: None
*/
