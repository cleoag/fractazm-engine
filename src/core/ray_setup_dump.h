#ifndef MB3D_CORE_RAY_SETUP_DUMP_H
#define MB3D_CORE_RAY_SETUP_DUMP_H

#include "core/types.h"
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

namespace mb3d {

#pragma pack(push, 1)
struct RaySetupDumpRecord {
    int32_t x = 0;
    int32_t y = 0;
    int32_t thread_id = 0;
    int32_t seed_before_first_step = 0;
    int32_t camera_optic = 0;
    int32_t cut_options = 0;
    int32_t bin_steps = 0;
    int32_t ray_ready_captured = 0;
    int32_t calcde_enter_captured = 0;
    int32_t calcde_retry_index = 0;
    int32_t inside_rendering = 0;
    int32_t calc_inside = 0;
    int32_t in_and_out = 0;
    int32_t calcde_inside_rendering = 0;
    int32_t calcde_calc_inside = 0;
    int32_t calc_sit = 0;
    int32_t it_result = 0;
    int32_t max_it = 0;
    int32_t cut_plane_hit = 0;
    int32_t vary_destop = 0;
    double cafx = 0.0;
    double cafy = 0.0;
    double fovx_off = 0.0;
    double fovx_mul = 0.0;
    double fovy = 0.0;
    double step_width = 0.0;
    double de_stop = 0.0;
    double ray_ms_destop = 0.0;
    double ms_destop = 0.0;
    double ray_mzz = 0.0;
    double mzz = 0.0;
    double zend = 0.0;
    double start_destop = 0.0;
    double max_ray_length = 0.0;
    double zzpos_mul = 0.0;
    double de_mul_vary = 0.0;
    double de_mul_const = 0.0;
    double smooth_it = 0.0;
    double rout = 0.0;
    double otrap = 0.0;
    double r_stop = 0.0;
    double ddescale = 0.0;
    double mct_de_offset = 0.0;
    double mct_de_offset006 = 0.0;
    double mvgrads_fov[3] = {0.0, 0.0, 0.0};
    double view_vec[3] = {0.0, 0.0, 0.0};
    double march_vec[3] = {0.0, 0.0, 0.0};
    double ray_ready_start[3] = {0.0, 0.0, 0.0};
    double calcde_enter_pos[3] = {0.0, 0.0, 0.0};
    double act_pos[3] = {0.0, 0.0, 0.0};
    double ystart[3] = {0.0, 0.0, 0.0};
    double vgrads0[3] = {0.0, 0.0, 0.0};
    double vgrads1[3] = {0.0, 0.0, 0.0};
};
#pragma pack(pop)

static_assert(std::is_trivially_copyable<RaySetupDumpRecord>::value,
              "RaySetupDumpRecord must stay trivially copyable");
static_assert(sizeof(RaySetupDumpRecord) == 488,
              "RaySetupDumpRecord size drifted; update Pascal/C++ dump contract together");

void InitRaySetupDumpRecord(RaySetupDumpRecord& record, int x, int y, int threadId, int32_t seedBefore);
void CaptureRayReadyDump(RaySetupDumpRecord& record, const TMCTparameter& mct, const TIteration3Dext& it3d);
void CaptureFirstCalcDEEnterDump(RaySetupDumpRecord& record,
                                 const TMCTparameter& mct,
                                 const TIteration3Dext& it3d,
                                 int cutPlaneIndex,
                                 bool afterCutPlane);
void MarkNoFirstCalcDEDump(RaySetupDumpRecord& record, int cutPlaneIndex);
bool WriteRaySetupDump(const std::string& path,
                       const std::vector<RaySetupDumpRecord>& records,
                       int width,
                       int height);

} // namespace mb3d

#endif // MB3D_CORE_RAY_SETUP_DUMP_H
