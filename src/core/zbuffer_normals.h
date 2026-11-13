#ifndef MB3D_CORE_ZBUFFER_NORMALS_H
#define MB3D_CORE_ZBUFFER_NORMALS_H

#include "core/types.h"
#include <span>

namespace mb3d {

// ImageProcess.NormalsOnZbuf: replace only foreground normals, after primary
// marching and before hard shadows/AO. Surface rows use GetCalcRect's width.
void RecalculateNormalsFromZBuffer(const TMandHeader10& header,
                                  std::span<TsiLight5> surface);

} // namespace mb3d
#endif
