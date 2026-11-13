#pragma once
#include "native_ao_abi.h"
#ifndef __METAL_VERSION__
#include <algorithm>
#include <cmath>
#endif
namespace mb3d_native_ao {
#ifdef __METAL_VERSION__
#define MB3D_NATIVE_AO_THREAD thread
using metal::isfinite; using metal::fabs; using metal::rint;
using metal::min; using metal::max;
#else
#define MB3D_NATIVE_AO_THREAD
using std::isfinite; using std::fabs; using std::rint;
using std::min; using std::max;
#endif

inline U32 dispatch(U32 selector,U32 optic) {
    if ((selector&12u)==12u) return DistanceEstimate;
    U32 mode=(selector>>1)&7u;
    // ATi=6 is unreachable here: the ordinary CPU dispatch already chose DE.
    if (mode==6u || mode==7u) return 0xffffffffu;
    if (optic==2u && mode>=2u && mode<=5u) mode+=4u;
    return mode;
}
inline bool random_mode(U32 mode) { return mode==4u||mode==5u||mode==8u||mode==9u; }
inline bool threshold_mode(U32 mode) { return mode==3u||mode==5u||mode==7u||mode==9u; }
inline bool panorama(U32 mode) { return mode>=6u&&mode<=9u; }
inline bool hiq(U32 mode) { return mode>=2u&&mode<=9u; }
inline U32 initial_seed(U32 lane) { return (lane+2u)*0x324594a1u+0x24563487u; }
inline U32 next_seed(U32 seed) { return 214013u*seed+2531011u; }
// Exact affine exponentiation modulo 2^32, at most 32 iterations.
inline U32 advance_seed(U32 seed,U32 count) {
    U32 a=214013u,c=2531011u,scale=1u,offset=0u;
    for(U32 bit=0;bit<32u&&count;++bit,count>>=1u){
        if(count&1u){offset=a*offset+c;scale=a*scale;}
        c=(a+1u)*c;a=a*a;
    }
    return scale*seed+offset;
}
inline bool rounded(float x,MB3D_NATIVE_AO_THREAD I32& result) {
    if (!isfinite(x)||x<=-1073741824.0f||x>=1073741824.0f) return false;
    result=I32(rint(x)); return true;
}
inline bool valid(MB3D_NATIVE_AO_THREAD const Params& p) {
    if (p.version!=Version||!p.width||!p.height||p.width>4096u||p.height>4096u||
        p.output_count<p.width*p.height||p.output_count>4096u*4096u||
        p.padding!=0.0f||p.levels<2u||p.levels>15u||
        !p.repeats||p.repeats>255u||!p.logical_lanes||p.logical_lanes>p.height||
        !p.work_limit||p.work_limit>0x7fffffffu||p.quality>3u||
        p.first_step_random>1u||p.dither>255u||
        !isfinite(p.threshold)||p.threshold<0.01f||
        !isfinite(p.depth_scale)||p.depth_scale<=0.0f||
        !isfinite(p.sum_multiplier)||p.sum_multiplier<0.0f) return false;
    if (!(p.algorithm<=9u||p.algorithm==DistanceEstimate)) return false;
    if (p.w_lo<0||p.w_lo>=I32(p.width)||p.w_hi!=I32(p.width)-1-p.w_lo||
        p.h_lo<0||p.h_lo>=I32(p.height)||p.h_hi!=I32(p.height)-1-p.h_lo) return false;
    for (U32 i=0;i<64u;++i) if (!isfinite(p.directions[i])||fabs(p.directions[i])>1.001f) return false;
    return true;
}
inline bool valid(MB3D_NATIVE_AO_THREAD const Params& p,MB3D_NATIVE_AO_THREAD const Level& l) {
    if (!valid(p)||!hiq(p.algorithm)||l.version!=Version||!l.index||l.index>p.levels||
        l.step!=(1u<<(l.index-1u))||l.samples!=(l.index==1u?5u:3u)||
        l.maximum_radius<=0||l.repeat_index>=p.repeats||l.reserved||
        l.reserved2[0]||l.reserved2[1]||l.add_limit!=(random_mode(p.algorithm)?16383u/p.repeats:16383u)) return false;
    if (!isfinite(l.minimum_radius)||l.minimum_radius<=0.0f||
        !isfinite(l.z_threshold)||l.z_threshold<=0.0f||
        !isfinite(l.sit)||l.sit<=0.0f||
        !isfinite(l.sum_multiplier)||l.sum_multiplier<=0.0f) return false;
    for (U32 i=0;i<6u;++i) {
        if (!isfinite(l.reciprocal_radius[i])||
            (i<l.samples?l.reciprocal_radius[i]<=0.0f:l.reciprocal_radius[i]!=0.0f)) return false;
    }
    return true;
}
// Exactly one legacy wrap/mirror, then reject. A modulo or repeated reflection
// changes the CPU sampling footprint at coarse levels and narrow dimensions.
inline BoundaryResult boundary_coordinates(MB3D_NATIVE_AO_THREAD const Params& p,I32 x,I32 y,bool quartet=false) {
    BoundaryResult r{x,y,0,0};
    if (x<=-1073741824||x>=1073741824||
        y<=-1073741824||y>=1073741824) {r.fault=Invalid;return r;}
    if (!quartet) {
        if (panorama(p.algorithm)) {
            if (x<0) {x+=I32(p.width);if(x<0)return r;}
            else if (x>=I32(p.width)) {x-=I32(p.width);if(x>=I32(p.width))return r;}
        } else {
            if(x<0) {x=-x;if(x>=p.w_lo)return r;}
            else if(x>=I32(p.width)) {x=2*(I32(p.width)-1)-x;if(x<p.w_hi)return r;}
        }
    }
    if(y<0) {y=-y;if(y>=p.h_lo)return r;}
    else if(y>=I32(p.height)) {y=2*(I32(p.height)-1)-y;if(y<p.h_hi)return r;}
    if(x<0||x>=I32(p.width)||y<0||y>=I32(p.height)) {r.fault=Bounds;return r;}
    r.x=x;r.y=y;r.admitted=1;return r;
}
inline BoundaryResult boundary(MB3D_NATIVE_AO_THREAD const Params& p,I32 x,I32 y,bool quartet=false) {
    if(!valid(p))return {x,y,0,Invalid};
    return boundary_coordinates(p,x,y,quartet);
}
inline bool spend(MB3D_NATIVE_AO_THREAD Record& r,U32 limit) {
    if(r.samples>=limit) {r.fault|=WorkExhausted;return false;}
    ++r.samples;return true;
}
inline bool angle(float st,float sit,float threshold,bool squared,bool random,MB3D_NATIVE_AO_THREAD I32& result) {
    float denom=squared?st*st+threshold:threshold+fabs(st);
    float numerator=st*sit;
    if(random)numerator*=threshold;
    if(!isfinite(denom)||denom<=0.0f||!isfinite(numerator))return false;
    float value=numerator/denom;
    if(!isfinite(value))return false;
    value=max(-32768.0f,min(32767.0f,value));
    return rounded(value,result);
}
inline I32 angle15(I32 y,I32 x) {
    if(!x)return (y>=0?1:0)*16+8;
    if(!y)return (x>=0?1:0)*16;
    if(y<0) {
        if(x<0)return x>=y?7-(x*4)/y:(y*4)/x;
        return -y<x?15+(y*4)/x:8-(x*4)/y;
    }
    if(x>=0)return x>y?16+(y*4)/x:23-(x*4)/y;
    return y<-x?31+(y*4)/x:24-(x*4)/y;
}
}
