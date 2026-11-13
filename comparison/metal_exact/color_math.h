#pragma once
#include "soft_single.h"
#include "color_abi.h"
#include "shadow_abi.h"
#ifdef __METAL_VERSION__
#define MB3D_COLOR_CONST constant
#else
#define MB3D_COLOR_CONST const
#endif
namespace mb3d_metal_exact {
inline F32 color_single(F64 x){return F32::from_bits(x.to_f32_bits());}
inline F32 color_s(ExactU32 bits){return F32::from_bits(bits);}
inline F64 color_d(ExactU64 bits){return F64::from_bits(bits);}
inline F32 color_max0(F32 x){return x>F32(0)?x:F32(0);}
inline F32 color_min(F32 a,F32 b){return b<a?b:a;}
inline F32 color_clamp(F32 x,F32 lo,F32 hi){return color_min(hi,x<lo?lo:x);}
struct ColorVec {F32 x,y,z;};
inline ColorVec color_vec(MB3D_COLOR_CONST ExactU32* a){return {color_s(a[0]),color_s(a[1]),color_s(a[2])};}
inline ColorVec color_add(ColorVec a,ColorVec b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
inline ColorVec color_sub(ColorVec a,ColorVec b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
inline ColorVec color_mul(ColorVec a,ColorVec b){return {a.x*b.x,a.y*b.y,a.z*b.z};}
inline ColorVec color_scale(ColorVec a,F32 b){return {a.x*b,a.y*b,a.z*b};}
inline ColorVec color_mix(ColorVec a,ColorVec b,F32 t){return color_add(b,color_scale(color_sub(a,b),t));}
inline F32 color_dot_single(ColorVec a,ColorVec b){return (a.x*b.x+a.y*b.y)+a.z*b.z;}
inline F32 color_dot_double(ColorVec a,ColorVec b){return color_single((a.x.to_f64()*b.x.to_f64()+a.y.to_f64()*b.y.to_f64())+a.z.to_f64()*b.z.to_f64());}
inline ColorVec color_view(ExactTrig column,ExactTrig row) {
    F64 x=-color_d(column.sin_value),y=color_d(row.sin_value),z=color_d(column.cos_value)*color_d(row.cos_value);
    F64 len=(x*x+y*y)+z*z;
    if(len>color_d(0x39b4484bfeebc2a0ULL)){F64 inv=F64(1)/sqrt(len);x=x*inv;y=y*inv;z=z*inv;}
    return {color_single(x),color_single(y),color_single(z)};
}
inline F32 color_cos_table(MB3D_COLOR_CONST ExactColorScene& scene,F32 dot,F32 rough) {
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
    for(int i=0;i<4;++i){p[i]=color_s(scene.diffuse_table[scene.light_function][ip+i])*w[i];r[i]=color_s(scene.diffuse_table[scene.light_function+4][ip+i])*w[i];}
    F32 base=(p[0]+p[2])+(p[1]+p[3]),rough_value=(r[0]+r[2])+(r[1]+r[3]);
    return base+rough*(rough_value-base);
}
inline F32 color_ambient(MB3D_COLOR_CONST ExactColorScene& s,ExactSurface p) {
    F32 shadow=p.amb_shadow>=16383?F32(1):F32(p.amb_shadow)*color_s(0x38800200);
    F32 amplitude=color_s(s.ambient_amplitude);
    if(amplitude>F32(1)){
        F32 d=F32(1)-shadow;return d+(amplitude-F32(1))*(d*d-d);
    }
    return F32(1)-amplitude*shadow;
}
inline F32 color_z(MB3D_COLOR_CONST ExactColorScene& s,ExactSurface p) {
    F64 z;
    if((p.depth_packed>>16)>32767)z=color_d(s.back_distance);
    else {
        F64 zvalue=(F64(8388352)-F64(p.depth_packed>>8))/color_d(s.z_mul)+F64(1);
        z=((zvalue*zvalue-F64(1))*color_d(s.step_width))/color_d(s.z_corr);
    }
    return color_single(z+color_d(s.z_mid_difference));
}
inline bool valid_color(MB3D_COLOR_CONST ExactColorScene& s) {
    const ExactU64 geometry[5]={s.z_corr,s.z_mul,s.z_mid_difference,s.step_width,s.back_distance};
    for(unsigned i=0;i<5;++i)if(!isfinite(color_d(geometry[i])))return false;
    if(color_d(s.z_corr)<=F64(0)||color_d(s.z_mul)<=F64(0)||color_d(s.step_width)<=F64(0))return false;
    const ExactU32 scalar[14]={s.depth,s.shadow_start,s.shadow_z_mul,s.shadow_gradient,s.ambient_amplitude,
        s.palette_start,s.palette_mul,s.palette_z_mul,s.diffuse,s.specular,s.indirect_reflect,
        s.diffuse_shadowing,s.roughness,s.dynamic_fog_mul};
    for(unsigned i=0;i<14;++i)if(!isfinite(color_s(scalar[i])))return false;
    for(unsigned i=0;i<10;++i) {
        if(!isfinite(color_s(s.palette_divisors[i]))||s.palette_positions[i]<0||s.palette_positions[i]>32767)return false;
        if(i>0&&s.palette_positions[i]<s.palette_positions[i-1])return false;
        for(unsigned c=0;c<3;++c)if(!isfinite(color_s(s.palette_diffuse[i][c]))||!isfinite(color_s(s.palette_specular[i][c])))return false;
    }
    for(unsigned c=0;c<3;++c) {
        if(!isfinite(color_s(s.light_direction[c]))||!isfinite(color_s(s.light_color[c])))return false;
        for(unsigned i=0;i<2;++i)if(!isfinite(color_s(s.depth_colors[i][c]))||!isfinite(color_s(s.ambient_colors[i][c]))||!isfinite(color_s(s.fog_colors[i][c])))return false;
    }
    return s.version==1 && s.width>0 && s.height>0 && s.width<=4096 && s.height<=4096 &&
           !s.reserved && !s.reserved2 && s.light_pow==64 &&
           ((s.light_function==3 && s.hs_calculated==0)||(s.light_function==0 && s.hs_calculated==1)) &&
           s.hs_mask==1024 && s.hs_enabled==1;
}
// CalcPixelColor2's admitted branch: one global light, cyclic outside palette,
// total-light mode 1, two-color ambient/background and ordinary depth fog.
// All per-pixel operations retain their production Single/Double store order.
inline ExactColorPixel exact_color_pixel(MB3D_COLOR_CONST ExactColorScene& s,
    ExactTrig column,ExactTrig row,ExactSurface p,unsigned y) {
    ExactColorPixel out{};
    if(!valid_color(s)||p.fault||y>=s.height||p.si_gradient>32767||p.amb_shadow>65535||p.shadow>65535||p.o_trap>65535||p.reserved[0]||p.reserved[1]||p.reserved[2]){out.fault=1;return out;}
    for(unsigned i=0;i<3;++i)if(p.normal[i]<-32768||p.normal[i]>32767){out.fault=1;return out;}
    if(!isfinite(color_d(column.sin_value))||!isfinite(color_d(column.cos_value))||!isfinite(color_d(row.sin_value))||!isfinite(color_d(row.cos_value))){out.fault=1;return out;}
    F32 one(1),one_half=color_s(0x3f000000),zero(0);
    ColorVec dep=color_mix(color_vec(s.depth_colors[1]),color_vec(s.depth_colors[0]),F32(y)/F32(s.height));
    ColorVec result;F32 depth_factor,zpos=color_z(s,p);
    const unsigned depth=p.depth_packed>>16;
    if(depth<32768) {
        F32 rough=F32(p.depth_packed&255)*color_s(s.roughness);
        F32 amb=color_ambient(s,p),fog=color_s(s.diffuse_shadowing)*(amb-one)+one;
        const F64 norm_scale=color_d(0x3f0000200040021eULL);
        ColorVec normal={color_single(F64(p.normal[0])*norm_scale),color_single(F64(p.normal[1])*norm_scale),color_single(F64(p.normal[2])*norm_scale)};
        ColorVec light=color_vec(s.light_direction),lightcol=color_vec(s.light_color),view=color_view(column,row);
        F32 diffuse_weight=color_cos_table(s,color_dot_double(normal,light),rough);
        ColorVec unshadowed=color_scale(lightcol,diffuse_weight),lit{zero,zero,zero},spe{zero,zero,zero};
        bool no_shadow=(p.shadow&s.hs_mask)==0 || s.hs_calculated==0;
        bool subtract_ambient=(s.hs_calculated^s.hs_enabled)!=0;
        if(no_shadow)lit=color_scale(lightcol,diffuse_weight*(subtract_ambient?amb:fog));
        F32 twice_dot=F32(2)*color_dot_single(normal,view);
        F32 reflected=color_dot_single(light,color_sub(view,color_scale(normal,twice_dot)));
        if(reflected>zero) {
            F32 attenuation=(one+color_min(one,rough*F32(2))*(one/F32(s.light_pow)-one))*color_s(s.specular);
            if(attenuation>zero) {
                unshadowed=color_add(unshadowed,color_scale(lightcol,attenuation/F32(s.light_pow)));
                if(no_shadow) {
                    F32 power=reflected;for(unsigned i=s.light_pow>>1;i>0;i>>=1)power=power*power;
                    attenuation=(attenuation*power)*(subtract_ambient?amb:fog);
                    spe=color_scale(lightcol,attenuation);
                }
            }
        }
        F32 raw=(((F32(p.si_gradient)-color_s(s.palette_start))*color_s(s.palette_mul))+color_s(s.palette_z_mul)*zpos)*F32(16384);
        if(!isfinite(raw)){out.fault=2;return out;}
        int ir=color_clamp(raw,-color_s(0x4e6e6b28),color_s(0x4e6e6b28)).to_i32_rne()&32767;
        int right=5;
        if(s.palette_positions[right]<ir){do{++right;}while(right<10&&s.palette_positions[right]<ir);}
        else {while(right>1&&s.palette_positions[right-1]>=ir)--right;}
        int left=right-1;if(right>9)right=0;
        F32 amount=F32(ir-s.palette_positions[left])*color_s(s.palette_divisors[left]);
        ColorVec diffuse=color_mix(color_vec(s.palette_diffuse[right]),color_vec(s.palette_diffuse[left]),amount);
        ColorVec specular=color_mix(color_vec(s.palette_specular[right]),color_vec(s.palette_specular[left]),amount);
        F32 amb_b=(normal.y*one_half+one_half)*amb,amb_a=amb-amb_b;
        ColorVec ambient=color_mul(color_add(color_scale(color_vec(s.ambient_colors[0]),amb_a),color_scale(color_vec(s.ambient_colors[1]),amb_b)),diffuse);
        ColorVec scaled_diffuse=color_scale(diffuse,color_s(s.diffuse));
        lit=color_add(color_add(ambient,color_mul(scaled_diffuse,lit)),color_mul(specular,spe));
        // CalcTotalLight1: ambient bounce uses the unshadowed light sum.
        unshadowed=color_add(ambient,color_scale(color_mul(scaled_diffuse,unshadowed),amb*color_s(0x3e99999a)));
        F32 remainder=color_max0(one-amb),bounce=remainder*remainder;bounce=bounce*bounce;
        ColorVec scale=color_scale(diffuse,bounce*color_s(s.indirect_reflect));
        F32 add=remainder-bounce;scale=color_add(scale,{add,add,add});
        result=color_add(lit,color_mul(unshadowed,color_mul(diffuse,scale)));
        depth_factor=color_max0(F32(int(depth)-28000)*color_s(s.depth)+one);
    } else {
        result=dep;depth_factor=color_max0(one-F32(28000)*color_s(s.depth));
    }
    if(depth_factor<one){F32 t=one-depth_factor;depth_factor=one-t*t;}
    result=color_scale(result,depth_factor);
    unsigned ir=p.shadow&1023;
    F32 fog=((F32(ir)-color_s(s.shadow_start))-color_s(s.shadow_z_mul)*zpos)*color_s(s.shadow_gradient);
    F32 fog_b=color_min(one,F32(ir)*color_s(s.dynamic_fog_mul))*fog;
    result=color_add(result,color_scale(dep,color_max0(one-depth_factor)));
    result=color_add(result,color_add(color_scale(color_vec(s.fog_colors[0]),fog-fog_b),color_scale(color_vec(s.fog_colors[1]),fog_b)));
    if(!isfinite(result.x)||!isfinite(result.y)||!isfinite(result.z)){out.fault=2;return out;}
    unsigned red=color_clamp(result.x,zero,F32(255)).to_i32_rne();
    unsigned green=color_clamp(result.y,zero,F32(255)).to_i32_rne();
    unsigned blue=color_clamp(result.z,zero,F32(255)).to_i32_rne();
    out.rgb=red|(green<<8)|(blue<<16);return out;
}
}
#undef MB3D_COLOR_CONST
