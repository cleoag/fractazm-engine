#pragma once
#include "formula_abi.h"
#include "formula_selector.h"
#include "core/types.h"
#include <string>
#include <vector>

namespace mb3d::native_metal {
struct PreparedFormulaProgram {
    mb3d_native::NativeFormulaProgram device{};
    // DE combination (device.formula_type != 0): the second hybrid part,
    // uploaded directly after device in the same program buffer.
    mb3d_native::NativeFormulaProgram second{};
    std::size_t program_count() const { return device.formula_type ? 2 : 1; }
    // Conservative until a formula's read/write set proves per-pixel reset
    // equivalent to the CPU logical worker's persistent iteration state.
    bool requires_persistent_lane=false;
    std::vector<std::string> names;
    std::vector<mb3d_native::NativeMapDescriptor> maps;
    std::vector<float> map_pixels;
};

// The caller must bind its Native RenderContext and retain the already prepared
// MCT/constant buffers for this call. Copies all device inputs; no pixel work.
// Throws before GPU dispatch for a driver/formula/resource not implemented.
PreparedFormulaProgram PrepareFormulaProgram(const TMCTparameter& mct,
    const THeaderCustomAddon& addon, unsigned work_limit = 1u << 20);

// Internal navigation path: regenerate all scalar/slot inputs, retaining only
// immutable decoded maps from a matching scene snapshot. Never used by Full.
PreparedFormulaProgram PrepareFormulaProgramWithMaps(const TMCTparameter&,
    const THeaderCustomAddon&, const PreparedFormulaProgram* snapshot,
    unsigned work_limit = 1u << 20);

// Convenience preparation for the point/orbit probe. Copies caller records and
// scopes Native arithmetic/context; does not render an image or mutate inputs.
PreparedFormulaProgram PrepareLoadedFormulaProgram(const TMandHeader10& header,
    const THeaderCustomAddon& addon, const std::string& resource_root = {},
    unsigned work_limit = 1u << 20);
// Raw-program probe resources use the same decoder/layout as scene preparation.
// Explicit IDs are unique/nonnegative, at most three nonzero plus optional zero.
// A fresh Native context owns loading; device program/state are never modified.
// All decoded arrays commit together only after successful preparation.
void LoadRawFormulaMapResources(PreparedFormulaProgram& out,
    const std::vector<int>& map_ids, const std::string& resource_root);
mb3d_native::NativeFormulaState MakeFormulaPointState(
    const mb3d_native::NativeFormulaProgram&, float x, float y, float z,
    float stop = 0.001f);
} // namespace mb3d::native_metal
