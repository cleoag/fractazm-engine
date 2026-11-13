#pragma once
#include "native_primary_probe_abi.h"
namespace p2_inside {
enum : mb3d_native::U32 { SyntheticProbeAbi=0x50324953u,RealProbeAbi=0x50324952u };
struct Input {p2_primary::Input base;mb3d_inside::Params policy;};
struct Output {p2_primary::Output base;mb3d_inside::Record activity;};
using State=p2_primary::State;using RealState=p2_primary::RealState;
}
