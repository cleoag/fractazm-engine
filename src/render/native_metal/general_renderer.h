#pragma once
#include "render/metal_renderer.h"
namespace mb3d::native_metal {
// Device state an application keeps between renders of one shader library:
// the Metal device, a command queue, the loaded library and the setup checks
// that already passed. The library file is read and hashed once, at creation.
// A session renders one frame at a time; a render without a session behaves
// exactly as before.
struct RenderSession;
RenderSession* CreateRenderSession(const std::string& library_path, std::string& error);
void DestroyRenderSession(RenderSession* session);
NativeMetalAttempt RenderGeneralNativeMetal(TMandHeader10&, THeaderCustomAddon&,
    const RenderOptions&, const std::string& library_path, RenderSession* session = nullptr);
}
