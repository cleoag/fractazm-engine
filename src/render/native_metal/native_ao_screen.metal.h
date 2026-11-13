#pragma once
#include "native_ao_seed_chunk_abi.h"
#include "native_ao_math.h"
#include "comparison/metal_exact/shadow_abi.h"
using namespace metal;
namespace mb3d_native_ao {

inline bool launch_pixel(thread const Params& p,thread const Launch& run,U32 tid,thread U32& pixel) {
    if(tid>=run.count||run.offset>run.allocated_pixels||
       tid>=run.allocated_pixels-run.offset)return false;
    pixel=run.offset+tid;
    return pixel<p.output_count;
}
template<bool planning> inline bool scan_hiq_direction(thread const Params& p,thread const Level& l,
    device const U32* depth,device short* angles,U32 pixel,U32 d,I32 x,I32 y,I32 anchor,I32 lane,
    bool quartet,I32 zp,U32 jitter,bool random,bool squared,thread U32& seed,thread Record& record) {
    const float dx=p.directions[2*d],dy=p.directions[2*d+1];
    float sx=dx*l.minimum_radius+(random?-0.5f*float(jitter):float(anchor));
    float sy=dy*l.minimum_radius+(random?-0.5f*float(jitter):float(y));
    for(U32 sample=0;sample<l.samples;++sample) {
        if(!spend(record,p.work_limit))return false;
        I32 xx,yy;
        if(!rounded(sx,xx)||!rounded(sy,yy)){record.fault|=NonFinite;return false;}
        float radius_squared=0.0f;
        if(random) {
            seed=next_seed(seed);++record.rng_advances;
            xx+=I32((seed>>10)&jitter);yy+=I32((seed>>16)&jitter);
            radius_squared=float(xx)*float(xx)+float(yy)*float(yy);
            if(!isfinite(radius_squared)){record.fault|=NonFinite;return false;}
            if(radius_squared==0.0f){sx+=dx*float(l.step);sy+=dy*float(l.step);continue;}
            xx+=x;yy+=y;
        }
        BoundaryResult where=boundary_coordinates(p,xx,yy,quartet);
        if(where.fault){record.fault|=where.fault;return false;}
        if(!where.admitted){++record.border_rejects;break;}
        where.x+=lane;
        if(where.x<0||where.x>=I32(p.width)){record.fault|=Bounds;return false;}
        const U32 index=U32(where.y)*p.width+U32(where.x);
        if(index>=p.width*p.height){record.fault|=Bounds;return false;}
        ++record.depth_reads;
        if constexpr (!planning) {
        const float reciprocal=random?1.0f/sqrt(radius_squared):l.reciprocal_radius[sample];
        const float st=float(I32(depth[index])-zp)*reciprocal;
        I32 value;
        if(!isfinite(st)||!angle(st,l.sit,l.z_threshold,squared,random,value)){
            record.fault|=NonFinite;return false;
        }
        const U32 address=pixel*32u+(random?31u-d:d);
        if(I32(angles[address])<value)angles[address]=short(value);
        }
        sx+=dx*float(l.step);sy+=dy*float(l.step);
    }
    return true;
}
template<bool planning=false> inline bool scan_hiq(thread const Params& p,thread const Level& l,device const U32* depth,
    device short* angles,U32 pixel,U32 packed,thread U32& seed,thread Record& record) {
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
    for(U32 d=0;d<32u;++d) {
        if(!scan_hiq_direction<planning>(p,l,depth,angles,pixel,d,x,y,anchor,lane,
                quartet,zp,jitter,random,squared,seed,record))return false;
    }
    return true;
}
inline bool sum_hiq(thread const Params& p,thread const Level& l,device const short* angles,
    U32 pixel,thread U32& ambient,thread Record& record) {
    float sum=0.0f;
    for(U32 d=0;d<32u;++d) {
        const I32 angle=I32(angles[pixel*32u+d]);
        if(angle>-32768)sum+=atan(float(angle)*0.0002441406f);
    }
    const float value=sum*l.sum_multiplier;
    if(!isfinite(sum)||!isfinite(value)){record.fault|=NonFinite;return false;}
    I32 result;
    if(!rounded(clamp(value,0.0f,float(l.add_limit)),result)){record.fault|=NonFinite;return false;}
    ambient=random_mode(p.algorithm)?min(16383u,ambient+U32(result)):U32(result);
    return true;
}
}

#include "native_ao_certificate.metal.h"

