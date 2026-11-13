#pragma once
#ifndef MB3D_METAL_NATIVE_FLOAT
#error Generic interior color requires the hardware FP32 profile
#endif
#include "generic_color_abi.h"
#include "comparison/metal_exact/color_math.h"
#include "generic_exterior_color.metal.h"

namespace mb3d_native {
using namespace metal;
using namespace mb3d_metal_exact;
// extra_flags names the optional flag bits the calling kernel implements.
inline bool valid_interior_color(constant NativeInteriorColorParams& p,uint extra_flags=0u) {
    if((p.version!=6&&p.version!=7)||(p.flags&~(127u|extra_flags))||p.reserved[0]||p.reserved[1]||
       p.reserved_scalar[0]||p.reserved_scalar[1]||
       !isfinite(color_s(p.palette_start))||!isfinite(color_s(p.palette_mul)))return false;
    if(p.object_y[3])return false;
    // Each uploaded component belongs to a normalized paint-matrix ROW.
    // The column as a whole need not be unit length (nonorthogonal cameras).
    for(uint i=0;i<3;++i)if(!isfinite(color_s(p.object_y[i]))||
        color_s(p.object_y[i])<F32(-1)||color_s(p.object_y[i])>F32(1)||
        (!(p.flags&64u)&&p.object_y[i]))return false;
    if(p.gamma_mode < -1 || p.gamma_mode > 1 || p.squared>1 || p.depth_function>3 ||
       !isfinite(color_s(p.gamma)) || color_s(p.gamma)<F32(0) || color_s(p.gamma)>F32(1))return false;
    for(uint i=0;i<6;++i) {
        constant auto& l=p.lights[i];
        if(l.enabled>1||l.reserved[0]||l.reserved[1])return false;
        if(!l.enabled)continue;
        if(p.version==7 && ((l.hs_mask==0xffffffffu)!=(l.hs_calculated!=0)))return false;
        if(l.power<1||l.power>1024||(l.power&(l.power-1))||l.function>3||
           (l.hs_mask>65535&&!(p.version==7&&l.hs_mask==0xffffffffu))||l.hs_calculated>1||l.hs_enabled>1)return false;
        for(uint c=0;c<4;++c)if(!isfinite(color_s(l.direction[c]))||!isfinite(color_s(l.color[c])))return false;
    }
    for(uint i=0;i<4;++i) {
        if(p.positions[i]<0||p.positions[i]>32767||(i&&p.positions[i]<p.positions[i-1])||
           !isfinite(color_s(p.divisors[i])))return false;
        for(uint c=0;c<4;++c)if(!isfinite(color_s(p.colors[i][c])))return false;
    }
    return true;
}
inline ExactColorPixel general_color_pixel(constant ExactColorScene& s,
    constant NativeInteriorColorParams& interior,ExactTrig column,ExactTrig row,ExactSurface p,uint y) {
    ExactColorPixel out{};
    if(!valid_interior_color(interior)){out.fault=1;return out;}
    // Native exterior options are independent of Compatible/specialized shading.
    if(p.si_gradient<32768)return general_exterior_color_pixel(s,interior,column,row,p,y);
    if(!valid_native_color_base(s)||p.fault||y>=s.height||p.si_gradient>65535||
       p.amb_shadow>65535||p.shadow>65535||p.o_trap>65535||
       p.reserved[0]||p.reserved[1]||p.reserved[2]||(p.depth_packed>>16)>=32768){out.fault=1;return out;}
    for(uint i=0;i<3;++i)if(p.normal[i]<-32768||p.normal[i]>32767){out.fault=1;return out;}
    if(!isfinite(color_d(column.sin_value))||!isfinite(color_d(column.cos_value))||
       !isfinite(color_d(row.sin_value))||!isfinite(color_d(row.cos_value))){out.fault=1;return out;}
    F32 one(1),one_half=color_s(0x3f000000),zero(0);
    ColorVec dep=color_mix(color_vec(s.depth_colors[1]),color_vec(s.depth_colors[0]),native_material_depth(F32(y)/F32(s.height),color_view(column,row),interior));
    F32 zpos=color_z(s,p),rough=F32(p.depth_packed&255)*color_s(s.roughness);
    F32 amb=color_ambient(s,p);if(interior.squared)amb=(amb*amb+amb)*color_s(0x3f000000);
    F32 fog=color_s(s.diffuse_shadowing)*(amb-one)+one;
    const F64 norm_scale=color_d(0x3f0000200040021eULL);
    ColorVec normal={color_single(F64(p.normal[0])*norm_scale),color_single(F64(p.normal[1])*norm_scale),color_single(F64(p.normal[2])*norm_scale)};
    ColorVec unshadowed,lit,spe;
    native_directional_lighting(s,interior,p,normal,color_view(column,row),rough,amb,fog,unshadowed,lit,spe);
    const auto palette=native_material_palette(s,interior,{p.si_gradient,p.o_trap,zpos});
    if(palette.fault){out.fault=palette.fault;return out;}
    const ColorVec diffuse=palette.diffuse,specular=palette.specular;
    F32 amb_b=(native_material_ambient_y(normal,interior)*one_half+one_half)*amb,amb_a=amb-amb_b;
    ColorVec ambient=color_mul(color_add(color_scale(color_vec(s.ambient_colors[0]),amb_a),
        color_scale(color_vec(s.ambient_colors[1]),amb_b)),diffuse);
    ColorVec scaled_diffuse=color_scale(diffuse,color_s(s.diffuse));
    lit=color_add(color_add(ambient,color_mul(scaled_diffuse,lit)),color_mul(specular,spe));
    unshadowed=color_add(ambient,color_scale(color_mul(scaled_diffuse,unshadowed),amb*(interior.squared?color_s(0x3de147ae):color_s(0x3e99999a))));
    F32 remainder=color_max0(one-amb),bounce=remainder*remainder;bounce=bounce*bounce;
    ColorVec scale=color_scale(diffuse,bounce*color_s(s.indirect_reflect));
    F32 add=remainder-bounce;scale=color_add(scale,{add,add,add});
    ColorVec result=color_add(lit,color_mul(unshadowed,color_mul(diffuse,scale)));
    F32 depth_factor=color_max0(F32(int(p.depth_packed>>16)-28000)*color_s(s.depth)+one);
    if(((interior.flags&4)||interior.squared)&&depth_factor<one){F32 t=one-depth_factor;if(interior.squared)t=t*t;if(interior.flags&4)t=t*t;depth_factor=one-t;}
    result=color_scale(result,depth_factor);
    const uint step_count=p.shadow&1023;
    F32 dynamic_fog=((F32(step_count)-color_s(s.shadow_start))-color_s(s.shadow_z_mul)*zpos)*color_s(s.shadow_gradient);
    if(!isfinite(dynamic_fog)){out.fault=2;return out;}
    if(interior.flags&16)dynamic_fog=color_max0(dynamic_fog);
    F32 fog_mix=F32(step_count)*color_s(s.dynamic_fog_mul);
    if(!isfinite(fog_mix)){out.fault=2;return out;}
    F32 fog_b=color_min(one,fog_mix)*dynamic_fog;
    if(!isfinite(fog_b)){out.fault=2;return out;}
    result=color_add(result,color_scale(dep,color_max0(one-depth_factor)));
    if(interior.flags&32){dynamic_fog=color_clamp(dynamic_fog,zero,one);fog_b=color_clamp(fog_b,zero,one);result=color_scale(result,one-dynamic_fog);}
    result=color_add(result,color_add(color_scale(color_vec(s.fog_colors[0]),dynamic_fog-fog_b),
                                     color_scale(color_vec(s.fog_colors[1]),fog_b)));
    if(!isfinite(result.x)||!isfinite(result.y)||!isfinite(result.z)){out.fault=2;return out;}
    result=native_color_output(result,interior);
    uint red=color_clamp(result.x,zero,F32(255)).to_i32_rne();
    uint green=color_clamp(result.y,zero,F32(255)).to_i32_rne();
    uint blue=color_clamp(result.z,zero,F32(255)).to_i32_rne();
    out.rgb=red|(green<<8)|(blue<<16);return out;
}
} // namespace mb3d_native

