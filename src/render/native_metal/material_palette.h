#pragma once
// Shared Native material primitive. It returns linear palette components, never
// final RGB: primary output and reflection accumulation retain their own policies.
#ifndef MB3D_METAL_NATIVE_FLOAT
#error Native material palette requires the hardware FP32 profile
#endif
#include "generic_color_abi.h"
#include "comparison/metal_exact/color_math.h"
#ifdef __METAL_VERSION__
#define MP_CONSTANT constant
#else
#define MP_CONSTANT const
#endif
namespace mb3d_native {
using namespace mb3d_metal_exact;
struct MaterialPaletteContext {U32 gradient,orbit_trap;F32 z;};
// specular_w is the palette transparency (TSVec w of ColSpe, or the interior
// alpha) that SR transmission reads from SDsvecs[0].
struct MaterialPalette {ColorVec diffuse{},specular{};F32 specular_w{};U32 fault=0;};
// Constant palette metadata is validated by the material entry point. This
// function checks the dynamic coordinates and preserves each legacy stop rule.
inline MaterialPalette native_material_palette(MP_CONSTANT ExactColorScene& s,
    MP_CONSTANT NativeInteriorColorParams& options,MaterialPaletteContext context) {
    MaterialPalette out;
    if(context.gradient>65535||context.orbit_trap>65535||!isfinite(context.z)){out.fault=1;return out;}
    const bool inside=context.gradient>=32768;
    const U32 coordinate=inside?context.gradient:((options.flags&8u)?(context.orbit_trap&32767u):context.gradient);
    const F32 start=color_s(inside?options.palette_start:s.palette_start);
    const F32 multiplier=color_s(inside?options.palette_mul:s.palette_mul);
    const F32 raw=((F32(coordinate)-start)*multiplier+color_s(s.palette_z_mul)*context.z)*F32(16384);
    if(!isfinite(raw)){out.fault=2;return out;}
    int index=color_clamp(raw,-color_s(0x4e6e6b28),color_s(0x4e6e6b28)).to_i32_rne();
    if(options.flags&1u)index&=32767;
    if(inside) {
        F32 alpha;
        // Interior uses >last; exterior uses >=last. Exact-stop ties choose
        // the previous stop, including the last interior stop in step mode.
        if(!(options.flags&1u)&&(index<0||index>options.positions[3])) {
            const int stop=index<0?0:3;
            out.diffuse=color_vec(options.colors[stop]);alpha=color_s(options.colors[stop][3]);
        } else {
            int right=1;while(right<4&&options.positions[right]<index)++right;
            const int left=right-1;right&=3;
            if(options.flags&2u){out.diffuse=color_vec(options.colors[left]);alpha=color_s(options.colors[left][3]);}
            else {
                const F32 amount=F32(index-options.positions[left])*color_s(options.divisors[left]);
                if(!isfinite(amount)){out.fault=2;return out;}
                out.diffuse=color_mix(color_vec(options.colors[right]),color_vec(options.colors[left]),amount);
                alpha=color_s(options.colors[left][3])+
                    (color_s(options.colors[right][3])-color_s(options.colors[left][3]))*amount;
            }
        }
        out.specular={alpha,alpha,alpha};out.specular_w=alpha;
    } else {
        if(!(options.flags&1u)&&(index<0||index>=s.palette_positions[9])) {
            const int stop=index<0?0:9;
            out.diffuse=color_vec(s.palette_diffuse[stop]);out.specular=color_vec(s.palette_specular[stop]);
            out.specular_w=color_s(s.palette_specular[stop][3]);
        } else {
            int right=5;
            if(s.palette_positions[right]<index){do{++right;}while(right<10&&s.palette_positions[right]<index);}
            else {while(right>1&&s.palette_positions[right-1]>=index)--right;}
            const int left=right-1;if(right>9)right=0;
            if(options.flags&2u){out.diffuse=color_vec(s.palette_diffuse[left]);out.specular=color_vec(s.palette_specular[left]);
                out.specular_w=color_s(s.palette_specular[left][3]);}
            else {
                const F32 amount=F32(index-s.palette_positions[left])*color_s(s.palette_divisors[left]);
                if(!isfinite(amount)){out.fault=2;return out;}
                out.diffuse=color_mix(color_vec(s.palette_diffuse[right]),color_vec(s.palette_diffuse[left]),amount);
                out.specular=color_mix(color_vec(s.palette_specular[right]),color_vec(s.palette_specular[left]),amount);
                out.specular_w=color_s(s.palette_specular[left][3])+
                    (color_s(s.palette_specular[right][3])-color_s(s.palette_specular[left][3]))*amount;
            }
        }
    }
    // Consumers retain their original finite-result/fault publication boundary.
    // In particular, exterior reflection records palette outputs before its
    // final radiance check; interior reflection rejects overflow before that.
    return out;
}
}
#undef MP_CONSTANT
