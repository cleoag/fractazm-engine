#pragma once
#include "native_ao_math.h"
namespace mb3d_native_ao {
inline U32 threshold15_unaligned(U32 lower,U32 next) {
    return (lower>>8)|((next&255u)<<8);
}
inline U32 threshold15_first(U32 center,U32 edge,U32 lower,bool scalar_tail=false,U32 next=0) {
    if(scalar_tail)return (center+((edge+1u+threshold15_unaligned(lower,next))>>1))>>1;
    return ((center+1u)+((edge+1u+lower)>>1))>>1;
}
inline U32 threshold15_middle(U32 center,U32 previous,U32 next) {
    return ((center+1u)+((previous+next+1u)>>1))>>1;
}
inline U32 threshold15_last(U32 center,U32 previous,U32 edge,bool vertical) {
    return ((center+1u)+((previous+edge+U32(vertical))>>1))>>1;
}
inline bool threshold15_encode(U32 packed,I32 subtract,float correction,MB3D_NATIVE_AO_THREAD U32& result) {
    if(!isfinite(correction)||correction<=0||subtract<0||subtract>32767*256)return false;
    if(packed&0x80000000u){result=0;return true;}
    I32 value;
    if(!rounded(float(I32(packed>>8)-subtract)*correction,value))return false;
    result=U32(max(0,min(65535,value)));return true;
}
}
