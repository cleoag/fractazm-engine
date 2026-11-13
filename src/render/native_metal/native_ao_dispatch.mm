#include "native_ao_dispatch.h"
#include "seed_scan_dispatch.h"
#include "native_camera_abi.h"
#include "pixel_independence.h"
#include "command_batch.h"
#include <array>
#include <algorithm>
#include <cstring>
#include <map>
#include <string>

namespace mb3d {
namespace {
using namespace mb3d_native_ao;
using metal_exact::gpu::Metal;
struct Dispatch {
    Metal& metal;NativeAOResult& result;const std::function<void()>& poll;
    std::map<std::string,id<MTLComputePipelineState>> pipelines;
    U32 slice_size=1024;
    const native_metal::NativeImageCommandPolicy* command_policy=nullptr;
    native_metal::NativeCommandBatch* active_segment=nullptr;
    void run(const char* name,U32 count,const std::function<void(id<MTLComputeCommandEncoder>)>& bind,U32 group_width=0) {
        require_ao(bool(poll)&&count>0,"AO cancellation callback/dispatch count");poll();
        auto found=pipelines.find(name);
        if(found==pipelines.end())found=pipelines.emplace(name,metal.pipeline(metal_exact::gpu::ns(name))).first;
        if(active_segment) {
            active_segment->encode(found->second,count,bind,group_width);
            return;
        }
        const double ms=metal_exact::gpu::dispatchStage(metal,found->second,count,bind,1,group_width);
        require_ao(std::isfinite(ms)&&ms>=0,"AO GPU timestamp");
        result.gpu_ms+=ms;result.max_command_ms=std::max(result.max_command_ms,ms);
        ++result.dispatches;++result.commands;poll();
    }
    void slices(U32 count,U32 allocation,const std::function<void(const Launch&)>& run,
                native_metal::NativeImagePhase phase=native_metal::NativeImagePhase::Standalone) {
        require_ao(active_segment==nullptr,"Nested Native AO command segment");
        const auto body=[&]{
            for(U32 base=0;base<count;base+=slice_size)
                run(Launch{base,std::min(slice_size,count-base),allocation,0});
        };
        // Sampling/threshold scans, all RNG and median continuations and
        // readbacks remain standalone. A segment never crosses an axis/level.
        if(!command_policy||!command_policy->grouped(phase)){body();return;}
        const auto statistics=native_metal::RunNativeCommandSegment(metal,poll,command_policy->limits(),
            [&](native_metal::NativeCommandBatch& segment){
                active_segment=&segment;
                try{body();}catch(...){active_segment=nullptr;throw;}
                active_segment=nullptr;
            });
        result.dispatches+=statistics.completed_dispatches;
        result.commands+=statistics.completed_commands;
        result.gpu_ms+=statistics.gpu_ms;
        result.max_command_ms=std::max(result.max_command_ms,statistics.max_command_ms);
    }
};
template<class T>std::span<const T> view(id<MTLBuffer> buffer,std::size_t count) {
    require_ao(buffer!=nil&&buffer.length==count*sizeof(T),"AO actual buffer length");
    return {static_cast<const T*>(buffer.contents),count};
}
id<MTLBuffer> zero_buffer(Metal&m,std::uint64_t count,std::uint64_t stride,std::uint64_t budget) {
    auto buffer=m.buffer(checked_ao_bytes(count,stride,budget));std::memset(buffer.contents,0,buffer.length);return buffer;
}
void marker(Dispatch& d) {
    require_ao(d.metal.arithmeticProfile==std::array<U32,3>{2,4,32},"AO requires actual Native Metal library");
    auto screen=zero_buffer(d.metal,9,sizeof(U32),36),de=zero_buffer(d.metal,5,sizeof(U32),20);
    d.run("native_ao_abi",1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:screen offset:0 atIndex:0];});
    d.run("native_ao_de_abi",1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:de offset:0 atIndex:0];});
    validate_markers(d.metal.arithmeticProfile,view<U32>(screen,9),view<U32>(de,5));
}
}

