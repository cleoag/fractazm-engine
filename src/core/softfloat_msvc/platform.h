#pragma once

// Project-owned configuration for the pinned SoftFloat sources. MSVC uses
// upstream's portable integer primitives instead of GCC builtins or __int128.
#ifndef _MSC_VER
#error "This SoftFloat platform configuration requires MSVC."
#endif

#define LITTLEENDIAN 1
#define INLINE static inline
#define THREAD_LOCAL __declspec(thread)
