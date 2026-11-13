// Original CODE samples the sphere map at folded XYZ, retaining Extended folds.
constexpr unsigned int FORMULA_ABOXMAPSFOLD = 10180;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ABoxMapSFoldPrepare_x87(double*, const double*);
extern "C" void ABoxMapSFoldFinish_x87(double*, const double*);
#endif

inline cpu_double3 aboxMapSFold(cpu_double3 p, constant double* params,
                               double& de, TLMSfunction map) {
#if defined(_MSC_VER) && defined(_M_X64)
    // Slots 8..15 hold four 80-bit values, never rounded across the callback.
    double state[16] = {p.x,p.y,p.z,de};
    ABoxMapSFoldPrepare_x87(state,params);
    TVec3D sample={state[4],state[5],state[6]};
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E = numeric::Extended80;
    const E fold(params[3]);
    auto box = [&](E v) { return abs(v+fold) - (abs(v-fold)+v); };
    // FST (not FSTP) at 0026/003B/0051 rounds only the callback buffer.
    // Keep the three original Extended folds alive across the map call.
    const E x=box(E(p.x)), y=box(E(p.y)), z=box(E(p.z));
    TVec3D sample={static_cast<double>(x),static_cast<double>(y),static_cast<double>(z)};
#else
    // Portable arithmetic remains uncertified against original x87 CODE.
    const double fold=params[3];
    const double x=abs(p.x+fold)-(abs(p.x-fold)+p.x);
    const double y=abs(p.y+fold)-(abs(p.y-fold)+p.y);
    const double z=abs(p.z+fold)-(abs(p.z-fold)+p.z);
    TVec3D sample={x,y,z};
#endif
    if (map) {
        // Delphi passes the same buffer for input and result.
        map(&sample,&sample,int(params[4]));
    } else {
        const auto rgb=cpu_map_sphere_sample(sample[0],sample[1],sample[2],int(params[4]));
        sample[0]=rgb.x; sample[1]=rgb.y; sample[2]=rgb.z;
    }
#if defined(_MSC_VER) && defined(_M_X64)
    state[4]=sample[0];state[5]=sample[1];state[6]=sample[2];
    ABoxMapSFoldFinish_x87(state,params);
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    const E offset(static_cast<float>(params[5]));
    const E scale(static_cast<float>(params[6]));
    const E rx=(E(sample[0])+offset)*scale+x;
    const E ry=(E(sample[1])+offset)*scale+y;
    const E rz=(E(sample[2])+offset)*scale+z;
    const E radius=(rx*rx+ry*ry)+rz*rz;
    const E m=!(radius>=E(params[2])) ? E(params[1])
        : (E(1)>=radius ? E(params[0])/radius : E(params[0]));
    de=static_cast<double>(E(de)*m);
    return cpu_double3(static_cast<double>(x*m+E(params[7])),
                       static_cast<double>(y*m+E(params[8])),
                       static_cast<double>(z*m+E(params[9])));
#else
    const double offset=static_cast<float>(params[5]);
    const double scale=static_cast<float>(params[6]);
    const double rx=x+(sample[0]+offset)*scale;
    const double ry=y+(sample[1]+offset)*scale;
    const double rz=z+(sample[2]+offset)*scale;
    const double radius=(rx*rx+ry*ry)+rz*rz;
    const double m=radius<params[2]?params[1]:(radius<=1.?params[0]/radius:params[0]);
    de*=m;
    return cpu_double3(x*m+params[7],y*m+params[8],z*m+params[9]);
#endif
}

template<>
cpu_double3 execute_formula<FORMULA_ABOXMAPSFOLD>(cpu_double3 p, constant double* params, double& de) {
    return aboxMapSFold(p,params,de,nullptr);
}

template<>
cpu_double3 execute_formula_with_sphere_map<FORMULA_ABOXMAPSFOLD>(cpu_double3 p, double* params,
                                                               double& de, TLMSfunction map) {
    return aboxMapSFold(p,params,de,map);
}

constant FormulaInfo formula_info_ABoxMapSFold = {
    .id = FORMULA_ABOXMAPSFOLD, .name = "ABoxMapSFold", .param_count = 7,
    .complexity = 720, .uses_transcendental = false
};
