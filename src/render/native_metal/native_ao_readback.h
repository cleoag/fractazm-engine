#pragma once
// Validation only. No CPU ambient values are synthesized or repaired here.
#include "native_ao_math.h"
#include "native_ao_certificate_abi.h"
#include "native_ao_seed_chunk_abi.h"
#include "native_ao_de_math.h"
#include "seed_scan_plan.h"
#include "general_geometry_abi.h"
#include "comparison/metal_exact/shadow_abi.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <stdexcept>

namespace mb3d_native_ao {
struct Totals {
    std::uint64_t records=0,active=0,changed=0,samples=0,depth_reads=0,border_rejects=0,rng_advances=0;
    std::uint64_t levels=0,repeats=0,de_queries=0,de_steps=0,cut_skips=0,rays=0,refinements=0,formula_work=0;
    std::uint64_t smoothing_calls=0,smoothing_recovered=0,smoothing_stagnant=0,smoothing_invalid=0;
    std::uint64_t fp32_range_exits=0,fp32_range_exit_pixels=0;
    std::uint64_t difs_carried_reads=0,difs_carried_pixels=0; // DE AO only
};
inline void require_ao(bool yes,const char* why){if(!yes)throw std::invalid_argument(why);}
inline void validate_markers(std::span<const U32>profile,std::span<const U32>screen,std::span<const U32>de) {
    const U32 wanted_profile[]={2,4,32},wanted_screen[]={Version,336,80,64,32,32,32,32,32},wanted_de[]={DEVersion,208,128,224,32};
    require_ao(profile.size()==3&&screen.size()==9&&de.size()==5,"AO marker allocation");
    require_ao(std::equal(profile.begin(),profile.end(),wanted_profile)&&
        std::equal(screen.begin(),screen.end(),wanted_screen)&&std::equal(de.begin(),de.end(),wanted_de),"AO actual library/ABI marker");
}
inline void validate_wavelet_faults(std::span<const U32>faults,std::size_t count) {
    require_ao(faults.size()==count&&count>0,"AO wavelet fault count");
    for(U32 fault:faults)require_ao(!fault,"AO wavelet bounds/fault");
}
inline void validate_certificate_table(const mb3d_native_ao_reuse::BaseTable& table,const Params& p,const Level& l) {
    require_ao(table.version==1&&table.count==192&&!table.reserved[0]&&!table.reserved[1]&&
        std::memcmp(&table.params,&p,sizeof(p))==0&&std::memcmp(&table.level,&l,sizeof(l))==0,
        "AO certificate descriptor binding");
    // Entry faults are consumed in original sample order. A later entry after
    // a guaranteed border rejection must not reject an otherwise valid frame.
}
inline U32 certificate_count(U32 record,const Level& level) {
    const U32 tag=(1u<<28)|(level.repeat_index<<16)|(level.index<<8);
    require_ao((record&~255u)==tag&&(record&255u)<=32,"AO certificate record identity/count");
    return record&255u;
}
inline U32 validate_certificate_usage(U32 record,U32 known,const Level& level,bool inactive,bool shortcut,bool forced_unknown) {
    const U32 used=certificate_count(record,level);
    require_ao(used<=known&&(!(inactive||shortcut||forced_unknown)||used==0),"AO certificate actual use");
    return used;
}
inline std::size_t checked_ao_bytes(std::uint64_t count,std::uint64_t stride,std::uint64_t budget) {
    require_ao(count&&stride&&count<=std::numeric_limits<std::size_t>::max()/stride,"AO allocation multiplication");
    const auto bytes=count*stride;require_ao(bytes<=budget,"AO allocation budget");return std::size_t(bytes);
}
inline std::uint64_t screen_scratch_bytes(const Params&p,bool certificates=false,bool chunk_prefix=false) {
    require_ao(valid(p),"AO scratch params");const std::uint64_t n=std::uint64_t(p.width)*p.height;
    require_ao(!certificates||random_mode(p.algorithm),"Certificates require random screen AO");
    require_ao(!chunk_prefix||(certificates&&p.logical_lanes<=32),"Chunk prefixes require certified collective owners");
    // Includes output, records, all auxiliary buffers and both marker buffers;
    // caller input and the existing Metal context remain caller-owned.
    std::uint64_t bytes=14*sizeof(U32)+std::uint64_t(p.output_count)*(sizeof(ExactSurface)+sizeof(Record));
    if(hiq(p.algorithm)){bytes+=n*(2*sizeof(std::uint32_t)+32*sizeof(std::int16_t)+sizeof(std::uint32_t))+std::uint64_t(p.logical_lanes)*sizeof(LaneState);
        if(random_mode(p.algorithm))bytes+=n*sizeof(SeedRecord)+4*sizeof(U32);
        if(certificates){
            // Private bundle, two ABI buffers, atomic fault, 32-byte direction
            // blocks and separate GPU classification/actual-use records.
            bytes+=sizeof(mb3d_native_ao_reuse::BaseTable)+9*sizeof(U32)+n*40;
            if(chunk_prefix){
                const std::uint64_t items=((p.height+p.logical_lanes-1u)/p.logical_lanes)*p.width;
                const auto chunks=((items+127u)/128u)*p.logical_lanes;
                bytes+=n*sizeof(mb3d_native_ao_chunk::Pixel)+chunks*sizeof(mb3d_native_ao_chunk::Chunk)+4*sizeof(U32);
            }
        }
    }
    else if(p.algorithm<=1)bytes+=n*(std::uint64_t(p.levels)+1)*sizeof(std::uint16_t)+
        std::uint64_t(p.output_count)*sizeof(PreprocessRecord)+std::uint64_t(p.height)*8+sizeof(Range15)+sizeof(MedianState);
    else throw std::invalid_argument("DE AO requires separate scratch plan");
    return bytes;
}
inline std::uint64_t de_scratch_bytes(const DEParams&p) {
    require_ao(valid_de(p),"DE AO scratch params");
    std::uint64_t extra=6*sizeof(U32); // Pixel ABI and minimal seed buffer.
    if(p.first_step_random){
        const auto scan=mb3d::native_metal::PlanSeedScan(p.width,p.height,p.task_count,
            mb3d_seed_scan::Contiguous,p.task_rows,mb3d_seed_scan::DrawCounts,p.initial_seed);
        extra+=scan.words*sizeof(U32)+scan.scratch_bytes+sizeof(U32);
    }
    return extra+14*sizeof(U32)+std::uint64_t(p.width)*p.height*(sizeof(ExactSurface)+sizeof(DERecord))+
        std::uint64_t(p.task_count)*sizeof(DETaskState)+2*sizeof(mb3d_native::NativeFormulaProgram);
}
inline void unchanged_surface(const ExactSurface& before,const ExactSurface& after) {
    require_ao(!before.fault&&!after.fault,"AO surface fault");
    require_ao(!before.reserved[0]&&!before.reserved[1]&&!before.reserved[2]&&
               !after.reserved[0]&&!after.reserved[1]&&!after.reserved[2],"AO surface reserved");
    auto a=before,b=after;a.amb_shadow=b.amb_shadow=0;
    require_ao(std::memcmp(&a,&b,sizeof(a))==0,"AO changed non-ambient surface fields");
    require_ao(before.amb_shadow<=65535&&after.amb_shadow<=65535,"AO ambient storage range");
}
inline void common_record(const Record&r,const ExactSurface&before,const ExactSurface&after,U32 algorithm,U32 limit) {
    require_ao(r.version==Version&&!r.fault&&r.algorithm==algorithm&&r.active<=1&&r.changed<=1,"AO record identity/fault");
    require_ao(r.initial_ambient==before.amb_shadow&&r.final_ambient==after.amb_shadow&&
        r.changed==U32(before.amb_shadow!=after.amb_shadow),"AO output/record binding");
    require_ao(r.samples<=limit&&r.depth_reads<=r.samples+1u&&r.border_rejects<=r.samples&&r.rng_advances<=r.samples,
        "AO work counters");unchanged_surface(before,after);
}
inline void add(Totals&t,const Record&r) {
    ++t.records;t.active+=r.active;t.changed+=r.changed;t.samples+=r.samples;t.depth_reads+=r.depth_reads;
    t.border_rejects+=r.border_rejects;t.rng_advances+=r.rng_advances;t.levels+=r.levels;t.repeats+=r.repeats;
    t.de_queries+=r.de_queries;t.de_steps+=r.de_steps;t.cut_skips+=r.cut_skips;
}
inline Totals validate_screen(const Params&p,std::span<const ExactSurface>before,
    std::span<const ExactSurface>after,std::span<const Record>records) {
    require_ao(valid(p)&&p.algorithm<=9,"AO screen params");
    require_ao(before.size()==p.output_count&&after.size()==before.size()&&records.size()==before.size(),"AO screen allocation counts");
    Totals totals;const auto prefix=std::size_t(p.width)*p.height;
    for(std::size_t i=0;i<records.size();++i){const auto&r=records[i];common_record(r,before[i],after[i],p.algorithm,p.work_limit);
        const bool active=i<prefix&&!(before[i].depth_packed&0x80000000u);
        require_ao(r.active==U32(active)&&!r.de_queries&&!r.de_steps&&!r.cut_skips,"AO screen active/DE accounting");
        if(i>=prefix){require_ao(!r.samples&&!r.levels&&!r.repeats&&!r.depth_reads&&!r.border_rejects&&!r.rng_advances,"AO tail must not be sampled");
            require_ao(after[i].amb_shadow==(random_mode(p.algorithm)?0u:before[i].amb_shadow),"AO full-frame-prefix tail policy");}
        else if(hiq(p.algorithm)){
            require_ao(r.levels==p.levels*p.repeats&&r.repeats==p.repeats,"AO missing HiQ level/repeat");
            if(!active)require_ao(!r.samples&&!r.depth_reads&&!r.border_rejects&&!r.rng_advances,"AO background sampled");
            else require_ao(r.samples>0&&after[i].amb_shadow<=16383,"AO active sample/range");
            require_ao(random_mode(p.algorithm)?r.rng_advances==r.samples:r.rng_advances==0,"AO RNG draws not bound to candidates");
        }else{
            require_ao(r.levels==(active?p.levels:0)&&r.repeats==U32(active)&&!r.rng_advances,"AO threshold completion");
            if(active)require_ao(r.samples>0&&after[i].amb_shadow<=16383,"AO threshold sample/range");
            else require_ao(!r.samples&&!r.depth_reads&&!r.border_rejects,"AO threshold background sampled");
        }
        add(totals,r);
    }
    return totals;
}
inline void validate_random_lanes(const Params&p,const Level&level,std::span<const LaneState>lanes) {
    require_ao(valid(p,level)&&random_mode(p.algorithm)&&lanes.size()==p.logical_lanes,"AO random lane allocation");
    for(U32 i=0;i<p.logical_lanes;++i){const auto&l=lanes[i];
        const U32 expected_y=i+((p.height-1u-i)/p.logical_lanes+1u)*p.logical_lanes;
        require_ao(!l.fault&&!l.reserved&&l.finished==1&&l.x==0&&l.y==expected_y&&
            l.level==level.index&&l.repeat_index==level.repeat_index,"AO unfinished/random lane identity");}
}
inline void validate_threshold_preprocess(const Params&p,const Range15&range,const MedianState&median,
    std::span<const PreprocessRecord>records) {
    require_ao(valid(p)&&p.algorithm<=1&&records.size()==p.output_count,"AO threshold allocation");
    require_ao(range.version==Version&&!range.fault&&range.minimum<range.maximum&&range.maximum<=32768&&
        isfinite(range.correction)&&range.correction>0&&isfinite(range.angle_scale)&&range.angle_scale>0&&
        isfinite(range.threshold)&&range.threshold>0&&range.subtract==I32(range.minimum<<8),"AO threshold range result");
    const U32 n=p.width*p.height,interior=(p.width>2?p.width-2:0)*(p.height>2?p.height-2:0);
    require_ao(median.version==Version&&!median.fault&&!median.reserved&&median.finished==1&&median.cursor==n&&
        median.processed==interior&&median.reads>=interior&&median.reads<=8u*interior&&median.writes<=interior,"AO median continuation result");
    for(U32 i=0;i<p.output_count;++i){const auto&r=records[i];
        require_ao(r.version==Version&&!r.fault&&!r.reserved[0]&&!r.reserved[1],"AO threshold preprocessing fault");
        if(i<n)require_ao(r.normalizations==1&&r.wavelet_passes==2*(p.levels-1)&&
            r.writes==1+r.wavelet_passes&&r.reads>=1+3*r.wavelet_passes&&r.reads<=1+4*r.wavelet_passes,"AO missing normalization/wavelet records");
        else require_ao(!r.normalizations&&!r.wavelet_passes&&!r.reads&&!r.writes,"AO threshold tail preprocess");
    }
}
// Rays (and so first-step draws) of the DE AO pixel at index i when active:
// the host replica of de_local_rays' count, which validate_de checks per pixel.
inline U32 de_expected_rays(const DEParams&p,std::size_t i) {
    const int y=int(i/p.width)+p.top;
    U32 rays=p.quality?(p.dither?U32(float(y%int(p.dither+1u))*.5f/float(p.dither)>.1f):1u):3u;
    for(U32 row=1;row<=p.quality;++row)rays+=p.row_counts[row];
    return rays;
}
inline bool de_active(const ExactSurface&s){return (s.depth_packed>>16)<32768&&s.si_gradient<32768;}
inline Totals validate_de(const DEParams&p,std::span<const ExactSurface>before,
    std::span<const ExactSurface>after,std::span<const DERecord>records,std::span<const DETaskState>tasks,U32 smoothing_policy=1,bool independent=false) {
    require_ao(valid_de(p),"DE AO params");const auto n=std::size_t(p.width)*p.height;
    require_ao(before.size()==n&&after.size()==n&&records.size()==n&&tasks.size()==(independent?0:p.task_count),"DE AO allocation counts");
    Totals totals;
    for(std::size_t i=0;i<n;++i){const auto&r=records[i];const auto&c=r.common;
        common_record(c,before[i],after[i],DistanceEstimate,p.work_limit);
        require_ao(mb3d_native::valid_smoothing_counters(r.smoothing,r.formula_work,smoothing_policy),"DE AO smoothing status/work");
        require_ao(r.completed==1&&!r.formula_fault&&r.selected_inside<=1,"DE AO incomplete/fault record");
        require_ao(r.fp32_range_exits<=std::uint64_t(r.formula_calls)*mb3d_native::kNativeRangeExitsPerDistance,"DE AO FP32 range exits");
        require_ao(mb3d_native::valid_difs_carried_reads(r.difs_carried_reads,r.formula_calls,0)&&!r.reserved[0]&&!r.reserved[1]&&!r.reserved[2],"DE AO dIFS carried reads");
        const bool active=de_active(before[i]);
        require_ao(c.active==U32(active)&&!c.border_rejects&&c.levels==U32(active)&&c.repeats==U32(active),"DE AO active completion");
        if(!active)require_ao(!after[i].amb_shadow&&!r.rays&&!r.refinements&&!r.formula_work&&!r.formula_calls&&
            !c.samples&&!c.depth_reads&&!c.de_queries&&!c.de_steps&&!c.cut_skips&&!c.rng_advances&&!r.selected_inside&&
            !r.difs_carried_reads,"Inactive DE AO work");
        else{
            require_ao(r.rays==de_expected_rays(p,i)&&r.refinements<=12&&r.selected_inside==U32(de_selected_inside(p.initial_inside,p.in_and_out,before[i].o_trap)!=0),"DE AO ray/policy/refine accounting");
            require_ao(c.rng_advances==(p.first_step_random?r.rays:0u)&&c.de_steps>=r.rays&&c.cut_skips<=r.rays&&
                c.de_queries>=1+r.rays-c.cut_skips&&r.formula_calls==c.de_queries&&r.formula_work>0&&
                c.samples>=c.de_queries+c.de_steps&&after[i].amb_shadow<=16383,"DE AO missing actual work");
        }
        totals.smoothing_calls+=r.smoothing.calls;totals.smoothing_recovered+=r.smoothing.recovered;
        totals.smoothing_stagnant+=r.smoothing.stagnant;totals.smoothing_invalid+=r.smoothing.invalid;
        add(totals,c);totals.rays+=r.rays;totals.refinements+=r.refinements;totals.formula_work+=r.formula_work;
        totals.fp32_range_exits+=r.fp32_range_exits;totals.fp32_range_exit_pixels+=r.fp32_range_exits!=0;
        totals.difs_carried_reads+=r.difs_carried_reads;totals.difs_carried_pixels+=r.difs_carried_reads!=0;
    }
    for(U32 i=0;i<tasks.size();++i){const auto&t=tasks[i];const U32 pixels=std::min(p.task_rows,p.height-i*p.task_rows)*p.width;
        require_ao(t.version==Version&&!t.fault&&t.finished==1&&t.cursor==pixels&&t.processed==pixels&&
            !t.reserved[0]&&!t.reserved[1]&&!t.reserved[2],"DE AO incomplete persistent task");}
    return totals;
}
}
