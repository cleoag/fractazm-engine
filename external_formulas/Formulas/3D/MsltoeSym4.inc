// Original x87 instruction/store order on Win64; portable fallback is not certified.
constexpr unsigned int FORMULA_MSLTOESYM4 = 8359;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void MsltoeSym4Original_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_MSLTOESYM4>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z};
    MsltoeSym4Original_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    mb3d::numeric::ExtendedScope scope;
    E x=p.x, y=p.y, z=p.z;
    if (!(abs(x) > abs(z)*params[0])) std::swap(x,z);
    if (!(abs(x) > abs(y)*params[1])) std::swap(x,y);
    // At 0047 ST(2) is the same Z used at 0040, not Y. Preserve the
    // shipped comparison even though its parameter is named YZ_mul.
    if (!(abs(z) > abs(z)*params[2])) std::swap(y,z);
    if (x*z >= E(0)) z=-z;
    if (x*y >= E(0)) y=-y;
    const E xy=x*y, xz=x*z;
    return cpu_double3(double((x*x-y*y)-z*z+params[3]),double(xy+xy+params[4]),
                       double(xz+xz+params[5]));
#else
    const double XZ_mul = params[0];
    const double XY_mul = params[1];
    const double YZ_mul = params[2];

    // Julia constants
    const double Cx = params[3];
    const double Cy = params[4];
    const double Cz = params[5];

    double x = p.x, y = p.y, z = p.z;

    // Conditional axis exchanges — ASM uses fcom with x87 stack shift
    // Equality and unordered comparisons take the exchange branch.
    if (!(abs(x) > abs(z) * XZ_mul)) { double t = x; x = z; z = t; }
    if (!(abs(x) > abs(y) * XY_mul)) { double t = x; x = y; y = t; }
    if (!(abs(y) > abs(z) * YZ_mul)) { double t = y; y = z; z = t; }

    // Sign corrections (from ASM): negate when SAME sign as x (x*coord >= 0)
    // ASM: ftst(x*z), shr ah,1 → CF=(x*z<0); jb SKIP → negate z only when CF=0 (x*z>=0)
    // Net effect: z is forced to have opposite sign from x.
    if (x * z >= 0.0) z = -z;
    if (x * y >= 0.0) y = -y;

    // Power-2 formula
    cpu_double3 result;
    result.x = x * x - y * y - z * z + Cx;
    result.y = 2.0 * x * y + Cy;
    result.z = 2.0 * x * z + Cz;

    return result;
#endif
}

// Alternative direct function implementation
cpu_double3 MsltoeSym4_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_MSLTOESYM4>(p,params,de);
}

// Formula metadata
constant FormulaInfo formula_info_MsltoeSym4 = {
    .id = FORMULA_MSLTOESYM4,
    .name = "MsltoeSym4",
    .param_count = 3,
    .complexity = 168,
    .uses_transcendental = false
};

/*
Original M3 Description:
A formula made by Msltoe at fractalforums.com.

Mathematical Formula:
if abs(x) <= abs(z) * XZ_sym_mul then exchange(x,z)
if abs(x) <= abs(y) * XY_sym_mul then exchange(x,y)
if abs(y) <= abs(z) * YZ_sym_mul then exchange(y,z)
if (x*z >= 0) then z = -z   [negate when same sign as x, per ASM]
if (x*y >= 0) then y = -y   [negate when same sign as x, per ASM]
x' = x*x - y*y - z*z
y' = 2*x*y + Cy
z' = 2*x*z + Cz

Valid sym_mul values: <=0 disable, 1 enable conditional, 1e30 unconditional.

Parameters:
  [0] XZ_sym_mul: double (default: 1)
  [1] XY_sym_mul: double (default: 1)
  [2] YZ_sym_mul: double (default: 1)
  [3..5] Cx, Cy, Cz: Julia constants (from iteration context)
*/
