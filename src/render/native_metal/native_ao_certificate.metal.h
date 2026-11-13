#pragma once
#include "native_ao_certificate_math.h"
#include "native_ao_math.h"
using namespace metal;
// Included after the shared scan_hiq_direction primitive, before seed entries.
kernel void native_ao_reuse_abi(device uint* out [[buffer(0)]],uint tid [[thread_position_in_grid]]) {
    if(tid)return;out[0]=1;out[1]=sizeof(mb3d_native_ao_reuse::BaseSample);
    out[2]=sizeof(mb3d_native_ao_reuse::BaseTable);out[3]=192;
}
kernel void native_ao_reuse_table(constant mb3d_native_ao::Params& params [[buffer(0)]],
    constant mb3d_native_ao::Level& level [[buffer(1)]],
    device mb3d_native_ao_reuse::BaseTable& table [[buffer(2)]],uint d [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;using namespace mb3d_native_ao_reuse;
    if(d>=32u)return;Params p=params;Level l=level;
    if(d==0u){table.version=1;table.count=192;table.reserved[0]=0;table.reserved[1]=0;table.params=p;table.level=l;}
    const bool admitted=valid(p,l)&&random_mode(p.algorithm);
    const U32 jitter=threshold_mode(p.algorithm)?l.step*2u-1u:l.step-1u;
    const float dx=p.directions[2*d],dy=p.directions[2*d+1];
    float sx=dx*l.minimum_radius+(-0.5f*float(jitter));
    float sy=dy*l.minimum_radius+(-0.5f*float(jitter));
    for(U32 sample=0;sample<6u;++sample){
        BaseSample entry{};
        if(!admitted)entry.fault=Invalid;
        else if(sample<l.samples){
            if(!rounded(sx,entry.x)||!rounded(sy,entry.y))entry.fault=NonFinite;
            sx+=dx*float(l.step);sy+=dy*float(l.step);
        }
        table.entries[d*6u+sample]=entry;
    }
}
kernel void native_ao_certificate_abi(device uint* out [[buffer(0)]],uint tid [[thread_position_in_grid]]) {
    if(tid)return;out[0]=1;out[1]=32;out[2]=4;out[3]=4;
}
kernel void native_ao_certificate_prepare(
    constant mb3d_native_ao_reuse::BaseTable& table [[buffer(0)]],
    device uchar* certificates [[buffer(1)]],constant mb3d_native_ao::Launch& launch [[buffer(2)]],
    constant uint& certificate_bytes [[buffer(3)]],device uint* classified [[buffer(4)]],
    device atomic_uint* fault [[buffer(5)]],constant uint& classified_count [[buffer(6)]],
    uint tid [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;using namespace mb3d_native_ao_certificate;
    Launch run=launch;if(tid>=run.count)return;
    Params p=table.params;Level l=table.level;
    if(table.version!=1u||table.count!=192u||table.reserved[0]||table.reserved[1]||
       !valid(p,l)||!random_mode(p.algorithm)||run.reserved||run.offset>run.allocated_pixels||
       tid>=run.allocated_pixels-run.offset){atomic_fetch_or_explicit(fault,U32(Invalid),memory_order_relaxed);return;}
    const U32 n=p.width*p.height;
    if(run.allocated_pixels!=n||certificate_bytes!=n*32u||classified_count!=n){
        atomic_fetch_or_explicit(fault,U32(Invalid),memory_order_relaxed);return;
    }
    const U32 pixel=run.offset+tid;const I32 x=I32(pixel%p.width),y=I32(pixel/p.width);
    U32 known=0;
    // Each invocation owns an entire aligned32-byte output block. Adjacent
    // pixels never share a packed store word; there are no group collectives.
    for(U32 d=0;d<32u;++d){
        const U32 count=DirectionValidated(p,l,x,y,d,table);
        certificates[pixel*32u+d]=uchar(count);known+=count!=0u;
    }
    classified[pixel]=(1u<<28)|(l.repeat_index<<16)|(l.index<<8)|known;
}
namespace mb3d_native_ao {
// Planning never consumes the reciprocal radius. Every admitted rounded base
// component lies strictly inside +/-2^30; adding jitter<=32767 and an image
// coordinate<=4095 is int32-safe. A float square of any such integer is finite
// and the radius is zero iff both integers are zero. Thus this path can reuse
// the exact per-level rounded table without redoing floating arithmetic.
inline bool scan_seed_reused_direction(thread const Params& p,thread const Level& l,
    constant mb3d_native_ao_reuse::BaseTable& table,U32 pixel,U32 direction,
    thread U32& seed,thread Record& record) {
    const I32 x=I32(pixel%p.width),y=I32(pixel/p.width);
    const U32 jitter=threshold_mode(p.algorithm)?2u*l.step-1u:l.step-1u;
    for(U32 sample=0;sample<l.samples;++sample){
        if(!spend(record,p.work_limit))return false;
        const auto base=table.entries[direction*6u+sample];
        if(base.fault){record.fault|=base.fault;return false;}
        if(base.reserved||base.x<=-1073741824||base.x>=1073741824||
           base.y<=-1073741824||base.y>=1073741824){record.fault|=Invalid;return false;}
        seed=next_seed(seed);++record.rng_advances;
        I32 xx=base.x+I32((seed>>10)&jitter),yy=base.y+I32((seed>>16)&jitter);
        if(!xx&&!yy)continue;
        xx+=x;yy+=y;const BoundaryResult where=boundary_coordinates(p,xx,yy,false);
        if(where.fault){record.fault|=where.fault;return false;}
        if(!where.admitted){++record.border_rejects;break;}
        if(where.x<0||where.x>=I32(p.width)){record.fault|=Bounds;return false;}
        const U32 index=U32(where.y)*p.width+U32(where.x);
        if(index>=p.width*p.height){record.fault|=Bounds;return false;}
        ++record.depth_reads;
    }
    return true;
}
inline bool scan_seed_certified(thread const Params& p,thread const Level& l,device const U32* depth,
    device short* angles,U32 pixel,U32 packed,thread U32& seed,thread Record& record,device const uchar* certificates,
    U32 force_unknown,thread U32& used,constant mb3d_native_ao_reuse::BaseTable* table=nullptr) {
    constexpr bool planning=true;
    const bool random=random_mode(p.algorithm),squared=threshold_mode(p.algorithm);
    const I32 x=I32(pixel%p.width),y=I32(pixel/p.width);
    I32 anchor=x,lane=0;
    // Standard only: retain original nonfinal interior quartet rounding anchor.
    if(!random&&!squared&&l.index!=p.levels&&x>=l.maximum_radius) {
        const I32 candidate=l.maximum_radius+((x-l.maximum_radius)/4)*4;
        if(candidate<I32(p.width)-l.maximum_radius-3){anchor=candidate;lane=x-candidate;}
    }
    const bool quartet=!random&&!squared&&l.index!=p.levels&&x>=l.maximum_radius&&
        anchor<I32(p.width)-l.maximum_radius-3;
    const I32 zp=I32((packed&0xffffff00u)>>1);
    const U32 jitter=squared?l.step*2u-1u:l.step-1u;
    // Consecutive certified directions consume a known number of draws.
    // Affine LCG composition modulo 2^32 lets one jump replace all those
    // jumps. Flush before an unknown direction or fault, preserving its exact
    // entering seed and the observable first-fault state. Work/usage counts
    // retain their original per-direction updates and budget checks.
    U32 pending_draws=0;
    for(U32 d=0;d<32u;++d) {
        const U32 count=force_unknown?0u:U32(certificates[pixel*32u+d]);
        if(count>l.samples){seed=advance_seed(seed,pending_draws);record.fault|=Invalid;return false;}
        if(count&&record.samples<=p.work_limit&&count<=p.work_limit-record.samples){
            pending_draws+=count;record.samples+=count;record.rng_advances+=count;
            ++used;continue;
        }
        // Fall back only for this direction, with its original current state.
        if(pending_draws){seed=advance_seed(seed,pending_draws);pending_draws=0;}
        if(table){if(!scan_seed_reused_direction(p,l,*table,pixel,d,seed,record))return false;}
        else if(!scan_hiq_direction<planning>(p,l,depth,angles,pixel,d,x,y,anchor,lane,
                quartet,zp,jitter,random,squared,seed,record))return false;
    }
    if(pending_draws)seed=advance_seed(seed,pending_draws);
    return true;
}
}
