#pragma once
// Native-only lighting; no changes to Compatible arithmetic or ABI.
#include "generic_color_abi.h"
#include "material_coordinates.metal.h"
#include "material_maps.metal.h"
#include "comparison/metal_exact/color_math.h"
namespace mb3d_native {
using namespace mb3d_metal_exact;
// PreCalcDepthCol: preserve row-coordinate arithmetic, then interpolate in
// source order. Only Native primary color consumes this extended descriptor.
inline F32 native_depth_position(F32 y,U32 function) {
    if(function==1)return y*y;
    if(function==0)return y;
    return sqrt(color_max0(y));
}
// Dynamic-fog step count of a packed Shadow word: ConvertVLight with
// volumetric light, else the plain ten-bit count.
inline uint native_fog_steps(uint shadow,uint flags) {
    const uint v=shadow&1023u;
    return (flags&kNativeVolumeLightFlag)?(v&127u)<<((v>>7)&7u):v;
}
inline bool valid_native_color_base(constant ExactColorScene& s) {
    const ExactU64 geometry[5]={s.z_corr,s.z_mul,s.z_mid_difference,s.step_width,s.back_distance};
    for(unsigned i=0;i<5;++i)if(!isfinite(color_d(geometry[i])))return false;
    if(color_d(s.z_corr)<=F64(0)||color_d(s.z_mul)<=F64(0)||color_d(s.step_width)<=F64(0))return false;
    const ExactU32 scalar[14]={s.depth,s.shadow_start,s.shadow_z_mul,s.shadow_gradient,s.ambient_amplitude,
        s.palette_start,s.palette_mul,s.palette_z_mul,s.diffuse,s.specular,s.indirect_reflect,
        s.diffuse_shadowing,s.roughness,s.dynamic_fog_mul};
    for(unsigned i=0;i<14;++i)if(!isfinite(color_s(scalar[i])))return false;
    // ColPos is the header's uint16 position; CalcColors accepts every value
    // (an index at or beyond ColPos[9] takes the last color).
    for(unsigned i=0;i<10;++i) {
        if(!isfinite(color_s(s.palette_divisors[i]))||s.palette_positions[i]<0||s.palette_positions[i]>65535)return false;
        if(i>0&&s.palette_positions[i]<s.palette_positions[i-1])return false;
        for(unsigned c=0;c<3;++c)if(!isfinite(color_s(s.palette_diffuse[i][c]))||!isfinite(color_s(s.palette_specular[i][c])))return false;
    }
    for(unsigned c=0;c<3;++c) {
        if(!isfinite(color_s(s.light_direction[c]))||!isfinite(color_s(s.light_color[c])))return false;
        for(unsigned i=0;i<2;++i)if(!isfinite(color_s(s.depth_colors[i][c]))||!isfinite(color_s(s.ambient_colors[i][c]))||!isfinite(color_s(s.fog_colors[i][c])))return false;
    }
    return s.version==1 && s.width>0 && s.height>0 && s.width<=4096 && s.height<=4096 &&
           !s.reserved && !s.reserved2;
}
inline F32 native_light_cos(constant ExactColorScene& scene,unsigned function,F32 dot,F32 rough) {
    F32 t=F32(62)-F32(60)*dot;
    int ip=t.to_i32_rtz()-1;
    if(ip<0){ip=0;t=F32(0);}else if(ip>124){ip=124;t=F32(1);}
    else t=t-F32(t.to_i32_rtz()); // Here t >= 1, so truncation equals floor.
    // Production MakeSplineCoeff stores four Single coefficients after Double
    // expressions; retain the unrounded c0/c3 in the following expressions.
    F64 x=t.to_f64(),sixth=color_d(0x3fc5555555555555ULL);
    F64 c3=((x*x)*x)*sixth,c0=(sixth-((F64(1)-x)*x)*color_d(0x3fe0000000000000ULL))-c3;
    F64 c2=((x+c0)-c3)-c3;
    F32 w[4]={color_single(c0),color_single(((F64(1)-c2)-c3)-c0),color_single(c2),color_single(c3)};
    F32 p[4],r[4];
    for(int i=0;i<4;++i){p[i]=color_s(scene.diffuse_table[function][ip+i])*w[i];r[i]=color_s(scene.diffuse_table[function+4][ip+i])*w[i];}
    F32 base=(p[0]+p[2])+(p[1]+p[3]),rough_value=(r[0]+r[2])+(r[1]+r[3]);
    return base+rough*(rough_value-base);
}

inline bool valid_light_geometry(constant NativeLightGeometry& g,constant NativeInteriorColorParams& p) {
    if(g.version!=kNativeLightGeometryVersion||g.reserved[0]||g.reserved[1]||
       !isfinite(color_s(g.zz_mid_difference))||g.start[3]||g.y_add[3]||g.x_add[3])return false;
    for(uint i=0;i<3;++i) {
        if(!isfinite(color_s(g.start[i]))||!isfinite(color_s(g.y_add[i]))||!isfinite(color_s(g.x_add[i]))||g.matrix[i][3])return false;
        for(uint j=0;j<3;++j)if(!isfinite(color_s(g.matrix[i][j])))return false;
    }
    for(uint i=0;i<6;++i) {
        constant auto& l=g.lights[i];
        if(p.lights[i].enabled&&(l.options>15||!isfinite(color_s(l.max_length2))||
           !isfinite(color_s(l.depth_position))||!isfinite(color_s(l.z_position))))return false;
    }
    return true;
}
// CalcObjPos in the paint frame: camera start, view rotated by the paint
// matrix, and the Single depth along it. Offsets are full-frame pixels.
struct NativePaintPosition { ColorVec camera,absolute,object; };
inline NativePaintPosition native_paint_position(constant NativeLightGeometry& g,constant ExactColorScene& s,
    ColorVec view,ExactSurface p,int x_offset,int y_offset,bool panorama) {
    NativePaintPosition out;out.camera=color_vec(g.start);
    if(!panorama) {
        const F32 yo=F32(y_offset),xo=F32(x_offset);
        ColorVec c=out.camera;
        c={c.x+color_s(g.y_add[0])*yo,c.y+color_s(g.y_add[1])*yo,c.z+color_s(g.y_add[2])*yo};
        out.camera={c.x+color_s(g.x_add[0])*xo,c.y+color_s(g.x_add[1])*xo,c.z+color_s(g.x_add[2])*xo};
    }
    F64 z;
    if((p.depth_packed>>16)>32767)z=color_d(s.back_distance);
    else {
        F64 zvalue=(F64(8388352)-F64(p.depth_packed>>8))/color_d(s.z_mul)+F64(1);
        z=((zvalue*zvalue-F64(1))*color_d(s.step_width))/color_d(s.z_corr);
    }
    const ColorVec m0=color_vec(g.matrix[0]),m1=color_vec(g.matrix[1]),m2=color_vec(g.matrix[2]);
    out.absolute={(view.x*m0.x+view.y*m1.x)+view.z*m2.x,(view.x*m0.y+view.y*m1.y)+view.z*m2.y,
                  (view.x*m0.z+view.y*m1.z)+view.z*m2.z};
    out.object=color_add(out.camera,color_scale(out.absolute,color_single(z)));
    return out;
}
// CalcPosLightShape: flux enters as the squared ray distance to the light,
// transparency as the light's visible radius term.
inline void native_light_shape(thread F32& flux,thread F32& transp,uint options,bool squared) {
    const F32 one(1),radius=transp,inverse=one/radius;
    const bool positional=(options&1u)!=0;
    switch(options&14u) {
    case 4:
        transp=flux*inverse;transp=transp*transp;transp=transp*transp;
        flux=F32::from_native(7e-3f)*(radius-flux)*(inverse*inverse);
        break;
    case 2: {
        transp=one;flux=one-sqrt(flux*inverse);
        const F32 n(squared?80:50);
        flux=F32::from_native(5e-2f)*(flux/(n-flux*n+flux))*inverse; // Math3D.FastPow
        if(positional)flux=flux*F32::from_native(1.5f);
        break;
    }
    case 6:
        flux=flux*F32::from_native(1.05f)*inverse;
        if(flux<one) {
            transp=positional?color_max0(flux-F32::from_native(0.95f))*F32(10):F32::from_native(0.7f);
            flux=F32::from_native(4e-3f)*sqrt(F32::from_native(1.05f)-flux*flux)*inverse;
            if(!positional)flux=flux*F32::from_native(0.7f);
        } else if(positional) {
            transp=(flux-F32::from_native(0.95f))*F32(10);flux=F32::from_native(17.8e-3f)*(F32::from_native(1.05f)-flux)*inverse;
        } else {
            transp=(flux-one)*F32(6)+F32::from_native(0.7f);flux=F32::from_native(12.5e-3f)*(F32::from_native(1.05f)-flux)*inverse;
        }
        break;
    default: {
        transp=one;
        const F32 ratio=flux*inverse,denominator=(ratio+F32::from_native(4e-4f))*F32(squared?40:25);
        flux=(one-ratio)*F32::from_native(0.1f)/(denominator*denominator)*inverse;
        break;
    }
    }
}
// CalcPixelColor2/CalcPixelColorSqr visible light sources, after dynamic fog.
inline ColorVec native_visible_lights(constant ExactColorScene& s,constant NativeInteriorColorParams& options,
    constant NativeLightGeometry& g,ExactSurface p,ColorVec result,ColorVec dep,ColorVec view,NativePaintPosition at) {
    const F32 zero(0),one(1);
    const uint zpos=p.depth_packed>>16;
    const F32 inverse_depth=one/F32(int(zpos<32768?32768-zpos:zpos));
    const bool squared=options.squared!=0,far_fog=(options.flags&4u)!=0;
    for(uint i=0;i<6;++i) {
        constant auto& l=options.lights[i];constant auto& q=g.lights[i];
        if(!l.enabled||!(q.options&14u))continue;
        const bool positional=(q.options&1u)!=0;
        const ColorVec ln=color_vec(l.direction);
        F32 flux;
        if(positional) {
            // SqrDistSV: squared cross product of camera-light offset and view.
            const ColorVec d=color_sub(at.camera,ln),v=at.absolute;
            const F32 cx=d.y*v.z-d.z*v.y,cy=d.z*v.x-d.x*v.z,cz=d.x*v.y-d.y*v.x;
            flux=(cx*cx+cy*cy)+cz*cz;
        } else {
            if(zpos<32768)continue;
            flux=color_max0(one-color_dot_single(ln,view));
        }
        F32 transp=color_s(q.max_length2)*F32::from_native(1e-8f);
        if(!(flux<transp))continue;
        if(positional) {
            if(color_dot_single(color_sub(ln,at.camera),at.absolute)<zero)continue;
            if(zpos<32768) {
                // The light is hidden when the surface lies before its sphere.
                const F32 entry=-sqrt(transp-flux);
                F32 surface;
                if(abs(at.absolute.x.value)>0.5f)surface=(at.object.x-ln.x)/at.absolute.x;
                else if(abs(at.absolute.y.value)>0.5f)surface=(at.object.y-ln.y)/at.absolute.y;
                else surface=(at.object.z-ln.z)/at.absolute.z;
                if(entry>surface)continue;
            }
        }
        native_light_shape(flux,transp,q.options,squared);
        const F32 position=color_s(q.depth_position);
        F32 depth_fog=one-color_max0(one+(position-F32(28000))*color_s(s.depth));
        if(squared){if(depth_fog>zero){depth_fog=depth_fog*depth_fog;if(far_fog)depth_fog=depth_fog*depth_fog;}}
        else if(far_fog&&depth_fog>zero)depth_fog=depth_fog*depth_fog;
        flux=flux*(one-depth_fog*F32::from_native(0.9f));
        const ColorVec light=color_vec(l.color);
        const uint shape=q.options&14u;
        if(shape==2||shape==8) {result=color_add(result,color_scale(light,flux));continue;}
        depth_fog=color_max0(depth_fog);
        const F32 steps=F32(int(native_fog_steps(p.shadow,options.flags)))*(F32(32768)-position)*inverse_depth;
        const F32 fog=(((steps-color_s(s.shadow_start))-color_s(s.shadow_z_mul)*(color_s(q.z_position)+color_s(g.zz_mid_difference)))*
            color_s(s.shadow_gradient))*(one-transp)*(one-depth_fog);
        const F32 fog_b=color_min(one,steps*color_s(s.dynamic_fog_mul))*fog;
        const ColorVec mixed=color_add(color_scale(result,transp),color_scale(dep,depth_fog*(one-transp)));
        const ColorVec fogged=color_add(color_scale(color_vec(s.fog_colors[0]),fog-fog_b),color_scale(color_vec(s.fog_colors[1]),fog_b));
        result=color_add(color_add(mixed,fogged),color_scale(light,flux));
    }
    return result;
}

inline void native_directional_lighting(constant ExactColorScene& s,constant NativeInteriorColorParams& options,
    ExactSurface p,ColorVec normal,ColorVec view,F32 rough,F32 amb,F32 fog,
    thread ColorVec& unshadowed,thread ColorVec& lit,thread ColorVec& spe,
    constant NativeLightGeometry* geometry=nullptr,ColorVec object=ColorVec{},
    constant NativeMaterialMaps* maps=nullptr,device const float* samples=nullptr) {
    F32 zero(0),one(1);unshadowed={zero,zero,zero};lit=unshadowed;spe=unshadowed;
    for(uint i=0;i<6;++i) {
        constant auto& l=options.lights[i];
        if(!l.enabled) {
            // Light-map light (iLightOption 2): the sphere-sampled normal blended
            // toward the map average by roughness; unshadowed, ambient-weighted.
            if(maps&&((maps->light_mask>>i)&1u)) {
                constant auto& m=maps->lights[i];
                const bool squared=options.squared!=0;
                const ColorVec sampled=color_scale(material_sphere(m,samples,normal,squared),F32(255));
                const ColorVec value=color_mix(color_vec(squared?m.average_squared:m.average),sampled,rough);
                unshadowed=color_add(unshadowed,value);
                lit=color_add(lit,color_scale(value,amb));
            }
            continue;
        }
        ColorVec light=color_vec(l.direction),lightcol=color_vec(l.color);
        // Positional light: LN holds the paint-frame position. Out-of-range
        // lights contribute nothing; the inverse square scales all terms.
        const bool positional=geometry&&(geometry->lights[i].options&1u);
        F32 distance=one;
        if(positional) {
            ColorVec to_light=color_sub(light,object);
            const F32 length2=color_dot_single(to_light,to_light);
            if(length2>color_s(geometry->lights[i].max_length2))continue;
            to_light=color_scale(to_light,one/sqrt(length2));
            light={color_dot_double(to_light,color_vec(geometry->matrix[0])),
                   color_dot_double(to_light,color_vec(geometry->matrix[1])),
                   color_dot_double(to_light,color_vec(geometry->matrix[2]))};
            distance=one/(length2+color_s(0x0da24260u));
        }
        F32 weight=native_light_cos(s,l.function,color_dot_double(normal,light),rough);
        if(options.squared)weight=weight*weight;
        if(positional)weight=weight*distance;
        unshadowed=color_add(unshadowed,color_scale(lightcol,weight));
        const bool soft=options.version==7&&l.hs_mask==0xffffffffu;
        bool no_shadow=soft||(p.shadow&l.hs_mask)==0||l.hs_calculated==0;
        bool sub=(l.hs_calculated^l.hs_enabled)!=0;
        if(no_shadow){
            F32 contribution=weight*(sub?amb:fog);
            if(soft)contribution=contribution*(F32(p.shadow>>10)*color_s(as_type<uint>(1.f/63.f)));
            lit=color_add(lit,color_scale(lightcol,contribution));
        }
        F32 twice=F32(2)*color_dot_single(normal,view);
        F32 reflected=color_dot_single(light,color_sub(view,color_scale(normal,twice)));
        if(reflected>zero) {
            F32 attenuation=positional?
                (distance+color_min(one,rough*F32(2))*(one/F32(l.power)-distance))*color_s(s.specular):
                (one+color_min(one,rough*F32(2))*(one/F32(l.power)-one))*color_s(s.specular);
            if(attenuation>zero) {
                unshadowed=color_add(unshadowed,color_scale(lightcol,attenuation/F32(l.power)));
                if(no_shadow) {
                    F32 power=reflected;for(uint n=l.power>>1;n;n>>=1)power=power*power;
                    F32 contribution=(attenuation*power)*(sub?amb:fog);
                    if(soft)contribution=contribution*(F32(p.shadow>>10)*color_s(as_type<uint>(1.f/63.f)));
                    spe=color_add(spe,color_scale(lightcol,contribution));
                }
            }
        }
    }
}
// CalcTotalLight1 combines lit and unshadowed light with the indirect
// bounce; CalcTotalLight2 (iExModes) rescales a self-lit total to luminance.
inline F32 native_luminance(ColorVec c){return (c.x*F32::from_native(0.3f)+c.y*F32::from_native(0.59f))+c.z*F32::from_native(0.11f);}
inline ColorVec native_total_light(constant ExactColorScene& s,constant NativeInteriorColorParams& options,
    ColorVec lit,ColorVec unshadowed,ColorVec diffuse,F32 amb) {
    const F32 one(1);
    if(options.flags&kNativeTotalLight2Flag) {
        F32 bounce=color_max0(one-amb);bounce=bounce*bounce*color_s(s.indirect_reflect);
        const F32 before=native_luminance(lit)+F32(4);
        const ColorVec total=color_add(lit,color_scale(color_mul(lit,diffuse),bounce));
        const F32 after=color_max0(native_luminance(total))+F32(3);
        return color_scale(total,(before+(before+F32(128))*(bounce+F32::from_native(0.11f))*F32::from_native(0.04f))/after);
    }
    F32 remainder=color_max0(one-amb),bounce=remainder*remainder;bounce=bounce*bounce;
    ColorVec scale=color_scale(diffuse,bounce*color_s(s.indirect_reflect));
    F32 add=remainder-bounce;scale=color_add(scale,{add,add,add});
    return color_add(lit,color_mul(unshadowed,color_mul(diffuse,scale)));
}
inline ColorVec native_color_output(ColorVec c,constant NativeInteriorColorParams& options) {
    F32 zero(0),max(255);
    c={color_clamp(c.x,zero,max),color_clamp(c.y,zero,max),color_clamp(c.z,zero,max)};
    if(options.squared)c={sqrt(c.x*max),sqrt(c.y*max),sqrt(c.z*max)};
    if(options.gamma_mode) {
        ColorVec adjusted=options.gamma_mode>0?ColorVec{sqrt(c.x*max),sqrt(c.y*max),sqrt(c.z*max)}:
            ColorVec{c.x*c.x/max,c.y*c.y/max,c.z*c.z/max};
        c=color_mix(adjusted,c,color_s(options.gamma));
    }
    return c;
}
}
