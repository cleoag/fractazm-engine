#define MB3D_METAL_NATIVE_FLOAT 1
#include "render/native_metal/material_palette.h"
#include "render/native_metal/inside_palette_math.h"
#include <bit>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace mb3d_native;
using namespace mb3d_metal_exact;
static unsigned checks=0;
static void check(bool value){++checks;if(!value)throw std::runtime_error("material palette check");}
static uint32_t bits(float x){return std::bit_cast<uint32_t>(x);}
int main(){try{
    // Widening to binary64 is exact, including every subnormal leading-bit
    // position and the transition to the smallest normal FP32 value.
    for(unsigned leading=0;leading<=23;++leading)
    for(uint32_t fraction:{(1u<<leading)-1u,1u<<leading,(1u<<leading)+1u})
    for(uint32_t sign:{0u,0x80000000u}) {
        const float value=std::bit_cast<float>(sign|fraction);
        check(native_encode_double(value)==std::bit_cast<uint64_t>(double(value)));
    }
    ExactColorScene s{};NativeInteriorColorParams p{};
    s.palette_mul=bits(1.f/16384);s.palette_z_mul=bits(.5f/16384);
    p.version=4;p.palette_start=bits(32768);p.palette_mul=bits(1.f/16384);
    for(unsigned i=0;i<10;++i){s.palette_positions[i]=i*3000;s.palette_divisors[i]=bits(1.f/3000);
        for(unsigned c=0;c<3;++c){s.palette_diffuse[i][c]=bits(i*10.f+c);s.palette_specular[i][c]=bits(i*.5f+c);}
        s.palette_specular[i][3]=bits(i*.09f);}
    for(unsigned i=0;i<4;++i){p.positions[i]=i*8000;p.divisors[i]=bits(1.f/8000);
        for(unsigned c=0;c<4;++c)p.colors[i][c]=bits(i*10.f+c);}
    p.flags=2;
    for(unsigned i=0;i<10;++i){auto v=native_material_palette(s,p,{i*3000,0,F32(0)});
        const unsigned stop=(i==0||i==9)?i:i-1;
        check(!v.fault&&v.diffuse.x.value==stop*10.f&&v.specular.x.value==stop*.5f);}
    for(unsigned i=0;i<4;++i){auto v=native_material_palette(s,p,{32768+i*8000,0,F32(0)});
        const unsigned stop=i?i-1:0;check(!v.fault&&v.diffuse.x.value==stop*10.f&&v.specular.x.value==stop*10.f+3);}
    p.flags=10;auto trap=native_material_palette(s,p,{100,6000,F32(0)});
    check(!trap.fault&&trap.diffuse.x.value==10.f);
    for(unsigned flags:{0u,1u,2u,3u,8u,9u,10u,11u})for(unsigned gradient=32768;gradient<=65535;gradient+=127)
    for(float z:{-100000.f,-1.f,0.f,1.f,100000.f}) {
        p.flags=flags;auto a=native_material_palette(s,p,{gradient,1234,F32::from_native(z)});
        const auto b=mb3d_inside::inside_palette(p,gradient,.5f/16384,z);
        check(!a.fault&&!b.fault);
        check(a.diffuse.x.to_bits()==bits(b.diffuse[0])&&a.diffuse.y.to_bits()==bits(b.diffuse[1])&&a.diffuse.z.to_bits()==bits(b.diffuse[2]));
        check(a.specular.x.to_bits()==bits(b.specular)&&a.specular.y.to_bits()==bits(b.specular)&&a.specular.z.to_bits()==bits(b.specular));
        check(a.specular_w.to_bits()==bits(b.specular)); // CalcColorsInside: w is the alpha
    }
    // Exterior transparency: CalcColors' LinInterpolate2SVecs of ColSpe w.
    for(unsigned flags:{0u,1u,2u,3u})for(unsigned gradient=0;gradient<32768;gradient+=97) {
        p.flags=flags;const auto a=native_material_palette(s,p,{gradient,0,F32(0)});check(!a.fault);
        int index=int(std::lrint(float(gradient)*(1.f/16384)*16384.f));if(flags&1)index&=32767;
        float w;
        if(!(flags&1)&&(index<0||index>=s.palette_positions[9]))w=index<0?0.f:9*.09f;
        else {
            unsigned right=5;
            if(s.palette_positions[right]<index){do{++right;}while(right<10&&s.palette_positions[right]<index);}
            else while(right>1&&s.palette_positions[right-1]>=index)--right;
            const unsigned left=right-1;if(right>9)right=0;
            const float l=left*.09f,r=right*.09f,t=float(index-s.palette_positions[left])*(1.f/3000);
            w=(flags&2)?l:l+(r-l)*t;
        }
        check(a.specular_w.to_bits()==bits(w));
    }
    check(native_material_palette(s,p,{65536,0,F32(0)}).fault==1);
    check(native_material_palette(s,p,{0,65536,F32(0)}).fault==1);
    check(native_material_palette(s,p,{0,0,F32::from_native(std::numeric_limits<float>::quiet_NaN())}).fault==1);
    std::cout<<"PASS Native material palette checks="<<checks<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<" checks="<<checks<<'\n';return 1;}}
