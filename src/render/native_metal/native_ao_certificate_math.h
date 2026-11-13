#pragma once
#include "native_ao_certificate_abi.h"
#ifdef __METAL_VERSION__
#define CERT_THREAD thread
#define CERT_CONSTANT constant
#else
#define CERT_THREAD
#define CERT_CONSTANT
#endif
namespace mb3d_native_ao_certificate {
using namespace mb3d_native_ao;
using mb3d_native_ao_reuse::BaseSample;
using mb3d_native_ao_reuse::BaseTable;
enum : U32 { Unknown=0,AlwaysContinue=1,AlwaysBreak=2 };
// Caller has validated Params/Level and pixel ownership. Invalid/unproven
// derived samples remain unknown; the ordered consumer observes their faults.
inline U32 ClassifySampleValidated(CERT_THREAD const Params& p,I32 x,I32 y,
    CERT_THREAD const BaseSample& sample,U32 jitter) {
    if(sample.fault||sample.reserved||jitter>32767u||
       sample.x<=-1073741824||sample.x>=1073741824||
       sample.y<=-1073741824||sample.y>=1073741824)return Unknown;
    // Bounds above, valid dimensions<=4096 and jitter<=32767 make every
    // addition int32-safe. Local radius_squared is finite and zero iff both
    // local integers are zero, even after their conversion to float32.
    const I32 localHighX=sample.x+I32(jitter),localHighY=sample.y+I32(jitter);
    const I32 lowX=sample.x+x,highX=localHighX+x;
    const I32 lowY=sample.y+y,highY=localHighY+y;
    // Both coordinates are checked by the original boundary function before
    // either axis can return a rejection. A possibly faulting range is unknown.
    if(lowX<=-1073741824||highX>=1073741824||
       lowY<=-1073741824||highY>=1073741824)return Unknown;
    const bool pano=p.algorithm==8u||p.algorithm==9u;
    const I32 minX=pano?-I32(p.width):(p.w_lo>1?1-p.w_lo:0);
    const I32 maxX=pano?2*I32(p.width)-1:I32(p.width)-1+p.w_lo;
    const I32 minY=p.h_lo>1?1-p.h_lo:0,maxY=I32(p.height)-1+p.h_lo;
    if(lowX>=minX&&highX<=maxX&&lowY>=minY&&highY<=maxY)return AlwaysContinue;
    const bool zeroPossible=sample.x<=0&&localHighX>=0&&sample.y<=0&&localHighY>=0;
    if(!zeroPossible&&(highX<minX||lowX>maxX||highY<minY||lowY>maxY))return AlwaysBreak;
    return Unknown;
}
// Zero means no certificate. Entries after a proven break are not inspected;
// their faults were never consumed by the original direction loop either.
inline U32 DirectionValidated(CERT_THREAD const Params& p,CERT_THREAD const Level& l,
    I32 x,I32 y,U32 direction,CERT_CONSTANT const BaseTable& table) {
    const U32 jitter=(p.algorithm==5u||p.algorithm==9u)?2u*l.step-1u:l.step-1u;
    for(U32 s=0;s<l.samples;++s){
        const BaseSample sample=table.entries[direction*6u+s];
        const U32 outcome=ClassifySampleValidated(p,x,y,sample,jitter);
        if(outcome==Unknown)return 0;
        if(outcome==AlwaysBreak)return s+1u;
    }
    return l.samples;
}
}
#undef CERT_THREAD
#undef CERT_CONSTANT
