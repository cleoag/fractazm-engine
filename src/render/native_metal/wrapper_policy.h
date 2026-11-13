#pragma once
#include "formula_abi.h"
namespace mb3d_native {
// Qualification is for this closed ordered chain, not all callers of these
// formulas. The shader's Makin fixed path is diagnostic-only: production
// never selects policy1 for an enabled Makin slot, including mixed chains.
inline U32 ProductionWrapperPolicy(const NativeFormulaProgram& p) {
    for(const auto& s:p.slots)
        if(s.enabled && s.namespace_id==ExternalFormula && s.id==9401)return 0;
    if(p.version!=kFormulaAbiVersion || p.driver!=Hybrid4DAnalytic ||
       p.slot_count!=6 || p.start_slot!=0 || p.repeat_slot!=0 || p.end_slot!=2 || p.de_option!=6)return 0;
    constexpr U32 ids[]{10350,5823,10343},counts[]{25,6,21},traits[]{0,0,2};
    constexpr I32 options[]{6,-1,6};
    for(U32 i=0;i<kFormulaSlots;++i) {
        const auto& s=p.slots[i];
        if(i>=3) {if(s.enabled)return 0;continue;}
        if(s.version!=kFormulaAbiVersion || s.namespace_id!=ExternalFormula ||
           s.id!=ids[i] || s.enabled!=1 || s.param_count!=counts[i] ||
           s.traits!=traits[i] || s.de_option!=options[i] || s.wrapper_flags!=1 || s.difs_style)return 0;
    }
    return 1;
}
}
