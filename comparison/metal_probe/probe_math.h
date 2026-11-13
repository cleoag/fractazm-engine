#pragma once
#include "probe_abi.h"
#ifdef __METAL_VERSION__
#define PROBE_REF thread
#else
#include <cmath>
#define PROBE_REF
using std::sqrt; using std::sin; using std::cos; using std::atan2;
using std::log; using std::exp2; using std::pow; using std::fabs;
using std::isfinite;
#endif

// Shared experimental arithmetic for CPU FP32/FP64 differential diagnostics.
// This is not the production numerical profile or the full M3P caller contract.
template<class T> struct V3 { T x,y,z; };
template<class T> inline V3<T> vadd(V3<T> a,V3<T> b) { return {a.x+b.x,a.y+b.y,a.z+b.z}; }
template<class T> inline V3<T> vsub(V3<T> a,V3<T> b) { return {a.x-b.x,a.y-b.y,a.z-b.z}; }
template<class T> inline V3<T> vmul(V3<T> a,T s) { return {a.x*s,a.y*s,a.z*s}; }
template<class T> inline T vdot(V3<T> a,V3<T> b) { return (a.x*b.x+a.y*b.y)+a.z*b.z; }
template<class T> inline T vlength(V3<T> a) { return sqrt(vdot(a,a)); }
template<class T> inline V3<T> vcross(V3<T> a,V3<T> b) {
    return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
}
template<class T> inline V3<T> vunit(V3<T> a) {
    T d=vlength(a); return d>T(0) ? vmul(a,T(1)/d) : V3<T>{T(0),T(1),T(0)};
}
template<class T> inline T pmax(T a,T b) { return a>b ? a:b; }
template<class T> inline T pmin(T a,T b) { return a<b ? a:b; }
template<class T> inline T clamp01(T a) { return pmin(T(1),pmax(T(0),a)); }

template<class T> struct BoxResult { V3<T> position; T distance,vary,trap; };
// Source: external_formulas/Formulas/dIFS/boxIFS.inc, finite parameters.
// Twist deliberately uses FP32/FP64 libm here, not the original x87 arithmetic.
template<class T> inline BoxResult<T> box_formula(V3<T> p,PROBE_REF const T* a) {
    V3<T> q={p.x*a[3]+a[6],p.y*a[3]+a[5],p.z*a[3]+a[4]};
    T x=q.x,y=q.y;
    if (a[8]!=T(0)) {
        T angle=q.z*a[8],s=sin(angle),c=cos(angle);
        x=s*q.x+c*q.y; y=c*q.x-s*q.y;
    }
    T dx=fabs(x)-a[2]-q.z*a[7],dy=fabs(y)-a[1],dz=fabs(q.z)-a[0];
    T distance=pmax(pmax(dz,dx),dy);
    bool apply=a[9]!=T(0);
    if (!apply) distance/=a[3];
    V3<T> committed=apply?q:p;
    int option=int(a[10])&3;
    T source=option==0?T(4):option==1?committed.x:option==2?committed.y:committed.z;
    return {committed,distance,apply?a[3]:T(1),source*a[12]+a[11]};
}

// Three statically specialized shapes. Only these declared parameter subsets
// are supported; there is no runtime formula compilation or CPU fallback.
template<class T,int Formula> inline T shape_de(V3<T> p,ProbeUInt iterations) {
    if(Formula==0) {
        // SphereIFS: Radius=1, Scale=1, adds=0, reciprocal sizes=(1,.8,1.25),
        // Apply=0, Inside_radius=-1. OTrap is not part of this synthetic shader.
        T sx=p.x,sy=p.y*T(.8),sz=p.z*T(1.25);
        T radius=sqrt((sx*sx+sy*sy)+sz*sz);
        return pmax(radius-T(1),T(-1)-radius)/T(1.25);
    } else if(Formula==1) {
        T a[16]={T(.8),T(.8),T(.8),T(1),T(0),T(0),T(0),T(.1),T(.65f),T(0),T(0),T(0),T(0),T(0),T(0),T(0)};
        return box_formula(p,a).distance;
    } else {
        // Menger3 core with identity R1/R2, Scale=3, CScaleXYZ=1.
        // A finite six-iteration synthetic box DE is used, not the M3P driver.
        T derivative=T(1);
        for(ProbeUInt i=0;i<iterations;++i) {
            T x=fabs(p.x),y=fabs(p.y),z=fabs(p.z),tmp;
            if(x<y) { tmp=x;x=y;y=tmp; }
            if(x<z) { tmp=x;x=z;z=tmp; }
            if(y<z) { tmp=y;y=z;z=tmp; }
            derivative*=T(3);
            p.x=x*T(3)-T(2);p.y=y*T(3)-T(2);p.z=z*T(3);
            if(p.z>T(1)) p.z-=T(2);
        }
        return (pmax(pmax(fabs(p.x),fabs(p.y)),fabs(p.z))-T(1))/derivative;
    }
}
template<class T,int Formula> inline T counted_de(V3<T> p,PROBE_REF const ProbeScene& scene,
                                                  PROBE_REF ProbeUInt& calls) {
    ++calls; return shape_de<T,Formula>(p,scene.iterations);
}

