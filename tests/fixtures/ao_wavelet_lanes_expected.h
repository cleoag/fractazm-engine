// Independent completed Pascal v3 captures. Provenance: comparison/results/stage-a-20261018/ao-wavelet-lanes-v1/fixture-identity.json
#pragma once
#include <cstdint>
struct AOWaveletLaneFixture { int mode; int repeats; int threads; uint64_t ambHash; float threshold = 2.0f; };
inline constexpr AOWaveletLaneFixture kAOWaveletLaneFixtures[] = {
    {4, 1, 1, 0xa989c09854781fdaULL},
    {4, 1, 2, 0x8a81828babf812fdULL},
    {4, 2, 1, 0xe1124cab1167e918ULL},
    {4, 2, 2, 0x5f73533f90cd0bbfULL},
    {5, 1, 1, 0xf412f3ccc7a360ebULL},
    {5, 1, 2, 0x9038b79d23bcf495ULL},
    {5, 2, 1, 0xbad15bdeb5ef6868ULL},
    {5, 2, 2, 0x9a9033e8babaa4abULL},
};
