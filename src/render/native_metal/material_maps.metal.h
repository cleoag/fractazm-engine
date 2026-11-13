#pragma once
// Native material maps: CPU GetLightMapPixel, GetLightMapPixelSphere and
// GetDiffMapWrap3D over the loader's raw samples. Native arithmetic is
// binary32 throughout; the CPU's binary64 coordinate steps round here.
#include "generic_color_abi.h"
#include "comparison/metal_exact/color_math.h"
#include "material_coordinates.metal.h"
namespace mb3d_native {
using namespace mb3d_metal_exact;
inline bool valid_material_map(constant NativeMaterialMap& m,uint sample_count) {
    if(m.word_map>1||m.reserved||m.width>16384||m.height>16384||(m.width==0)!=(m.height==0))return false;
    if(!isfinite(color_s(m.intensity)))return false;
    for(uint c=0;c<4;++c)if(!isfinite(color_s(m.average[c]))||!isfinite(color_s(m.average_squared[c])))return false;
    for(uint i=0;i<3;++i)for(uint j=0;j<4;++j)if(!isfinite(color_s(m.rotation[i][j])))return false;
    // An unloaded map (width 0) samples black, as GetLightMapPixel does.
    if(!m.width)return !m.offset&&!m.x_factor&&!m.y_factor;
    if(color_s(m.x_factor)!=F32(m.width)||color_s(m.y_factor)!=F32(m.height))return false;
    const ulong count=ulong(m.width+1)*ulong(m.height+1)*3ul;
    return ulong(m.offset)+count<=ulong(sample_count);
}
// interior.flags gives the palette bit (iColOnOT&1) and the light geometry.
inline bool valid_material_maps(constant NativeMaterialMaps& maps,constant NativeInteriorColorParams& interior) {
    if(maps.version!=kNativeMaterialMapVersion||maps.reserved[0]||maps.reserved[1]||
       maps.diffuse_mode>9||maps.diffuse_mode==1||maps.yc_combine>1||maps.light_mask>63||
       (maps.background&~(kNativeBackgroundMap|kNativeBackgroundAddLight|kNativeBackgroundAmbient|kNativeBackgroundRotatedAmbient))||
       (maps.background==kNativeBackgroundRotatedAmbient?false:(maps.background&&!(maps.background&kNativeBackgroundMap)))||
       (!maps.diffuse_mode&&!maps.light_mask&&!maps.background)||(!maps.diffuse_mode&&maps.yc_combine))return false;
    if(maps.background&&!(interior.flags&kNativeLightGeometryFlag))return false;
    if((maps.background&kNativeBackgroundMap)&&!valid_material_map(maps.background_map,maps.sample_count))return false;
    if(maps.diffuse_mode) {
        if(((maps.diffuse_mode&1u)!=0)!=((interior.flags&8u)!=0))return false;
        if(maps.diffuse_mode>5&&!(interior.flags&kNativeLightGeometryFlag))return false;
        const ExactU32 scalars[8]={maps.offset_x,maps.offset_y,maps.rotation_sin,maps.rotation_cos,maps.scale,
            maps.mid[0],maps.mid[1],maps.mid[2]};
        for(uint i=0;i<8;++i)if(!isfinite(color_s(scalars[i])))return false;
        if(!valid_material_map(maps.diffuse,maps.sample_count))return false;
    }
    for(uint i=0;i<6;++i) {
        if(!((maps.light_mask>>i)&1u))continue;
        if(interior.lights[i].enabled||!valid_material_map(maps.lights[i],maps.sample_count))return false;
    }
    return true;
}
// MakeSplineCoeff: four Single weights; c0/c3 feed the later expressions
// before their own rounding.
struct MaterialWeights {F32 w[4];};
inline MaterialWeights material_spline(F32 t) {
    const F64 x=t.to_f64(),sixth=color_d(0x3fc5555555555555ULL);
    const F64 c3=((x*x)*x)*sixth,c0=(sixth-((F64(1)-x)*x)*color_d(0x3fe0000000000000ULL))-c3;
    const F64 c2=((x+c0)-c3)-c3;
    return {{color_single(c0),color_single(((F64(1)-c2)-c3)-c0),color_single(c2),color_single(c3)}};
}
inline F32 material_clamp01(F32 x){const F32 zero(0),one(1);return x<=zero?zero:(x>one?one:x);}
// fmod(x,1) for the finite coordinates of the callers.
inline F32 material_fraction(F32 x){return x-F32::from_native(metal::trunc(x.value));}
inline F32 material_floor(F32 x){return F32::from_native(metal::floor(x.value));}
// GetLightMapPixel: wrap 0 none, 1 both axes, 2 horizontal only. Outlined:
// a material inlines it at up to fifteen sites (six unrolled light maps,
// diffuse, box and background modes), which took the backend compiler minutes
// per reflection shade pipeline; each caller multiplies or selects its result,
// so no FMA contraction crosses the call.
__attribute__((noinline)) ColorVec material_pixel(constant NativeMaterialMap& m,device const float* samples,F32 x,F32 y,bool squared,uint wrap) {
    const F32 zero(0);
    if(m.width<=4)return {zero,zero,zero};
    const F32 xs=material_clamp01(x)*color_s(m.x_factor),ys=material_clamp01(y)*color_s(m.y_factor);
    const int w=int(m.width),h=int(m.height);
    // Clamp01S bounds finite coordinates; the lower clamp only guards NaN.
    const int xf=metal::clamp(xs.to_i32_rtz(),0,w-1);
    int yf=metal::clamp(ys.to_i32_rtz(),0,h-1);
    const MaterialWeights xv=material_spline(xs-F32(xf)),yv=material_spline(ys-F32(yf));
    int nx1,nx2,nx3;
    if(wrap) {nx1=xf>0?-1:w-1;nx2=xf<w-1?2:2-w;nx3=1;}
    else {nx1=xf>0?-1:0;nx3=xf<w-1?1:0;nx2=xf<w-2?2:nx3;}
    if(wrap==1)yf=yf>0?yf-1:yf+h-1;else --yf;
    const int columns[4]={xf+nx1,xf,xf+nx3,xf+nx2};
    // SampleLightMapRow normalization (Single constants of the CPU).
    const F32 normalize=m.word_map?color_s(squared?0x2f800100u:0x37800080u):color_s(squared?0x37810182u:0x3b808081u);
    F32 rows[4][3];
    for(int row=0;row<4;++row,++yf) {
        const int cy=yf<0?(wrap==1?h+yf:0):(yf>=h?(wrap==1?yf%h:h-1):yf);
        for(uint channel=0;channel<3;++channel) {
            F32 s[4];
            for(int k=0;k<4;++k) {
                const F32 raw=F32::from_native(samples[m.offset+(uint(cy)*(m.width+1)+uint(columns[k]))*3u+channel]);
                s[k]=(squared?raw*raw:raw)*(xv.w[k]*normalize);
            }
            rows[row][channel]=(s[0]+s[2])+(s[1]+s[3]);
        }
    }
    F32 out[3];
    for(uint c=0;c<3;++c)
        out[c]=(((rows[0][c]*yv.w[0]+rows[1][c]*yv.w[1])+rows[2][c]*yv.w[2])+rows[3][c]*yv.w[3])*color_s(m.intensity);
    return {out[0],out[1],out[2]};
}
// RotateSVectorS: v'[j]=x*m[0][j]+y*m[1][j]+z*m[2][j].
inline ColorVec material_rotate(ColorVec v,ColorVec m0,ColorVec m1,ColorVec m2) {
    return {(v.x*m0.x+v.y*m1.x)+v.z*m2.x,(v.x*m0.y+v.y*m1.y)+v.z*m2.y,(v.x*m0.z+v.y*m1.z)+v.z*m2.z};
}
// GetLightMapPixelSphere with the map's PicRotMatrix.
inline ColorVec material_sphere(constant NativeMaterialMap& m,device const float* samples,ColorVec v,bool squared) {
    const F32 zero(0),one(1),one_half=color_s(0x3f000000u);
    const F32 length2=(v.x*v.x+v.y*v.y)+v.z*v.z;
    // NormalizeSVec adds 1e-100 in binary64: a zero vector stays zero.
    ColorVec n=length2>zero?color_scale(v,one/sqrt(length2)):ColorVec{zero,zero,zero};
    n=material_rotate(n,color_vec(m.rotation[0]),color_vec(m.rotation[1]),color_vec(m.rotation[2]));
    const F32 u=F32::from_native(metal::atan2(n.x.value,n.z.value))*color_s(0xbe22f983u)+one_half;
    const F32 limit=color_s(0x3f7ffffeu); // ArcSinSafe: |d| > 0.9999999 → ±π/2
    const F32 latitude=n.y>limit?color_s(0x3fc90fdbu):(n.y< -limit?-color_s(0x3fc90fdbu):F32::from_native(metal::asin(n.y.value)));
    return material_pixel(m,samples,u,one_half-latitude*color_s(0x3ea2f983u),squared,2);
}
// GetDiffMapWrap3D: object position (CalcObjPos) and the paint-rotated normal.
inline F32 material_wrap_negative(F32 x) {
    return x<F32(0)?(x+F32(1))-F32::from_native(metal::round(x.value)):x;
}
inline F32 material_box_coordinate(F32 a,F32 offset) {
    const F32 f=a+offset;return material_fraction((f-material_floor(f))+F32(1));
}
inline ColorVec material_wrap3d(constant NativeMaterialMaps& maps,device const float* samples,ColorVec normal,
    constant NativeLightGeometry& g,ColorVec object,bool squared) {
    const F32 one(1),scale=color_s(maps.scale);
    F32 s1[3]={(object.x+color_s(maps.mid[0]))*scale,(object.y+color_s(maps.mid[1]))*scale,(object.z+color_s(maps.mid[2]))*scale};
    ColorVec s2=material_rotate(normal,color_vec(g.matrix[0]),color_vec(g.matrix[1]),color_vec(g.matrix[2]));
    const F32 sum=(abs(s2.x)+abs(s2.y))+abs(s2.z);
    if(sum>F32::from_native(1e-30f))s2=color_scale(s2,one/sum);
    s2={abs(s2.x),abs(s2.y),abs(s2.z)};
    constant auto& m=maps.diffuse;
    const F32 ox=color_s(maps.offset_x),oy=color_s(maps.offset_y);
    if(maps.diffuse_mode>7) {
        for(uint i=0;i<3;++i)s1[i]=material_wrap_negative(s1[i]);
        const ColorVec a=color_scale(material_pixel(m,samples,material_box_coordinate(s1[1],ox),material_box_coordinate(s1[2],oy),squared,1),s2.x);
        const ColorVec b=color_scale(material_pixel(m,samples,material_box_coordinate(s1[0],ox),material_box_coordinate(s1[2],oy),squared,1),s2.y);
        const ColorVec c=color_scale(material_pixel(m,samples,material_box_coordinate(s1[0],ox),material_box_coordinate(s1[1],oy),squared,1),s2.z);
        return color_add(color_add(a,b),c);
    }
    for(uint i=0;i<3;++i)s1[i]=F32::from_native(metal::sin(s1[i].value));
    const F32 x=material_wrap_negative((s1[1]*s2.x+s1[0]*(s2.y+s2.z))+ox);
    const F32 y=material_wrap_negative((s1[2]*(s2.x+s2.y)+s1[1]*s2.z)+oy);
    return material_pixel(m,samples,x-material_floor(x),y-material_floor(y),squared,1);
}
// CalcPixelColor2/CalcPixelColorSqr/CalcPixelColorSvec background branch: the
// loaded background map sampled along AbsViewVec, scaled to 0..255.
inline ColorVec native_background_color(constant NativeMaterialMaps& maps,device const float* samples,
    ColorVec absolute,bool squared) {
    return color_scale(material_sphere(maps.background_map,samples,absolute,squared),F32(255));
}
// bUseSmallBGpicForAmb: the ambient light from the background map (BGsmallLM
// shares its pixels) around the paint-rotated normal, toward the depth color
// by the background depth fog, times the ambient shadow.
inline ColorVec native_background_ambient(constant NativeMaterialMaps& maps,device const float* samples,
    constant NativeLightGeometry& g,ColorVec normal,ColorVec dep,F32 amb,F32 depth,bool squared,bool far_fog) {
    const F32 one(1);
    const ColorVec rotated=material_rotate(normal,color_vec(g.matrix[0]),color_vec(g.matrix[1]),color_vec(g.matrix[2]));
    ColorVec light=native_background_color(maps,samples,rotated,squared);
    F32 t=color_max0(one-color_max0(one-F32(28000)*depth));
    if(far_fog)t=t*t;
    if(maps.background&kNativeBackgroundAddLight)light=color_add(light,color_scale(dep,t));
    else light=color_add(dep,color_scale(color_sub(light,dep),one-t)); // LinInterpolate2SVecs
    return color_scale(light,amb);
}
// The ambient gradient's Y: bUseSmallBGpicForAmb without a loaded map rotates
// the normal by the paint matrix first (RotateSVectorS), then bAmbRelObj
// takes the object-relative Y as usual.
inline F32 native_ambient_y(constant NativeMaterialMaps* maps,constant NativeLightGeometry* g,ColorVec normal,
                            constant NativeInteriorColorParams& p) {
    if(maps&&(maps->background&kNativeBackgroundRotatedAmbient))
        normal=material_rotate(normal,color_vec(g->matrix[0]),color_vec(g->matrix[1]),color_vec(g->matrix[2]));
    return native_material_ambient_y(normal,p);
}
}
