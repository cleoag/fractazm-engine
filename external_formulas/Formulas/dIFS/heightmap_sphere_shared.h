#pragma once
// Common original-code state layout. No global callback state is used.
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" int SphereHeightMapPrepare_x87(double*,const double*);
extern "C" void SphereHeightMapFinish_x87(double*,const double*,int,int);
extern "C" int SphereHMVoidPrepare_x87(double*,const double*);
extern "C" void SphereHMVoidFinish_x87(double*,const double*,int,int);
#endif

template<bool Void>
inline cpu_double3 execute_spherical_heightmap(cpu_double3 p,double* params,double& de,TLMSfunction map){
    const numeric::X87Scope extendedScope;
    double s[]={p.x,p.y,p.z,de,params[DIFS_ROLD_SLOT],params[MAX_FORMULA_PARAMS-1],0.,0.,0.,0.,.5};
#if defined(_MSC_VER) && defined(_M_X64)
    const bool sample=(Void?SphereHMVoidPrepare_x87(s,params):SphereHeightMapPrepare_x87(s,params))!=0;
#else
    const numeric::X87Real x=(numeric::X87Real)p.x-params[2],y=(numeric::X87Real)p.y-params[3],z=(numeric::X87Real)p.z-params[4];
    const numeric::X87Real rx=((x*params[8]+y*params[9])+z*params[10])*params[5];
    const numeric::X87Real ry=((x*params[11]+y*params[12])+z*params[13])*params[6];
    const numeric::X87Real rz=(x*params[14]+y*params[15])+z*params[16];
    const numeric::X87Real distance=numeric::x87Sqrt((rz*rz+ry*ry)+rx*rx)-numeric::X87Real(1.);
    numeric::X87Real threshold=numeric::x87Abs((numeric::X87Real)params[7])+s[4];
    if constexpr(Void)threshold+=params[22];
    const bool sample=threshold>=numeric::x87Abs(distance);
    if(sample){
        s[6]=double(rx);s[7]=double(ry);s[8]=double(rz);s[9]=double(distance);
        if constexpr(Void){
            numeric::X87Real m=numeric::x87Abs((numeric::X87Real)s[6]),v=numeric::x87Abs((numeric::X87Real)s[7]);
            if(v>=m)m=v;v=numeric::x87Abs((numeric::X87Real)s[8]);if(v>=m)m=v;
            s[9]=double((m-numeric::X87Real(1.))*params[20]+(numeric::X87Real(1.)-params[20])*s[9]);
        }
    } else s[3]=double(Void?numeric::x87Abs(distance):distance);
#endif
    if(sample){
        const int number=int(params[0]);
        if(map){TVec3D xyz={s[6],s[7],s[8]},rgb={};map(&rgb,&xyz,number);for(int i=0;i<3;++i)s[6+i]=rgb[i];}
        else {auto rgb=cpu_map_sphere_sample(s[6],s[7],s[8],number);s[6]=rgb.x;s[7]=rgb.y;s[8]=rgb.z;}
        const int channel=int(params[1])&3,trap=int(params[17])&3;
#if defined(_MSC_VER) && defined(_M_X64)
        if constexpr(Void)SphereHMVoidFinish_x87(s,params,channel,trap);
        else SphereHeightMapFinish_x87(s,params,channel,trap);
#else
        numeric::X87Real d=(numeric::X87Real)s[9]-((numeric::X87Real)s[6+channel]-numeric::X87Real(.5))*params[7];
        if constexpr(Void)d=numeric::x87Abs(d)-params[21];
        s[3]=double(d);s[5]=double(((numeric::X87Real)s[6+trap]+params[18])*params[19]);
#endif
    }
    de=s[3];params[MAX_FORMULA_PARAMS-1]=s[5];return p;
}
