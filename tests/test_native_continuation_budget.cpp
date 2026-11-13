#include "render/native_metal/continuation_budget.h"
#include "render/native_metal/continuation_abi.h"
#include <cassert>
#include <iostream>
using namespace mb3d::native_metal::continuation;
int main(){
 constexpr uint64_t mib=1024*1024;
 const PeakCharges coexist{80*mib,24*mib,32*mib,8*mib,4*mib};
 auto independent=plan_pool(coexist,MaxRayFrameBytes,4096,4,true,256*mib);
 assert(independent.admitted&&independent.frames==4096&&independent.peak<=256*mib);
 auto actual=plan_pool(coexist,sizeof(mb3d_native::continuation::RayFrame),4096,4,true,256*mib);
 assert(actual.admitted&&actual.frames==4096&&actual.peak<independent.peak);
 auto dependent=plan_pool(coexist,MaxRayFrameBytes,4,4,false,256*mib);
 assert(dependent.admitted&&dependent.frames==4);
 const uint64_t stride=MaxRayFrameBytes+SlotOverheadBytes,base=148*mib;
 auto limited=plan_pool(coexist,MaxRayFrameBytes,4096,4,true,base+3*stride);
 assert(limited.admitted&&limited.frames==3&&limited.peak==base+3*stride);
 assert(!plan_pool(coexist,MaxRayFrameBytes,4,4,false,base+3*stride).admitted);
 assert(!plan_pool(coexist,MaxRayFrameBytes,4,4,false,base+4*stride-1).admitted);
 assert(plan_pool(coexist,MaxRayFrameBytes,4,4,false,base+4*stride).admitted);
 assert(!plan_pool(coexist,MaxRayFrameBytes,4096,4,true,base+stride-1).admitted);
 for(uint64_t bad:{0ULL,MaxRayFrameBytes+1,std::numeric_limits<uint64_t>::max()})
  assert(!plan_pool({},bad,1,1,true,UINT64_MAX).admitted);
 assert(!plan_pool({},16,0,1,true,UINT64_MAX).admitted);
 assert(!plan_pool({},16,4097,1,true,UINT64_MAX).admitted);
 assert(!plan_pool({},16,4,0,true,UINT64_MAX).admitted);
 assert(!plan_pool({},16,4,65,true,UINT64_MAX).admitted);
 assert(!plan_pool({},16,3,4,false,UINT64_MAX).admitted);
 assert(!plan_pool({UINT64_MAX,1,0,0,0},16,1,1,true,UINT64_MAX).admitted);
 assert(!plan_pool({0,UINT64_MAX,0,0,0},16,1,1,true,UINT64_MAX).admitted);
 for(uint64_t bytes=1;bytes<=MaxRayFrameBytes;bytes+=17){
  auto p=plan_pool({},bytes,1,1,true,MaxRayFrameBytes+SlotOverheadBytes);
  assert(p.admitted&&p.stride%16==0&&p.stride>=bytes+SlotOverheadBytes&&p.peak==p.stride);
 }
 std::cout<<"{\"status\":\"PASS\",\"max_frame_bytes\":"<<MaxRayFrameBytes
          <<",\"max_frames\":"<<MaxActiveFrames<<",\"independent_peak\":"<<independent.peak
          <<",\"dependent_peak\":"<<dependent.peak<<"}\n";
}
