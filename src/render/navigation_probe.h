#pragma once
// Distance from a point to the fractal, measured with the scene's own distance
// estimate on the CPU. Navigation uses it to keep the camera's start plane out
// of the fractal, as the MB3D navigator does with GetLocalAbsoluteDE.
#include "core/external_formulas.h"
#include "core/header_trafos.h"
#include "core/ray_marcher.h"
#include "core/render_context.h"
#include "render/navigation_camera.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <string>

namespace mb3d::navigation {

class SurfaceProbe {
public:
    SurfaceProbe(const TMandHeader10& sourceHeader, const THeaderCustomAddon& sourceAddon, const std::string& root)
        : context_(std::make_shared<RenderContext>(root)), header_(sourceHeader), addon_(sourceAddon) {
        std::memset(static_cast<void*>(&it_), 0, sizeof(it_));
        // A formula that reads a map needs the map files; those scenes are not probed.
        if (usesMaps(addon_)) return;
        try {
            context_->precision = PrecisionMode::Native;
            ScopedRenderContext binding(context_);
            mct_ = GetMCTparasFromHeader(header_, &addon_, 0);
            if (!mct_.bMCTisValid || !mct_.CalcDE || !(mct_.StepWidth > 0)) return;
            mct_.pIt3Dext = &it_;
            IniIt3D(&mct_, &it_);
            inside_ = mct_.bInsideRendering != 0;
            valid_ = true;
        } catch (...) {
            valid_ = false;
        }
    }

    bool valid() const { return valid_; }

    // Share of the distance estimate the scene's ray marcher dares to step.
    double stride() const { return std::clamp(double(mct_.sZstepDiv), 0.05, 1.0); }

    // Distance from `point` to the fractal in scene units, as the scene's
    // distance estimate gives it (the MB3D navigator trusts it the same way; the
    // ray marcher steps a fraction of it). 0 when the point is in the fractal or
    // the estimate fails.
    double clearance(const Vec& point) {
        if (!valid_) return 0;
        try {
            ScopedRenderContext binding(context_);
            double estimate = 0;
            bool inSet = measure(point, inside_, estimate);
            // A scene rendered "in and outside" is open on both sides of its surface.
            if (inSet && mct_.bInAndOutside) inSet = measure(point, !inside_, estimate);
            if (inSet || !std::isfinite(estimate)) return 0;
            return std::max(0.0, estimate - double(mct_.DEstop)) * mct_.StepWidth;
        } catch (...) {
            return 0;
        }
    }

private:
    static bool usesMaps(const THeaderCustomAddon& addon) {
        const bool interpolated = (addon.bOptions1 & 3) == 1;
        for (int slot = 0; slot <= (interpolated ? 2 : 5); ++slot) {
            const auto& formula = addon.Formulas[slot];
            if (!interpolated && formula.iItCount == 0) continue;
            const ExternalFormulaInfo* info = nullptr;
            if (formula.iFnr >= 20 && formula.CustomFname[0] != 0)
                info = FindExternalFormula(reinterpret_cast<const char*>(formula.CustomFname));
            const int count = std::min(16, static_cast<int>(formula.iOptionCount));
            for (int option = 0; option < count; ++option) {
                if (info ? (info->mapValueMask & (uint16_t(1) << option)) != 0 : formula.byOptionType[option] == 2)
                    return true;
            }
        }
        return false;
    }

    bool measure(const Vec& point, bool inside, double& estimate) {
        mct_.bInsideRendering = inside;
        mct_.bCalcInside = inside;
        mct_.msDEstop = mct_.DEstop;
        mct_.mZZ = 0;
        it_.CalcSIT = 0;
        it_.C1 = point[0];
        it_.C2 = point[1];
        it_.C3 = point[2];
        estimate = mct_.CalcDE(&it_, &mct_);
        return it_.ItResultI >= mct_.MaxItsResult || !(estimate >= mct_.msDEstop);
    }

    std::shared_ptr<RenderContext> context_;
    TMandHeader10 header_;
    THeaderCustomAddon addon_;
    TMCTparameter mct_{};
    TIteration3Dext it_;
    bool inside_ = false;
    bool valid_ = false;
};

// Geometry of the start plane of a camera: MB3D starts its rays on a
// rectangle and fans them out by the field of view, which is a pinhole camera
// standing `near` behind the rectangle and clipped by it.
struct StartPlane {
    double near = 0;    // viewpoint to plane
    double radius = 0;  // half diagonal of the rectangle rendered at `aspect`
    static StartPlane of(const TMandHeader10& header, double span, double aspect) {
        StartPlane plane;
        const double authored = double(std::max(1, header.Width)) / double(std::max(1, header.Height));
        const double spanY = span / authored;
        const double half = std::min(std::abs(header.dFOVy), 180.0) * (M_PI / 180.0) * 0.5;
        plane.near = spanY * 0.5 * std::cos(half) / std::max(0.01, std::sin(half));   // HeaderTrafos.CalcCamPos
        // A frame of another shape keeps the authored rectangle inside it.
        const double halfX = span * 0.5 * (aspect > authored ? aspect / authored : 1.0);
        const double halfY = aspect > 0 && aspect < authored ? span * 0.5 / aspect : spanY * 0.5;
        plane.radius = std::hypot(halfX, halfY);
        return plane;
    }
};

}  // namespace mb3d::navigation
