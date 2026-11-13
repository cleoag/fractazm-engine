#include "tube_original_arm64.h"
// Original M3F arithmetic; portable fallback remains uncertified.
constexpr unsigned int FORMULA_DODECATUBEIFS = 10230;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void dodecatubeIFSOriginal_x87(double*, const double*, const float*, const int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_DODECATUBEIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    const int apply=int(params[11]);
    const cpu_double3 transformed(p.x*params[3]+params[6],p.y*params[3]+params[5],p.z*params[3]+params[4]);
    if(apply!=0) params[DIFS_VARYSCALE_SLOT]*=params[3];
    double state[]={transformed.y,transformed.z,transformed.x,params[MAX_FORMULA_PARAMS-1],0.};
    dodecatubeIFSOriginal_x87(state,params,nullptr,&apply);
    params[MAX_FORMULA_PARAMS-1]=state[3];
    de=state[4];
    return apply!=0 ? transformed : p;
#elif defined(MB3D_SOFTWARE_EXT80)
    Arm64TubeState state(p,params);
    using E=Arm64TubeState::E;
    E& a=state.a; E& b=state.b; E& c=state.c;
    for(int i=0;i<2;++i) {
        a=abs(a); b=abs(b); c=abs(c);
        E dot=a*params[16]+(b*params[17]+c*params[18]);
        E factor=-(dot+abs(dot));
        a=double(factor*params[16]+a);
        b=double(factor*params[17]+b);
        c=double(factor*params[18]+c);
        dot=a*params[17]+((-(b*params[18]))+(-(c*params[16])));
        factor=dot+abs(dot);
        a=double(-(factor*params[17])+a);
        b=double(factor*params[18]+b);
        c=double(factor*params[16]+c);
    }
    a=abs(a); b=abs(b); c=abs(c);
    return state.finish(de);
#else

    const double Radius = params[0];              // [edi-0x10]
    const double Sub_Y = params[1];               // [edi-0x18]
    const double Sub_Z = params[2];               // [edi-0x20]
    const double Scale = params[3];               // [edi-0x30] .2Doubles
    const double Z_add = params[4];               // [edi-0x48]
    const double Y_add = params[5];               // [edi-0x40]
    const double X_add = params[6];               // [edi-0x40] (packed with Y_add)
    const int Apply_scale_add = int(params[11]); // [edi-0x6c]
    const double Sphere_pos = params[12];         // [edi-0x74]
    const double Sphere_R = params[13];           // [edi-0x7c]
    const double OTClr_Tube = params[14];         // [edi-0x84] — orbit trap color for tube
    const double OTClr_Sphere = params[15];       // [edi-0x8c] — orbit trap color for sphere

    const double c_phi1 = params[16];  // [edi+0x00]
    const double c_phi2 = params[17];   // [edi+0x08]
    const double c_half = params[18];                 // [edi+0x10]

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;
    double state_x = p.x;
    double state_y = p.y;
    double state_z = p.z;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
        state_x = x;
        state_y = y;
        state_z = z;
    }


    for (int iter = 0; iter < 2; iter++) {
        y = fabs(y);
        z = fabs(z);
        x = fabs(x);

        double dot1 = y * c_phi1 + z * c_phi2 + x * c_half;
        double s1 = -(dot1 + fabs(dot1)); // -2*max(dot1, 0)
        y += s1 * c_phi1;
        z += s1 * c_phi2;
        x += s1 * c_half;

        double dot2 = y * c_phi2 - z * c_half - x * c_phi1;
        double s2 = dot2 + fabs(dot2); // 2*max(dot2, 0)
        y -= s2 * c_phi2;
        z += s2 * c_half;
        x += s2 * c_phi1;
    }

    y = fabs(y);
    z = fabs(z);
    x = fabs(x);

    double sin_xz = params[7];
    double cos_xz = params[8];
    double rot1_y = y * cos_xz - x * sin_xz;   // new [esp]
    double rot1_x = y * sin_xz + x * cos_xz;   // new [esp+0x10]
    y = rot1_y;
    x = rot1_x;

    double sin_xy = params[9];
    double cos_xy = params[10];
    double rot2_y = y * cos_xy - z * sin_xy;   // new [esp]
    double rot2_z = y * sin_xy + z * cos_xy;   // new [esp+8]
    y = rot2_y;
    z = rot2_z;

    double dz = z - Sub_Y;
    double dx = x - Sub_Z;
    double tube_de = sqrt(dz * dz + dx * dx) - Radius;

    double sy_val = fabs(y) - Sphere_pos;
    double sphere_de = sqrt(sy_val * sy_val + dz * dz + dx * dx) - Sphere_R;

    const bool tube = sphere_de > tube_de;
    double shape_de = tube ? tube_de : sphere_de;
    params[MAX_FORMULA_PARAMS - 1] = tube ? OTClr_Tube : OTClr_Sphere;

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return cpu_double3(state_x, state_y, state_z);
#endif
}

cpu_double3 dodecatubeIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_DODECATUBEIFS>(p, params, de);
}

constant FormulaInfo formula_info_dodecatubeIFS = {
    .id = FORMULA_DODECATUBEIFS,
    .name = "dodecatubeIFS",
    .param_count = 16,
    .complexity = 466,
    .uses_transcendental = false
};

