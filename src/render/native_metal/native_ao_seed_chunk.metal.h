#pragma once
#include "native_ao_seed_chunk_abi.h"
// Included after the unchanged ordered seed implementation.
namespace mb3d_native_ao_chunk {
using namespace mb3d_native_ao;
inline uint owner_pixels(thread const Params& p,uint owner) {
    return ((p.height-1u-owner)/p.logical_lanes+1u)*p.width;
}
inline uint image_pixel(thread const Params& p,uint owner,uint ordinal) {
    return (owner+(ordinal/p.width)*p.logical_lanes)*p.width+ordinal%p.width;
}
inline bool shape(thread const Params& p,thread const Run& r) {
    const uint items=((p.height+p.logical_lanes-1u)/p.logical_lanes)*p.width;
    return r.chunks_per_owner==(items+Width-1u)/Width&&
        r.total_chunks==r.chunks_per_owner*p.logical_lanes;
}
inline bool known_pixel(thread const Params& p,thread const Level& l,uint pixel,
    thread const Record& r,device const uchar* certificates,thread Pixel& out) {
    out={0,0,0,0};if(r.fault)return false;if(!r.active)return true;
    const uint x=pixel%p.width,y=pixel/p.width;
    const float envelope=1.001f*(l.minimum_radius+float(l.samples)*float(l.step))+2.f*float(l.step)+2.f;
    const bool bounded=isfinite(envelope)&&envelope>0&&envelope<float(min(p.width,p.height));
    const uint margin=bounded?uint(ceil(envelope)):0u;
    const bool interior=bounded&&x>=margin&&y>=margin&&margin<p.width-x&&margin<p.height-y;
    if(interior){out.draws=32u*l.samples;out.shortcut=1;}
    else {
        for(uint d=0;d<32u;++d){const uint n=certificates[pixel*32u+d];
            if(!n||n>l.samples)return false;out.draws+=n;}
        out.used=32u;
    }
    return r.samples<=p.work_limit&&out.draws<=p.work_limit-r.samples;
}
inline void seed_lane(device const Record* records,thread const Params& p,thread const Level& l,
    device LaneState* lanes,uint reset,uint allocation,uint lane_allocation,device SeedRecord* seeds,
    uint lane,bool invalid,device const uchar* certificates,device uint* usage,uint force_unknown,
    device Chunk* chunks,thread const Run& run,constant mb3d_native_ao_reuse::BaseTable* table) {
    if(lane>=lane_allocation)return;
    // Malformed state and unproved chunks keep the original fault path.
    const bool shape_valid=valid(p,l)&&shape(p,run);
    if(invalid||!shape_valid||!random_mode(p.algorithm)||allocation!=p.width*p.height||
       lane_allocation!=p.logical_lanes||reset>1||!shape(p,run)||force_unknown){
        run_seed_lane<true>(records,p,l,lanes,reset,allocation,lane_allocation,seeds,lane,
            invalid||!shape_valid,certificates,usage,force_unknown,table);return;
    }
    LaneState state=lanes[lane];if(reset)state={initial_seed(lane),0,lane,l.index,l.repeat_index,0,0,0};
    if(state.fault||state.finished){lanes[lane]=state;return;}
    if(state.level!=l.index||state.repeat_index!=l.repeat_index||state.reserved||
       state.x>=p.width||state.y>=p.height||state.y%p.logical_lanes!=lane){
        run_seed_lane<true>(records,p,l,lanes,reset,allocation,lane_allocation,seeds,lane,false,certificates,usage,force_unknown,table);return;
    }
    const uint ordinal=(state.y/p.logical_lanes)*p.width+state.x;
    const uint index=lane*run.chunks_per_owner+ordinal/Width;
    if(ordinal%Width||index>=run.total_chunks||chunks[index].known!=1u){
        run_seed_lane<true>(records,p,l,lanes,reset,allocation,lane_allocation,seeds,lane,false,certificates,usage,force_unknown,table);return;
    }
    Chunk chunk=chunks[index];chunk.seed=state.seed;chunk.executed=1;
    state.seed=advance_seed(state.seed,chunk.draws);
    const uint end=min(ordinal+Width,owner_pixels(p,lane));
    state.x=end%p.width;state.y=lane+(end/p.width)*p.logical_lanes;
    state.finished=state.y>=p.height;chunks[index]=chunk;lanes[lane]=state;
}
}
kernel void native_ao_chunk_abi(device uint* out [[buffer(0)]],uint tid [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao_chunk;if(tid)return;
    out[0]=mb3d_native_ao_chunk::Version;out[1]=sizeof(Pixel);out[2]=sizeof(Chunk);out[3]=Width;
}
kernel void native_ao_chunk_prepare(constant mb3d_native_ao_reuse::BaseTable& table [[buffer(0)]],
    device const mb3d_native_ao::Record* records [[buffer(1)]],device const uchar* certificates [[buffer(2)]],
    device mb3d_native_ao_chunk::Pixel* prefixes [[buffer(3)]],device mb3d_native_ao_chunk::Chunk* chunks [[buffer(4)]],
    constant mb3d_native_ao_chunk::Run& launch [[buffer(5)]],device atomic_uint* fault [[buffer(6)]],
    uint lane [[thread_index_in_threadgroup]],uint3 group [[threadgroup_position_in_grid]],
    uint3 group_size [[threads_per_threadgroup]]) {
    using namespace mb3d_native_ao;using namespace mb3d_native_ao_chunk;
    Params p=table.params;Level l=table.level;Run run=launch;
    // Uniform validation before barriers. Host admission binds exact buffers.
    if(any(group_size!=uint3(Width,1,1))||!valid(p,l)||!random_mode(p.algorithm)||
       table.version!=1||table.count!=192||table.reserved[0]||table.reserved[1]||!shape(p,run)||
       run.offset>run.total_chunks||run.count>run.total_chunks-run.offset||group.x>=run.count||group.y||group.z){
        atomic_fetch_or_explicit(fault,uint(Invalid),memory_order_relaxed);return;
    }
    const uint index=run.offset+group.x,owner=index/run.chunks_per_owner;
    const uint ordinal=(index%run.chunks_per_owner)*Width+lane;
    const bool live=ordinal<owner_pixels(p,owner);Pixel value{};bool known=true;uint pixel=0;
    if(live){pixel=image_pixel(p,owner,ordinal);const Record record=records[pixel];known=known_pixel(p,l,pixel,record,certificates,value);}
    threadgroup uint sums[Width],unknown[Width];sums[lane]=value.draws;unknown[lane]=!known;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    for(uint step=1;step<Width;step*=2){const uint add=lane>=step?sums[lane-step]:0u;
        const uint bad=lane>=step?unknown[lane-step]:0u;
        threadgroup_barrier(mem_flags::mem_threadgroup);sums[lane]+=add;unknown[lane]|=bad;
        threadgroup_barrier(mem_flags::mem_threadgroup);}
    value.prefix=sums[lane]-value.draws;if(live)prefixes[pixel]=value;
    if(lane==Width-1u)chunks[index]={unknown[lane]?0u:1u,sums[lane],0,0};
}
kernel void native_ao_chunk_lanes(device const mb3d_native_ao::Record* records [[buffer(0)]],
    device mb3d_native_ao::LaneState* lanes [[buffer(3)]],constant uint& reset [[buffer(4)]],
    constant uint& allocation [[buffer(5)]],constant uint& lane_allocation [[buffer(6)]],
    device mb3d_native_ao::SeedRecord* seeds [[buffer(7)]],constant mb3d_native_ao_reuse::BaseTable& table [[buffer(8)]],
    device const uchar* certificates [[buffer(9)]],constant uint& certificate_bytes [[buffer(10)]],
    device uint* usage [[buffer(11)]],constant uint& force_unknown [[buffer(12)]],constant uint& usage_count [[buffer(13)]],
    constant uint& count [[buffer(14)]],device mb3d_native_ao_chunk::Chunk* chunks [[buffer(15)]],
    constant mb3d_native_ao_chunk::Run& launch [[buffer(16)]],uint3 position [[thread_position_in_grid]],
    uint3 group [[threadgroup_position_in_grid]],uint3 group_size [[threads_per_threadgroup]],uint3 grid_size [[threads_per_grid]]) {
    using namespace mb3d_native_ao;const uint lane=position.x;Params p=table.params;Level l=table.level;
    if(lane_allocation<1u||lane_allocation>32u||any(group_size!=uint3(lane_allocation,1,1))||
       any(group!=uint3(0))||any(grid_size!=uint3(lane_allocation,1,1))||count<1u||count>8u){
        if(lane<lane_allocation)lanes[lane].fault|=Invalid;return;}
    const bool invalid=table.version!=1u||table.count!=192u||table.reserved[0]||table.reserved[1]||
        certificate_bytes!=p.width*p.height*32u||usage_count!=p.width*p.height||force_unknown>1u;
    threadgroup uint faults[32];mb3d_native_ao_chunk::Run run=launch;
    for(uint chunk=0;chunk<count;++chunk){
        mb3d_native_ao_chunk::seed_lane(records,p,l,lanes,chunk?0u:reset,allocation,lane_allocation,seeds,lane,
            invalid,certificates,usage,force_unknown,chunks,run,&table);
        faults[lane]=lanes[lane].fault;threadgroup_barrier(mem_flags::mem_threadgroup);
        uint failed=0;for(uint owner=0;owner<lane_allocation;++owner)failed|=faults[owner];
        threadgroup_barrier(mem_flags::mem_threadgroup);if(failed)break;
    }
}
kernel void native_ao_chunk_scatter(constant mb3d_native_ao_reuse::BaseTable& table [[buffer(0)]],
    device const mb3d_native_ao_chunk::Pixel* prefixes [[buffer(1)]],device const mb3d_native_ao_chunk::Chunk* chunks [[buffer(2)]],
    device mb3d_native_ao::SeedRecord* seeds [[buffer(3)]],device uint* usage [[buffer(4)]],
    constant mb3d_native_ao_chunk::Run& launch [[buffer(5)]],device atomic_uint* fault [[buffer(6)]],uint tid [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;using namespace mb3d_native_ao_chunk;
    Params p=table.params;Level l=table.level;Run run=launch;
    if(!valid(p,l)||!random_mode(p.algorithm)||table.version!=1u||table.count!=192u||
       table.reserved[0]||table.reserved[1]||!shape(p,run)||!run.count||run.count>8u||
       run.offset>=run.chunks_per_owner||run.count>run.chunks_per_owner-run.offset){
        atomic_fetch_or_explicit(fault,uint(Invalid),memory_order_relaxed);return;}
    const uint owner=(tid/Width)/run.count,ordinal=(run.offset+(tid/Width)%run.count)*Width+tid%Width;
    if(owner>=p.logical_lanes||ordinal>=owner_pixels(p,owner))return;
    const Chunk chunk=chunks[owner*run.chunks_per_owner+ordinal/Width];if(!chunk.executed)return;
    const uint pixel=image_pixel(p,owner,ordinal);const Pixel value=prefixes[pixel];
    const uint begin=advance_seed(chunk.seed,value.prefix),end=advance_seed(begin,value.draws);
    seeds[pixel]={1,begin,end,value.draws,0,pixel,l.index,l.repeat_index,value.shortcut,{0,0,0}};
    usage[pixel]=(1u<<28)|(l.repeat_index<<16)|(l.index<<8)|value.used;
}
