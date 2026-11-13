#pragma once
// Native camera shading uses the same palette/fog contract as general color.
// Only the camera view and full-frame material Y are supplied explicitly.
#include "generic_color.metal.h"
namespace mb3d_native {
// CalcPixelColor2/CalcPixelColorSqr diffuse-map section: replaces the palette
// diffuse color (specular stays). Wrap-3D modes require the bound geometry.
inline ColorVec native_diffuse_map(constant NativeMaterialMaps& maps,device const float* samples,
    constant NativeLightGeometry* geometry,ColorVec object,ExactSurface p,ColorVec normal,F32 rough,bool squared,
    ColorVec diffuse) {
    const F32 one(1);
    ColorVec mapped;
    if(maps.diffuse_mode>3) {
        mapped=maps.diffuse_mode>5?material_wrap3d(maps,samples,normal,*geometry,object,squared):
            material_sphere(maps.diffuse,samples,normal,squared);
        const ColorVec average=color_scale(color_vec(maps.diffuse.average),rough*color_s(0x3b808081u));
        mapped=color_add(color_scale(mapped,one-rough),average);
    } else {
        // Orbit trap and smooth iteration gradient as map coordinates.
        const F32 step=F32::from_native(3.05186851e-5f),one_half=color_s(0x3f000000u);
        const F32 t=F32(int(p.o_trap&0x7fffu))*step-one_half,t2=F32(int(p.si_gradient))*step-one_half;
        const F32 c=color_s(maps.rotation_cos),sn=color_s(maps.rotation_sin),scale=color_s(maps.scale);
        mapped=material_pixel(maps.diffuse,samples,
            material_fraction(((c*t+sn*t2)+color_s(maps.offset_x))*scale+one),
            material_fraction(((c*t2-sn*t)+color_s(maps.offset_y))*scale+one),squared,1);
    }
    if(maps.yc_combine)mapped=color_scale(diffuse,native_luminance(mapped)/(F32::from_native(0.01f)+native_luminance(diffuse)));
    return mapped;
}
inline ExactColorPixel camera_exterior_color_pixel(constant ExactColorScene& s,
    constant NativeInteriorColorParams& options,ColorVec camera_view,F32 y_position,ExactSurface p,unsigned y,
    constant NativeLightGeometry* geometry,NativePaintPosition at,
    constant NativeMaterialMaps* maps,device const float* samples) {
    ExactColorPixel out{};
    if(!valid_native_color_base(s)||p.fault||y>=s.height||p.si_gradient>32767||p.amb_shadow>65535||p.shadow>65535||p.o_trap>65535||p.reserved[0]||p.reserved[1]||p.reserved[2]){out.fault=1;return out;}
    for(unsigned i=0;i<3;++i)if(p.normal[i]<-32768||p.normal[i]>32767){out.fault=1;return out;}
    if(!isfinite(camera_view.x)||!isfinite(camera_view.y)||!isfinite(camera_view.z)||!isfinite(y_position)){out.fault=1;return out;}
    F32 one(1),one_half=color_s(0x3f000000),zero(0);
    ColorVec dep=color_mix(color_vec(s.depth_colors[1]),color_vec(s.depth_colors[0]),native_material_depth(y_position,camera_view,options));
    ColorVec result;F32 depth_factor,zpos=color_z(s,p);
    const unsigned depth=p.depth_packed>>16;
    if(depth<32768) {
        F32 rough=F32(p.depth_packed&255)*color_s(s.roughness);
        F32 amb=color_ambient(s,p);if(options.squared)amb=(amb*amb+amb)*color_s(0x3f000000);
    F32 fog=color_s(s.diffuse_shadowing)*(amb-one)+one;
        const F64 norm_scale=color_d(0x3f0000200040021eULL);
        ColorVec normal={color_single(F64(p.normal[0])*norm_scale),color_single(F64(p.normal[1])*norm_scale),color_single(F64(p.normal[2])*norm_scale)};
        ColorVec unshadowed,lit,spe;
        native_directional_lighting(s,options,p,normal,camera_view,rough,amb,fog,unshadowed,lit,spe,geometry,at.object,maps,samples);
        const auto palette=native_material_palette(s,options,{p.si_gradient,p.o_trap,zpos});
        if(palette.fault){out.fault=palette.fault;return out;}
        ColorVec diffuse=palette.diffuse;const ColorVec specular=palette.specular;
        if(maps&&maps->diffuse_mode)diffuse=native_diffuse_map(*maps,samples,geometry,at.object,p,normal,rough,options.squared!=0,diffuse);
        ColorVec ambient;
        if(maps&&(maps->background&kNativeBackgroundAmbient))
            ambient=color_mul(native_background_ambient(*maps,samples,*geometry,normal,dep,amb,color_s(s.depth),options.squared!=0,(options.flags&4)!=0),diffuse);
        else {
            F32 amb_b=(native_ambient_y(maps,geometry,normal,options)*one_half+one_half)*amb,amb_a=amb-amb_b;
            ambient=color_mul(color_add(color_scale(color_vec(s.ambient_colors[0]),amb_a),color_scale(color_vec(s.ambient_colors[1]),amb_b)),diffuse);
        }
        ColorVec scaled_diffuse=color_scale(diffuse,color_s(s.diffuse));
        lit=color_add(color_add(ambient,color_mul(scaled_diffuse,lit)),color_mul(specular,spe));
        // CalcTotalLight1: ambient bounce uses the unshadowed light sum.
        unshadowed=color_add(ambient,color_scale(color_mul(scaled_diffuse,unshadowed),amb*(options.squared?color_s(0x3de147ae):color_s(0x3e99999a))));
        result=native_total_light(s,options,lit,unshadowed,diffuse,amb);
        depth_factor=color_max0(F32(int(depth)-28000)*color_s(s.depth)+one);
    } else {
        result=maps&&(maps->background&kNativeBackgroundMap)?native_background_color(*maps,samples,at.absolute,options.squared!=0):dep;
        depth_factor=color_max0(one-F32(28000)*color_s(s.depth));
    }
    if(((options.flags&4)||options.squared)&&depth_factor<one){F32 t=one-depth_factor;if(options.squared)t=t*t;if(options.flags&4)t=t*t;depth_factor=one-t;}
    // bAddBGlight leaves the background map unscaled by the depth fog.
    const uint background=maps?maps->background:0u;
    if(depth<32768||!(background&kNativeBackgroundMap)||!(background&kNativeBackgroundAddLight))result=color_scale(result,depth_factor);
    unsigned ir=native_fog_steps(p.shadow,options.flags);
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
    if(geometry)result=native_visible_lights(s,options,*geometry,p,result,dep,camera_view,at);
    if(!isfinite(result.x)||!isfinite(result.y)||!isfinite(result.z)){out.fault=2;return out;}
    result=native_color_output(result,options);
    unsigned red=color_clamp(result.x,zero,F32(255)).to_i32_rne();
    unsigned green=color_clamp(result.y,zero,F32(255)).to_i32_rne();
    unsigned blue=color_clamp(result.z,zero,F32(255)).to_i32_rne();
    out.rgb=red|(green<<8)|(blue<<16);return out;
}
// geometry is non-null exactly when the scene carries light position
// options; at is then the CalcObjPos paint position of this pixel.
inline ExactColorPixel camera_general_color_pixel(constant ExactColorScene& s,
    constant NativeInteriorColorParams& interior,ColorVec camera_view,F32 y_position,ExactSurface p,uint y,
    constant NativeLightGeometry* geometry=nullptr,NativePaintPosition at=NativePaintPosition{},
    constant NativeMaterialMaps* maps=nullptr,device const float* samples=nullptr) {
    ExactColorPixel out{};
    if(!valid_interior_color(interior,kNativeTotalLight2Flag|kNativeVolumeLightFlag|(geometry?kNativeLightGeometryFlag:0u)|
        (maps?kNativeMaterialMapFlag:0u))||(maps&&maps->diffuse_mode>5&&!geometry)){out.fault=1;return out;}
    // Native exterior options are independent of Compatible/specialized shading.
    if(p.si_gradient<32768)return camera_exterior_color_pixel(s,interior,camera_view,y_position,p,y,geometry,at,maps,samples);
    if(!valid_native_color_base(s)||p.fault||y>=s.height||p.si_gradient>65535||
       p.amb_shadow>65535||p.shadow>65535||p.o_trap>65535||
       p.reserved[0]||p.reserved[1]||p.reserved[2]||(p.depth_packed>>16)>=32768){out.fault=1;return out;}
    for(uint i=0;i<3;++i)if(p.normal[i]<-32768||p.normal[i]>32767){out.fault=1;return out;}
    if(!isfinite(camera_view.x)||!isfinite(camera_view.y)||!isfinite(camera_view.z)||!isfinite(y_position)){out.fault=1;return out;}
    F32 one(1),one_half=color_s(0x3f000000),zero(0);
    ColorVec dep=color_mix(color_vec(s.depth_colors[1]),color_vec(s.depth_colors[0]),native_material_depth(y_position,camera_view,interior));
    F32 zpos=color_z(s,p),rough=F32(p.depth_packed&255)*color_s(s.roughness);
    F32 amb=color_ambient(s,p);if(interior.squared)amb=(amb*amb+amb)*color_s(0x3f000000);
    F32 fog=color_s(s.diffuse_shadowing)*(amb-one)+one;
    const F64 norm_scale=color_d(0x3f0000200040021eULL);
    ColorVec normal={color_single(F64(p.normal[0])*norm_scale),color_single(F64(p.normal[1])*norm_scale),color_single(F64(p.normal[2])*norm_scale)};
    ColorVec unshadowed,lit,spe;
    native_directional_lighting(s,interior,p,normal,camera_view,rough,amb,fog,unshadowed,lit,spe,geometry,at.object,maps,samples);
    const auto palette=native_material_palette(s,interior,{p.si_gradient,p.o_trap,zpos});
    if(palette.fault){out.fault=palette.fault;return out;}
    ColorVec diffuse=palette.diffuse;const ColorVec specular=palette.specular;
    if(maps&&maps->diffuse_mode)diffuse=native_diffuse_map(*maps,samples,geometry,at.object,p,normal,rough,interior.squared!=0,diffuse);
    ColorVec ambient;
    if(maps&&(maps->background&kNativeBackgroundAmbient))
        ambient=color_mul(native_background_ambient(*maps,samples,*geometry,normal,dep,amb,color_s(s.depth),interior.squared!=0,(interior.flags&4)!=0),diffuse);
    else {
        F32 amb_b=(native_ambient_y(maps,geometry,normal,interior)*one_half+one_half)*amb,amb_a=amb-amb_b;
        ambient=color_mul(color_add(color_scale(color_vec(s.ambient_colors[0]),amb_a),
            color_scale(color_vec(s.ambient_colors[1]),amb_b)),diffuse);
    }
    ColorVec scaled_diffuse=color_scale(diffuse,color_s(s.diffuse));
    lit=color_add(color_add(ambient,color_mul(scaled_diffuse,lit)),color_mul(specular,spe));
    unshadowed=color_add(ambient,color_scale(color_mul(scaled_diffuse,unshadowed),amb*(interior.squared?color_s(0x3de147ae):color_s(0x3e99999a))));
    ColorVec result=native_total_light(s,interior,lit,unshadowed,diffuse,amb);
    F32 depth_factor=color_max0(F32(int(p.depth_packed>>16)-28000)*color_s(s.depth)+one);
    if(((interior.flags&4)||interior.squared)&&depth_factor<one){F32 t=one-depth_factor;if(interior.squared)t=t*t;if(interior.flags&4)t=t*t;depth_factor=one-t;}
    result=color_scale(result,depth_factor);
    const uint step_count=native_fog_steps(p.shadow,interior.flags);
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
    if(geometry)result=native_visible_lights(s,interior,*geometry,p,result,dep,camera_view,at);
    if(!isfinite(result.x)||!isfinite(result.y)||!isfinite(result.z)){out.fault=2;return out;}
    result=native_color_output(result,interior);
    uint red=color_clamp(result.x,zero,F32(255)).to_i32_rne();
    uint green=color_clamp(result.y,zero,F32(255)).to_i32_rne();
    uint blue=color_clamp(result.z,zero,F32(255)).to_i32_rne();
    out.rgb=red|(green<<8)|(blue<<16);return out;
}
}
