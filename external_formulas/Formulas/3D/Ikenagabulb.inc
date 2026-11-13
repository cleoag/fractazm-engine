// Ikenagabulb: immutable M3F CODE is the parity reference.
constexpr unsigned int FORMULA_IKENAGABULB = 10277;

// Generic triplex multiplication helper used by the first and third multiply.
static cpu_double3 tpx_mul(cpu_double3 u, cpu_double3 v) {
    double rxy_u = sqrt(u.x * u.x + u.y * u.y);
    double rxy_v = sqrt(v.x * v.x + v.y * v.y);

    double prod = rxy_u * rxy_v;
    double rx, ry;

    if (prod == 0.0) {
        rx = -v.z * u.z;
        ry = v.z * u.z;
    } else {
        double alpha = v.z * u.z / prod;
        double factor = 1.0 - alpha;
        rx = (u.x * v.x - u.y * v.y) * factor;
        ry = (v.x * u.y + u.x * v.y) * factor;
    }

    double rz = rxy_u * v.z + rxy_v * u.z;

    return cpu_double3(rx, ry, rz);
}

// ASM 0x158..0x213 does not reuse the generic y-term for the middle (Sa * C)
// multiply. The executable path is:
//   x = (Sa.x * Cx - Sa.y * Cy) * factor
//   y = (Cx * Sa.y + Cy * Sa.z) * factor
//   z = rxy(Sa) * Cz + rxy(C) * Sa.z
static cpu_double3 tpx_mul_middle(cpu_double3 sa, cpu_double3 c) {
    double rxy_sa = sqrt(sa.x * sa.x + sa.y * sa.y);
    double rxy_c = sqrt(c.x * c.x + c.y * c.y);

    double prod = rxy_sa * rxy_c;
    double rx, ry;

    if (prod == 0.0) {
        rx = -c.z * sa.z;
        ry = c.z * sa.z;
    } else {
        double alpha = c.z * sa.z / prod;
        double factor = 1.0 - alpha;
        rx = (sa.x * c.x - sa.y * c.y) * factor;
        ry = (c.x * sa.y + sa.z * c.y) * factor;
    }

    double rz = rxy_sa * c.z + rxy_c * sa.z;
    return cpu_double3(rx, ry, rz);
}

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void IkenagabulbOriginal_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_IKENAGABULB>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    // Original x87 spills and zero-XY branch still evaluate the cubic Z term.
    double state[24] = {p.x,p.y,p.z};
    IkenagabulbOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    using E=Extended80;
    ExtendedScope scope;
    const auto multiply=[](cpu_double3 u,cpu_double3 v,bool middle) {
        const E ux=u.x,uy=u.y,uz=u.z,vx=v.x,vy=v.y,vz=v.z;
        const E ru=double(sqrt(ux*ux+uy*uy)),rv=double(sqrt(vx*vx+vy*vy));
        const E product=ru*rv;
        E x,y;
        // SAHF/JE takes the special branch for zero and unordered products.
        if(!(product>E(0) || product<E(0))) { x=(-uz)*vz; y=uz*vz; }
        else {
            const E factor=E(1)-(uz*vz)/product;
            x=(ux*vx-uy*vy)*factor;
            y=(vx*uy+(middle ? uz : ux)*vy)*factor;
        }
        return cpu_double3(double(x),double(y),double(ru*vz+rv*uz));
    };
    const cpu_double3 a(params[0],params[1],params[2]),b(params[3],params[4],params[5]);
    const cpu_double3 c(params[7],params[8],params[9]);
    const auto sa=multiply(p,a,false),sac=multiply(sa,c,true),sb=multiply(p,b,false);
    const E linearX=double((E(sac.x)-sb.x)-c.x),linearY=double((E(sac.y)-sb.y)-c.y);
    const E x=p.x,y=p.y,z=p.z,k=params[6],square=double(x*x+y*y);
    E cubeX=0,cubeY=0;
    if(square!=E(0)) {
        const E factor=double(E(1)-((z*k)*z)/square);
        cubeX=double((x*x-(y*k)*y)*(factor*x));
        cubeY=double(((x*k)*x-y*y)*(factor*y));
    }
    const E cubeZ=double((square*k-z*z)*z);
    return cpu_double3(double(linearX+cubeX),double(linearY+cubeY),double(cubeZ+sac.z));
#else
    // Portable fallback remains uncertified.
    cpu_double3 a = cpu_double3(params[0], params[1], params[2]);  // a-x, a-y, a-z
    cpu_double3 b = cpu_double3(params[3], params[4], params[5]);   // b-x, b-y, b-z
    const double K = params[6];                                      // [CONSTANTS] K = 3.0
    cpu_double3 C = cpu_double3(params[7], params[8], params[9]);   // appended Julia C

    cpu_double3 S = p;
    // Save original S
    double x = S.x;
    double y = S.y;
    double z = S.z;

    // Step 1: Compute S t* a
    cpu_double3 Sa = tpx_mul(S, a);

    // Step 2: Compute (S*a) t* C using the executable middle-multiply contract.
    cpu_double3 SaC = tpx_mul_middle(Sa, C);

    // Step 3: Compute S t* b
    cpu_double3 Sb = tpx_mul(S, b);

    // Step 4: linear_part = SaC - Sb - C
    cpu_double3 linear_part = SaC - Sb - C;

    // Step 5: Compute S^3 using direct triplex cube formula
    // From ASM 0x316-0x3F5:
    double rxy2 = x * x + y * y;

    cpu_double3 S3;
    if (rxy2 == 0.0) {
        // Original 0342..0357 zeroes X/Y, then evaluates Z at 03D8.
        S3 = cpu_double3(0.0,0.0,-z*z*z);
    } else {
        // z_factor = 1 - K * z^2 / rxy2
        double z_factor = 1.0 - K * z * z / rxy2;

        S3.x = x * (x * x - K * y * y) * z_factor;
        S3.y = y * (K * x * x - y * y) * z_factor;
        S3.z = z * (K * rxy2 - z * z);
    }

    // Step 6: Final result
    // S' = S^3 + linear_part
    // linear_part already has SaC - Sb (C subtraction done by engine)
    cpu_double3 result = cpu_double3(
        linear_part.x + S3.x,
        linear_part.y + S3.y,
        S3.z + SaC.z
    );

    return result;
#endif
}

constant FormulaInfo formula_info_ikenagabulb = {
    .id = FORMULA_IKENAGABULB,
    .name = "Ikenagabulb",
    .param_count = 9,
    .complexity = 0,
    .uses_transcendental = false
};
