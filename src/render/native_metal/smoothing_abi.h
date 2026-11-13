#pragma once
namespace mb3d_native {
enum : unsigned { kNativeSmoothingVersion=1, kNativeSmoothingMaxCalls=1u<<28 };
struct NativeSmoothingCounters { unsigned calls,recovered,stagnant,invalid; };
inline bool valid_smoothing_counters(NativeSmoothingCounters s,unsigned work,
    unsigned policy,bool allow_invalid=false) {
    if(policy>1||s.calls>work||s.calls>kNativeSmoothingMaxCalls||s.recovered>s.calls)return false;
    unsigned remaining=s.calls-s.recovered;
    if(s.stagnant>remaining)return false;remaining-=s.stagnant;
    if(s.invalid>remaining||(!allow_invalid&&s.invalid))return false;
    return policy!=0||(!s.recovered&&!s.stagnant);
}
#ifndef __METAL_VERSION__
static_assert(sizeof(NativeSmoothingCounters)==16);
#endif
}
