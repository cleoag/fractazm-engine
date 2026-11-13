#pragma once
#include "general_geometry_abi.h"
namespace mb3d_native {
// Diagnostic observation of a completed prefix is distinct from successful
// pixel admission. Faulted records remain failures in the caller.
inline bool valid_smoothing_observation(const NativeGeometryRecord& g,unsigned policy) {
    return g.version==kGeneralGeometryVersion&&valid_difs_carried_reads(g.difs_carried_reads,g.de_calls,g.normal_calls)&&
        g.gradient_range_clamps<=g.de_calls&&valid_fp32_range_exits(g.fp32_range_exits,g.de_calls,g.normal_calls)&&
        !g.formula_fault.reserved[0]&&!g.formula_fault.reserved[1]&&
        g.de_calls<=kGeneralMaxDistanceCalls&&g.formula_work<=kGeneralMaxFormulaWork&&
        valid_smoothing_counters(g.smoothing,g.formula_work,policy,true);
}
}
