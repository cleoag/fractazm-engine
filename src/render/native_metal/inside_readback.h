#pragma once
// Host-only review candidate. Call after GPU completion and before committing
// siLight/RGB or the loaded header. This never creates or repairs pixel data.
#include "native_inside_reflection_math.h"
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
namespace mb3d_inside {
struct PrimaryFacts {U32 fault,hit,start_inside;};
struct ReflectionFacts {U32 fault,active,initial_primary,otrap,ao_rays,hits,shadow_rays;};
struct PrimaryTotals {std::uint64_t records=0,attempts=0,retries=0,initial_hits=0,interior_surfaces=0,smooth_calls=0;};
struct ReflectionTotals {std::uint64_t records=0,selected_pixels=0,ao_rays=0,absorption_bounces=0,initial_primary=0,cut_ao_skips=0,cut_shadow_limits=0;};
inline std::size_t checked_inside_bytes(std::uint64_t pixels,std::uint64_t budget) {
    if(!pixels||pixels>4096ull*4096ull||pixels>std::numeric_limits<std::size_t>::max()/sizeof(Record)||
       pixels*sizeof(Record)>budget)throw std::invalid_argument("inside record allocation/budget");
    return static_cast<std::size_t>(pixels)*sizeof(Record);
}
inline const char* validate_primary_record(const Params&p,const Record&r,const PrimaryFacts&f){
    if(!valid(p))return "inside policy ABI";
    if(r.version!=1||r.reserved||r.fault||f.fault)return "inside record ABI/fault";
    if(f.hit>1||f.start_inside>1||(!f.hit&&f.start_inside))return "inside pixel classification";
    if(r.retries>1||r.attempts!=1+r.retries||r.start_inside!=f.start_inside)return "inside bounded attempt accounting";
    if(r.retries&&!p.in_and_out)return "inside unexpected retry";
    const I32 wanted=r.retries?(p.initial_inside?0:-1):p.initial_inside;
    if(!flag_word(r.final_inside)||r.final_inside!=wanted)return "inside final policy";
    if(r.smooth_calls!=U32(f.hit&&f.start_inside&&r.final_inside))return "inside smooth DE accounting";
    return nullptr;
}
inline PrimaryTotals validate_primary_records(const Params&p,std::span<const Record>records,std::span<const PrimaryFacts>pixels){
    if(records.size()!=pixels.size()||records.empty()||records.size()>4096ull*4096ull)throw std::invalid_argument("inside record count");
    PrimaryTotals out;
    for(std::size_t i=0;i<records.size();++i){const auto&r=records[i];const auto&f=pixels[i];
        if(const char*why=validate_primary_record(p,r,f))throw std::invalid_argument(why);
        ++out.records;out.attempts+=r.attempts;out.retries+=r.retries;out.initial_hits+=f.hit&&r.start_inside;
        out.interior_surfaces+=f.hit&&r.final_inside;out.smooth_calls+=r.smooth_calls;}
    return out;
}
inline const char* validate_reflection_record(const Params&p,const ReflectionConfig&c,U32 cut_options,
    const ReflectionRecord&r,const ReflectionFacts&f){
    if(!valid(p)||!valid_reflection(c)||(cut_options&~0x77u))return "reflection inside policy ABI";
    if(r.version!=1||r.fault||f.fault||f.active>1||f.initial_primary>1||f.otrap>65535)return "reflection inside ABI/fault";
    if(!f.active){if(r.selected_inside||r.inside_ao_rays||r.absorption_bounces||r.initial_primary||r.cut_ao_skips||r.cut_shadow_limits)
        return "reflection inactive pixel work";return nullptr;}
    const I32 wanted=select_reflection_inside(p,f.otrap);
    if(r.selected_inside!=wanted||!flag_word(r.selected_inside))return "reflection selected policy";
    if(r.initial_primary!=f.initial_primary)return "reflection initial-primary marker";
    if(c.transmission){
        // Refractions toggle the inside state per ray from an outside start:
        // interior AO rays and absorbing hits are subsets of the pixel's work.
        if(wanted||r.inside_ao_rays>f.ao_rays||r.absorption_bounces>f.hits)return "reflection transmission accounting";
    } else {
    if(r.inside_ao_rays>f.ao_rays||(!wanted&&r.inside_ao_rays))return "reflection inside AO accounting";
    // Point AO temporarily clears bIsIFS; scene IFS does not suppress this count.
    if(wanted&&r.inside_ao_rays!=f.ao_rays)return "reflection inside AO missing rays";
    if(r.absorption_bounces!=(material_absorption(p,wanted)?f.hits:0u))return "reflection absorption accounting";
    }
    if(r.cut_ao_skips>f.ao_rays||r.cut_shadow_limits>f.shadow_rays||
       (!(cut_options&7u)&&(r.cut_ao_skips||r.cut_shadow_limits)))return "reflection cut work accounting";
    return nullptr;
}
inline ReflectionTotals validate_reflection_records(const Params&p,const ReflectionConfig&c,U32 cuts,
    std::span<const ReflectionRecord>records,std::span<const ReflectionFacts>pixels){
    if(records.size()!=pixels.size()||records.empty()||records.size()>4096ull*4096ull)throw std::invalid_argument("reflection inside record count");
    ReflectionTotals out;
    for(std::size_t i=0;i<records.size();++i){const auto&r=records[i];const auto&f=pixels[i];
        if(const char*why=validate_reflection_record(p,c,cuts,r,f))throw std::invalid_argument(why);
        ++out.records;out.selected_pixels+=f.active&&r.selected_inside;out.ao_rays+=r.inside_ao_rays;
        out.absorption_bounces+=r.absorption_bounces;out.initial_primary+=r.initial_primary;
        out.cut_ao_skips+=r.cut_ao_skips;out.cut_shadow_limits+=r.cut_shadow_limits;}
    return out;
}
}
