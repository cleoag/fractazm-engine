#include "render/native_metal/continuation_abi.h"
#include "render/native_metal/continuation_budget.h"
#include <iostream>
#include <cassert>
using namespace mb3d_native::continuation;
int main(){
 static_assert(sizeof(Header)==20&&sizeof(ObserverBindings)==16);
 static_assert(MaxDepth==mb3d::native_metal::continuation::MaxActivationDepth);
 static_assert(MaxFrameBytes==mb3d::native_metal::continuation::MaxRayFrameBytes);
 assert(alignof(RayFrame)==16);
 // Output doubles as source for Metal compile-time host/device layout checks.
 std::cout<<"#include \"src/render/native_metal/continuation_abi.h\"\n";
 #define SIZE(T) std::cout<<"static_assert(sizeof(mb3d_native::continuation::" #T ")=="<<sizeof(T)<<",\"host/device layout\");\n";
 SIZE(Header) SIZE(OwnerFrame) SIZE(InsideFrame) SIZE(PrimaryFrame)
 SIZE(RefinementFrame) SIZE(NormalFrame) SIZE(NormalSampleFrame) SIZE(DistanceEntryFrame)
 SIZE(CombinedFrame) SIZE(DistanceFrame) SIZE(OrbitReplayFrame) SIZE(OrbitPassFrame)
 SIZE(FormulaStepFrame) SIZE(WrapperFrame) SIZE(BodyFrame) SIZE(RayFrame)
 #undef SIZE
 #define RAW_FRAME(ID) std::cout<<"static_assert(sizeof(mb3d_native::continuation_" #ID "::Frame)=="<<sizeof(mb3d_native::continuation_##ID::Frame)<<",\"raw frame layout\");\n";std::cout<<"static_assert(__builtin_offsetof(mb3d_native::continuation_" #ID "::Frame,result)=="<<offsetof(mb3d_native::continuation_##ID::Frame,result)<<",\"raw return layout\");\n";
 RAW_FRAME(10274) RAW_FRAME(10265) RAW_FRAME(10220) RAW_FRAME(10321) RAW_FRAME(10089)
 #undef RAW_FRAME
 #define OFFSET(N) std::cout<<"static_assert(__builtin_offsetof(mb3d_native::continuation::RayFrame," #N ")=="<<offsetof(RayFrame,N)<<",\"host/device layout\");\n";
 OFFSET(lane) OFFSET(geometry) OFFSET(pixel) OFFSET(owner) OFFSET(inside)
 OFFSET(primary) OFFSET(refinement) OFFSET(normal) OFFSET(sample) OFFSET(de)
 OFFSET(combined) OFFSET(distance) OFFSET(replay) OFFSET(orbit) OFFSET(step) OFFSET(wrapper) OFFSET(body)
 #undef OFFSET
 std::cout<<"kernel void p2_layout_probe(device uint* out [[buffer(0)]]) {out[0]=sizeof(mb3d_native::continuation::RayFrame);}\n";
}
