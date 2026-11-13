// Planar height map, ported from the shipped HeightMapIFS.m3f x86 CODE.
// Expanded parameters: map number; X/Y/H offsets; X/Y/H scales;
// row-major 3x3 rotation (7..15); height/trap channels; trap offset/multiplier.
constexpr unsigned int FORMULA_HEIGHTMAPIFS = 10189;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" int HeightMapPrepare_x87(double* state, const double* params);
extern "C" void HeightMapFinish_x87(double* state, const double* params, int heightChannel, int trapChannel);
#endif

template<>
cpu_double3 execute_formula_with_direct_map<FORMULA_HEIGHTMAPIFS>(
    cpu_double3 p, constant double* params, double& de, TLMSfunction map) {
    const numeric::X87Scope extendedScope;
    double state[] = {p.x, p.y, p.z, de, params[DIFS_ROLD_SLOT],
                      params[MAX_FORMULA_PARAMS - 1], 0.0, 0.0, 0.0, 0.0};
#if defined(_MSC_VER) && defined(_M_X64)
    const bool sample = HeightMapPrepare_x87(state, params) != 0;
#else
    // The x87 stack finishes with first/middle/last matrix triples as X/Y/Z.
    const numeric::X87Real x=p.x, y=p.y, z=p.z;
    const numeric::X87Real rx=(x*params[7]+y*params[8])+z*params[9];
    const numeric::X87Real ry=(x*params[10]+y*params[11])+z*params[12];
    const numeric::X87Real rz=(x*params[13]+y*params[14])+z*params[15];
    const numeric::X87Real center=static_cast<numeric::X87Real>(params[6])*numeric::X87Real(0.5);
    // An unordered FCOM (NaN) takes the early-return path as well.
    const bool sample = numeric::x87Abs(static_cast<numeric::X87Real>(params[6]))+state[4]
                        >= numeric::x87Abs((rz+params[3])-center);
    if (sample) {
        state[6]=static_cast<double>(rx*params[4]-params[1]);
        state[7]=static_cast<double>(ry*params[5]-params[2]);
        state[9]=static_cast<double>(rz);
    } else {
        state[3]=static_cast<double>((center-params[3])-rz);
    }
#endif
    if (sample) {
        const int mapNumber=static_cast<int32_t>(params[0]);
        if (map) {
            TVec3D uv={state[6],state[7],0.0}, rgb={};
            map(&rgb,&uv,mapNumber);
            for (int i=0;i<3;++i) state[6+i]=rgb[i];
        } else {
            const auto rgb=cpu_map_direct_sample(state[6],state[7],mapNumber);
            state[6]=rgb.x;state[7]=rgb.y;state[8]=rgb.z;
        }
        const int heightChannel=static_cast<int32_t>(params[16]) & 3;
        const int trapChannel=static_cast<int32_t>(params[17]) & 3;
        // Masked channel 3 intentionally selects rotated Z, as in the original
        // stack layout. The early-return path preserves the incoming trap.
#if defined(_MSC_VER) && defined(_M_X64)
        HeightMapFinish_x87(state,params,heightChannel,trapChannel);
#else
        state[3]=static_cast<double>((static_cast<numeric::X87Real>(state[6+heightChannel])
                     *params[6]-params[3])-state[9]);
        state[5]=static_cast<double>((static_cast<numeric::X87Real>(state[6+trapChannel])
                     +params[18])*params[19]);
#endif
    }
    de=state[3];
    params[MAX_FORMULA_PARAMS - 1]=state[5];
    return p;
}

template<>
cpu_double3 execute_formula<FORMULA_HEIGHTMAPIFS>(cpu_double3 p, constant double* params, double& de) {
    return execute_formula_with_direct_map<FORMULA_HEIGHTMAPIFS>(p,params,de,nullptr);
}

cpu_double3 HeightMapIFS_formula(cpu_double3 p, constant double* params, double& de) {
    return execute_formula<FORMULA_HEIGHTMAPIFS>(p,params,de);
}

constant FormulaInfo formula_info_HeightMapIFS = {
    .id = FORMULA_HEIGHTMAPIFS,
    .name = "HeightMapIFS",
    .param_count = 13,
    .complexity = 420,
    .uses_transcendental = false
};
