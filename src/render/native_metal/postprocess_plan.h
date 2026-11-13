#pragma once
// Pure host planning shared by admission and dispatch. No Metal API or device work.
#include "dof_abi.h"
#include "zbuffer_abi.h"
#include "comparison/metal_exact/color_abi.h"
#include "comparison/metal_exact/shadow_abi.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace mb3d::native_metal {
struct DOFLimits {
    bool spatial_candidates=true;
    uint32_t tile_batch=8,source_slice=65536,gather_slice=512,sort_slice=65536;
    uint64_t sort_work_limit=uint64_t(1)<<28,gather_work_limit=uint64_t(1)<<31;
    uint64_t candidate_scan_work_limit=uint64_t(1)<<32;
};

struct DOFAllocationPlan {
    uint32_t pixels=0,candidate_capacity=0;
    uint64_t sources=0,order=0,global=0,pass=0,sort_state=0;
    uint64_t tiles=0,candidates=0,ranks=0,spatial_records=0,accumulators=0,rgb=0,marker=0;
    uint64_t bytes() const {
        return sources+order+global+pass+sort_state+tiles+candidates+ranks+
               spatial_records+accumulators+2*rgb+marker;
    }
};

inline DOFAllocationPlan PlanNativeDOF(const mb3d_dof::Params& p,const DOFLimits& l={}) {
    using namespace mb3d_dof;
    if(!p.version) {
        const Params zero{};
        if(std::memcmp(&p,&zero,sizeof(p)))throw std::invalid_argument("Malformed disabled Native DOF descriptor");
        return {};
    }
    if(p.version!=Version||!p.width||!p.height||p.width>4096||p.height>4096||p.plain>1||p.passes<1||p.passes>4||
       !l.tile_batch||l.tile_batch>64||!l.source_slice||l.source_slice>MaxSlice||!l.gather_slice||l.gather_slice>MaxSlice||
       !l.sort_slice||l.sort_slice>MaxSlice||!l.sort_work_limit||!l.gather_work_limit||!l.candidate_scan_work_limit||
       p.reserved0||p.reserved1||p.reserved2||p.reserved_float!=0||
       !std::isfinite(p.clip_radius)||p.clip_radius<.1f||p.clip_radius>1000||
       !std::isfinite(p.aperture)||p.aperture<.0001f||p.aperture>2||
       !std::isfinite(p.inverse_z_mul)||p.inverse_z_mul<=0||!std::isfinite(p.inverse_z_corr)||p.inverse_z_corr<=0||
       !std::isfinite(p.fov_factor)||p.fov_factor<0||!std::isfinite(p.focus_low)||!std::isfinite(p.focus_high)||p.focus_low>p.focus_high)
        throw std::invalid_argument("Invalid Native DOF allocation/dispatch descriptor");
    const uint64_t n=uint64_t(p.width)*p.height,b=l.tile_batch;
    // Round(radius+.5) can exceed ceil(radius) at odd integer radii.
    const uint64_t r=uint64_t(std::min(1000.f,std::ceil(std::clamp(p.clip_radius,.1f,1000.f)+.5f)));
    const auto cap=uint32_t(std::min(n,(16+2*r)*(16+2*r)));
    return {uint32_t(n),cap,n*sizeof(Source),n*sizeof(Item),sizeof(Global),sizeof(Pass),sizeof(SortState),
            b*sizeof(Tile),b*cap*sizeof(uint32_t),n*sizeof(uint32_t),b*sizeof(SpatialRecord),
            b*256*sizeof(Accum),n*sizeof(ExactColorPixel),9*sizeof(uint32_t)};
}

inline uint64_t NativeDOFAllocationBytes(const mb3d_dof::Params& p,const DOFLimits& l={}) {
    return PlanNativeDOF(p,l).bytes();
}

struct ZNormalsAllocationPlan {
    uint64_t coordinates=0,records=0,status=0,marker=0,surface=0;
    uint64_t bytes() const {return coordinates+records+status+marker+surface;}
};

inline ZNormalsAllocationPlan PlanNativeZNormals(const mb3d_zn::Params& p) {
    using namespace mb3d_zn;
    if(!p.version) {
        const Params zero{};
        if(std::memcmp(&p,&zero,sizeof(p)))throw std::invalid_argument("Malformed disabled Native Z normals descriptor");
        return {};
    }
    if(p.version!=Version||!p.width||!p.height||p.width>MaxExtent||p.height>MaxExtent||
       !p.full_width||!p.full_height||p.full_width>MaxExtent||p.full_height>MaxExtent||p.mode>3||
       p.left< -4096||p.left>4096||p.top< -4096||p.top>4096||p.count!=p.width*p.height||
       p.reserved[0]||p.reserved[1]||p.reserved[2]||p.reserved_float!=0||
       !std::isfinite(p.step)||p.step<=0||!std::isfinite(p.fov)||p.fov<0||
       !std::isfinite(p.aspect)||p.aspect<=0||!std::isfinite(p.optic_z)||p.optic_z<=0||
       !std::isfinite(p.inverse_z_mul)||p.inverse_z_mul<=0||!std::isfinite(p.depth_correction)||p.depth_correction<=0||
       !std::isfinite(p.stop_factor))throw std::invalid_argument("Invalid Native Z normals allocation/dispatch descriptor");
    const uint64_t n=p.count;
    return {n*sizeof(Coordinate),n*sizeof(Record),sizeof(uint32_t),5*sizeof(uint32_t),n*sizeof(ExactSurface)};
}

inline uint64_t NativeZNormalsAllocationBytes(const mb3d_zn::Params& p) {
    return PlanNativeZNormals(p).bytes();
}
}
