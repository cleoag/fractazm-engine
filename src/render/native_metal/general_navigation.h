#pragma once
#include "navigation_preview.h"
#include <functional>
#include <array>
namespace mb3d::native_metal {
// Ordered sparse navigation, approximate headlight output. Not final geometry.
class GeneralNavigationSession {
public:
 explicit GeneralNavigationSession(std::string library,bool force_ordered_control=false);
 ~GeneralNavigationSession();
 NativeMetalAttempt render(const TMandHeader10&,const THeaderCustomAddon&,const RenderOptions&,unsigned phase,const std::function<bool()>& superseded={},bool fast_step=false,uint64_t scene_revision=0,bool reuse_preparation=true);
 void reset();
 bool active() const;
 uint64_t coveredPixels() const;
 unsigned phases() const;
 double gpuSeconds() const;
 double maxCommandSeconds() const;
 uint64_t deviceCreations() const;
 uint64_t scenePreparations() const;
 uint64_t resourceUploads() const;
 std::string resourceIdentity() const;
 std::array<double,4> preparationTimes() const;
 double prepareSeconds() const;
 double compileSeconds() const;
 double gpuBeginSeconds() const;
 double gpuEndSeconds() const;
private:
 struct Impl;std::unique_ptr<Impl> impl;
};
}