kernel void native_ao_abi(device uint* result [[buffer(0)]],uint tid [[thread_position_in_grid]]) {
    if(tid)return;
    result[0]=mb3d_native_ao::Version;result[1]=sizeof(mb3d_native_ao::Params);
    result[2]=sizeof(mb3d_native_ao::Level);result[3]=sizeof(mb3d_native_ao::Record);
    result[4]=sizeof(mb3d_native_ao::LaneState);result[5]=32;
    result[6]=sizeof(mb3d_native_ao::Range15);result[7]=sizeof(mb3d_native_ao::MedianState);
    result[8]=sizeof(mb3d_native_ao::PreprocessRecord);
}

// Called once at every repeat boundary. Input/output must be distinct; only
// repeat0 imports primary surfaces and performs the random-AO whole-buffer clear.
kernel void native_ao_begin(device const ExactSurface* input [[buffer(0)]],
    device ExactSurface* output [[buffer(1)]],device uint* depth [[buffer(2)]],
    device short* angles [[buffer(3)]],device mb3d_native_ao::Record* records [[buffer(4)]],
    constant mb3d_native_ao::Params& params [[buffer(5)]],
    constant mb3d_native_ao::Launch& launch [[buffer(6)]],
    constant mb3d_native_ao::Level& level [[buffer(7)]],uint tid [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;Params p=params;Level l=level;Launch run=launch;U32 pixel;
    if(!launch_pixel(p,run,tid,pixel))return;
    if(!valid(p,l)||run.reserved||run.allocated_pixels!=p.output_count||l.index!=1u){
        records[pixel].fault|=Invalid;return;
    }
    if(l.repeat_index==0u){
        Record r{};r.version=Version;r.algorithm=p.algorithm;r.initial_ambient=input[pixel].amb_shadow;
        r.fault=input[pixel].fault?Invalid:0u;output[pixel]=input[pixel];
        if(random_mode(p.algorithm))output[pixel].amb_shadow=0;
        r.final_ambient=output[pixel].amb_shadow;r.changed=r.initial_ambient!=r.final_ambient;
        records[pixel]=r;
    }
    if(pixel>=p.width*p.height)return;
    const U32 raw=output[pixel].depth_packed;
    depth[pixel]=(raw&0x80000000u)?0u:(raw&0xffffff00u)>>1;
    for(U32 d=0;d<32u;++d)angles[pixel*32u+d]=-32768;
    records[pixel].active=(raw&0x80000000u)?0u:1u;
}

kernel void native_ao_hiq_sample(device ExactSurface* surface [[buffer(0)]],
    device const uint* depth [[buffer(1)]],device short* angles [[buffer(2)]],
    device mb3d_native_ao::Record* records [[buffer(3)]],
    constant mb3d_native_ao::Params& params [[buffer(4)]],
    constant mb3d_native_ao::Level& level [[buffer(5)]],
    constant mb3d_native_ao::Launch& launch [[buffer(6)]],uint tid [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;Params p=params;Level l=level;Launch run=launch;U32 pixel;
    if(!launch_pixel(p,run,tid,pixel)||pixel>=p.width*p.height)return;
    Record r=records[pixel];
    if(!valid(p,l)||random_mode(p.algorithm)||run.reserved||run.allocated_pixels!=p.output_count){r.fault|=Invalid;records[pixel]=r;return;}
    if(r.fault)return;
    U32 seed=0,ambient=surface[pixel].amb_shadow;
    if(r.active&&scan_hiq(p,l,depth,angles,pixel,surface[pixel].depth_packed,seed,r)&&l.index==p.levels)
        sum_hiq(p,l,angles,pixel,ambient,r);
    ++r.levels;if(l.index==p.levels)++r.repeats;
    r.final_ambient=ambient;r.changed=ambient!=r.initial_ambient;
    records[pixel]=r;if(!r.fault)surface[pixel].amb_shadow=ambient;
}

// One state per logical CPU row lane, with 128 pixels maximum per dispatch.
// Host dispatches a fixed upper bound; finished lanes are inert. RNG advance is
// conditional on actual foreground samples/border breaks, so no host seed table.
kernel void native_ao_random_lanes(device ExactSurface* surface [[buffer(0)]],
    device const uint* depth [[buffer(1)]],device short* angles [[buffer(2)]],
    device mb3d_native_ao::Record* records [[buffer(3)]],
    constant mb3d_native_ao::Params& params [[buffer(4)]],
    constant mb3d_native_ao::Level& level [[buffer(5)]],
    device mb3d_native_ao::LaneState* lanes [[buffer(6)]],
    constant uint& reset [[buffer(7)]],constant uint& allocated_pixels [[buffer(8)]],
    constant uint& allocated_lanes [[buffer(9)]],
    uint lane [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;Params p=params;Level l=level;
    if(lane>=allocated_lanes)return;
    LaneState state=lanes[lane];
    if(!valid(p,l)||!random_mode(p.algorithm)||allocated_pixels!=p.output_count||
       allocated_lanes!=p.logical_lanes||reset>1u){state.fault|=Invalid;lanes[lane]=state;return;}
    if(reset)state={initial_seed(lane),0,lane,l.index,l.repeat_index,0,0,0};
    if(state.fault||state.finished)return;
    if(state.level!=l.index||state.repeat_index!=l.repeat_index||state.reserved||state.x>=p.width||state.y>=p.height){state.fault|=Invalid;lanes[lane]=state;return;}
    for(U32 n=0;n<128u&&state.y<p.height;++n){
        const U32 pixel=state.y*p.width+state.x;
        if(pixel>=allocated_pixels){state.fault|=Bounds;break;}
        Record r=records[pixel];U32 ambient=surface[pixel].amb_shadow;
        if(!r.fault&&r.active&&scan_hiq(p,l,depth,angles,pixel,surface[pixel].depth_packed,state.seed,r)&&l.index==p.levels)
            sum_hiq(p,l,angles,pixel,ambient,r);
        ++r.levels;if(l.index==p.levels)++r.repeats;
        r.final_ambient=ambient;r.changed=ambient!=r.initial_ambient;records[pixel]=r;
        if(r.fault){state.fault|=r.fault;break;}
        surface[pixel].amb_shadow=ambient;
        if(++state.x==p.width){state.x=0;state.y+=p.logical_lanes;}
    }
    if(state.y>=p.height)state.finished=1;
    lanes[lane]=state;
}


kernel void native_ao_seed_abi(device uint* out [[buffer(0)]],uint i [[thread_position_in_grid]]) {
    if(i)return;out[0]=1;out[1]=sizeof(mb3d_native_ao::SeedRecord);out[2]=sizeof(mb3d_native_ao::LaneState);out[3]=sizeof(mb3d_native_ao::Launch);
}
namespace mb3d_native_ao {
// Both entries use the same owner, reset, work, fault and cursor lifecycle.
template<bool certified> inline void run_seed_lane(device const Record* records,
    thread const Params& p,thread const Level& l,device LaneState* lanes,U32 reset,
    U32 allocation,U32 lane_allocation,device SeedRecord* seeds,U32 lane,bool additional_invalid,
    device const uchar* certificates,device U32* usage,U32 force_unknown,constant mb3d_native_ao_reuse::BaseTable* table=nullptr) {
    if(lane>=lane_allocation)return;LaneState state=lanes[lane];
    if(!valid(p,l)||!random_mode(p.algorithm)||allocation!=p.width*p.height||lane_allocation!=p.logical_lanes||reset>1||additional_invalid){state.fault|=Invalid;lanes[lane]=state;return;}
    if(reset)state={initial_seed(lane),0,lane,l.index,l.repeat_index,0,0,0};
    if(state.fault||state.finished)return;
    if(state.level!=l.index||state.repeat_index!=l.repeat_index||state.reserved||state.x>=p.width||state.y>=p.height){state.fault|=Invalid;lanes[lane]=state;return;}
    for(uint k=0;k<128&&state.y<p.height;++k){const uint pixel=state.y*p.width+state.x;
        if(pixel>=allocation){state.fault|=Bounds;break;}
        Record r=records[pixel];const uint before=r.rng_advances;U32 used=0;
        SeedRecord plan{1,state.seed,state.seed,0,r.fault,pixel,l.index,l.repeat_index};
        if(!r.fault&&r.active){
            // A conservative envelope covers |direction|<=1.001, rounding,
            // both jitter modes and all samples. Inside it no direction can
            // reach a border break; even zero-radius attempts advance the RNG.
            const float envelope=1.001f*(l.minimum_radius+float(l.samples)*float(l.step))+2.f*float(l.step)+2.f;
            uint margin=0;const bool bounded=isfinite(envelope)&&envelope>0&&envelope<float(min(p.width,p.height));
            if(bounded)margin=uint(ceil(envelope));
            const bool interior=bounded&&state.x>=margin&&state.y>=margin&&margin<p.width-state.x&&margin<p.height-state.y;
            if(interior){plan.shortcut=1;const uint draws=32u*l.samples;
                if(r.samples>p.work_limit||draws>p.work_limit-r.samples)r.fault|=WorkExhausted;
                else{state.seed=advance_seed(state.seed,draws);r.samples+=draws;r.rng_advances+=draws;}
            }else {
                if constexpr(certified)scan_seed_certified(p,l,nullptr,nullptr,pixel,0,state.seed,r,certificates,force_unknown,used,table);
                else scan_hiq<true>(p,l,nullptr,nullptr,pixel,0,state.seed,r);
            }
        }
        plan.end=state.seed;plan.draws=r.rng_advances-before;plan.fault=r.fault;seeds[pixel]=plan;
        if constexpr(certified)usage[pixel]=(1u<<28)|(l.repeat_index<<16)|(l.index<<8)|used;
        if(r.fault){state.fault|=r.fault;break;}
        if(++state.x==p.width){state.x=0;state.y+=p.logical_lanes;}
    }
    if(state.y>=p.height)state.finished=1;lanes[lane]=state;
}
}
kernel void native_ao_seed_lanes(device const mb3d_native_ao::Record* records [[buffer(0)]],
    constant mb3d_native_ao::Params& params [[buffer(1)]],constant mb3d_native_ao::Level& level [[buffer(2)]],
    device mb3d_native_ao::LaneState* lanes [[buffer(3)]],constant uint& reset [[buffer(4)]],
    constant uint& allocation [[buffer(5)]],constant uint& lane_allocation [[buffer(6)]],
    device mb3d_native_ao::SeedRecord* seeds [[buffer(7)]],uint lane [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;Params p=params;Level l=level;
    run_seed_lane<false>(records,p,l,lanes,reset,allocation,lane_allocation,seeds,lane,false,nullptr,nullptr,0);
}
kernel void native_ao_seed_lanes_certified(device const mb3d_native_ao::Record* records [[buffer(0)]],
    
    device mb3d_native_ao::LaneState* lanes [[buffer(3)]],constant uint& reset [[buffer(4)]],
    constant uint& allocation [[buffer(5)]],constant uint& lane_allocation [[buffer(6)]],
    device mb3d_native_ao::SeedRecord* seeds [[buffer(7)]],
    constant mb3d_native_ao_reuse::BaseTable& table [[buffer(8)]],
    device const uchar* certificates [[buffer(9)]],constant uint& certificate_bytes [[buffer(10)]],
    device uint* usage [[buffer(11)]],constant uint& force_unknown [[buffer(12)]],
    constant uint& usage_count [[buffer(13)]],uint lane [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;Params p=table.params;Level l=table.level;
    const bool invalid=table.version!=1u||table.count!=192u||table.reserved[0]||table.reserved[1]||
        certificate_bytes!=p.width*p.height*32u||usage_count!=p.width*p.height||force_unknown>1u;
    run_seed_lane<true>(records,p,l,lanes,reset,allocation,lane_allocation,seeds,lane,invalid,certificates,usage,force_unknown);
}
// One group contains every logical owner. The two barriers preserve the
// old host first-fault checkpoint after each unchanged 128-pixel chunk.
// No lane can enter chunk k+1 when any owner faulted during chunk k.
kernel void native_ao_seed_lanes_collective(device const mb3d_native_ao::Record* records [[buffer(0)]],
    device mb3d_native_ao::LaneState* lanes [[buffer(3)]],constant uint& reset [[buffer(4)]],
    constant uint& allocation [[buffer(5)]],constant uint& lane_allocation [[buffer(6)]],
    device mb3d_native_ao::SeedRecord* seeds [[buffer(7)]],
    constant mb3d_native_ao_reuse::BaseTable& table [[buffer(8)]],
    device const uchar* certificates [[buffer(9)]],constant uint& certificate_bytes [[buffer(10)]],
    device uint* usage [[buffer(11)]],constant uint& force_unknown [[buffer(12)]],
    constant uint& usage_count [[buffer(13)]],constant uint& chunks [[buffer(14)]],
    uint3 position [[thread_position_in_grid]],uint3 group [[threadgroup_position_in_grid]],
    uint3 group_size [[threads_per_threadgroup]],uint3 grid_size [[threads_per_grid]]) {
    using namespace mb3d_native_ao;const uint lane=position.x;Params p=table.params;Level l=table.level;
    // Uniform rejection before any collective, including malformed launches.
    if(lane_allocation<1u||lane_allocation>32u||group_size.x!=lane_allocation||
       group_size.y!=1u||group_size.z!=1u||any(group!=uint3(0))||
       any(grid_size!=uint3(lane_allocation,1,1))||chunks<1u||chunks>8u){
        if(lane<lane_allocation)lanes[lane].fault|=Invalid;
        return;
    }
    threadgroup uint faults[32];
    const bool invalid=table.version!=1u||table.count!=192u||table.reserved[0]||table.reserved[1]||
        certificate_bytes!=p.width*p.height*32u||usage_count!=p.width*p.height||force_unknown>1u;
    for(uint chunk=0;chunk<chunks;++chunk){
        run_seed_lane<true>(records,p,l,lanes,chunk==0?reset:0u,allocation,lane_allocation,seeds,lane,invalid,certificates,usage,force_unknown);
        faults[lane]=lanes[lane].fault;
        threadgroup_barrier(mem_flags::mem_threadgroup);
        uint failed=0;for(uint owner=0;owner<lane_allocation;++owner)failed|=faults[owner];
        // All owners must read the vote before any can overwrite its slot.
        threadgroup_barrier(mem_flags::mem_threadgroup);
        if(failed)break;
    }
}
kernel void native_ao_seeded_pixels(device ExactSurface* surface [[buffer(0)]],device const uint* depth [[buffer(1)]],
    device short* angles [[buffer(2)]],device mb3d_native_ao::Record* records [[buffer(3)]],
    constant mb3d_native_ao::Params& params [[buffer(4)]],constant mb3d_native_ao::Level& level [[buffer(5)]],
    constant mb3d_native_ao::Launch& launch [[buffer(6)]],device const mb3d_native_ao::SeedRecord* seeds [[buffer(7)]],
    uint tid [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;Params p=params;Level l=level;Launch run=launch;uint pixel;
    if(!launch_pixel(p,run,tid,pixel)||pixel>=p.width*p.height)return;Record r=records[pixel];
    const auto plan=seeds[pixel];
    if(!valid(p,l)||!random_mode(p.algorithm)||run.reserved||run.allocated_pixels!=p.output_count||
        plan.version!=1||plan.shortcut>1||plan.reserved[0]||plan.reserved[1]||plan.reserved[2]||plan.pixel!=pixel||plan.level!=l.index||plan.repeat!=l.repeat_index||plan.fault){r.fault|=Invalid;records[pixel]=r;return;}
    if(r.fault)return;uint seed=plan.start,ambient=surface[pixel].amb_shadow,before=r.rng_advances;
    if(r.active&&scan_hiq(p,l,depth,angles,pixel,surface[pixel].depth_packed,seed,r)&&l.index==p.levels)sum_hiq(p,l,angles,pixel,ambient,r);
    if(seed!=plan.end||r.rng_advances-before!=plan.draws)r.fault|=Invalid;
    ++r.levels;if(l.index==p.levels)++r.repeats;r.final_ambient=ambient;r.changed=ambient!=r.initial_ambient;
    records[pixel]=r;if(!r.fault)surface[pixel].amb_shadow=ambient;
}
// Horizontal then vertical ping-pong passes exactly preserve unsigned integer
// averaging (including intentional uint wrap). All source pixels are GPU depth.
kernel void native_ao_wavelet(device const uint* input [[buffer(0)]],device uint* output [[buffer(1)]],
    device uint* faults [[buffer(2)]],constant mb3d_native_ao::Params& params [[buffer(3)]],
    constant uint& step [[buffer(4)]],constant uint& axis [[buffer(5)]],
    constant mb3d_native_ao::Launch& launch [[buffer(6)]],uint tid [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;Params p=params;Launch run=launch;U32 pixel;
    if(!launch_pixel(p,run,tid,pixel)||pixel>=p.width*p.height)return;
    if(!valid(p)||!hiq(p.algorithm)||!step||step>32768u||axis>1u||run.reserved||run.allocated_pixels!=p.output_count){faults[pixel]|=Invalid;return;}
    const I32 x=I32(pixel%p.width),y=I32(pixel/p.width),delta=I32(step);
    U32 a,b;
    if(axis==0u){a=U32(y)*p.width+U32(max(0,x-delta));b=U32(y)*p.width+U32(min(I32(p.width)-1,x+delta));}
    else{a=U32(max(0,y-delta))*p.width+U32(x);b=U32(min(I32(p.height)-1,y+delta))*p.width+U32(x);}
    if(a>=p.width*p.height||b>=p.width*p.height){faults[pixel]|=Bounds;return;}
    output[pixel]=(input[pixel]+(input[a]+input[b])/2u)/2u;
}

#include "native_ao_seed_chunk.metal.h"
