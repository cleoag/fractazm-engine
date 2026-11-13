#include "ifs_original_state.h"
// helistairsIFS: original 600-byte SSE2/x87 formula, DEoption 20.
constexpr unsigned int FORMULA_HELISTAIRSIFS = 10306;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void HelistairsSnap_x87(double* state);
#endif

template<>
cpu_double3 execute_formula<FORMULA_HELISTAIRSIFS>(cpu_double3 p, constant double* params, double &de) {
#ifdef MB3D_SOFTWARE_EXT80
    using namespace mb3d::numeric;
    Arm64ApplyIFSState native(p,params,5);
    using E=Arm64ApplyIFSState::E;
    const auto r=native.rotate(params+7);
    const E x=r.x,y=r.y,rz=r.z,n=int(params[16]),height=params[17];
    const E thick=params[18],position=params[19],width=params[20];
    const E radius=double(position>E(0) ? sqrt(x*x+y*y) : (abs(y)>abs(x) ? abs(y) : abs(x)));
    const E angle=(extendedAtan2(y,x)*params[22])*n;
    const E step=angle.integer();
    const E u=double(((angle-step)*params[23])/n);
    const E half=double((-abs(height))*params[24]),period=double(abs((-abs(height))*n));
    const E local=rz+step*(-abs(height));
    const E z=double(local-round(local/period)*period);
    E distance;
    if(std::signbit(params[17])) {
        E d;
        if(-z>u-half) d=z-half;
        else if(!(-z>u+half)) d=z+half;
        else d=-u;
        d=abs(d)-thick;
        const E radial=abs(radius-abs(position))-width;
        distance=radial>d ? radial : d;
    } else {
        const E vertical=abs(abs(z)+half),angular=std::signbit(double(z)) ? u : -u;
        distance=(angular>vertical ? angular : vertical)-thick;
        const E radial=radius-abs(position);
        if(radial-width>distance) distance=radial-width;
        const E upper=E(double(radial))+width;
        if(!(upper>distance)) distance=upper;
    }
    params[MAX_FORMULA_PARAMS-1]=double(abs(E(params[0])+E(int(params[ITER_COUNT_SLOT]))*params[6]));
    return native.finish(distance,de);
#else
    const double scale=params[1];
    const bool apply=int(params[5]) != 0;
    const cpu_double3 transformed(p.x*scale+params[4],p.y*scale+params[3],p.z*scale+params[2]);
    if (apply) {
        params[DIFS_VARYSCALE_SLOT]*=scale;
        params[DIFS_COMMIT_XYZ_SLOT]=1.0;
    }
    const cpu_double3 r=rotMatrix3FromFloats(params,7)*transformed;
    const double n=double(int(params[16])), height=params[17];
    const double thick=params[18], position=params[19], width=params[20];
    // Position selects the circular or square radius independently of Style.
    double radius=position>0.0 ? sqrt(r.x*r.x+r.y*r.y)
        : (fabs(r.y)>fabs(r.x) ? fabs(r.y) : fabs(r.x));
    const double half=-fabs(height)*.5;
#if defined(_MSC_VER) && defined(_M_X64)
    double snap[]={r.x,r.y,r.z,n,height,0.1591549430918953358,6.2831853071795864769};
    HelistairsSnap_x87(snap);
    const double u=snap[0], z=snap[1];
#else
    const long double angle=std::atan2((long double)r.y,(long double)r.x)*double(0.1591549430918953358)*n;
    const long double rounded=std::nearbyint(angle);
    const int step=std::isfinite(rounded) && rounded>=-2147483648.0L && rounded<=2147483647.0L
        ? int(rounded) : std::numeric_limits<int32_t>::min();
    const double u=(angle-step)*double(6.2831853071795864769)/n;
    const double period=fabs(-fabs(height)*n);
    const long double local=(long double)r.z-(long double)step*fabs(height);
    const double z=local-std::nearbyint(local/period)*period;
#endif
    double distance;
    // These branches inspect the sign bit, including negative zero.
    if (std::signbit(height)) {
        double d;
        if (-z > u-half) d=z-half;
        else if (!(-z > u+half)) d=z+half;
        else d=-u;
        d=fabs(d)-thick;
        const double radial=fabs(radius-fabs(position))-width;
        distance=radial>d ? radial : d;
    } else {
        const double vertical=fabs(fabs(z)+half);
        const double angular=std::signbit(z) ? u : -u;
        distance=(angular>vertical ? angular : vertical)-thick;
        const double radial=radius-fabs(position);
        // Original stores this intermediate in Rout before selecting the DE.
        distance=radial-width>distance ? radial-width : distance;
        distance=radial+width>distance ? distance : radial+width;
    }
    de=apply ? distance : distance/scale;
    params[MAX_FORMULA_PARAMS-1]=fabs(params[0]+params[ITER_COUNT_SLOT]*params[6]);
    return apply ? transformed : p;
#endif
}

cpu_double3 helistairsIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HELISTAIRSIFS>(p,params,de);
}
constant FormulaInfo formula_info_helistairsIFS = {
    .id=FORMULA_HELISTAIRSIFS, .name="helistairsIFS",
    .param_count=15, .complexity=600, .uses_transcendental=true
};
