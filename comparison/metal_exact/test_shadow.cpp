#include "shadow_host.h"
#include "shadow_math.h"
#include "scene_host_internal.h"
#include "core/renderer.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
int main(int argc,char** argv){
    const mb3d::ScopedNumericPrecision precision(mb3d::PrecisionMode::Compatible);
    if(argc!=2)return 2;
    unsigned checks=0,failures=0;
    auto check=[&](bool ok,const char* what,unsigned i){++checks;if(!ok&&++failures<=20)std::fprintf(stderr,"%s pixel %u\n",what,i);};
    auto same_state=[](const mb3d_metal_exact::ShadowRowState& a,const mb3d_metal_exact::ShadowRowState& b){
        if(a.last_z.bits!=b.last_z.bits)return false;
        for(unsigned light=0;light<6;++light)
            if(a.last_found[light].bits!=b.last_found[light].bits||a.threshold[light]!=b.threshold[light])return false;
        return true;
    };
    try{
      for(const auto size:{std::pair{64u,48u},std::pair{63u,47u},std::pair{127u,95u}}){
        auto scene=mb3d::metal_exact::PrepareExactScene(argv[1],size.first,size.second);
        auto primary=mb3d::metal_exact::ReferencePrimary(scene,4);
        auto expected=mb3d::metal_exact::ReferenceSurface(scene,4);
        auto params=mb3d::metal_exact::PrepareSurface(scene);
        check(params.version==2&&sizeof(ExactSurfaceParams)==64,"surface ABI v2",0);
        for(unsigned control=0;control<6;++control){
            auto invalid=params;
            if(control==0)invalid.version=3;
            if(control==1)invalid.step_width=0;
            if(control==2)invalid.step_width=0x7ff0000000000000ULL;
            if(control==3)invalid.color_stop_multiplier=0x7ff8000000000000ULL;
            if(control==4)invalid.reserved[2]=1;
            if(control==5)invalid.color_multiplier=0x7ff8000000000000ULL;
            check(mb3d_metal_exact::pack_surface(scene.device,invalid,primary[0]).fault==1,
                  "malformed surface parameters rejected",control);
        }
        auto invalid_primary=primary[0];invalid_primary.reserved_tail=1;
        check(mb3d_metal_exact::pack_surface(scene.device,params,invalid_primary).fault==1,
              "reserved primary tail rejected",0);
        std::vector<ExactSurface> surface(primary.size());
        for(unsigned i=0;i<primary.size();++i){surface[i]=mb3d_metal_exact::pack_surface(scene.device,params,primary[i]);
            check(!std::memcmp(&surface[i],&expected[i],sizeof(ExactSurface)),"surface",i);
            if(std::memcmp(&surface[i],&expected[i],sizeof(ExactSurface))&&failures<=20){const auto&a=surface[i];const auto&b=expected[i];std::fprintf(stderr," depth %x/%x sig %u/%u otrap %u/%u shadow %u/%u amb %u/%u fault %u\n",a.depth_packed,b.depth_packed,a.si_gradient,b.si_gradient,a.o_trap,b.o_trap,a.shadow,b.shadow,a.amb_shadow,b.amb_shadow,a.fault);}
        }
        for(bool enabled:{false,true}){
            auto shadow=mb3d::metal_exact::PrepareShadow(scene,enabled);
            auto ref=mb3d::metal_exact::ReferenceShadow(scene,expected,enabled);
            std::vector<ExactShadowRay> prepared(primary.size());
            unsigned blocked=0,active=0,marching=0,backfaces=0,first_marching=0;
            for(unsigned i=0;i<primary.size();++i){
                prepared[i]=mb3d_metal_exact::prepare_shadow_ray(scene.device,shadow,primary[i],surface[i],i/scene.device.width);
                check(mb3d_metal_exact::valid_prepared_shadow_ray(prepared[i]),"prepared ray valid",i);
                if(prepared[i].active)++active;
                if(prepared[i].backface)++backfaces;
                if(prepared[i].active&&!prepared[i].backface){++marching;first_marching=i;}
            }
            for(unsigned y=0;y<scene.device.height;++y){mb3d_metal_exact::ShadowRowState state{},split_state{};
                for(unsigned x=0;x<scene.device.width;++x){unsigned i=y*scene.device.width+x;
                    auto got=mb3d_metal_exact::shadow_pixel(scene.device,shadow,primary[i],surface[i],state,y);
                    auto split=mb3d_metal_exact::shadow_pixel_prepared(scene.device,shadow,prepared[i],surface[i],split_state);
                    check(!std::memcmp(&got,&ref[i],sizeof(got)),"shadow",i);
                    check(!std::memcmp(&split,&got,sizeof(got)),"prepared shadow equals legacy",i);
                    check(!std::memcmp(&split,&ref[i],sizeof(got)),"prepared shadow equals production",i);
                    check(same_state(state,split_state),"prepared row state equals legacy after each pixel",i);
                    if(got.shadow&1024)++blocked;
                }
            }
            if(enabled){
                check(blocked>0,"discriminating enabled shadows",0);
                check(active>0&&marching>0&&backfaces>0,"discriminating prepared ray branches",0);
                auto header=scene.host->file.header;auto addon=scene.host->file.addon;
                header.bCalculateHardShadow|=1;
                mb3d::RenderOptions options;options.precision=mb3d::PrecisionMode::Compatible;options.no_ao=true;options.thread_count=4;
                auto production=mb3d::Render(header,addon,options);
                if(!production.success)throw std::runtime_error(production.error);
                for(unsigned i=0;i<ref.size();++i){auto p=mb3d::metal_exact::ExportSurface(production.silight[i]);
                    check(!std::memcmp(&p,&ref[i],sizeof(p)),"full production shadow oracle",i);
                }
                auto limited=shadow;limited.max_steps=1;unsigned exhausted=0;
                for(unsigned y=0;y<scene.device.height;++y){mb3d_metal_exact::ShadowRowState state{},split_state{};
                    for(unsigned x=0;x<scene.device.width;++x){unsigned i=y*scene.device.width+x;
                        auto p=mb3d_metal_exact::shadow_pixel(scene.device,limited,primary[i],surface[i],state,y);
                        auto split=mb3d_metal_exact::shadow_pixel_prepared(scene.device,limited,prepared[i],surface[i],split_state);
                        check(!std::memcmp(&split,&p,sizeof(p)),"prepared exhausted ray equals legacy",i);
                        check(same_state(state,split_state),"prepared exhausted row state equals legacy",i);
                        if(p.fault&2)++exhausted;
                    }
                }
                check(exhausted>0,"exhausted shadows report a fault",0);
                limited=shadow;limited.max_steps=0;mb3d_metal_exact::ShadowRowState state{};
                check(mb3d_metal_exact::shadow_pixel(scene.device,limited,primary[0],surface[0],state,0).fault==1,"zero shadow work limit rejected",0);
                check(mb3d_metal_exact::prepare_shadow_ray(scene.device,limited,primary[0],surface[0],0).fault==1,"preparation rejects zero work limit",0);
                check(mb3d_metal_exact::shadow_pixel_prepared(scene.device,limited,prepared[0],surface[0],state).fault==1,"prepared row rejects zero work limit",0);
                limited=shadow;limited.max_iterations_result=shadow.max_iterations_result==61?51:61;
                check(mb3d_metal_exact::valid_shadow(limited),"other catalog iteration limit is valid on its own",0);
                check(!mb3d_metal_exact::valid_shadow_scene(scene.device,limited),"shadow iteration limit must match its scene",0);
                check(mb3d_metal_exact::shadow_pixel(scene.device,limited,primary[0],surface[0],state,0).fault==1,"legacy rejects mismatched iteration limit",0);
                check(mb3d_metal_exact::prepare_shadow_ray(scene.device,limited,primary[0],surface[0],0).fault==1,"preparation rejects mismatched iteration limit",0);
                check(mb3d_metal_exact::shadow_pixel_prepared(scene.device,limited,prepared[0],surface[0],state).fault==1,"prepared row rejects mismatched iteration limit",0);
                auto bad_scene=scene.device;bad_scene.width=0;
                check(mb3d_metal_exact::prepare_shadow_ray(bad_scene,shadow,primary[0],surface[0],0).fault==1,"preparation rejects zero dimensions",0);
                check(mb3d_metal_exact::prepare_shadow_ray(scene.device,shadow,primary[0],surface[0],scene.device.height).fault==1,"preparation rejects row beyond image",0);
                auto bad_geometry=primary[first_marching];bad_geometry.fault=4;
                check(mb3d_metal_exact::prepare_shadow_ray(scene.device,shadow,bad_geometry,surface[first_marching],first_marching/scene.device.width).fault==1,"preparation rejects faulted geometry",first_marching);
                for(unsigned invalid=0;invalid<6;++invalid){
                    auto ray=prepared[first_marching];
                    if(invalid==0)ray.version=2;
                    if(invalid==1)ray.reserved[0]=1;
                    if(invalid==2)ray.position[0]=0x7ff8000000000000ULL;
                    if(invalid==3)ray.threshold=0;
                    if(invalid==4)ray.initial_stop=0xbf800000u;
                    if(invalid==5)ray.initial_de=0x7ff0000000000000ULL;
                    auto before=state;
                    check(mb3d_metal_exact::shadow_pixel_prepared(scene.device,shadow,ray,surface[first_marching],state).fault==1,"corrupt prepared ray rejected",first_marching);
                    check(same_state(before,state),"rejected prepared ray leaves row state unchanged",first_marching);
                }
                // Explicitly exercise the observable decrement on a background
                // pixel, even if this scene's naturally warm counters end first.
                auto background=surface[0];background.depth_packed=0x80000000u;
                auto empty=mb3d_metal_exact::prepare_shadow_ray(scene.device,shadow,primary[0],background,0);
                mb3d_metal_exact::ShadowRowState legacy_state{},prepared_state{};
                legacy_state.threshold[0]=3;prepared_state.threshold[0]=3;
                legacy_state.last_z=mb3d_metal_exact::F64(7);prepared_state.last_z=mb3d_metal_exact::F64(7);
                auto original_background=mb3d_metal_exact::shadow_pixel(scene.device,shadow,primary[0],background,legacy_state,0);
                auto split_background=mb3d_metal_exact::shadow_pixel_prepared(scene.device,shadow,empty,background,prepared_state);
                check(!std::memcmp(&original_background,&split_background,sizeof(ExactSurface))&&same_state(legacy_state,prepared_state)&&prepared_state.threshold[0]==2,"background preserves row decrement and early return",0);
            }
        }
      }
    }catch(const std::exception&e){std::fprintf(stderr,"%s\n",e.what());return 2;}
    std::printf("exact surface/shadow: %u checks, %u failures\n",checks,failures);
    return failures?1:0;
}
