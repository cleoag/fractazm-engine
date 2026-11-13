#pragma once
#include "general_normals_abi.h"
#include "core/types.h"
namespace mb3d::native_metal {
mb3d_native::NativeNormalParams PrepareGeneralNormals(const TMCTparameter& m,
    const mb3d_native::NativeFormulaProgram& program);
}
