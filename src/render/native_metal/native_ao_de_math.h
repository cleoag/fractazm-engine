#pragma once
#include "native_ao_de_abi.h"
#include "native_ao_math.h"

namespace mb3d_native_ao {
#ifdef __METAL_VERSION__
using metal::floor;
#else
using std::floor;
#endif
inline bool valid_de(DEParams p) {
    if(p.version!=Version||!p.width||!p.height||p.width>4096||p.height>4096||
       !p.full_width||!p.full_height||p.full_width>4096||p.full_height>4096||
       !p.task_rows||p.task_rows>p.height||p.task_count!=(p.height+p.task_rows-1u)/p.task_rows||
       p.quality>3||p.dither>255||p.first_step_random>1||!p.max_steps||p.max_steps>65536||
       !p.work_limit||p.work_limit>0x7fffffffu||p.vary_stop>1||p.in_and_out>1||
       (p.cut_options&~0x77u)||p.reserved||p.padding[0]!=0||p.padding[1]!=0||p.cut[3]!=0||
       p.left< -1073741824||p.top< -1073741824||p.left>1073741824||p.top>1073741824)return false;
    const float positive[]={p.base_stop,p.step_width,p.max_distance_base,p.z_corr,p.z_mul,
        p.step_multiplier,p.angular_spacing,p.correction_weight};
    for(float value:positive)if(!isfinite(value)||value<=0)return false;
    if(!isfinite(p.stop_factor)||p.stop_factor<0||!isfinite(p.minimum_dot)||
       p.minimum_dot< -1||p.minimum_dot>1||p.step_multiplier<=1||p.correction_weight>1)return false;
    U32 rays=p.quality?1u:3u;
    for(U32 row=0;row<4;++row){
        if((row==0||row>p.quality)&&p.row_counts[row])return false;
        if(row&&row<=p.quality){if(!p.row_counts[row]||p.row_counts[row]>32)return false;rays+=p.row_counts[row];}
    }
    if(rays>33)return false;
    for(U32 i=0;i<3;++i){
        if(!isfinite(p.cut[i])||p.view_matrix[i][3]!=0)return false;
        for(U32 j=0;j<3;++j)if(!isfinite(p.view_matrix[i][j]))return false;
    }
    return true;
}
// CPU std::lround for the final nonnegative AO amount, with checked cast.
inline bool de_ambient(float amount,U32 rays,MB3D_NATIVE_AO_THREAD U32& result) {
    if(!rays||rays>33||!isfinite(amount)||amount<0)return false;
    const float value=16383.0f*(1.0f-amount/float(rays));
    if(!isfinite(value)||value>=65535.5f)return false;
    result=value<=0?0u:U32(floor(value+0.5f));return true;
}
inline U32 de_selected_inside(U32 original,U32 in_and_out,U32 trap) {
    return in_and_out?U32((trap&0x8000u)==0):original;
}
inline float de_random_factor(MB3D_NATIVE_AO_THREAD U32& seed) {
    seed=next_seed(seed);return (float(seed>>8)*(1.0f/16777216.0f))*1.5f+0.5f;
}
}
