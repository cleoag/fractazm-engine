#pragma once
#include "comparison/metal_exact/scene_abi.h"
#include "comparison/metal_exact/normal_abi.h"
#include <bit>
#include <cmath>
#include <cstddef>
#include <initializer_list>
namespace mb3d::native_metal {
// Execution-derived capability, not recovery from a faulty record. Examine the
// complete primary/normal allocation before allowing a GPU-to-GPU handoff.
inline bool RequiresGeneralInitialInside(const ExactPixel* pixels,const ExactNormalSamples* normals,size_t count) {
    bool inside=false;
    for(size_t i=0;i<count;++i) {
        const auto& p=pixels[i];
        if(p.fault||p.reserved_tail||normals[i].reserved||p.start_inside>1||p.rm_result>2)return false;
        for(auto word:normals[i].distance)
            if(!std::isfinite(std::bit_cast<double>(word)))return false;
        if(!std::isfinite(std::bit_cast<double>(normals[i].orbit_trap)))return false;
        for(unsigned axis=0;axis<3;++axis)
            for(auto word:{p.ray[axis],p.start[axis],p.position[axis],p.raw_normal[axis]})
                if(!std::isfinite(std::bit_cast<double>(word)))return false;
        for(auto word:{p.depth,p.axial_depth,p.final_de,p.orbit_trap})
            if(!std::isfinite(std::bit_cast<double>(word)))return false;
        for(auto word:{p.step_count,p.smooth_iterations,p.final_stop})
            if(!std::isfinite(std::bit_cast<float>(word)))return false;
        inside|=p.rm_result!=0&&p.start_inside!=0;
    }
    return inside;
}
}
