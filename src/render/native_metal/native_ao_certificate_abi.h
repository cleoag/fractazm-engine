#pragma once
#include "native_ao_abi.h"
namespace mb3d_native_ao_reuse {
using namespace mb3d_native_ao;
struct BaseSample { I32 x,y;U32 fault,reserved; };
struct BaseTable { U32 version,count,reserved[2];Params params;Level level;BaseSample entries[192]; };
#ifndef __METAL_VERSION__
static_assert(sizeof(BaseSample)==16);
static_assert(sizeof(BaseTable)==3504);
#endif
}
