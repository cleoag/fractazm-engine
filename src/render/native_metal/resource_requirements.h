#pragma once
#include "core/types.h"
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

namespace mb3d::native_metal {
// A Native frame may use half the device's recommended working set. The DOF
// and Z-normal workspaces keep their own 1 GiB dispatch caps.
constexpr uint64_t kNativeStageWorkspaceCap=uint64_t(1)<<30;
inline uint64_t NativeDeviceBudget(uint64_t recommended_working_set){
    return recommended_working_set?recommended_working_set/2:kNativeStageWorkspaceCap;
}
// Capture requested lighting resources before legacy preparation can turn a
// failed load into an inactive effective flag. IDs use original light indices.
// Formula maps have their separate exported, owned GPU table and validation.
struct LightingResourceRequirement {
    std::string kind;
    int light_index=-1;
    unsigned map_id=0;
    std::string filename;
};
inline std::vector<LightingResourceRequirement> NativeLightingRequirements(const TMandHeader10& header) {
    const auto& light=header.Light;
    std::vector<LightingResourceRequirement> result;
    const unsigned diffuse=unsigned(light.bColorMap)|(unsigned(light.Lights[1].AdditionalByteEx)<<8);
    if(diffuse)result.push_back({"diffuse_map",-1,diffuse,{}});
    for(int i=0;i<6;++i) {
        const auto& lamp=light.Lights[i];
        // MakeLightValsFromHeaderLight treats map zero and option three as off.
        if((lamp.Loption&3u)==2u&&lamp.LightMapNr)
            result.push_back({"light_map",i,lamp.LightMapNr,{}});
    }
    if(light.BGbmp[0]) {
        const auto* first=reinterpret_cast<const char*>(light.BGbmp);
        result.push_back({"background_map",-1,0,
            std::string(first,std::find(first,first+sizeof(light.BGbmp),'\0'))});
    }
    return result;
}
inline void ValidateNativeLightingRequirements(const std::vector<LightingResourceRequirement>& requirements,
                                               bool camera_maps=false) {
    // Native camera color and the camera-aware reflection material sample
    // diffuse, light and background maps (NativeMaterialMaps); no other kernel
    // samples a lighting resource. Admission follows the request, never the
    // file's existence: a failed load leaves the shared loader's inactive
    // state, which the GPU reproduces.
    for(const auto& r:requirements) {
        const bool map=r.kind=="diffuse_map"||r.kind=="light_map"||r.kind=="background_map";
        if(camera_maps&&map)continue;
        std::string reason="Unsupported Native requested "+r.kind;
        if(r.light_index>=0)reason+=" at light "+std::to_string(r.light_index);
        if(r.map_id)reason+=" #"+std::to_string(r.map_id);
        if(!r.filename.empty())reason+=" "+r.filename;
        throw std::invalid_argument(reason);
    }
}
}
