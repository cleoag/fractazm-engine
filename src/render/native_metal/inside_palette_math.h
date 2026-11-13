#pragma once
#include "native_inside_math.h"
#include "generic_color_abi.h"
#ifdef __METAL_VERSION__
#define IP_CONSTANT constant
#else
#include <bit>
#define IP_CONSTANT const
#endif
namespace mb3d_inside {
using mb3d_native::NativeInteriorColorParams;
inline float palette_float(U32 bits){
#ifdef __METAL_VERSION__
    return as_type<float>(bits);
#else
    return std::bit_cast<float>(bits);
#endif
}
struct InteriorPalette {float diffuse[3],specular;U32 fault,reserved[3];};
struct PaletteInput {NativeInteriorColorParams palette;U32 gradient,reserved[3];float z_multiplier,zpos;U32 reserved2[2];};
#ifndef __METAL_VERSION__
static_assert(sizeof(InteriorPalette)==32);static_assert(sizeof(PaletteInput)==576);
#endif
inline InteriorPalette inside_palette(IP_CONSTANT NativeInteriorColorParams& p,U32 gradient,float zmul,float zpos){
    InteriorPalette out{};
    if(p.version!=4||(p.flags&~63u)||p.reserved[0]||p.reserved[1]||p.reserved_scalar[0]||p.reserved_scalar[1]||
       gradient<32768||gradient>65535||!isfinite(zmul)||!isfinite(zpos)||
       !isfinite(palette_float(p.palette_start))||!isfinite(palette_float(p.palette_mul))){out.fault=Invalid;return out;}
    for(U32 i=0;i<4;++i){if(p.positions[i]<0||p.positions[i]>32767||(i&&p.positions[i]<p.positions[i-1])||
        !isfinite(palette_float(p.divisors[i]))){out.fault=Invalid;return out;}
        for(U32 c=0;c<4;++c)if(!isfinite(palette_float(p.colors[i][c]))){out.fault=Invalid;return out;}}
    const float raw=((float(gradient)-palette_float(p.palette_start))*palette_float(p.palette_mul)+zmul*zpos)*16384.f;
    if(!isfinite(raw)){out.fault=NonFinite;return out;}
    // PascalRoundToInt saturates its input before ties-to-even rounding.
    I32 index=round_indefinite(raw<-1.0e9f?-1.0e9f:(raw>1.0e9f?1.0e9f:raw));
    if(p.flags&1u)index&=32767;
    // Inside uses strict >last (exterior uses >=last). A step at an exact
    // stop selects the preceding stop, including the last inside stop.
    if(!(p.flags&1u)&&(index<0||index>p.positions[3])){
        const U32 stop=index<0?0u:3u;
        for(U32 c=0;c<3;++c)out.diffuse[c]=palette_float(p.colors[stop][c]);
        out.specular=palette_float(p.colors[stop][3]);return out;
    }
    I32 right=1;while(right<4&&p.positions[right]<index)++right;
    const I32 left=right-1;right&=3;
    if(p.flags&2u){
        for(U32 c=0;c<3;++c)out.diffuse[c]=palette_float(p.colors[left][c]);
        out.specular=palette_float(p.colors[left][3]);return out;
    }
    const float weight=float(index-p.positions[left])*palette_float(p.divisors[left]);
    if(!isfinite(weight)){out.fault=NonFinite;return out;}
    for(U32 c=0;c<4;++c){const float a=palette_float(p.colors[left][c]);
        const float value=a+weight*(palette_float(p.colors[right][c])-a);
        if(!isfinite(value)){out.fault=NonFinite;return out;}
        if(c<3)out.diffuse[c]=value;else out.specular=value;}
    return out;
}
inline InteriorPalette palette_probe(IP_CONSTANT PaletteInput& in){
    if(in.reserved[0]||in.reserved[1]||in.reserved[2]||in.reserved2[0]||in.reserved2[1]){
        InteriorPalette out{};out.fault=Invalid;return out;}
    return inside_palette(in.palette,in.gradient,in.z_multiplier,in.zpos);
}
}
#undef IP_CONSTANT