kernel void native_general_color_abi(device uint* words [[buffer(0)]],uint tid [[thread_position_in_grid]]) {
    if(tid)return;words[0]=6;words[1]=sizeof(mb3d_native::NativeInteriorColorParams);
    words[2]=sizeof(ExactColorScene);words[3]=sizeof(ExactSurface);words[4]=sizeof(ExactColorPixel);
}
kernel void native_light_geometry_abi(device uint* words [[buffer(0)]],uint tid [[thread_position_in_grid]]) {
    if(tid)return;words[0]=mb3d_native::kNativeLightGeometryVersion;
    words[1]=sizeof(mb3d_native::NativeLightGeometry);words[2]=sizeof(mb3d_native::NativePointLight);
}
kernel void native_general_soft_color_abi(device uint* words [[buffer(0)]],uint tid [[thread_position_in_grid]]) {
    if(tid)return;words[0]=7;words[1]=sizeof(mb3d_native::NativeInteriorColorParams);
    words[2]=sizeof(ExactColorScene);words[3]=sizeof(ExactSurface);words[4]=sizeof(ExactColorPixel);
}
kernel void native_general_color(constant ExactColorScene& scene [[buffer(0)]],
    device const ExactTrig* columns [[buffer(1)]],device const ExactTrig* rows [[buffer(2)]],
    device const ExactSurface* surface [[buffer(3)]],device ExactColorPixel* output [[buffer(4)]],
    constant uint* launch [[buffer(5)]],constant mb3d_native::NativeInteriorColorParams& interior [[buffer(6)]],
    uint2 xy [[thread_position_in_grid]]) {
    // Extent is the independently allocated image, not the mutable scene.
    // Check before addition so a hostile row offset cannot wrap into row0.
    const uint width=launch[1],height=launch[2];
    if(!width||width>4096||!height||height>4096||xy.x>=width||
       launch[0]>=height||xy.y>=height-launch[0])return;
    const uint y=launch[0]+xy.y,index=y*width+xy.x;
    if(scene.width!=width||scene.height!=height){
        ExactColorPixel rejected{};rejected.fault=1;output[index]=rejected;return;
    }
    output[index]=mb3d_native::general_color_pixel(scene,interior,columns[xy.x],rows[y],surface[index],y);
}
