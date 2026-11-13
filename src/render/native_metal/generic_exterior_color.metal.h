#pragma once
#include "native_directional_color.metal.h"
#include "material_palette.h"
// Native-only CalcColors/CalcPixelColor2 extension; exact color source remains unchanged.
namespace mb3d_native {
using namespace mb3d_metal_exact;
inline ExactColorPixel general_exterior_color_pixel(constant ExactColorScene& s,
    constant NativeInteriorColorParams& options,ExactTrig column,ExactTrig row,ExactSurface p,unsigned y) {
    ExactColorPixel out{};
    if(!valid_native_color_base(s)||p.fault||y>=s.height||p.si_gradient>32767||p.amb_shadow>65535||p.shadow>65535||p.o_trap>65535||p.reserved[0]||p.reserved[1]||p.reserved[2]){out.fault=1;return out;}
    for(unsigned i=0;i<3;++i)if(p.normal[i]<-32768||p.normal[i]>32767){out.fault=1;return out;}
    if(!isfinite(color_d(column.sin_value))||!isfinite(color_d(column.cos_value))||!isfinite(color_d(row.sin_value))||!isfinite(color_d(row.cos_value))){out.fault=1;return out;}
    F32 one(1),one_half=color_s(0x3f000000),zero(0);
    ColorVec dep=color_mix(color_vec(s.depth_colors[1]),color_vec(s.depth_colors[0]),native_material_depth(F32(y)/F32(s.height),color_view(column,row),options));
    ColorVec result;F32 depth_factor,zpos=color_z(s,p);
    const unsigned depth=p.depth_packed>>16;
    if(depth<32768) {
        F32 rough=F32(p.depth_packed&255)*color_s(s.roughness);
        F32 amb=color_ambient(s,p);if(options.squared)amb=(amb*amb+amb)*color_s(0x3f000000);
    F32 fog=color_s(s.diffuse_shadowing)*(amb-one)+one;
        const F64 norm_scale=color_d(0x3f0000200040021eULL);
        ColorVec normal={color_single(F64(p.normal[0])*norm_scale),color_single(F64(p.normal[1])*norm_scale),color_single(F64(p.normal[2])*norm_scale)};
        ColorVec unshadowed,lit,spe;
        native_directional_lighting(s,options,p,normal,color_view(column,row),rough,amb,fog,unshadowed,lit,spe);
        const auto palette=native_material_palette(s,options,{p.si_gradient,p.o_trap,zpos});
        if(palette.fault){out.fault=palette.fault;return out;}
        const ColorVec diffuse=palette.diffuse,specular=palette.specular;
        F32 amb_b=(native_material_ambient_y(normal,options)*one_half+one_half)*amb,amb_a=amb-amb_b;
        ColorVec ambient=color_mul(color_add(color_scale(color_vec(s.ambient_colors[0]),amb_a),color_scale(color_vec(s.ambient_colors[1]),amb_b)),diffuse);
        ColorVec scaled_diffuse=color_scale(diffuse,color_s(s.diffuse));
        lit=color_add(color_add(ambient,color_mul(scaled_diffuse,lit)),color_mul(specular,spe));
        // CalcTotalLight1: ambient bounce uses the unshadowed light sum.
        unshadowed=color_add(ambient,color_scale(color_mul(scaled_diffuse,unshadowed),amb*(options.squared?color_s(0x3de147ae):color_s(0x3e99999a))));
        F32 remainder=color_max0(one-amb),bounce=remainder*remainder;bounce=bounce*bounce;
        ColorVec scale=color_scale(diffuse,bounce*color_s(s.indirect_reflect));
        F32 add=remainder-bounce;scale=color_add(scale,{add,add,add});
        result=color_add(lit,color_mul(unshadowed,color_mul(diffuse,scale)));
        depth_factor=color_max0(F32(int(depth)-28000)*color_s(s.depth)+one);
    } else {
        result=dep;depth_factor=color_max0(one-F32(28000)*color_s(s.depth));
    }
    if(((options.flags&4)||options.squared)&&depth_factor<one){F32 t=one-depth_factor;if(options.squared)t=t*t;if(options.flags&4)t=t*t;depth_factor=one-t;}
    result=color_scale(result,depth_factor);
    unsigned ir=p.shadow&1023;
    F32 fog=((F32(ir)-color_s(s.shadow_start))-color_s(s.shadow_z_mul)*zpos)*color_s(s.shadow_gradient);
    if(!isfinite(fog)){out.fault=2;return out;}
    if(options.flags&16)fog=color_max0(fog);
    F32 fog_mix=F32(ir)*color_s(s.dynamic_fog_mul);
    if(!isfinite(fog_mix)){out.fault=2;return out;}
    F32 fog_b=color_min(one,fog_mix)*fog;
    if(!isfinite(fog_b)){out.fault=2;return out;}
    result=color_add(result,color_scale(dep,color_max0(one-depth_factor)));
    if(options.flags&32){fog=color_clamp(fog,zero,one);fog_b=color_clamp(fog_b,zero,one);result=color_scale(result,one-fog);}
    result=color_add(result,color_add(color_scale(color_vec(s.fog_colors[0]),fog-fog_b),color_scale(color_vec(s.fog_colors[1]),fog_b)));
    if(!isfinite(result.x)||!isfinite(result.y)||!isfinite(result.z)){out.fault=2;return out;}
    result=native_color_output(result,options);
    unsigned red=color_clamp(result.x,zero,F32(255)).to_i32_rne();
    unsigned green=color_clamp(result.y,zero,F32(255)).to_i32_rne();
    unsigned blue=color_clamp(result.z,zero,F32(255)).to_i32_rne();
    out.rgb=red|(green<<8)|(blue<<16);return out;
}
}
