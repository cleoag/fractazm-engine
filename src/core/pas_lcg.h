// Shared deterministic LCG to replace Random() across the Pascal <-> C++
// render pipelines. Mirrors PasLcg.pas (patches/PasLcg.pas) byte for byte.
// See docs/PARITY_TRACE.md.
#ifndef MB3D_CORE_PAS_LCG_H
#define MB3D_CORE_PAS_LCG_H

#include <cstdint>

namespace mb3d {

inline uint32_t PasLcgInitFromThread(int threadId) {
    return static_cast<uint32_t>(threadId + 1) * 0x324594A1u + 0x24563487u;
}

inline uint32_t PasLcgNext(uint32_t& state) {
    state = state * 214013u + 2531011u;
    return state;
}

inline double PasLcgU01(uint32_t& state) {
    PasLcgNext(state);
    return static_cast<double>(state >> 8) / 16777216.0;
}

} // namespace mb3d

#endif
