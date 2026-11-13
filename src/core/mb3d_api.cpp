#include "mb3d/mb3d.h"
#include "core/renderer.h"
#include "core/mb3d_render_control_api.h"
#include "io/m3p_parser.h"
#include <cstring>
#include <cstdlib>
#include <exception>

extern "C" {

MB3DRenderResult mb3d_render_file(const char* m3p_path, const MB3DRenderOptions* opts) {
    return mb3d_render_file_with_control(m3p_path, opts, nullptr, nullptr);
}

MB3DRenderResult mb3d_render_file_with_control(const char* m3p_path,
        const MB3DRenderOptions* opts, MB3DRenderControl* control,
        MB3DRenderStatus* terminal_status) {
    return mb3d_render_file_with_precision(m3p_path, opts, MB3D_PRECISION_DEFAULT,
                                          control, terminal_status);
}

MB3DRenderResult mb3d_render_file_with_precision(const char* m3p_path,
        const MB3DRenderOptions* opts, MB3DPrecisionMode precision,
        MB3DRenderControl* control, MB3DRenderStatus* terminal_status) {
    MB3DRenderResult result;
    std::memset(&result, 0, sizeof(result));
    if (terminal_status) *terminal_status = MB3D_RENDER_FAILED;

    if (precision != MB3D_PRECISION_DEFAULT && precision != MB3D_PRECISION_COMPATIBLE &&
        precision != MB3D_PRECISION_NATIVE) {
        std::strncpy(result.error, "Invalid precision mode", sizeof(result.error) - 1);
        return result;
    }

    if (!m3p_path) {
        result.success = 0;
        std::strncpy(result.error, "m3p_path is NULL", sizeof(result.error) - 1);
        return result;
    }

    try {
    // Load .m3p file
    auto loadResult = mb3d::LoadM3PFile(m3p_path);
    if (!loadResult.success) {
        result.success = 0;
        std::strncpy(result.error, loadResult.error.c_str(), sizeof(result.error) - 1);
        return result;
    }

    // Build render options
    mb3d::RenderOptions renderOpts;
    if (precision != MB3D_PRECISION_DEFAULT)
        renderOpts.precision = precision == MB3D_PRECISION_NATIVE
            ? mb3d::PrecisionMode::Native : mb3d::PrecisionMode::Compatible;
    renderOpts.verbose = false;
    if (control) renderOpts.control = control->value;

    if (opts) {
        renderOpts.width_override = opts->width;
        renderOpts.height_override = opts->height;
        renderOpts.thread_count = opts->thread_count;
        renderOpts.no_shadows = (opts->no_shadows != 0);
        renderOpts.no_ao = (opts->no_ao != 0);
        renderOpts.no_dof = (opts->no_dof != 0);
        renderOpts.no_reflections = (opts->no_reflections != 0);

        if (opts->max_iterations > 0) {
            loadResult.header.Iterations = opts->max_iterations;
        }
    }

    // Render
    auto renderResult = mb3d::Render(loadResult.header, loadResult.addon, renderOpts);
    if (!renderResult.success) {
        result.success = 0;
        if (terminal_status) *terminal_status = mb3d_api_render_status(renderResult.status);
        std::strncpy(result.error, renderResult.error.c_str(), sizeof(result.error) - 1);
        return result;
    }

    // Copy image data to malloc'd buffer (caller frees with mb3d_free)
    size_t imageSize = static_cast<size_t>(renderResult.width) * renderResult.height * 3;
    result.pixels = static_cast<unsigned char*>(std::malloc(imageSize));
    if (!result.pixels) {
        result.success = 0;
        std::strncpy(result.error, "Failed to allocate pixel buffer", sizeof(result.error) - 1);
        return result;
    }
    std::memcpy(result.pixels, renderResult.image.data(), imageSize);

    // Copy z-buffer data
    size_t zbufSize = static_cast<size_t>(renderResult.width) * renderResult.height * sizeof(unsigned short);
    result.zbuffer = static_cast<unsigned short*>(std::malloc(zbufSize));
    if (result.zbuffer) {
        for (int i = 0; i < renderResult.width * renderResult.height; i++) {
            result.zbuffer[i] = renderResult.silight[i].Zpos;
        }
    }

    result.width = renderResult.width;
    result.height = renderResult.height;
    result.success = 1;
    if (terminal_status) *terminal_status = MB3D_RENDER_SUCCESS;
    return result;
    } catch (const mb3d::RenderStopped& error) {
        if (terminal_status) *terminal_status = mb3d_api_render_status(error.status());
        std::strncpy(result.error, error.what(), sizeof(result.error) - 1);
    } catch (const std::exception& error) {
        std::strncpy(result.error, error.what(), sizeof(result.error) - 1);
    } catch (...) {
        std::strncpy(result.error, "Unknown render failure", sizeof(result.error) - 1);
    }
    // Worker failures must never escape the public C ABI.
    std::free(result.pixels);
    std::free(result.zbuffer);
    result.pixels = nullptr;
    result.zbuffer = nullptr;
    result.success = 0;
    return result;
}

void mb3d_free(void* ptr) {
    std::free(ptr);
}

} // extern "C"