NativeAOResult RunNativeScreenAO(Metal&m,const NativeAOPreparation& preparation,id<MTLBuffer> input,
    std::uint64_t budget,const native_metal::ScreenAOExecutionPolicy& execution,const std::function<void()>& poll) {
    using namespace mb3d_native_ao;const auto&p=preparation.params;
    require_ao(valid(p)&&p.algorithm<=9,"AO screen preparation");
    execution.validate(preparation);
    const bool parallel_random=execution.parallel_random();
    const bool certified=execution.uses_certificates();
    const bool chunkPrefix=execution.uses_chunk_prefix();
    require_ao(screen_scratch_bytes(p,certified,chunkPrefix)<=budget,"AO total scratch budget");
    const U32 n=p.width*p.height,out_n=p.output_count;
    const auto original=view<ExactSurface>(input,out_n);
    NativeAOResult result;Dispatch d{m,result,poll,{}};marker(d);
    d.command_policy=&execution.commands();
    // Non-random HiQ writes only its pixel's surface/angles/record and reads
    // immutable depth. Wavelet axes use distinct input/output buffers; complete
    // each axis/level before the next. Larger independent batches amortize
    // command submission without changing sample order or per-pixel limits.
    // Threshold median and random logical lanes keep their existing bounds.
    d.slice_size=execution.raster_slice_batch();
    result.surface=zero_buffer(m,out_n,sizeof(ExactSurface),budget);
    auto records=zero_buffer(m,out_n,sizeof(Record),budget);
    if(hiq(p.algorithm)){
        require_ao(preparation.levels.size()==std::size_t(p.levels)*p.repeats,"AO missing prepared levels/repeats");
        auto depth=zero_buffer(m,n,sizeof(U32),budget),scratch=zero_buffer(m,n,sizeof(U32),budget);
        auto angles=zero_buffer(m,std::uint64_t(n)*32,sizeof(std::int16_t),budget),faults=zero_buffer(m,n,sizeof(U32),budget);
        auto lanes=zero_buffer(m,p.logical_lanes,sizeof(LaneState),budget);
        id<MTLBuffer> seedPlans=nil;
        id<MTLBuffer> bundle=nil,certificates=nil,classified=nil,usage=nil,certificateFault=nil;
        const U32 certificateBytes=n*32u,forceUnknown=execution.force_unknown();
        namespace chunk=mb3d_native_ao_chunk;
        const U32 chunkOwners=((p.height+p.logical_lanes-1u)/p.logical_lanes*p.width+127u)/128u;
        const U32 chunkTotal=chunkOwners*p.logical_lanes;
        id<MTLBuffer> chunkPixels=nil,chunkRecords=nil;
        if(chunkPrefix){
            auto abi=zero_buffer(m,4,sizeof(U32),16);
            d.run("native_ao_chunk_abi",1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:abi offset:0 atIndex:0];});
            const U32 expected[]={chunk::Version,sizeof(chunk::Pixel),sizeof(chunk::Chunk),chunk::Width};
            require_ao(std::memcmp(abi.contents,expected,sizeof(expected))==0,"AO chunk ABI mismatch");
            chunkPixels=zero_buffer(m,n,sizeof(chunk::Pixel),budget);
            chunkRecords=zero_buffer(m,chunkTotal,sizeof(chunk::Chunk),budget);
        }
        if(random_mode(p.algorithm)&&parallel_random){
            auto abi=zero_buffer(m,4,sizeof(U32),16);
            d.run("native_ao_seed_abi",1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:abi offset:0 atIndex:0];});
            const U32 expected[]={1,sizeof(SeedRecord),sizeof(LaneState),sizeof(Launch)};
            require_ao(std::memcmp(abi.contents,expected,sizeof(expected))==0,"AO seed ABI mismatch");
            seedPlans=zero_buffer(m,n,sizeof(SeedRecord),budget);
        }
        if(certified){
            auto baseABI=zero_buffer(m,4,sizeof(U32),16),certificateABI=zero_buffer(m,4,sizeof(U32),16);
            d.run("native_ao_reuse_abi",1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:baseABI offset:0 atIndex:0];});
            d.run("native_ao_certificate_abi",1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:certificateABI offset:0 atIndex:0];});
            const U32 expectedBase[]={1,sizeof(mb3d_native_ao_reuse::BaseSample),sizeof(mb3d_native_ao_reuse::BaseTable),192};
            const U32 expectedCertificate[]={1,32,4,4};
            require_ao(std::memcmp(baseABI.contents,expectedBase,sizeof(expectedBase))==0&&
                std::memcmp(certificateABI.contents,expectedCertificate,sizeof(expectedCertificate))==0,"AO certificate ABI mismatch");
            bundle=zero_buffer(m,1,sizeof(mb3d_native_ao_reuse::BaseTable),budget);
            certificates=zero_buffer(m,n,32,budget);classified=zero_buffer(m,n,sizeof(U32),budget);
            usage=zero_buffer(m,n,sizeof(U32),budget);certificateFault=zero_buffer(m,1,sizeof(U32),budget);
            result.certificate_version=1;
        }
        for(U32 repeat=0;repeat<p.repeats;++repeat){
            const auto&first=preparation.levels[std::size_t(repeat)*p.levels];
            d.slices(out_n,out_n,[&](const Launch&launch){d.run("native_ao_begin",launch.count,[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:input offset:0 atIndex:0];[e setBuffer:result.surface offset:0 atIndex:1];
                [e setBuffer:depth offset:0 atIndex:2];[e setBuffer:angles offset:0 atIndex:3];[e setBuffer:records offset:0 atIndex:4];
            [e setBytes:&p length:sizeof(p) atIndex:5];[e setBytes:&launch length:sizeof(launch) atIndex:6];[e setBytes:&first length:sizeof(first) atIndex:7];});},native_metal::NativeImagePhase::ScreenBegin);
            for(U32 index=1;index<=p.levels;++index){
                const auto&level=preparation.levels[std::size_t(repeat)*p.levels+index-1];
                require_ao(valid(p,level)&&level.index==index&&level.repeat_index==repeat,"AO prepared level order");
                if(certified){
                    d.run("native_ao_reuse_table",32,[&](id<MTLComputeCommandEncoder> e){
                        [e setBytes:&p length:sizeof(p) atIndex:0];[e setBytes:&level length:sizeof(level) atIndex:1];
                        [e setBuffer:bundle offset:0 atIndex:2];});
                    validate_certificate_table(view<mb3d_native_ao_reuse::BaseTable>(bundle,1)[0],p,level);
                    // All previous readers have completed. These buffers are
                    // render-local and immutable for the ensuing seed phase.
                    std::memset(certificateFault.contents,0,sizeof(U32));
                    const U32 slice=execution.certificate_pixel_batch();
                    for(U32 base=0;base<n;base+=slice){const Launch launch{base,std::min(slice,n-base),n,0};
                        d.run("native_ao_certificate_prepare",launch.count,[&](id<MTLComputeCommandEncoder> e){
                            [e setBuffer:bundle offset:0 atIndex:0];[e setBuffer:certificates offset:0 atIndex:1];
                            [e setBytes:&launch length:sizeof(launch) atIndex:2];[e setBytes:&certificateBytes length:4 atIndex:3];
                            [e setBuffer:classified offset:0 atIndex:4];[e setBuffer:certificateFault offset:0 atIndex:5];
                            [e setBytes:&n length:4 atIndex:6];});
                    }
                    require_ao(view<U32>(certificateFault,1)[0]==0,"AO certificate producer fault");
                    const auto counts=view<U32>(classified,n);
                    for(U32 pixel=0;pixel<n;++pixel){if(!(pixel&4095u))poll();
                        result.certificate_known+=certificate_count(counts[pixel],level);}
                    ++result.certificate_levels;result.certificate_pixels+=n;
                    if(chunkPrefix){
                        for(U32 base=0;base<chunkTotal;base+=1024u){
                            const chunk::Run run{chunkOwners,chunkTotal,base,std::min(1024u,chunkTotal-base)};
                            d.run("native_ao_chunk_prepare",run.count*chunk::Width,[&](id<MTLComputeCommandEncoder> e){
                                [e setBuffer:bundle offset:0 atIndex:0];[e setBuffer:records offset:0 atIndex:1];
                                [e setBuffer:certificates offset:0 atIndex:2];[e setBuffer:chunkPixels offset:0 atIndex:3];
                                [e setBuffer:chunkRecords offset:0 atIndex:4];[e setBytes:&run length:sizeof(run) atIndex:5];
                                [e setBuffer:certificateFault offset:0 atIndex:6];
                            },chunk::Width);
                        }
                        require_ao(view<U32>(certificateFault,1)[0]==0,"AO chunk producer fault");
                    }
                }
                if(random_mode(p.algorithm)&&parallel_random){
                    const U32 maximum_pixels=((p.height+p.logical_lanes-1)/p.logical_lanes)*p.width;
                    const U32 quota=execution.rng().continuation;
                    const U32 batches=(maximum_pixels+quota-1)/quota;
                    const U32 chunks=execution.seed_continuation_chunks();
                    for(U32 batch=0;batch<batches;batch+=chunks){const U32 reset=batch==0;
                        const U32 count=std::min(chunks,batches-batch);
                        const chunk::Run chunkRun{chunkOwners,chunkTotal,batch,count};
                        d.run(chunkPrefix?"native_ao_chunk_lanes":chunks>1?"native_ao_seed_lanes_collective":certified?"native_ao_seed_lanes_certified":"native_ao_seed_lanes",p.logical_lanes,[&](id<MTLComputeCommandEncoder> e){
                            [e setBuffer:records offset:0 atIndex:0];
                            if(!certified){[e setBytes:&p length:sizeof(p) atIndex:1];[e setBytes:&level length:sizeof(level) atIndex:2];}
                            [e setBuffer:lanes offset:0 atIndex:3];[e setBytes:&reset length:4 atIndex:4];[e setBytes:&n length:4 atIndex:5];
                            [e setBytes:&p.logical_lanes length:4 atIndex:6];[e setBuffer:seedPlans offset:0 atIndex:7];
                            if(certified){[e setBuffer:bundle offset:0 atIndex:8];[e setBuffer:certificates offset:0 atIndex:9];
                                [e setBytes:&certificateBytes length:4 atIndex:10];[e setBuffer:usage offset:0 atIndex:11];
                                [e setBytes:&forceUnknown length:4 atIndex:12];[e setBytes:&n length:4 atIndex:13];}
                            if(chunks>1)[e setBytes:&count length:4 atIndex:14];
                            if(chunkPrefix){[e setBuffer:chunkRecords offset:0 atIndex:15];[e setBytes:&chunkRun length:sizeof(chunkRun) atIndex:16];}
                        },chunks>1?p.logical_lanes:execution.rng().group_width);
                        // Publish precisely the known chunks that executed before the
                        // original all-owner fault frontier, even if this batch failed.
                        if(chunkPrefix){
                            d.run("native_ao_chunk_scatter",p.logical_lanes*count*chunk::Width,[&](id<MTLComputeCommandEncoder> e){
                                [e setBuffer:bundle offset:0 atIndex:0];[e setBuffer:chunkPixels offset:0 atIndex:1];
                                [e setBuffer:chunkRecords offset:0 atIndex:2];[e setBuffer:seedPlans offset:0 atIndex:3];
                                [e setBuffer:usage offset:0 atIndex:4];[e setBytes:&chunkRun length:sizeof(chunkRun) atIndex:5];
                                [e setBuffer:certificateFault offset:0 atIndex:6];
                            });
                            require_ao(view<U32>(certificateFault,1)[0]==0,"AO chunk scatter fault");
                        }
                        for(const auto& lane:view<LaneState>(lanes,p.logical_lanes))require_ao(!lane.fault,"AO seed continuation fault");
                    }
                    validate_random_lanes(p,level,view<LaneState>(lanes,p.logical_lanes));
                    const auto planned=view<SeedRecord>(seedPlans,n);
                    for(U32 pixel=0;pixel<n;++pixel){const auto& plan=planned[pixel];
                        if(!(pixel&4095u))poll();
                        require_ao(plan.version==1&&!plan.fault&&plan.shortcut<=1&&!plan.reserved[0]&&!plan.reserved[1]&&!plan.reserved[2]&&plan.pixel==pixel&&plan.level==level.index&&plan.repeat==level.repeat_index,"AO seed plan record");
                        result.seed_planning_draws+=plan.draws;result.seed_shortcuts+=plan.shortcut;}
                    if(certified){
                        const auto counts=view<U32>(classified,n),uses=view<U32>(usage,n);
                        const auto current=view<Record>(records,out_n);
                        for(U32 pixel=0;pixel<n;++pixel){if(!(pixel&4095u))poll();
                            result.certificate_used+=validate_certificate_usage(uses[pixel],certificate_count(counts[pixel],level),
                                level,!current[pixel].active,planned[pixel].shortcut,forceUnknown);}
                    }
                    const U32 pixelsPerCommand=execution.seeded_pixel_batch();
                    for(U32 base=0;base<n;base+=pixelsPerCommand){const Launch launch{base,std::min(pixelsPerCommand,n-base),out_n,0};
                        d.run("native_ao_seeded_pixels",launch.count,[&](id<MTLComputeCommandEncoder> e){
                            [e setBuffer:result.surface offset:0 atIndex:0];[e setBuffer:depth offset:0 atIndex:1];[e setBuffer:angles offset:0 atIndex:2];[e setBuffer:records offset:0 atIndex:3];
                            [e setBytes:&p length:sizeof(p) atIndex:4];[e setBytes:&level length:sizeof(level) atIndex:5];[e setBytes:&launch length:sizeof(launch) atIndex:6];[e setBuffer:seedPlans offset:0 atIndex:7];});
                    }
                }else
                if(random_mode(p.algorithm)){
                    const U32 maximum_pixels=((p.height+p.logical_lanes-1)/p.logical_lanes)*p.width;
                    const U32 quota=execution.rng().continuation;
                    for(U32 batch=0;batch<(maximum_pixels+quota-1)/quota;++batch){const U32 reset=batch==0;
                        d.run("native_ao_random_lanes",p.logical_lanes,[&](id<MTLComputeCommandEncoder> e){
                            [e setBuffer:result.surface offset:0 atIndex:0];[e setBuffer:depth offset:0 atIndex:1];[e setBuffer:angles offset:0 atIndex:2];[e setBuffer:records offset:0 atIndex:3];
                            [e setBytes:&p length:sizeof(p) atIndex:4];[e setBytes:&level length:sizeof(level) atIndex:5];[e setBuffer:lanes offset:0 atIndex:6];
                            [e setBytes:&reset length:sizeof(reset) atIndex:7];[e setBytes:&out_n length:sizeof(out_n) atIndex:8];[e setBytes:&p.logical_lanes length:sizeof(U32) atIndex:9];},execution.rng().group_width);
                        for(const auto&lane:view<LaneState>(lanes,p.logical_lanes))require_ao(!lane.fault,"AO random continuation fault");
                    }
                    validate_random_lanes(p,level,view<LaneState>(lanes,p.logical_lanes));
                }else d.slices(n,out_n,[&](const Launch&launch){d.run("native_ao_hiq_sample",launch.count,[&](id<MTLComputeCommandEncoder> e){
                    [e setBuffer:result.surface offset:0 atIndex:0];[e setBuffer:depth offset:0 atIndex:1];[e setBuffer:angles offset:0 atIndex:2];[e setBuffer:records offset:0 atIndex:3];
                    [e setBytes:&p length:sizeof(p) atIndex:4];[e setBytes:&level length:sizeof(level) atIndex:5];[e setBytes:&launch length:sizeof(launch) atIndex:6];});});
                if(index<p.levels){const U32 step=1u<<index;
                    for(U32 axis=0;axis<2;++axis)d.slices(n,out_n,[&](const Launch&launch){d.run("native_ao_wavelet",launch.count,[&](id<MTLComputeCommandEncoder> e){
                        [e setBuffer:axis?scratch:depth offset:0 atIndex:0];[e setBuffer:axis?depth:scratch offset:0 atIndex:1];[e setBuffer:faults offset:0 atIndex:2];
                        [e setBytes:&p length:sizeof(p) atIndex:3];[e setBytes:&step length:sizeof(step) atIndex:4];[e setBytes:&axis length:sizeof(axis) atIndex:5];[e setBytes:&launch length:sizeof(launch) atIndex:6];});},native_metal::NativeImagePhase::ScreenWavelet);
                }
            }
        }
        validate_wavelet_faults(view<U32>(faults,n),n);
    }else{
        require_ao(preparation.levels.empty()&&p.levels<=8,"AO threshold preparation");
        auto levels=zero_buffer(m,std::uint64_t(n)*p.levels,sizeof(std::uint16_t),budget),scratch=zero_buffer(m,n,sizeof(std::uint16_t),budget);
        auto prep=zero_buffer(m,out_n,sizeof(PreprocessRecord),budget),ranges=zero_buffer(m,p.height,8,budget);
        auto range=zero_buffer(m,1,sizeof(Range15),budget),median=zero_buffer(m,1,sizeof(MedianState),budget);
        d.run("native_ao15_row_range",p.height,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:input offset:0 atIndex:0];[e setBuffer:ranges offset:0 atIndex:1];
            [e setBytes:&p length:sizeof(p) atIndex:2];[e setBytes:&out_n length:sizeof(out_n) atIndex:3];[e setBytes:&p.height length:sizeof(U32) atIndex:4];});
        d.run("native_ao15_range",1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:ranges offset:0 atIndex:0];[e setBuffer:range offset:0 atIndex:1];
            [e setBytes:&p length:sizeof(p) atIndex:2];[e setBytes:&p.height length:sizeof(U32) atIndex:3];});
        require_ao(!view<Range15>(range,1)[0].fault,"AO threshold range fault");
        d.slices(out_n,out_n,[&](const Launch&launch){d.run("native_ao15_encode",launch.count,[&](id<MTLComputeCommandEncoder> e){
            [e setBuffer:input offset:0 atIndex:0];[e setBuffer:result.surface offset:0 atIndex:1];[e setBuffer:levels offset:0 atIndex:2];[e setBuffer:records offset:0 atIndex:3];[e setBuffer:prep offset:0 atIndex:4];
            [e setBytes:&p length:sizeof(p) atIndex:5];[e setBuffer:range offset:0 atIndex:6];[e setBytes:&launch length:sizeof(launch) atIndex:7];});},native_metal::NativeImagePhase::ThresholdEncode);
        const U32 medianQuota=execution.median().continuation;
        for(U32 batch=0;batch<(n+medianQuota-1)/medianQuota;++batch){const U32 reset=batch==0;
            d.run("native_ao15_median",1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:levels offset:0 atIndex:0];[e setBuffer:median offset:0 atIndex:1];
                [e setBytes:&p length:sizeof(p) atIndex:2];[e setBytes:&n length:sizeof(n) atIndex:3];[e setBytes:&reset length:sizeof(reset) atIndex:4];});
            require_ao(!view<MedianState>(median,1)[0].fault,"AO median continuation fault");
        }
        for(U32 level=2;level<=p.levels;++level){const U32 step=1u<<(level-2);
            for(U32 axis=0;axis<2;++axis)d.slices(n,out_n,[&](const Launch&launch){d.run("native_ao15_wavelet",launch.count,[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:axis?scratch:levels offset:axis?0:std::size_t(level-2)*n*2 atIndex:0];
                [e setBuffer:axis?levels:scratch offset:axis?std::size_t(level-1)*n*2:0 atIndex:1];[e setBuffer:prep offset:0 atIndex:2];
                [e setBytes:&p length:sizeof(p) atIndex:3];[e setBytes:&step length:sizeof(step) atIndex:4];[e setBytes:&axis length:sizeof(axis) atIndex:5];[e setBytes:&launch length:sizeof(launch) atIndex:6];});},native_metal::NativeImagePhase::ThresholdWavelet);
        }
        validate_threshold_preprocess(p,view<Range15>(range,1)[0],view<MedianState>(median,1)[0],view<PreprocessRecord>(prep,out_n));
        const U32 words=n*p.levels;
        d.slices(n,out_n,[&](const Launch&launch){d.run("native_ao15_scan",launch.count,[&](id<MTLComputeCommandEncoder> e){
            [e setBuffer:result.surface offset:0 atIndex:0];[e setBuffer:levels offset:0 atIndex:1];[e setBuffer:records offset:0 atIndex:2];[e setBytes:&p length:sizeof(p) atIndex:3];
            [e setBuffer:range offset:0 atIndex:4];[e setBytes:&launch length:sizeof(launch) atIndex:5];[e setBytes:&words length:sizeof(words) atIndex:6];});});
    }
    result.totals=validate_screen(p,original,view<ExactSurface>(result.surface,out_n),view<Record>(records,out_n));
    if(random_mode(p.algorithm)&&parallel_random)require_ao(result.seed_planning_draws==result.totals.rng_advances,"AO total seed planning mismatch");
    return result;
}

