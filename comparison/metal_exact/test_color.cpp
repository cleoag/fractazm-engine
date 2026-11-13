#include "color_host.h"
#include "color_math.h"
#include "scene_host_internal.h"
#include "core/colorizer.h"
#include "core/renderer.h"
#include <bit>
#include <cstdio>
#include <stdexcept>
using namespace mb3d;
using namespace mb3d::metal_exact;
using namespace mb3d_metal_exact;
namespace {
unsigned checks=0;
void check(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
ExactSurface export_pixel(const TsiLight5& s){ExactSurface p{};p.normal[0]=s.NormalX;p.normal[1]=s.NormalY;p.normal[2]=s.NormalZ;p.depth_packed=uint32_t(s.RoughZposFine)|(uint32_t(s.Zpos)<<16);p.si_gradient=s.SIgradient;p.o_trap=s.OTrap;p.shadow=s.Shadow;p.amb_shadow=s.AmbShadow;return p;}
}
int main(int argc,char**argv) {
    const mb3d::ScopedNumericPrecision precision(mb3d::PrecisionMode::Compatible);
 try {
    if(argc!=2)throw std::invalid_argument("Usage: test_metal_exact_color SCENE");
    for(bool hard_shadows:{false,true}) {
        auto scene=PrepareExactScene(argv[1],64,48,65536,hard_shadows);
        auto color=PrepareExactColor(scene);
        check(valid_color(color.device),"prepared color capability");
        for(unsigned function=0;function<4;++function) {
            auto table=color.device;table.light_function=function;
            for(int i=0;i<=512;++i)for(float rough:{0.0f,0.125f,1.0f}) {
                const float dot=float(i-256)/250.0f;
                const float expected=GetCosTabVal(function,dot,rough);
                auto actual=color_cos_table(table,F32::from_float(dot),F32::from_float(rough));
                check(actual.bits==std::bit_cast<uint32_t>(expected),"independent production diffuse table/spline");
            }
        }
        RenderOptions options;options.precision=mb3d::PrecisionMode::Compatible;options.thread_count=2;
        auto header=scene.host->file.header;auto addon=scene.host->file.addon;
        auto rendered=Render(header,addon,options);
        check(rendered.success,"production reference render");
        std::vector<ExactSurface> surface;for(const auto& p:rendered.silight)surface.push_back(export_pixel(p));
        auto reference=ReferenceColor(scene,surface,2);
        unsigned hit=0,miss=0;
        for(size_t i=0;i<surface.size();++i) {
            auto actual=exact_color_pixel(color.device,color.columns[i%64],color.rows[i/64],surface[i],unsigned(i/64));
            unsigned rgb=unsigned(rendered.image[i*3])|unsigned(rendered.image[i*3+1])<<8|unsigned(rendered.image[i*3+2])<<16;
            if(actual.fault||actual.rgb!=reference[i].rgb||reference[i].rgb!=rgb)
                std::fprintf(stderr,"color mismatch HS%d pixel%zu GPUshared %06x CPUadapter %06x fullRender %06x fault%u\n",hard_shadows,i,actual.rgb,reference[i].rgb,rgb,actual.fault);
            check(actual.fault==0,"color fault");
            check(actual.rgb==reference[i].rgb,"exact shared algorithm vs production color");
            check(reference[i].rgb==rgb,"CPU color adapter vs full Render RGB");
            ((surface[i].depth_packed>>16)<32768?hit:miss)++;
        }
        check(hit>0&&miss>0,"real scene has foreground/background");
        auto invalid=color.device;invalid.version=0;
        check(exact_color_pixel(invalid,color.columns[0],color.rows[0],surface[0],0).fault!=0,"bad color ABI rejection");
        invalid=color.device;invalid.light_pow=32;
        check(exact_color_pixel(invalid,color.columns[0],color.rows[0],surface[0],0).fault!=0,"unsupported light power rejection");
        invalid=color.device;invalid.palette_start=0x7fc00000;
        check(exact_color_pixel(invalid,color.columns[0],color.rows[0],surface[0],0).fault!=0,"nonfinite palette rejection");
        invalid=color.device;invalid.z_mul=0;
        check(exact_color_pixel(invalid,color.columns[0],color.rows[0],surface[0],0).fault!=0,"zero depth divisor rejection");
        auto faulty=surface[0];faulty.normal[0]=32768;
        check(exact_color_pixel(color.device,color.columns[0],color.rows[0],faulty,0).fault!=0,"invalid normal width rejection");
        faulty=surface[0];faulty.reserved[2]=1;
        check(exact_color_pixel(color.device,color.columns[0],color.rows[0],faulty,0).fault!=0,"reserved surface field rejection");
        faulty=surface[0];faulty.fault=1;
        check(exact_color_pixel(color.device,color.columns[0],color.rows[0],faulty,0).fault!=0,"upstream fault rejection");
    }
    std::printf("exact color %u checks PASS\n",checks);return 0;
 }catch(const std::exception& e){std::fprintf(stderr,"exact color failure after %u checks: %s\n",checks,e.what());return 1;}
}
