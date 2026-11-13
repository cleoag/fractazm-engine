#include "render/native_metal/native_ao_dispatch.h"
#include "core/numeric_precision.h"
#include <vector>
#include <cstring>
#include <cstdio>
using namespace mb3d;using namespace mb3d_native_ao;
namespace gpu=mb3d::metal_exact::gpu;
unsigned checks=0;void check(bool b,const char* s){++checks;if(!b)throw std::runtime_error(s);}
template<class F>void reject(F f){bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}check(rejected,"AO malformed control accepted");}
int main(int argc,char**argv){@autoreleasepool{try{
    if(argc!=2&&argc!=3)throw std::runtime_error("usage: ao_seed_test NATIVE_LIBRARY [MODE]");
    unsigned selected=0;if(argc==3){const std::string value=argv[2];
        if(value!="2"&&value!="4"&&value!="5"&&value!="8"&&value!="9")throw std::runtime_error("invalid AO test mode");
        selected=unsigned(value[0]-'0');}
    ScopedNumericPrecision precision(PrecisionMode::Native);gpu::Metal m(argv[1],3,8,0,false,true);
    for(uint32_t seed:{0u,1u,0xffffffffu,0x12345678u}){uint32_t exact=seed;
        for(uint32_t n=0;n<=1024;++n){check(advance_seed(seed,n)==exact,"Affine seed jump differs");exact=next_seed(exact);}}
    uint64_t active=0,draws=0,shortcuts=0;
    for(unsigned mode:{2u,4u,5u,8u,9u})if(!selected||selected==mode)for(unsigned width:{3u,37u,65u,129u})for(unsigned workers:{1u,3u}){
        std::printf("CASE mode=%u width=%u workers=%u\n",mode,width,workers);std::fflush(stdout);
        TMandHeader10 h{};h.Width=width;h.Height=width==129?97:17;h.dZoom=1;h.dFOVy=45;h.dZstart=-2;h.dZend=2;h.bCalc3D=1;
        h.bCalcAmbShadowAutomatic=1|((mode>=6?mode-4:mode)<<1);h.bPlanarOptic=mode>=6?2:0;
        h.sAmbShadowThreshold=1;h.SSAORcount=2;h.bSSAO24BorderMirrorSize=30;
        auto p=PrepareNativeScreenAO(h,width+1,h.Height,workers);const auto n=p.params.output_count;
        std::vector<ExactSurface> input(n);
        for(unsigned i=0;i<n;++i){input[i].depth_packed=(i%13==0?0x80000000u:((i*12791)%8388352u)<<8);input[i].amb_shadow=(i*17)%16000;}
        auto buffer=m.buffer(n*sizeof(ExactSurface),input.data());
        using Grouping=native_metal::ScreenAOExecutionPolicy::LaneGrouping;
        const native_metal::NativeImageCommandPolicy serial(native_metal::NativeImageCommandPolicy::Mode::Serial);
        const native_metal::ScreenAOExecutionPolicy ordered(p,false,Grouping::LegacySimd,serial),
            parallel(p,true,Grouping::IndependentOwners,serial);
        const native_metal::ScreenAOExecutionPolicy legacyParallel(p,true,Grouping::LegacySimd,serial);
        check(parallel.rng().group_width==1&&ordered.rng().group_width==0,"Wrong physical lane grouping");
        auto old=RunNativeScreenAO(m,p,buffer,1ull<<30,ordered,[]{});
        std::vector<ExactSurface> reference(n);std::memcpy(reference.data(),old.surface.contents,n*sizeof(ExactSurface));
        if(random_mode(mode)) {
            const native_metal::ScreenAOExecutionPolicy groupedOrdered(p,false,Grouping::IndependentOwners,serial);
            auto regrouped=RunNativeScreenAO(m,p,buffer,1ull<<30,groupedOrdered,[]{});
            check(!std::memcmp(regrouped.surface.contents,reference.data(),n*sizeof(ExactSurface))&&
                  !std::memcmp(&regrouped.totals,&old.totals,sizeof(Totals)),"Ordered AO physical regrouping differs");
            check(regrouped.dispatches==old.dispatches&&regrouped.seed_planning_draws==old.seed_planning_draws&&
                  regrouped.seed_shortcuts==old.seed_shortcuts,"Ordered AO regrouping changed work/dispatches");
            check(!std::memcmp(input.data(),buffer.contents,n*sizeof(ExactSurface)),"Ordered AO regrouping mutated input");
        }
        auto legacySeeded=RunNativeScreenAO(m,p,buffer,1ull<<30,legacyParallel,[]{});
        check(!std::memcmp(legacySeeded.surface.contents,reference.data(),n*sizeof(ExactSurface))&&
              !std::memcmp(&legacySeeded.totals,&old.totals,sizeof(Totals)),"Legacy seeded control differs");
        for(unsigned repeat=0;repeat<3;++repeat){auto fast=RunNativeScreenAO(m,p,buffer,1ull<<30,parallel,[]{});
            check(std::memcmp(reference.data(),fast.surface.contents,n*sizeof(ExactSurface))==0,"Seeded AO changed packed surfaces");
            check(std::memcmp(&old.totals,&fast.totals,sizeof(Totals))==0,"Seeded AO changed semantic counts");
            check(fast.dispatches==legacySeeded.dispatches&&fast.seed_planning_draws==legacySeeded.seed_planning_draws&&
                fast.seed_shortcuts==legacySeeded.seed_shortcuts,"Physical grouping changed seed work/dispatches");
            check(std::memcmp(input.data(),buffer.contents,n*sizeof(ExactSurface))==0,"AO mutated input");
            check(fast.seed_planning_draws==(random_mode(mode)?fast.totals.rng_advances:0),"Planning work missing");
            shortcuts+=fast.seed_shortcuts;active+=fast.totals.changed;draws+=fast.seed_planning_draws;
        }
        reject([&]{RunNativeScreenAO(m,p,buffer,screen_scratch_bytes(p.params)-1,parallel,[]{});});
        reject([&]{unsigned calls=0;RunNativeScreenAO(m,p,buffer,1ull<<30,parallel,[&]{if(++calls==5)throw std::runtime_error("cancel");});});
        auto limited=p;limited.params.work_limit=1;
        reject([&]{RunNativeScreenAO(m,limited,buffer,1ull<<30,native_metal::ScreenAOExecutionPolicy(limited),[]{});});
        auto broken=p;broken.levels[0].reserved=1;
        reject([&]{RunNativeScreenAO(m,broken,buffer,1ull<<30,native_metal::ScreenAOExecutionPolicy(broken),[]{});});
        unsigned polls=0;bool mismatch=false;
        try{RunNativeScreenAO(m,limited,buffer,1ull<<30,parallel,[&]{++polls;});}
        catch(const std::invalid_argument& e){mismatch=std::string(e.what())=="Native screen AO execution policy input mismatch";}
        check(mismatch&&!polls,"Stale AO policy reached device");
        check(!std::memcmp(buffer.contents,input.data(),n*sizeof(ExactSurface)),"Stale AO policy mutated input");
    }
    {TMandHeader10 h{};h.Width=3;h.Height=3;h.dZoom=1;h.dFOVy=45;h.dZstart=-2;h.dZend=2;h.bCalc3D=1;
     h.bCalcAmbShadowAutomatic=9;h.sAmbShadowThreshold=1;h.SSAORcount=1;
     const auto prepared=PrepareNativeScreenAO(h,3,3,1);const auto p=prepared.params;const auto level=prepared.levels[0];
     for(unsigned fault=0;fault<4;++fault){
        ExactSurface surface[9]{};uint32_t depth[9]{};short angles[9*32]{};Record records[9]{};SeedRecord plans[9]{};
        for(unsigned i=0;i<9;++i){records[i].version=Version;records[i].algorithm=p.algorithm;plans[i]={1,0,0,0,0,i,level.index,level.repeat_index};}
        if(fault==0)plans[0].version=2;if(fault==1)plans[0].pixel=1;if(fault==2)plans[0].level=0;if(fault==3)plans[0].draws=1;
        auto a=m.buffer(sizeof(surface),surface),b=m.buffer(sizeof(depth),depth),c=m.buffer(sizeof(angles),angles),d=m.buffer(sizeof(records),records),e=m.buffer(sizeof(plans),plans);
        Launch launch{0,9,9,0};gpu::dispatchStage(m,m.pipeline(@"native_ao_seeded_pixels"),9,[&](id<MTLComputeCommandEncoder> enc){
            [enc setBuffer:a offset:0 atIndex:0];[enc setBuffer:b offset:0 atIndex:1];[enc setBuffer:c offset:0 atIndex:2];[enc setBuffer:d offset:0 atIndex:3];
            [enc setBytes:&p length:sizeof(p) atIndex:4];[enc setBytes:&level length:sizeof(level) atIndex:5];[enc setBytes:&launch length:sizeof(launch) atIndex:6];[enc setBuffer:e offset:0 atIndex:7];});
        check(static_cast<Record*>(d.contents)[0].fault!=0,"Malformed seed plan accepted on GPU");
     }
    }
    check(active>0&&(selected==2||(draws>0&&shortcuts>0)),"AO controls inactive");
    std::printf("{\"status\":\"PASS\",\"checks\":%u,\"active\":%llu,\"planning_draws\":%llu}\n",checks,(unsigned long long)active,(unsigned long long)draws);return 0;
}catch(const std::exception&e){std::fprintf(stderr,"AO seed failed: %s\n",e.what());return 1;}}}