template<class T,int Formula> inline ProbePixel render_pixel(ProbeUInt x,ProbeUInt y,
                                                            PROBE_REF const ProbeScene& scene) {
    ProbePixel result={};
    V3<T> origin={T(3),T(2.3),T(4)};
    V3<T> forward=vunit(vmul(origin,T(-1)));
    V3<T> right=vunit(vcross(forward,V3<T>{T(0),T(1),T(0)}));
    V3<T> up=vcross(right,forward);
    T u=((T(x)+T(.5))/T(scene.width)*T(2)-T(1))*T(scene.width)/T(scene.height)*T(scene.fov_scale);
    T v=(T(1)-(T(y)+T(.5))/T(scene.height)*T(2))*T(scene.fov_scale);
    V3<T> ray=vunit(vadd(forward,vadd(vmul(right,u),vmul(up,v))));
    T distance=T(0),epsilon=T(scene.epsilon);
    V3<T> point=origin;
    for(ProbeUInt step=0;step<scene.max_steps;++step) {
        result.steps=step+1;
        point=vadd(origin,vmul(ray,distance));
        T de=counted_de<T,Formula>(point,scene,result.de_calls);
        if(!isfinite(de)) { result.fault=1;break; }
        if(de<epsilon) { result.hit=1;break; }
        distance+=pmax(de*T(scene.step_scale),epsilon*T(.2));
        if(distance>T(scene.max_distance)) break;
        if(step+1==scene.max_steps) result.fault=2;
    }
    result.depth=float(distance);
    if(!result.hit) {
        T sky=clamp01(ray.y*T(.5)+T(.5));
        result.red=float(T(.06)+T(.12)*sky);
        result.green=float(T(.09)+T(.16)*sky);
        result.blue=float(T(.16)+T(.20)*sky);
        return result;
    }
    T e=epsilon*T(2);
    V3<T> normal=vunit(V3<T>{
        counted_de<T,Formula>(vadd(point,V3<T>{e,0,0}),scene,result.de_calls)-counted_de<T,Formula>(vadd(point,V3<T>{-e,0,0}),scene,result.de_calls),
        counted_de<T,Formula>(vadd(point,V3<T>{0,e,0}),scene,result.de_calls)-counted_de<T,Formula>(vadd(point,V3<T>{0,-e,0}),scene,result.de_calls),
        counted_de<T,Formula>(vadd(point,V3<T>{0,0,e}),scene,result.de_calls)-counted_de<T,Formula>(vadd(point,V3<T>{0,0,-e}),scene,result.de_calls)});
    V3<T> light=vunit(vsub(V3<T>{T(4),T(6),T(3)},point));
    V3<T> start=vadd(point,vmul(normal,epsilon*T(4)));
    T travel=epsilon*T(4),shadow=T(1);
    for(ProbeUInt i=0;i<scene.shadow_steps;++i) {
        T d=counted_de<T,Formula>(vadd(start,vmul(light,travel)),scene,result.de_calls);
        if(!isfinite(d)) { result.fault=1;break; }
        if(d<epsilon) { shadow=T(.15);break; }
        travel+=pmax(d*T(.7),epsilon);
        if(travel>T(8)) break;
        if(i+1==scene.shadow_steps) result.fault|=4;
    }
    T occlusion=T(0),weight=T(1);
    for(ProbeUInt i=1;i<=scene.ao_samples;++i) {
        T offset=T(i)*T(.06);
        T d=counted_de<T,Formula>(vadd(point,vmul(normal,offset)),scene,result.de_calls);
        if(!isfinite(d)) { result.fault=1;break; }
        occlusion+=pmax(T(0),offset-d)/offset*weight;weight*=T(.5);
    }
    T ao=clamp01(T(1)-occlusion*T(.4));
    T diffuse=pmax(T(0),vdot(normal,light));
    T shade=(T(.18)+T(.82)*diffuse*shadow)*ao;
    T stripe=T(.5)+T(.5)*sin(point.x*T(5)+point.y*T(3)+point.z*T(2));
    result.red=float(sqrt(clamp01((T(.22)+T(.55)*stripe)*shade)));
    result.green=float(sqrt(clamp01((T(.48)+T(.18)*stripe)*shade)));
    result.blue=float(sqrt(clamp01((T(.78)-T(.45)*stripe)*shade)));
    result.nx=float(normal.x);result.ny=float(normal.y);result.nz=float(normal.z);result.ao=float(ao);
    return result;
}

template<class T> inline T arithmetic(ProbeUInt operation,T a,T b,T c) {
    switch(operation) {
    case 0:return (a+b)-a;
    case 1:return (a*b)+c;
    case 2:return a/b;
    case 3:return sqrt(a);
    case 4:return sin(a);
    case 5:return cos(a);
    case 6:return atan2(a,b);
    case 7:return log(a);
    case 8:return exp2(a);
    case 9:return pow(a,b);
    default:return T(1)/a;
    }
}
#undef PROBE_REF
