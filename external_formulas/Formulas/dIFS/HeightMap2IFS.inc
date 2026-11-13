// Original HeightMap2IFS: Riemann inversion followed by planar height sampling.
// params 0..20: expanded options; 21..22: original constants.
constexpr unsigned int FORMULA_HEIGHTMAP2IFS = 10198;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" int HeightMap2IFSPrepare_x87(double*, const double*, int);
extern "C" void HeightMap2IFSFinish_x87(double*, const double*, int, int);
#endif

template<>
cpu_double3 execute_formula_with_direct_map<FORMULA_HEIGHTMAP2IFS>(
    cpu_double3 p, constant double* params, double& de, TLMSfunction map) {
    const numeric::X87Scope extendedScope;
    double s[] = {p.x,p.y,p.z,de,params[DIFS_ROLD_SLOT],
                  params[MAX_FORMULA_PARAMS-1],0.,0.,0.,0.,.5};
#if defined(_MSC_VER) && defined(_M_X64)
    const bool sample=HeightMap2IFSPrepare_x87(s,params,int(params[20]))!=0;
#else
    numeric::X87Real x=p.x,y=p.y,z=p.z;
    numeric::X87Real inv=numeric::X87Real(1.)/((x*x+y*y)+z*z);
    // AND clears carry before JBE: clamp on <= or unordered, using bit zero.
    if (!(inv>params[21])) inv=params[21+(int(params[20])&1)];
    x*=inv;y*=inv;z*=inv;s[3]=double(inv);
    const numeric::X87Real rx=(x*params[7]+y*params[8])+z*params[9];
    const numeric::X87Real ry=(x*params[10]+y*params[11])+z*params[12];
    const numeric::X87Real rz=(x*params[13]+y*params[14])+z*params[15];
    const numeric::X87Real center=(numeric::X87Real)params[6]*numeric::X87Real(.5);
    const bool sample=numeric::x87Abs((numeric::X87Real)params[6])+s[4]>=numeric::x87Abs((rz+params[3])-center);
    if(sample){s[6]=double(rx*params[4]-params[1]);s[7]=double(ry*params[5]-params[2]);s[9]=double(rz);}
    else s[3]=double(((center-params[3])-rz)/s[3]);
#endif
    if(sample){
        const int number=int(params[0]);
        if(map){TVec3D uv={s[6],s[7],0.},rgb={};map(&rgb,&uv,number);for(int i=0;i<3;++i)s[6+i]=rgb[i];}
        else {auto rgb=cpu_map_direct_sample(s[6],s[7],number);s[6]=rgb.x;s[7]=rgb.y;s[8]=rgb.z;}
        const int channel=int(params[16])&3,trap=int(params[17])&3;
#if defined(_MSC_VER) && defined(_M_X64)
        HeightMap2IFSFinish_x87(s,params,channel,trap);
#else
        s[3]=double((((numeric::X87Real)s[6+channel]*params[6]-params[3])-s[9])/s[3]);
        s[5]=double(((numeric::X87Real)s[6+trap]+params[18])*params[19]);
#endif
    }
    de=s[3];params[MAX_FORMULA_PARAMS-1]=s[5];return p;
}
template<>
cpu_double3 execute_formula<FORMULA_HEIGHTMAP2IFS>(cpu_double3 p,constant double* params,double& de){
    return execute_formula_with_direct_map<FORMULA_HEIGHTMAP2IFS>(p,params,de,nullptr);
}
cpu_double3 HeightMap2IFS_formula(cpu_double3 p,constant double* params,double& de){return execute_formula<FORMULA_HEIGHTMAP2IFS>(p,params,de);}
constant FormulaInfo formula_info_HeightMap2IFS={.id=FORMULA_HEIGHTMAP2IFS,.name="HeightMap2IFS",.param_count=23,.complexity=460,.uses_transcendental=false};
