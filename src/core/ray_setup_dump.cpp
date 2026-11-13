#include "core/ray_setup_dump.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>

namespace mb3d {

namespace {

static inline int32_t PascalBool(LongBool value) {
    return value ? -1 : 0;
}

static inline int32_t PascalByteBool(ByteBool value) {
    return value ? -1 : 0;
}

static inline void CopyVec3(void* dst, const void* src) {
    std::memcpy(dst, src, sizeof(TVec3D));
}

} // namespace

void InitRaySetupDumpRecord(RaySetupDumpRecord& record, int x, int y, int threadId, int32_t seedBefore) {
    std::memset(&record, 0, sizeof(record));
    record.x = x;
    record.y = y;
    record.thread_id = threadId;
    record.seed_before_first_step = seedBefore;
    record.cut_plane_hit = -1;
}

void CaptureRayReadyDump(RaySetupDumpRecord& record, const TMCTparameter& mct, const TIteration3Dext& it3d) {
    record.ray_ready_captured = 1;
    record.camera_optic = mct.MCTCameraOptic;
    record.cut_options = mct.iCutOptions;
    record.bin_steps = mct.iDEAddSteps;
    record.inside_rendering = PascalBool(mct.bInsideRendering);
    record.calc_inside = PascalBool(mct.bCalcInside);
    record.in_and_out = PascalBool(mct.bInAndOutside);
    record.calc_sit = PascalByteBool(it3d.CalcSIT);
    record.it_result = 0;
    record.max_it = it3d.maxIt;
    record.vary_destop = PascalBool(mct.bVaryDEstop);
    record.cafx = mct.CAFX;
    record.cafy = mct.CAFY;
    record.fovx_off = static_cast<double>(mct.FOVXoff);
    record.fovx_mul = static_cast<double>(mct.FOVXmul);
    record.fovy = mct.FOVy;
    record.step_width = mct.StepWidth;
    record.de_stop = mct.DEstop;
    record.ray_ms_destop = mct.msDEstop;
    record.ray_mzz = mct.mZZ;
    record.zend = mct.Zend;
    record.start_destop = mct.DEstop;
    record.max_ray_length = mct.Zend;
    record.zzpos_mul = 1.0;
    if (mct.bVaryDEstop) {
        double aspect = 1.0;
        if (mct.MCTCameraOptic != 2) {
            const double cosVal = std::cos(std::sqrt(mct.CAFX * mct.CAFX + mct.CAFY * mct.CAFY));
            aspect = std::max(0.1, cosVal);
        }
        record.de_mul_vary = mct.mctDEstopFactor / aspect;
        record.de_mul_const = 1.0;
    } else {
        record.de_mul_vary = 0.0;
        record.de_mul_const = 1.0;
    }
    record.smooth_it = 0.0;
    record.rout = 0.0;
    record.otrap = 0.0;
    record.r_stop = static_cast<double>(it3d.RStop);
    record.ddescale = static_cast<double>(mct.dDEscale);
    record.mct_de_offset = static_cast<double>(mct.mctDEoffset);
    record.mct_de_offset006 = static_cast<double>(mct.mctDEoffset006);
    CopyVec3(record.mvgrads_fov, mct.mVgradsFOV);
    CopyVec3(record.view_vec, mct.mVgradsFOV);
    CopyVec3(record.march_vec, mct.mVgradsFOV);
    record.ray_ready_start[0] = it3d.C1;
    record.ray_ready_start[1] = it3d.C2;
    record.ray_ready_start[2] = it3d.C3;
    record.act_pos[0] = it3d.C1;
    record.act_pos[1] = it3d.C2;
    record.act_pos[2] = it3d.C3;
    CopyVec3(record.ystart, mct.Ystart);
    CopyVec3(record.vgrads0, mct.Vgrads[0]);
    CopyVec3(record.vgrads1, mct.Vgrads[1]);
}

void CaptureFirstCalcDEEnterDump(RaySetupDumpRecord& record,
                                 const TMCTparameter& mct,
                                 const TIteration3Dext& it3d,
                                 int cutPlaneIndex,
                                 bool afterCutPlane) {
    if (record.calcde_enter_captured != 0) {
        return;
    }
    record.calcde_enter_captured = 1;
    record.calcde_retry_index = afterCutPlane ? 1 : 0;
    record.calcde_inside_rendering = PascalBool(mct.bInsideRendering);
    record.calcde_calc_inside = PascalBool(mct.bCalcInside);
    record.calc_sit = PascalByteBool(it3d.CalcSIT);
    record.it_result = 0;
    record.max_it = it3d.maxIt;
    record.cut_plane_hit = cutPlaneIndex;
    record.ms_destop = mct.msDEstop;
    record.mzz = mct.mZZ;
    record.smooth_it = 0.0;
    record.rout = 0.0;
    record.otrap = 0.0;
    record.r_stop = static_cast<double>(it3d.RStop);
    record.ddescale = static_cast<double>(mct.dDEscale);
    record.mct_de_offset = static_cast<double>(mct.mctDEoffset);
    record.mct_de_offset006 = static_cast<double>(mct.mctDEoffset006);
    record.calcde_enter_pos[0] = it3d.C1;
    record.calcde_enter_pos[1] = it3d.C2;
    record.calcde_enter_pos[2] = it3d.C3;
}

void MarkNoFirstCalcDEDump(RaySetupDumpRecord& record, int cutPlaneIndex) {
    if (record.calcde_enter_captured != 0) {
        return;
    }
    record.cut_plane_hit = cutPlaneIndex;
}

bool WriteRaySetupDump(const std::string& path,
                       const std::vector<RaySetupDumpRecord>& records,
                       int width,
                       int height) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return false;
    }

    const char magic[8] = {'R', 'A', 'Y', 'S', 'E', 'T', 'P', '1'};
    const uint32_t version = 1;
    const uint32_t w = static_cast<uint32_t>(width);
    const uint32_t h = static_cast<uint32_t>(height);
    const uint32_t recordSize = static_cast<uint32_t>(sizeof(RaySetupDumpRecord));
    const uint32_t count = static_cast<uint32_t>(records.size());

    out.write(magic, sizeof(magic));
    out.write(reinterpret_cast<const char*>(&version), sizeof(version));
    out.write(reinterpret_cast<const char*>(&w), sizeof(w));
    out.write(reinterpret_cast<const char*>(&h), sizeof(h));
    out.write(reinterpret_cast<const char*>(&recordSize), sizeof(recordSize));
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    if (!records.empty()) {
        out.write(reinterpret_cast<const char*>(records.data()),
                  static_cast<std::streamsize>(records.size() * sizeof(RaySetupDumpRecord)));
    }
    return out.good();
}

} // namespace mb3d
