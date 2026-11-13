#include "render/native_metal/command_batch.h"
#include "render/native_metal/native_ao_dispatch.h"
#include "core/numeric_precision.h"
#include <cstdio>
#include <cstring>
#include <numeric>
#include <vector>
using namespace mb3d;
using namespace mb3d_native_ao;
using namespace mb3d::native_metal;
namespace gpu=mb3d::metal_exact::gpu;
unsigned checks=0;
void check(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
template<class F>void rejects(F f){bool failed=false;try{f();}catch(const std::exception&){failed=true;}check(failed,"Expected command rejection");}
NativeAOPreparation prepare(unsigned mode,unsigned width,unsigned height) {
    TMandHeader10 h{};h.Width=width;h.Height=height;h.dZoom=1;h.dFOVy=45;
    h.dZstart=-2;h.dZend=2;h.bCalc3D=1;h.sAmbShadowThreshold=1;
    h.bCalcAmbShadowAutomatic=1|((mode>=6?mode-4:mode)<<1);
    h.bPlanarOptic=mode>=6?2:0;h.SSAORcount=2;h.bSSAO24BorderMirrorSize=30;
    return PrepareNativeScreenAO(h,width+1,height,6);
}
struct Guarded {
    id<MTLBuffer> buffer;
    std::size_t bytes;
    Guarded(gpu::Metal& m,std::size_t n):buffer(m.buffer(n+128)),bytes(n){
        std::memset(buffer.contents,0xa5,n+128);std::memset(data(),0x7b,n);
    }
    void* data()const{return static_cast<unsigned char*>(buffer.contents)+64;}
    std::vector<unsigned char> snapshot()const {
        const auto* p=static_cast<const unsigned char*>(buffer.contents);
        for(unsigned i=0;i<64;++i)check(p[i]==0xa5&&p[64+bytes+i]==0xa5,"Command buffer canary");
        return {p,p+bytes+128};
    }
};
struct BeginBuffers {
    Guarded input,surface,depth,angles,records;
    BeginBuffers(gpu::Metal& m,const Params& p):input(m,p.output_count*sizeof(ExactSurface)),
        surface(m,p.output_count*sizeof(ExactSurface)),depth(m,p.output_count*4),
        angles(m,p.output_count*64),records(m,p.output_count*sizeof(Record)) {
        auto* v=static_cast<ExactSurface*>(input.data());
        for(unsigned i=0;i<p.output_count;++i){v[i]={};v[i].depth_packed=i%13?((i*12791)%8388352u)<<8:0x80000000u;v[i].amb_shadow=i*17%16000;}
    }
    std::vector<std::vector<unsigned char>> snapshot()const {
        return {input.snapshot(),surface.snapshot(),depth.snapshot(),angles.snapshot(),records.snapshot()};
    }
};
void encode_begin(NativeCommandBatch& batch,id<MTLComputePipelineState> pipeline,
                  const NativeAOPreparation& p,BeginBuffers& b,unsigned base,unsigned count,bool fail=false) {
    const Launch launch{base,count,p.params.output_count,0};
    batch.encode(pipeline,count,[&](id<MTLComputeCommandEncoder> e){
        [e setBuffer:b.input.buffer offset:64 atIndex:0];[e setBuffer:b.surface.buffer offset:64 atIndex:1];
        [e setBuffer:b.depth.buffer offset:64 atIndex:2];[e setBuffer:b.angles.buffer offset:64 atIndex:3];
        [e setBuffer:b.records.buffer offset:64 atIndex:4];[e setBytes:&p.params length:sizeof(p.params) atIndex:5];
        [e setBytes:&launch length:sizeof(launch) atIndex:6];[e setBytes:&p.levels[0] length:sizeof(Level) atIndex:7];
        if(fail)throw std::runtime_error("injected binding failure");
    });
}
void transport(gpu::Metal& m) {
    const auto p=prepare(2,37,17);const auto n=p.params.output_count;
    const auto pipeline=m.pipeline(@"native_ao_begin");
    std::vector<std::vector<unsigned char>> baseline;
    for(auto cap:{1u,2u,3u,8u})for(auto invocationCap:{256ull,1024ull})for(unsigned repeat=0;repeat<3;++repeat) {
        BeginBuffers b(m,p.params);const auto before=b.snapshot();std::vector<uint64_t> admitted;unsigned completed=0;
        m.observer={[&](uint64_t units){admitted.push_back(units);},[&]{++completed;}};
        const std::function<void()> poll=[]{};
        const auto s=RunNativeCommandSegment(m,poll,{cap,invocationCap},[&](NativeCommandBatch& batch){
            for(unsigned base=0;base<n;base+=128)encode_begin(batch,pipeline,p,b,base,std::min(128u,n-base));
        });
        const auto after=b.snapshot();if(baseline.empty())baseline=after;
        check(after==baseline,"Grouped complete begin buffers differ");check(after[0]==before[0],"Input modified");
        check(s.completed_dispatches==(n+127)/128&&s.completed_invocations==n,"Grouped work count");
        check(admitted.size()==s.completed_dispatches&&std::accumulate(admitted.begin(),admitted.end(),uint64_t(0))==n,"Admission count");
        unsigned expected=0,pending=0;uint64_t units=0;
        for(auto count:admitted){if(pending&&(pending==cap||units+count>invocationCap)){++expected;pending=0;units=0;}
            ++pending;units+=count;if(pending==cap||units==invocationCap){++expected;pending=0;units=0;}}
        if(pending)++expected;
        check(s.completed_commands==expected&&completed==expected,"Actual command completion count");
        check(std::isfinite(s.gpu_ms)&&s.gpu_ms>=s.max_command_ms&&s.max_command_ms>=0,"Whole-command timestamps");
        m.observer={};
    }
    m.observer={};
    for(unsigned failure=0;failure<8;++failure) {
        BeginBuffers b(m,p.params);const auto before=b.snapshot();unsigned admissions=0;bool cancel=false;
        std::function<void()> poll=[&]{if(cancel||(failure==7&&admissions==1))throw std::runtime_error("injected cancellation");};
        m.observer={[&](uint64_t){if(++admissions==(failure==0?1u:3u)&&(failure==0||failure==1||failure==5))throw std::runtime_error("injected work ceiling");},
                    [&]{if(failure==4)throw std::runtime_error("injected postcompletion cancellation");}};
        NativeCommandBatch batch(m,poll,{failure==5?2u:8u,1024});
        rejects([&]{
            for(unsigned base=0;base<n;base+=128){
                if(failure==6&&base==128){batch.encode(nil,1,[](id<MTLComputeCommandEncoder>){});continue;}
                encode_begin(batch,pipeline,p,b,base,std::min(128u,n-base),failure==2&&base==128);
            }
            if(failure==3)cancel=true;
            batch.flush();
        });
        const auto after=b.snapshot();
        check(after[0]==before[0],"Failed command mutated input");
        if(failure==4){check(after==baseline,"Postcompletion state was not retained");check(batch.statistics().completed_commands==1,"Completed failure statistics");}
        else if(failure==5){check(after!=before&&after!=baseline,"Prior flushed chunk was not observed");check(batch.statistics().completed_dispatches==2,"Prior completed dispatch count");}
        else {check(after==before,"Unsubmitted command executed");check(batch.statistics().completed_commands==0,"Unsubmitted counted completed");}
        cancel=false;m.observer={};
        rejects([&]{batch.flush();});
        rejects([&]{encode_begin(batch,pipeline,p,b,0,1);});
        check(b.snapshot()==after,"Failed graph could be reused");
    }
    m.observer={};
    const std::function<void()> poll=[]{};
    for(auto limits:{NativeCommandLimits{0,1024},NativeCommandLimits{9,1024},
                     NativeCommandLimits{1,0},NativeCommandLimits{1,65537}})
        rejects([&]{NativeCommandBatch invalid(m,poll,limits);});
    for(unsigned failure=0;failure<3;++failure) {
        NativeCommandBatch invalid(m,poll,{8,1024});
        rejects([&]{invalid.encode(pipeline,failure==0?0u:failure==1?1025u:1u,
            [](id<MTLComputeCommandEncoder>){},failure==2?unsigned(pipeline.maxTotalThreadsPerThreadgroup)+1:0);});
        check(invalid.statistics().completed_commands==0,"Invalid dispatch was submitted");
        rejects([&]{invalid.flush();});
    }
    // Drop the input's last external strong handle before submission and drain
    // its autorelease pool. Metal retains inputs until completion, not after
    // completion. Keep result handles so their produced bytes remain inspectable.
    __weak id<MTLBuffer> weakBuffers[5]{};
    id<MTLBuffer> retained[5]{};
    NativeCommandBatch lifetime(m,poll,{8,1024});
    @autoreleasepool {
        BeginBuffers b(m,p.params);
        weakBuffers[0]=b.input.buffer;weakBuffers[1]=b.surface.buffer;weakBuffers[2]=b.depth.buffer;
        weakBuffers[3]=b.angles.buffer;weakBuffers[4]=b.records.buffer;
        retained[1]=b.surface.buffer;retained[2]=b.depth.buffer;retained[3]=b.angles.buffer;retained[4]=b.records.buffer;
        for(unsigned base=0;base<n;base+=128)encode_begin(lifetime,pipeline,p,b,base,std::min(128u,n-base));
    }
    for(auto& weak:weakBuffers)check(weak!=nil,"Unsubmitted graph lost a bound resource");
    lifetime.flush();m.observer={};
    for(unsigned i=1;i<5;++i)check(retained[i].length==baseline[i].size()&&
        !std::memcmp(retained[i].contents,baseline[i].data(),baseline[i].size()),"Retained-resource complete state differs");
}
void stages(gpu::Metal& m) {
    using Mode=NativeImageCommandPolicy::Mode;using Group=ScreenAOExecutionPolicy::LaneGrouping;
    uint64_t saved=0,changed=0;
    for(unsigned mode:{0u,1u,2u,4u,9u})for(unsigned width:{37u,129u}) {
        const auto p=prepare(mode,width,17);const auto n=p.params.output_count;
        std::vector<ExactSurface> input(n);for(unsigned i=0;i<n;++i){input[i].depth_packed=i%13?((i*12791)%8388352u)<<8:0x80000000u;input[i].amb_shadow=i*17%16000;}
        auto buffer=m.buffer(n*sizeof(ExactSurface),input.data());
        const ScreenAOExecutionPolicy serial(p,true,Group::LegacySimd,NativeImageCommandPolicy(Mode::Serial));
        const auto old=RunNativeScreenAO(m,p,buffer,1ull<<30,serial,[]{});
        std::vector<ExactSurface> reference(n);std::memcpy(reference.data(),old.surface.contents,n*sizeof(ExactSurface));
        check(old.commands==old.dispatches,"Serial command count differs");
        for(unsigned cap:{2u,3u,8u})for(unsigned repeat=0;repeat<3;++repeat) {
            const ScreenAOExecutionPolicy plan(p,true,Group::LegacySimd,NativeImageCommandPolicy(Mode::BoundedSlices,{cap,65536}));
            const auto now=RunNativeScreenAO(m,p,buffer,1ull<<30,plan,[]{});
            check(!std::memcmp(reference.data(),now.surface.contents,n*sizeof(ExactSurface)),"Segmented AO RGB/depth surface changed");
            check(!std::memcmp(&old.totals,&now.totals,sizeof(Totals)),"Segmented AO work changed");
            check(now.dispatches==old.dispatches&&now.commands<=old.commands,"Logical dispatch or command count");
            check(now.seed_planning_draws==old.seed_planning_draws&&now.seed_shortcuts==old.seed_shortcuts,"Segment changed RNG planning");
            check(!std::memcmp(input.data(),buffer.contents,n*sizeof(ExactSurface)),"Segmented AO input changed");
            saved+=old.commands-now.commands;changed+=now.totals.changed;
        }
    }
    // HiQ's16384-pixel slices exercise the invocation cap before encoder cap8.
    const auto p=prepare(2,257,257);const auto n=p.params.output_count;
    std::vector<ExactSurface> input(n);for(unsigned i=0;i<n;++i)input[i].depth_packed=((i*12791)%8388352u)<<8;
    auto buffer=m.buffer(n*sizeof(ExactSurface),input.data());
    const auto old=RunNativeScreenAO(m,p,buffer,1ull<<30,ScreenAOExecutionPolicy(p,true,Group::LegacySimd,NativeImageCommandPolicy(Mode::Serial)),[]{});
    std::vector<ExactSurface> reference(n);std::memcpy(reference.data(),old.surface.contents,n*sizeof(ExactSurface));
    for(auto limit:{16384ull,32768ull,65536ull})for(unsigned repeat=0;repeat<3;++repeat) {
        const auto now=RunNativeScreenAO(m,p,buffer,1ull<<30,ScreenAOExecutionPolicy(p,true,Group::LegacySimd,NativeImageCommandPolicy(Mode::BoundedSlices,{8,limit})),[]{});
        check(!std::memcmp(reference.data(),now.surface.contents,n*sizeof(ExactSurface))&&!std::memcmp(&old.totals,&now.totals,sizeof(Totals)),"HiQ capped segments changed state");
        check(now.dispatches==old.dispatches&&now.commands<=old.commands,"HiQ capped segment counts");saved+=old.commands-now.commands;
    }
    check(saved>0&&changed>0,"Command optimization/effect controls inactive");
}
int main(int argc,char** argv){@autoreleasepool{try{
    if(argc!=2)throw std::runtime_error("usage: command_test NATIVE_LIBRARY");
    ScopedNumericPrecision precision(PrecisionMode::Native);gpu::Metal m(argv[1],3,8,0,false,true);
    transport(m);stages(m);
    std::printf("{\"status\":\"PASS\",\"checks\":%u}\n",checks);return 0;
}catch(const std::exception& e){std::fprintf(stderr,"Native command tests failed: %s\n",e.what());return 1;}}}