NativeAOResult RunNativeDistanceAO(Metal&m,const mb3d_native_ao::DEParams&p,const ExactScene&scene,
    const mb3d_native::NativeFormulaProgram&program,const mb3d_native::NativeFormulaProgram*second,id<MTLBuffer>camera,id<MTLBuffer>input,
    id<MTLBuffer>maps,id<MTLBuffer>map_pixels,U32 map_count,U32 map_elements,
    std::uint64_t budget,const native_metal::DistanceAOExecutionPolicy& execution,const std::function<void()>&poll) {
    using namespace mb3d_native_ao;using namespace mb3d_native;
    require_ao(valid_de(p)&&scene.width==p.width&&scene.height==p.height,"DE AO dimensions");
    execution.validate(program,p,second);
    require_ao(de_scratch_bytes(p)<=budget,"DE AO total scratch budget");
    const U32 n=p.width*p.height;const auto original=view<ExactSurface>(input,n);view<mb3d_camera::Ray>(camera,n);
    require_ao(maps&&map_pixels&&map_count<=256&&maps.length>=std::uint64_t(map_count)*sizeof(NativeMapDescriptor)&&
        map_pixels.length>=std::uint64_t(map_elements)*sizeof(float),"DE AO map allocation bounds");
    NativeAOResult result;Dispatch d{m,result,poll,{}};marker(d);
    result.surface=zero_buffer(m,n,sizeof(ExactSurface),budget);auto records=zero_buffer(m,n,sizeof(DERecord),budget);
    // Random steps and unaudited formulas retain the original persistent tasks.
    const bool independent=execution.independent();
    auto tasks=independent?nil:zero_buffer(m,p.task_count,sizeof(DETaskState),budget);
    // A DE combination's second part follows the first in the same buffer.
    require_ao(bool(second)==bool(program.formula_type)&&(!second||second->formula_type==program.formula_type),"DE AO combination program");
    const std::array<NativeFormulaProgram,2> programs{program,second?*second:NativeFormulaProgram{}};
    auto descriptor=m.buffer((second?2:1)*sizeof(program),programs.data());const std::array<U32,4>allocation={n,p.task_count,map_count,map_elements};
    if(independent) {
        auto identity=zero_buffer(m,5,sizeof(U32),20);
        d.run("native_ao_de_pixels_abi",1,[&](id<MTLComputeCommandEncoder>e){[e setBuffer:identity offset:0 atIndex:0];});
        const U32 expected[]={5,sizeof(Launch),sizeof(DEParams),sizeof(DERecord),kDEPixelBatch};
        const U32 proof=execution.pixel_proof(),seeded=p.first_step_random;
        require_ao(std::memcmp(identity.contents,expected,sizeof(expected))==0,"DE AO pixel ABI mismatch");
        // CalcAO_DE's random first step draws once per ray. Each task (task_rows
        // contiguous rows) restarts at initial_seed; an active pixel draws its
        // rays, a background pixel none. Each pixel enters with its task's seed
        // after all earlier pixels' draws; each task's exit follows the pixels.
        auto seeds=seeded?zero_buffer(m,n+p.task_count,sizeof(U32),budget):zero_buffer(m,1,sizeof(U32),budget);
        if(seeded){
            auto fault=zero_buffer(m,1,sizeof(U32),budget);
            const auto before=result.commands;
            for(U32 first=0;first<n;first+=kDEPixelBatch){
                const Launch launch{first,std::min<U32>(kDEPixelBatch,n-first),n,0};
                d.run("native_seed_de_weights",launch.count,[&](id<MTLComputeCommandEncoder> e){
                    [e setBuffer:input offset:0 atIndex:0];[e setBuffer:seeds offset:0 atIndex:1];
                    [e setBytes:&p length:sizeof(p) atIndex:2];[e setBytes:&launch length:sizeof(launch) atIndex:3];
                    [e setBuffer:fault offset:0 atIndex:4];});
                require_ao(*static_cast<const U32*>(fault.contents)==0,"DE AO seed weight fault");
            }
            const auto scanPlan=native_metal::PlanSeedScan(p.width,p.height,p.task_count,
                mb3d_seed_scan::Contiguous,p.task_rows,mb3d_seed_scan::DrawCounts,p.initial_seed);
            const auto scanned=native_metal::RunSeedScan(m,scanPlan,seeds,scanPlan.scratch_bytes,poll);
            result.seed_planning_draws=scanned.draws;result.gpu_ms+=scanned.gpu_ms;
            result.max_command_ms=std::max(result.max_command_ms,scanned.max_command_ms);
            result.dispatches+=scanned.commands;result.commands+=scanned.commands;
            result.seed_generation_commands+=result.commands-before;
        }
        const U32 batch=execution.entry().dispatch_batch;
        for(U32 first=0;first<n;first+=batch) {
            const Launch launch{first,std::min(batch,n-first),n,0};
            d.run("native_ao_de_pixels",launch.count,[&](id<MTLComputeCommandEncoder>e){
                [e setBytes:&scene length:sizeof(scene) atIndex:0];[e setBuffer:descriptor offset:0 atIndex:1];[e setBytes:&p length:sizeof(p) atIndex:2];
                [e setBuffer:camera offset:0 atIndex:3];[e setBuffer:input offset:0 atIndex:4];[e setBuffer:result.surface offset:0 atIndex:5];[e setBuffer:records offset:0 atIndex:6];
                [e setBuffer:maps offset:0 atIndex:8];[e setBuffer:map_pixels offset:0 atIndex:9];[e setBytes:allocation.data() length:sizeof(allocation) atIndex:10];[e setBytes:&launch length:sizeof(launch) atIndex:11];
                [e setBytes:&proof length:sizeof(proof) atIndex:12];[e setBuffer:seeds offset:0 atIndex:13];
                [e setBytes:&seeded length:sizeof(seeded) atIndex:14];});
        }
    } else for(U32 ordinal=0;ordinal<p.task_rows*p.width;++ordinal){const U32 reset=ordinal==0;
        d.run("native_ao_de_pixel",p.task_count,[&](id<MTLComputeCommandEncoder>e){
            [e setBytes:&scene length:sizeof(scene) atIndex:0];[e setBuffer:descriptor offset:0 atIndex:1];[e setBytes:&p length:sizeof(p) atIndex:2];
            [e setBuffer:camera offset:0 atIndex:3];[e setBuffer:input offset:0 atIndex:4];[e setBuffer:result.surface offset:0 atIndex:5];[e setBuffer:records offset:0 atIndex:6];[e setBuffer:tasks offset:0 atIndex:7];
            [e setBuffer:maps offset:0 atIndex:8];[e setBuffer:map_pixels offset:0 atIndex:9];[e setBytes:allocation.data() length:sizeof(allocation) atIndex:10];[e setBytes:&reset length:sizeof(reset) atIndex:11];});
        for(const auto&t:view<DETaskState>(tasks,p.task_count))require_ao(!t.fault,"DE AO persistent task fault");
    }
    // Only dIFS orbits count carried reads, and only the conditional contract
    // admits a dIFS program as pixels. A counted read voids that pass before
    // validation, since its fresh carried state may also fault where the
    // ordered tasks would not; they then compute the result from the
    // unchanged input. A fault without carried reads recurs in either order.
    if(execution.conditional_state()) {
        std::uint64_t reads=0,pixels=0;
        for(const auto&r:view<DERecord>(records,n)){reads+=r.difs_carried_reads;pixels+=r.difs_carried_reads!=0;}
        if(reads) {
            const native_metal::DistanceAOExecutionPolicy ordered(program,p,second,true);
            result.surface=nil;records=nil; // release before the ordered allocation
            auto rerun=RunNativeDistanceAO(m,p,scene,program,second,camera,input,maps,map_pixels,map_count,map_elements,budget,ordered,poll);
            rerun.difs_rerun_carried_reads=reads;rerun.difs_rerun_carried_pixels=pixels;
            rerun.seed_generation_commands+=result.seed_generation_commands;
            rerun.dispatches+=result.dispatches;rerun.commands+=result.commands;rerun.gpu_ms+=result.gpu_ms;
            rerun.max_command_ms=std::max(rerun.max_command_ms,result.max_command_ms);
            return rerun;
        }
    }
    result.totals=validate_de(p,original,view<ExactSurface>(result.surface,n),view<DERecord>(records,n),independent?std::span<const DETaskState>{}:view<DETaskState>(tasks,p.task_count),m.nativeSmoothingPolicy,independent);
    require_ao(!independent||!result.totals.difs_carried_reads,"DE AO carried read without a dIFS contract");
    require_ao(!independent||!p.first_step_random||result.seed_planning_draws==result.totals.rng_advances,"DE AO seed planning mismatch");
    result.difs_conditional_pixels=execution.conditional_state();
    return result;
}
}
