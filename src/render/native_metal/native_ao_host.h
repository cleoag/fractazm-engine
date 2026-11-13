#pragma once
#include "native_ao_abi.h"
#include "native_ao_de_abi.h"
#include "core/types.h"
#include <vector>
namespace mb3d {
struct NativeAOPreparation {
    mb3d_native_ao::Params params{};
    std::vector<mb3d_native_ao::Level> levels;
};
// Only ordinary screen-space AO. Full DE AO uses a separate camera/formula
// descriptor; it cannot be silently routed into a depth-buffer approximation.
NativeAOPreparation PrepareNativeScreenAO(const TMandHeader10& header,
    unsigned output_width,unsigned output_height,unsigned logical_workers,
    unsigned work_limit=1u<<24);
mb3d_native_ao::DEParams PrepareNativeDistanceAO(const TMandHeader10& header,
    const TMCTparameter& prepared,unsigned output_width,unsigned output_height,
    unsigned logical_workers,unsigned max_steps=4096,unsigned work_limit=65536);
}
