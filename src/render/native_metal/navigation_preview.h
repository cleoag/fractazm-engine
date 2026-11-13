#pragma once
#include "render/metal_renderer.h"
#include <memory>
namespace mb3d::native_metal {
// Approximate geometric display, not authored shading or final-scene acceptance.
enum class NavigationPattern { Dense=0, Grid=1, Stratified=2 };
struct NavigationPreviewOptions {
    NavigationPattern pattern=NavigationPattern::Stratified;
    uint32_t block_size=4, phase=0, seed=0x4d423344;
};
// Single serial owner. Phase 0 resets the camera; subsequent phases must be
// consecutive with identical scene/options. Failed requests invalidate accumulation.
// Device/pipeline objects survive camera resets; no buffers escape the owner.
class NavigationPreviewSession {
public:
    explicit NavigationPreviewSession(std::string library);
    ~NavigationPreviewSession();
    NavigationPreviewSession(const NavigationPreviewSession&)=delete;
    NavigationPreviewSession& operator=(const NavigationPreviewSession&)=delete;
    NativeMetalAttempt render(const TMandHeader10&,const THeaderCustomAddon&,
        const RenderOptions&,const NavigationPreviewOptions&,bool accumulate=true);
    uint64_t coveredPixels() const;
    uint64_t deviceCreations() const;
    uint64_t scenePreparations() const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
NativeMetalAttempt RenderNavigationPreview(const TMandHeader10&,const THeaderCustomAddon&,
    const RenderOptions&,const NavigationPreviewOptions&,const std::string& library);
}
