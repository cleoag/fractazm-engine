#pragma once
#include "seed_scan_abi.h"
#include "native_ao_de_math.h"
#include "comparison/metal_exact/shadow_abi.h"
// Integer-only segmented exclusive scan. Each logical task is a segment;
// hierarchy levels own disjoint blocks. No formula/ray state crosses tasks.
namespace mb3d_seed_scan {
using namespace metal;
inline bool shape(constant Params& p) {
    const ulong pixels=ulong(p.width)*p.height;
    return p.version==Version&&p.width&&p.height&&pixels+ulong(p.owners)<=0xfffffffful&&
        p.owners&&p.owners<=p.height&&p.rows_per_owner&&p.rows_per_owner<=p.height&&
        (p.mapping==Interleaved?p.rows_per_owner==(p.height+p.owners-1u)/p.owners:
            p.owners==(p.height+p.rows_per_owner-1u)/p.rows_per_owner)&&
        (p.mapping==Interleaved||p.mapping==Contiguous)&&
        (p.input_kind==PrimaryFlags||p.input_kind==DrawCounts)&&
        pixels*(p.input_kind==PrimaryFlags?1ul:33ul)<=0xfffffffful&&
        p.items&&p.items<=0xffffff00u&&ulong(p.owners)*p.items<=0xfffffffful&&
        p.groups==(p.items+Group-1u)/Group&&ulong(p.owners)*p.groups*Group<=0xfffffffful&&p.first<=1&&!p.reserved&&
        (!p.first||ulong(p.items)==ulong(p.rows_per_owner)*p.width);
}
inline bool seed_address(constant Params& p,uint owner,uint ordinal,thread uint& index) {
    if(ordinal>=p.items)return false;
    if(!p.first){index=owner*p.items+ordinal;return true;}
    const uint row=ordinal/p.width;
    const uint y=p.mapping==Interleaved?row*p.owners+owner:owner*p.rows_per_owner+row;
    if(y>=p.height)return false;
    index=y*p.width+ordinal%p.width;return true;
}
inline uint jump(uint seed,uint count) {
    uint a=214013u,c=2531011u,m=1u,b=0u;
    for(;count;count>>=1){if(count&1){b=a*b+c;m=a*m;}c=(a+1u)*c;a*=a;}
    return m*seed+b;
}
}
kernel void native_seed_scan_abi(device uint* out [[buffer(0)]],uint tid [[thread_position_in_grid]]) {
    if(tid)return;out[0]=mb3d_seed_scan::Version;out[1]=sizeof(mb3d_seed_scan::Params);out[2]=mb3d_seed_scan::Group;
}
kernel void native_seed_scan_blocks(device uint* words [[buffer(0)]],device uint* totals [[buffer(1)]],
    constant mb3d_seed_scan::Params& p [[buffer(2)]],device metal::atomic_uint* fault [[buffer(3)]],
    uint3 block [[threadgroup_position_in_grid]],uint3 lane [[thread_position_in_threadgroup]],
    uint3 width [[threads_per_threadgroup]]) {
    using namespace mb3d_seed_scan;
    if(!shape(p)||!p.count||p.offset>p.owners*p.groups||p.count>p.owners*p.groups-p.offset||
        p.count>1024u||width.x!=Group||width.y!=1||width.z!=1){
        if(!lane.x)atomic_fetch_or_explicit(fault,1u,memory_order_relaxed);return;
    }
    if(block.x>=p.count)return;
    const uint blockIndex=block.x+p.offset;
    const uint owner=blockIndex/p.groups,group=blockIndex%p.groups,ordinal=group*Group+lane.x;
    uint index=0;const bool active=seed_address(p,owner,ordinal,index);uint value=active?words[index]:0;
    if(p.first){
        if(value>(p.input_kind==PrimaryFlags?2u:33u)){atomic_fetch_or_explicit(fault,2u,memory_order_relaxed);value=0;}
        if(p.input_kind==PrimaryFlags)value=value==1u?1u:0u;
    }
    threadgroup uint partial[Group];partial[lane.x]=value;threadgroup_barrier(mem_flags::mem_threadgroup);
    for(uint shift=1;shift<Group;shift<<=1){
        const uint before=lane.x>=shift?partial[lane.x-shift]:0u;
        threadgroup_barrier(mem_flags::mem_threadgroup);
        partial[lane.x]+=before;threadgroup_barrier(mem_flags::mem_threadgroup);
    }
    if(active)words[index]=partial[lane.x]-value;
    if(lane.x==Group-1)totals[owner*p.groups+group]=partial[lane.x];
}
kernel void native_seed_scan_add(device uint* words [[buffer(0)]],device const uint* parent [[buffer(1)]],
    constant mb3d_seed_scan::Params& p [[buffer(2)]],device metal::atomic_uint* fault [[buffer(3)]],
    uint tid [[thread_position_in_grid]]) {
    using namespace mb3d_seed_scan;
    if(!shape(p)||!p.count||p.offset>p.owners*p.items||p.count>p.owners*p.items-p.offset||p.count>262144u){if(!tid)atomic_fetch_or_explicit(fault,1u,memory_order_relaxed);return;}
    if(tid>=p.count)return;tid+=p.offset;
    const uint owner=tid/p.items,ordinal=tid%p.items;uint index=0;
    if(seed_address(p,owner,ordinal,index))words[index]+=parent[owner*p.groups+ordinal/Group];
}
kernel void native_seed_scan_finish(device uint* words [[buffer(0)]],device const uint* totals [[buffer(1)]],
    constant mb3d_seed_scan::Params& p [[buffer(2)]],device metal::atomic_uint* fault [[buffer(3)]],
    uint tid [[thread_position_in_grid]]) {
    using namespace mb3d_seed_scan;
    if(!shape(p)||!p.first||!p.count||p.offset>p.owners*p.items||p.count>p.owners*p.items-p.offset||p.count>262144u){if(!tid)atomic_fetch_or_explicit(fault,1u,memory_order_relaxed);return;}
    if(tid>=p.count)return;tid+=p.offset;
    const uint owner=tid/p.items,ordinal=tid%p.items;uint index=0;
    const uint seed=p.input_kind==PrimaryFlags?(owner+2u)*0x324594a1u+0x24563487u:p.initial_seed;
    if(seed_address(p,owner,ordinal,index))words[index]=jump(seed,words[index]);
    if(!ordinal)words[p.width*p.height+owner]=jump(seed,totals[owner]);
}
kernel void native_seed_de_weights(device const ExactSurface* surface [[buffer(0)]],device uint* words [[buffer(1)]],
    constant mb3d_native_ao::DEParams& p [[buffer(2)]],constant mb3d_native_ao::Launch& run [[buffer(3)]],
    device metal::atomic_uint* fault [[buffer(4)]],uint tid [[thread_position_in_grid]]) {
    using namespace metal;using namespace mb3d_native_ao;
    if(!valid_de(p)||run.reserved||run.allocated_pixels!=p.width*p.height||
        run.offset>run.allocated_pixels||run.count>run.allocated_pixels-run.offset||run.count>262144u){
        if(!tid)atomic_fetch_or_explicit(fault,1u,memory_order_relaxed);return;
    }
    if(tid>=run.count)return;const uint index=run.offset+tid;
    const auto value=surface[index];uint rays=0;
    if(value.fault){atomic_fetch_or_explicit(fault,2u,memory_order_relaxed);return;}
    if((value.depth_packed>>16)<32768u&&value.si_gradient<32768u){
        const int y=int(index/p.width)+p.top;
        // Same FP32 pole predicate as de_local_rays, including signed remainder.
        rays=p.quality?(p.dither?uint(float(y%int(p.dither+1u))*.5f/float(p.dither)>.1f):1u):3u;
        for(uint row=1;row<=p.quality;++row)rays+=p.row_counts[row];
    }
    words[index]=rays;
}
