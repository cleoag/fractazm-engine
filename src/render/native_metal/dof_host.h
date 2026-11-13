#pragma once
#include "dof_abi.h"
#include "core/types.h"
namespace mb3d::native_metal {
// Disabled returns all-zero descriptor. No pixel work or mutable caller state.
mb3d_dof::Params PrepareNativeDOF(const TMandHeader10& header);
}
