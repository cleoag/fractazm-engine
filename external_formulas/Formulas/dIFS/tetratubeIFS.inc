#include "tube_original_arm64.h"
// Original M3F arithmetic; portable fallback remains uncertified.
constexpr unsigned int FORMULA_TETRATUBEIFS = 10219;


#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void tetratubeIFSOriginal_x87(double*, const double*, const float*, const int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_TETRATUBEIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    const int apply=int(params[11]);
    const cpu_double3 transformed(p.x*params[3]+params[6],p.y*params[3]+params[5],p.z*params[3]+params[4]);
    if(apply!=0) params[DIFS_VARYSCALE_SLOT]*=params[3];
    double state[]={transformed.y,transformed.z,transformed.x,params[MAX_FORMULA_PARAMS-1],0.};
    tetratubeIFSOriginal_x87(state,params,nullptr,&apply);
    params[MAX_FORMULA_PARAMS-1]=state[3];
    de=state[4];
    return apply!=0 ? transformed : p;
#elif defined(MB3D_SOFTWARE_EXT80)
    Arm64TubeState state(p,params);
    using E=Arm64TubeState::E;
    E s0=-state.c,s1=state.b,s2=state.a;
    const auto swap=[](E& a,E& b) { const E t=a; a=b; b=t; };
    if(s0>=s1) swap(s0,s1);
    if(s0>=s2) swap(s0,s2);
    s0=-s0; swap(s0,s1); s0=-s0;
    if(s0>=s2) swap(s0,s2);
    s0=-s0; swap(s0,s1);
    // JAE skips on ordered >=; unordered comparisons take the swap.
    if(!(s0>=s1)) swap(s0,s1);
    if(!(s0>=s2)) swap(s0,s2);
    swap(s0,s1);
    if(!(s0>=s2)) swap(s0,s2);
    state.a=s2; state.b=s1; state.c=s0;
    return state.finish(de);
#else

    const double Radius = params[0];
    const double Sub_Y = params[1];
    const double Sub_Z = params[2];
    const double Scale = params[3];
    const double Z_add = params[4];
    const double Y_add = params[5];
    const double X_add = params[6];
    const double cos_xz = params[7];
    const double sin_xz = params[8];
    const double cos_xy = params[9];
    const double sin_xy = params[10];
    const int Apply_scale_add = int(params[11]);
    const double Sphere_pos = params[12];
    const double Sphere_R = params[13];

    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;
    double x = p.x * Scale + X_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
    }

    double s0 = -x, s1 = z, s2 = y;
    if (s0 >= s1) { double t = s0; s0 = s1; s1 = t; }
    if (s0 >= s2) { double t = s0; s0 = s2; s2 = t; }
    s0 = -s0;
    { double t = s0; s0 = s1; s1 = t; }
    s0 = -s0;
    if (s0 >= s2) { double t = s0; s0 = s2; s2 = t; }
    s0 = -s0;
    { double t = s0; s0 = s1; s1 = t; }
    if (s0 < s1) { double t = s0; s0 = s1; s1 = t; }
    if (s0 < s2) { double t = s0; s0 = s2; s2 = t; }
    { double t = s0; s0 = s1; s1 = t; }
    if (s0 < s2) { double t = s0; s0 = s2; s2 = t; }
    double a = s2, b = s1, c = s0;

    double next_a = a * sin_xz - c * cos_xz;
    c = a * cos_xz + c * sin_xz;
    a = next_a;
    next_a = a * sin_xy - b * cos_xy;
    b = a * cos_xy + b * sin_xy;
    a = next_a;

    double db = b - Sub_Y;
    double dc = c - Sub_Z;
    double cyl_de = sqrt(db * db + dc * dc) - Radius;

    double sphere_de = sqrt((abs(a) - Sphere_pos) * (abs(a) - Sphere_pos)
                          + db * db + dc * dc) - Sphere_R;

    const double OTClr_Tube = params[14];
    const double OTClr_Sphere = params[15];
    double shape_de;
    if (!(sphere_de > cyl_de)) {
        shape_de = sphere_de;
        params[MAX_FORMULA_PARAMS - 1] = OTClr_Sphere;
    } else {
        shape_de = cyl_de;
        params[MAX_FORMULA_PARAMS - 1] = OTClr_Tube;
    }

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return Apply_scale_add != 0 ? cpu_double3(x, y, z) : p;
#endif
}

cpu_double3 tetratubeIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_TETRATUBEIFS>(p, params, de);
}

constant FormulaInfo formula_info_tetratubeIFS = {
    .id = FORMULA_TETRATUBEIFS,
    .name = "tetratubeIFS",
    .param_count = 16,
    .complexity = 480,
    .uses_transcendental = false
};

