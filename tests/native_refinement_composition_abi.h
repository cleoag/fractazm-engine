#pragma once
#include "native_refinement_probe_abi.h"
namespace p2_refinement {
struct RealState {
 RefinementFrame refinement;DistanceEntryFrame entry;CombinedFrame combined;DistanceFrame distance;
 OrbitReplayFrame replay;OrbitPassFrame orbit;FormulaStepFrame step;WrapperFrame wrapper;BodyFrame body;
 Output output;U32 initialized,done;
};
enum : U32 { RealProbeAbi=0x50325231u };
}
