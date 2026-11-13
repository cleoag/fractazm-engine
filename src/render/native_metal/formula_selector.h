#pragma once
#include "formula_abi.h"
namespace mb3d_native {
// Pipeline selectors are identities, not uploaded ABI fields. Reserving the
// high bit separates the ten public internal IDs from external registry IDs.
enum : U32 { kDisabledFormulaSelector=0xffffffffu, kInternalFormulaSelector=0x80000000u };
inline bool ValidFormulaSelectorIdentity(U32 name_space,U32 id,U32 enabled) {
    return enabled<=1&&name_space<=ExternalFormula&&
        (!enabled||(name_space==InternalFormula?id<10u:id<kInternalFormulaSelector));
}
inline U32 FormulaPipelineSelector(U32 name_space,U32 id,U32 enabled) {
    return !enabled?kDisabledFormulaSelector:name_space==InternalFormula?(kInternalFormulaSelector|id):id;
}
}
