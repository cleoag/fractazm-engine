#include "zbuffer_dispatch.h"
#include "comparison/metal_exact/shadow_abi.h"
#include <array>
#include <cmath>
#include <cstring>
#include <stdexcept>
namespace mb3d::native_metal {
namespace gpu=metal_exact::gpu;
using namespace mb3d_zn;
ZNormalsOutput RunNativeZNormals(gpu::Metal& m,const Params& p,id<MTLBuffer> source,const ZNormalsExecutionPolicy& execution,const std::function<void()>& poll){
    execution.validate(p);
    ZNormalsOutput out;const auto& plan=execution.allocation();const auto bytes=plan.bytes();if(!p.version){out.surface=source;return out;}
    gpu::require(source&&source.length>=uint64_t(p.count)*sizeof(ExactSurface),"Native Z normals input allocation");
    gpu::require(bytes<=uint64_t(1)<<30&&bytes<=m.device.maxBufferLength,"Native Z normals workspace exceeds bounds");
    if(poll)poll();
    auto positions=m.buffer(plan.coordinates),records=m.buffer(plan.records);
    auto status=m.buffer(plan.status),marker=m.buffer(plan.marker);
    out.surface=m.buffer(plan.surface);
    std::memset(marker.contents,0,marker.length);*static_cast<uint32_t*>(status.contents)=0;
    const Launch l{p.count,p.count,p.count,p.count,p.count,{0,0,0}};
    auto call=[&](id<MTLComputePipelineState> pipeline,uint32_t count,const auto& encode){
        if(poll)poll();gpu::dispatchStage(m,pipeline,count,encode);++out.activity.dispatches;
        out.activity.faults=*static_cast<uint32_t*>(status.contents);
        gpu::require(!out.activity.faults,"Native Z normals GPU fault");if(poll)poll();};
    call(m.pipeline(@"native_zn_abi"),1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:marker offset:0 atIndex:0];});
    const uint32_t identity[]={Version,sizeof(Params),sizeof(Coordinate),sizeof(Record),sizeof(Launch)};
    gpu::require(!std::memcmp(identity,marker.contents,sizeof(identity)),"Native Z normals ABI mismatch");
    call(m.pipeline(@"native_zn_reconstruct"),p.count,[&](id<MTLComputeCommandEncoder> e){
        [e setBytes:&p length:sizeof(p) atIndex:0];[e setBytes:&l length:sizeof(l) atIndex:1];
        [e setBuffer:source offset:0 atIndex:2];[e setBuffer:positions offset:0 atIndex:3];[e setBuffer:status offset:0 atIndex:4];});
    call(m.pipeline(@"native_zn_normals"),p.count,[&](id<MTLComputeCommandEncoder> e){
        [e setBytes:&p length:sizeof(p) atIndex:0];[e setBytes:&l length:sizeof(l) atIndex:1];
        [e setBuffer:source offset:0 atIndex:2];[e setBuffer:positions offset:0 atIndex:3];[e setBuffer:out.surface offset:0 atIndex:4];
        [e setBuffer:records offset:0 atIndex:5];[e setBuffer:status offset:0 atIndex:6];});
    const auto* r=static_cast<const Record*>(records.contents);const auto* c=static_cast<const Coordinate*>(positions.contents);
    const auto* output=static_cast<const ExactSurface*>(out.surface.contents);
    for(uint32_t i=0;i<p.count;++i){
        if(!(i&65535)&&poll)poll();
        gpu::require(r[i].version==Version&&r[i].index==i&&!r[i].fault&&!r[i].reserved[0]&&!r[i].reserved[1]&&
                     r[i].foreground<=1&&r[i].changed<=r[i].foreground&&r[i].neighbors<=4,"Native Z normals invalid work record");
        gpu::require(c[i].index==i&&!c[i].fault&&!c[i].reserved&&c[i].foreground==r[i].foreground&&
                     std::isfinite(c[i].x)&&std::isfinite(c[i].y)&&std::isfinite(c[i].z)&&std::isfinite(c[i].depth),"Native Z normals invalid coordinate record");
        gpu::require(!output[i].fault&&!output[i].reserved[0]&&!output[i].reserved[1]&&!output[i].reserved[2]&&
                     output[i].si_gradient<=65535&&output[i].o_trap<=65535&&output[i].shadow<=65535&&output[i].amb_shadow<=65535,
                     "Native Z normals invalid output surface");
        for(const auto normal:output[i].normal)gpu::require(normal>=-32768&&normal<=32767,"Native Z normals invalid packed normal");
        ++out.activity.records;out.activity.foreground+=r[i].foreground;out.activity.changed+=r[i].changed;out.activity.neighbors+=r[i].neighbors;
    }
    if(poll)poll();return out;
}
}
