#pragma once
#include <cstring>

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" int CylinderHeightMapPrepare_x87(double*, const void*);
extern "C" void CylinderHeightMapFinish_x87(double*, const void*);
extern "C" int CylinderHMVoidPrepare_x87(double*, const void*);
extern "C" void CylinderHMVoidFinish_x87(double*, const void*);
extern "C" int TorusHMVoidPrepare_x87(double*, const void*);
extern "C" void TorusHMVoidFinish_x87(double*, const void*);
#endif

// ARM keeps the existing binary64 libm angle boundary. This is not an
// implementation of x87 FPATAN; independently checked finite fixtures define
// the current support scope. Algebraic/radius intermediates remain extended.
inline numeric::X87Real heightmapAngle(numeric::X87Real y, numeric::X87Real x) {
#ifdef MB3D_SOFTWARE_EXT80
    return numeric::X87Real(atan2l(double(y),double(x)));
#else
    return atan2l(y,x);
#endif
}

// Kind: solid cylinder, cylindrical shell, toroidal shell. Scratch retains the
// original byte layout: channel 3 aliases the two Single stores in cylinders,
// and a Single distance plus the high half of the stored axis in the torus.
template<int Kind>
inline cpu_double3 execute_cyl_torus_heightmap(cpu_double3 p,double* params,double& de,TLMSfunction map) {
    const numeric::X87Scope extendedScope;
    double s[11]={p.x,p.y,p.z,de,params[MAX_FORMULA_PARAMS-1]};
    constexpr int constants=Kind==1?21:20;
#if defined(_MSC_VER) && defined(_M_X64)
    alignas(8) unsigned char packed[112]={};
    unsigned char* pv=packed+96;
    const double center=.5;std::memcpy(pv-8,&center,8);
    for(int i=0;i<constants;++i){
        if(i==0||i==1||i==17){const int32_t v=static_cast<int32_t>(params[i]);std::memcpy(pv-12-4*i,&v,4);}
        else {const float v=static_cast<float>(params[i]);std::memcpy(pv-12-4*i,&v,4);}
    }
    std::memcpy(pv,params+constants,(Kind==0?1:2)*sizeof(double));
    bool sample;
    if constexpr(Kind==0)sample=CylinderHeightMapPrepare_x87(s,pv)!=0;
    else if constexpr(Kind==1)sample=CylinderHMVoidPrepare_x87(s,pv)!=0;
    else sample=TorusHMVoidPrepare_x87(s,pv)!=0;
#else
    const numeric::X87Real x=(numeric::X87Real)p.x-params[2],y=(numeric::X87Real)p.y-params[3],z=(numeric::X87Real)p.z-params[4];
    numeric::X87Real a=(x*params[8]+y*params[9])+z*params[10];
    numeric::X87Real b=(x*params[11]+y*params[12])+z*params[13];
    numeric::X87Real axis=(x*params[14]+y*params[15])+z*params[16];
    numeric::X87Real distance,ring=0,threshold=numeric::x87Abs((numeric::X87Real)params[7]);
    auto storeSingle=[&](int offset,numeric::X87Real v){const float f=static_cast<float>(v);std::memcpy(reinterpret_cast<unsigned char*>(s+5)+offset,&f,4);};
    if constexpr(Kind==2){
        a=double(a);b=double(b);axis=double(axis);s[8]=double(axis);
        ring=numeric::x87Sqrt(a*a+b*b)-params[5];distance=numeric::x87Sqrt(ring*ring+axis*axis)-params[6];
        threshold+=params[constants+1];threshold+=params[constants+1];
    }else{
        numeric::X87Real radius=numeric::x87Sqrt(b*b+a*a);
        if constexpr(Kind==1)radius+=axis*params[20];
        storeSingle(28,radius);distance=radius-params[6];
        const numeric::X87Real cap=numeric::x87Abs(axis)-params[5];
        if(cap>=distance)distance=cap;
    }
    const bool sample=threshold>=numeric::x87Abs(distance);
    if(sample){
        storeSingle(24,distance);
        s[5]=double(heightmapAngle(a,b)*params[constants]);
        if constexpr(Kind==2)s[6]=double(heightmapAngle(ring,axis)*params[constants]);
        else s[6]=double(axis*params[constants]/params[6]);
    }else s[3]=double(Kind==0?distance:numeric::x87Abs(distance));
#endif
    if(sample){
        const int number=static_cast<int>(params[0]);
        if(map){TVec3D uv={s[5],s[6],0.},rgb={};map(&rgb,&uv,number);for(int i=0;i<3;++i)s[5+i]=rgb[i];}
        else {auto rgb=cpu_map_direct_sample(s[5],s[6],number);s[5]=rgb.x;s[6]=rgb.y;s[7]=rgb.z;}
#if defined(_MSC_VER) && defined(_M_X64)
        if constexpr(Kind==0)CylinderHeightMapFinish_x87(s,pv);
        else if constexpr(Kind==1)CylinderHMVoidFinish_x87(s,pv);
        else TorusHMVoidFinish_x87(s,pv);
#else
        float storedDistance,storedRadius;
        std::memcpy(&storedDistance,reinterpret_cast<unsigned char*>(s+5)+24,4);
        std::memcpy(&storedRadius,reinterpret_cast<unsigned char*>(s+5)+28,4);
        numeric::X87Real displacement=((numeric::X87Real)s[5+(int(params[1])&3)]-numeric::X87Real(.5))*params[7];
        if constexpr(Kind!=2)displacement*=storedRadius;
        numeric::X87Real distance=(numeric::X87Real)storedDistance-displacement;
        if constexpr(Kind!=0)distance=numeric::x87Abs(distance)-params[constants+1];
        s[3]=double(distance);s[4]=double(((numeric::X87Real)s[5+(int(params[17])&3)]+params[18])*params[19]);
#endif
    }
    de=s[3];params[MAX_FORMULA_PARAMS-1]=s[4];return p;
}
