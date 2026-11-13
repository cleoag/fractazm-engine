#pragma once
#include "native_ao_abi.h"
namespace mb3d_native_ao_chunk {
using mb3d_native_ao::U32;
enum : U32 { Version=1, Width=128 };
// Preparatory scratch is separate from canonical SeedRecord/usage. A failed
// continuation may publish only its actually executed logical prefix.
struct Pixel { U32 prefix,draws,shortcut,used; };
struct Chunk { U32 known,draws,seed,executed; };
struct Run { U32 chunks_per_owner,total_chunks,offset,count; };
#ifndef __METAL_VERSION__
static_assert(sizeof(Pixel)==16&&sizeof(Chunk)==16&&sizeof(Run)==16);
#endif
}
