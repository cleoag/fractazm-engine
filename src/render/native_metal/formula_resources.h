#pragma once
#include "formula_abi.h"
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string_view>

namespace mb3d::native_metal {
// Actual map callback arguments of the pinned ordinary Native lowering.
// These are not the Pascal/CPU 'MAP' substring prefetch hints: channel selectors
// and Log2MapPower must never become mandatory GPU assets. Parameter indices
// refer to the uploaded integer-expanded block, not M3P option storage.
struct FormulaMapBinding {
    unsigned id,parameter,option_value;
    std::string_view name;
};
inline constexpr std::array<FormulaMapBinding,9> NativeFormulaMapBindings{{
    {10180,4,3,"ABoxMapSFold"},
    {10006,0,0,"_MapTranslate"},
    {10189,0,0,"HeightMapIFS"},
    {10191,0,0,"SphereHeightMap"},
    {10193,0,0,"CylinderHeightMap"},
    {10194,0,0,"CylinderHMVoid"},
    {10197,0,0,"TorusHMVoid"},
    {10198,0,0,"HeightMap2IFS"},
    {10301,0,0,"SphereHMVoid"},
}};
inline const FormulaMapBinding* NativeFormulaMapRequirement(const mb3d_native::NativeFormulaSlot& slot,
                                                           std::string_view name) {
    if(slot.namespace_id!=mb3d_native::ExternalFormula||slot.enabled!=1)
        throw std::invalid_argument("Native map requirements need external formula identity");
    for(const auto& binding:NativeFormulaMapBindings)if(slot.id==binding.id||name==binding.name) {
        if(slot.id!=binding.id||name!=binding.name||slot.param_count<=binding.parameter)
            throw std::invalid_argument("Native formula map binding identity/parameter mismatch");
        return &binding;
    }
    return nullptr;
}
inline int NativeFormulaMapId(const mb3d_native::NativeFormulaSlot& slot,const FormulaMapBinding& binding) {
    if(slot.namespace_id!=mb3d_native::ExternalFormula||slot.enabled!=1||slot.id!=binding.id||slot.param_count<=binding.parameter)
        throw std::invalid_argument("Native formula map binding is outside uploaded parameters");
    const double value=slot.params[binding.parameter];
    // The integer ABI was expanded before upload. Read exactly what the GPU
    // casts to its callback ID; do not independently round original option data.
    if(!std::isfinite(value)||value<0||value>std::numeric_limits<int>::max()||std::trunc(value)!=value)
        throw std::invalid_argument("Native formula map ID is outside the integer callback range");
    return static_cast<int>(value);
}
}
