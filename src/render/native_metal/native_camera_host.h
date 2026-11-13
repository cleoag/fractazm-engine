#pragma once
#include "native_camera_abi.h"
#include "core/types.h"
#include <vector>
namespace mb3d::native_metal {
struct PreparedNativeCamera {
    mb3d_camera::Params device{};
    std::vector<mb3d_camera::Axis> columns,rows,paint_columns,paint_rows;
};
// A frame of every `stride`-th pixel of the full frame, starting at (x, y):
// allocated pixel (i, j) is frame pixel (x+i*stride, y+j*stride). Projection,
// distance thresholds and step sizes stay those of the full frame. Stride 1 is
// the whole frame and changes nothing.
struct Lattice {
    int stride=1,x=0,y=0;
    bool dense() const {return stride==1&&!x&&!y;}
};
// Scene-only: mct must come from this exact effective header and addon. This
// does not load a file, run formulas, normalize stereo away or admit effects.
PreparedNativeCamera PrepareNativeCamera(const TMandHeader10&,const TMCTparameter&,Lattice={});
}
