#include "dof_dispatch.h"
#include "comparison/metal_exact/shadow_abi.h"
#include "comparison/metal_exact/color_abi.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <stdexcept>
namespace mb3d::native_metal {
namespace gpu=metal_exact::gpu;
using namespace mb3d_dof;
DOFOutput RunNativeDOF(gpu::Metal& m,const Params& p,id<MTLBuffer> surface,id<MTLBuffer> input,
                      const DOFExecutionPolicy& execution,const std::function<void()>& poll){
    const auto& limits=execution.limits();execution.validate(p,limits);
    DOFOutput out;const auto& plan=execution.allocation();if(!p.version){out.rgb=input;return out;}
    const uint32_t n=plan.pixels,cap=plan.candidate_capacity,batch=limits.tile_batch;
    gpu::require(surface&&input&&surface.length>=uint64_t(n)*sizeof(ExactSurface)&&input.length>=uint64_t(n)*sizeof(ExactColorPixel),"Native DOF input allocation");
    const uint64_t bytes=plan.bytes();
    gpu::require(bytes<=uint64_t(1)<<30,"Native DOF workspace exceeds1GiB");
    auto src=m.buffer(plan.sources),order=m.buffer(plan.order);
    auto global=m.buffer(plan.global),pass=m.buffer(plan.pass),state=m.buffer(plan.sort_state);
    auto tiles=m.buffer(plan.tiles),list=m.buffer(plan.candidates);
    auto ranks=m.buffer(plan.ranks),spatialRecords=m.buffer(plan.spatial_records);
    std::memset(ranks.contents,255,ranks.length);
    auto accum=m.buffer(plan.accumulators);
    auto a=m.buffer(plan.rgb),b=m.buffer(plan.rgb);
    auto marker=m.buffer(plan.marker);std::memset(marker.contents,0,marker.length);
    auto depth=m.pipeline(@"native_dof_depth"),prepare=m.pipeline(@"native_dof_pass"),radii=m.pipeline(@"native_dof_radii");
    auto sort=m.pipeline(@"native_dof_sort"),candidate=m.pipeline(@"native_dof_candidates"),gather=m.pipeline(@"native_dof_gather"),finish=m.pipeline(@"native_dof_finish");
    auto globalPtr=static_cast<Global*>(global.contents);
    *globalPtr={32768,0,0,0,0,0,0,0};
    auto call=[&](id<MTLComputePipelineState> pipeline,uint32_t count,const auto& encode,uint32_t groupWidth=0){
        if(poll)poll();gpu::dispatchStage(m,pipeline,count,encode,1,groupWidth);++out.activity.dispatches;
        gpu::require(globalPtr->fault==0,"Native DOF GPU fault");if(poll)poll();};
    call(m.pipeline(@"native_dof_abi"),1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:marker offset:0 atIndex:0];});
    const uint32_t identity[]={Version,sizeof(Params),sizeof(Pass),sizeof(Source),sizeof(Global),sizeof(SortState),sizeof(Tile),sizeof(Accum),sizeof(Launch)};
    gpu::require(std::memcmp(identity,marker.contents,sizeof(identity))==0,"Native DOF ABI mismatch");
    call(depth,n,[&](id<MTLComputeCommandEncoder> e){[e setBytes:&p length:sizeof(p) atIndex:0];[e setBuffer:surface offset:0 atIndex:1];
        [e setBuffer:src offset:0 atIndex:2];[e setBuffer:order offset:0 atIndex:3];[e setBuffer:global offset:0 atIndex:4];[e setBytes:&n length:4 atIndex:5];});
    // Depth preparation already writes identity order for the single pixel.
    if(!p.plain&&n>1){SortState s{};s.version=Version;s.count=n;s.stack_size=1;s.stack_lo[0]=0;s.stack_hi[0]=int(n)-1;
        std::memcpy(state.contents,&s,sizeof(s));const SortLaunch l{n,n,limits.sort_slice,0,limits.sort_work_limit};
        uint64_t previous=0;
        do{call(sort,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:order offset:0 atIndex:0];[e setBuffer:state offset:0 atIndex:1];[e setBytes:&l length:sizeof(l) atIndex:2];});
            std::memcpy(&s,state.contents,sizeof(s));gpu::require(!s.fault&&s.version==Version&&s.count==n&&s.work>previous&&s.work<=limits.sort_work_limit,"Native DOF sort bounds/work fault");previous=s.work;
        }while(!s.done);out.activity.sort_work=s.work;}
    call(m.pipeline(@"native_dof_spatial_abi"),1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:marker offset:0 atIndex:0];});
    const uint32_t spatialIdentity[]={1,sizeof(SpatialRecord),4096,256};
    gpu::require(std::memcmp(spatialIdentity,marker.contents,sizeof(spatialIdentity))==0,"Native DOF spatial ABI mismatch");
    if(limits.spatial_candidates)call(m.pipeline(@"native_dof_inverse_rank"),n,[&](id<MTLComputeCommandEncoder> e){
        [e setBytes:&p length:sizeof(p) atIndex:0];[e setBuffer:src offset:0 atIndex:1];[e setBuffer:order offset:0 atIndex:2];
        [e setBuffer:ranks offset:0 atIndex:3];[e setBuffer:global offset:0 atIndex:4];[e setBytes:&n length:4 atIndex:5];});
    id<MTLBuffer> current=input,target=a;
    const uint32_t cols=(p.width+15)/16,rows=(p.height+15)/16,totalTiles=cols*rows;
    for(uint32_t passIndex=0;passIndex<p.passes;++passIndex){
        call(prepare,1,[&](id<MTLComputeCommandEncoder> e){[e setBytes:&p length:sizeof(p) atIndex:0];[e setBuffer:global offset:0 atIndex:1];[e setBuffer:pass offset:0 atIndex:2];[e setBytes:&passIndex length:4 atIndex:3];});
        call(radii,n,[&](id<MTLComputeCommandEncoder> e){[e setBytes:&p length:sizeof(p) atIndex:0];[e setBuffer:pass offset:0 atIndex:1];[e setBuffer:src offset:0 atIndex:2];[e setBuffer:global offset:0 atIndex:3];[e setBytes:&n length:4 atIndex:4];});
        const auto* sources=static_cast<const Source*>(src.contents);
        for(uint32_t i=0;i<n;++i){gpu::require(!sources[i].fault&&sources[i].index==i&&std::isfinite(sources[i].radius)&&sources[i].bound<=1000,"Native DOF radius record");
            out.activity.positive_radius_sources+=std::fabs(sources[i].radius)>0;}
        out.activity.maximum_radius=std::max(out.activity.maximum_radius,globalPtr->max_bound);
        for(uint32_t offset=0;offset<totalTiles;offset+=batch){
            const uint32_t count=std::min(batch,totalTiles-offset);auto* tile=static_cast<Tile*>(tiles.contents);
            std::memset(tiles.contents,0,tiles.length);std::memset(accum.contents,0,accum.length);
            for(uint32_t t=0;t<count;++t){const uint32_t index=offset+t,x=(index%cols)*16,y=(index/cols)*16;
                tile[t]={x,y,std::min(16u,p.width-x),std::min(16u,p.height-y),0,0,0,0};}
            Launch l{count,batch,cap,batch*cap,0,0,n,0};
            if(execution.use_spatial(globalPtr->max_bound)) {
                uint64_t checks=0;const uint32_t radius=globalPtr->max_bound;
                for(uint32_t t=0;t<count;++t){const auto& v=tile[t];
                    const uint32_t loX=v.x>radius?v.x-radius:0,loY=v.y>radius?v.y-radius:0;
                    checks+=uint64_t(std::min(p.width,v.x+v.width+radius)-loX)*(std::min(p.height,v.y+v.height+radius)-loY);}
                gpu::require(checks<=limits.candidate_scan_work_limit-out.activity.candidate_scan_checks,"Native DOF spatial scan work limit");
                std::memset(spatialRecords.contents,0,spatialRecords.length);
                call(m.pipeline(@"native_dof_spatial_candidates"),count*256,[&](id<MTLComputeCommandEncoder> e){
                    [e setBytes:&p length:sizeof(p) atIndex:0];[e setBytes:&l length:sizeof(l) atIndex:1];
                    [e setBuffer:src offset:0 atIndex:2];[e setBuffer:order offset:0 atIndex:3];[e setBuffer:tiles offset:0 atIndex:4];
                    [e setBuffer:list offset:0 atIndex:5];[e setBuffer:global offset:0 atIndex:6];[e setBuffer:ranks offset:0 atIndex:7];
                    [e setBuffer:spatialRecords offset:0 atIndex:8];},execution.spatial().group_width);
                uint64_t actual=0;const auto* sr=static_cast<const SpatialRecord*>(spatialRecords.contents);
                for(uint32_t t=0;t<count;++t){gpu::require(sr[t].version==1&&!sr[t].fault&&sr[t].kept==tile[t].count,"Native DOF spatial record");actual+=sr[t].checked;}
                gpu::require(actual==checks,"Native DOF spatial scan count mismatch");out.activity.candidate_scan_checks+=actual;
            } else
            for(uint32_t j=0;j<n;j+=limits.source_slice){l.source_begin=j;l.source_end=std::min(n,j+limits.source_slice);
                const uint64_t scans=uint64_t(count)*(l.source_end-l.source_begin);
                gpu::require(scans<=limits.candidate_scan_work_limit-out.activity.candidate_scan_checks,"Native DOF candidate scan work limit");
                call(candidate,count,[&](id<MTLComputeCommandEncoder> e){[e setBytes:&p length:sizeof(p) atIndex:0];[e setBytes:&l length:sizeof(l) atIndex:1];
                    [e setBuffer:src offset:0 atIndex:2];[e setBuffer:order offset:0 atIndex:3];[e setBuffer:tiles offset:0 atIndex:4];[e setBuffer:list offset:0 atIndex:5];[e setBuffer:global offset:0 atIndex:6];},1);
                out.activity.candidate_scan_checks+=scans;}
            uint32_t maximum=0;uint64_t work=0;
            for(uint32_t t=0;t<count;++t){gpu::require(!tile[t].fault&&tile[t].cursor==n&&tile[t].count<=cap,"Native DOF candidate bounds");maximum=std::max(maximum,tile[t].count);work+=uint64_t(tile[t].count)*tile[t].width*tile[t].height;}
            gpu::require(work<=limits.gather_work_limit-out.activity.candidate_checks,"Native DOF gather work limit");
            for(uint32_t j=0;j<maximum;j+=limits.gather_slice){l.source_begin=j;l.source_end=std::min(maximum,j+limits.gather_slice);
                call(gather,count*256,[&](id<MTLComputeCommandEncoder> e){[e setBytes:&p length:sizeof(p) atIndex:0];[e setBytes:&l length:sizeof(l) atIndex:1];
                    [e setBuffer:src offset:0 atIndex:2];[e setBuffer:tiles offset:0 atIndex:3];[e setBuffer:list offset:0 atIndex:4];[e setBuffer:current offset:0 atIndex:5];[e setBuffer:accum offset:0 atIndex:6];[e setBuffer:global offset:0 atIndex:7];});}
            l.source_begin=l.source_end=0;
            call(finish,count*256,[&](id<MTLComputeCommandEncoder> e){[e setBytes:&p length:sizeof(p) atIndex:0];[e setBytes:&l length:sizeof(l) atIndex:1];
                [e setBuffer:tiles offset:0 atIndex:2];[e setBuffer:accum offset:0 atIndex:3];[e setBuffer:input offset:0 atIndex:4];[e setBuffer:target offset:0 atIndex:5];[e setBuffer:global offset:0 atIndex:6];});
            const auto* records=static_cast<const Accum*>(accum.contents);
            for(uint32_t t=0;t<count;++t)for(uint32_t k=0;k<tile[t].width*tile[t].height;++k){const auto& r=records[t*256+k];
                gpu::require(!r.fault&&r.visited==tile[t].count&&r.contributed<=r.visited&&r.changed<=1,"Native DOF gather record");
                ++out.activity.records;
                out.activity.candidate_checks+=r.visited;out.activity.contributing_pairs+=r.contributed;
                if(passIndex+1==p.passes)out.activity.changed_pixels+=r.changed;}
            ++out.activity.tile_batches;
        }
        ++out.activity.passes;current=target;target=target==a?b:a;
    }
    out.rgb=current;return out;
}
}
