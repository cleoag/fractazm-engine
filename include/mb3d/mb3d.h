#pragma once

#include <stdint.h>

/**
 * mb3d Public C API
 *
 * Minimal C API for rendering .m3p files from external applications
 * and platform-specific wrappers. Image/buffer memory must be freed with
 * mb3d_free(); render controls have a separate destroy function.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Render options. Pass to mb3d_render_file to control rendering behavior.
 * Zero-initialize for defaults.
 */
typedef struct {
    int width;           /* Override image width  (0 = use from .m3p) */
    int height;          /* Override image height (0 = use from .m3p) */
    int thread_count;    /* Thread count (0 = hardware_concurrency)   */
    int max_iterations;  /* Override max iterations (0 = use from .m3p) */
    int no_shadows;      /* Nonzero to disable hard shadows */
    int no_ao;           /* Nonzero to disable ambient occlusion */
    int no_dof;          /* Nonzero to disable depth of field */
    int no_reflections;  /* Nonzero to disable reflections */
} MB3DRenderOptions;

/**
 * Render result. Contains the raw internal render surface and optional z-buffer.
 * Caller must free pixels and zbuffer with mb3d_free().
 *
 * Important:
 * - `pixels` currently reflects the internal RGB render surface before CLI save-time
 *   postprocessing such as Pascal-style ImageScale downscale or tiling crop.
 * - `zbuffer` currently reflects raw packed `siLight5.Zpos` values at the same internal size.
 * - This makes the C API stable for programmatic access, but not yet identical to the
 *   final saved-image semantics of the Pascal GUI release.
 */
typedef struct {
    unsigned char* pixels;    /* Raw RGB image data, 3 bytes per pixel. Caller must free with mb3d_free(). */
    unsigned short* zbuffer;  /* Raw packed Z-buffer, 1 value per pixel. May be NULL. Caller must free with mb3d_free(). */
    int width;                /* Raw internal surface width in pixels */
    int height;               /* Raw internal surface height in pixels */
    int success;              /* Nonzero on success, zero on failure */
    char error[256];          /* Error message if success == 0 */
} MB3DRenderResult;

/**
 * Render a .m3p parameter file.
 *
 * @param m3p_path   Path to the .m3p file
 * @param opts       Render options, or NULL for defaults
 * @return           Render result. Caller must free pixels/zbuffer with mb3d_free().
 */
MB3DRenderResult mb3d_render_file(const char* m3p_path, const MB3DRenderOptions* opts);

/** An owning, single-job control. It does not change the legacy options/result ABI. */
typedef struct MB3DRenderControl MB3DRenderControl;

typedef enum {
    MB3D_RENDER_SUCCESS = 0,
    MB3D_RENDER_FAILED = 1,
    MB3D_RENDER_CANCELLED = 2,
    MB3D_RENDER_WORK_LIMIT_EXCEEDED = 3,
    MB3D_RENDER_UNSUPPORTED = 4,
    MB3D_RENDER_PREPARED = 5,
    MB3D_RENDER_RUNNING = 6
} MB3DRenderStatus;

/**
 * Create a control for one ordinary CPU render. max_work == 0 is unlimited.
 * Work units are shared across the job's workers and stages; they are not
 * Pascal iteration counts or a wall-clock limit. Returns NULL on allocation failure.
 */
MB3DRenderControl* mb3d_render_control_create(uint64_t max_work);

/** Request cooperative cancellation; safe concurrently with the controlled render. */
void mb3d_render_control_cancel(MB3DRenderControl* control);

/**
 * Thread-safe render lifecycle queries. NULL returns 0 / MB3D_RENDER_FAILED.
 * The render entrypoint's terminal_status also covers loading/output-allocation
 * errors, which can occur before/after the controlled render lifecycle.
 */
uint64_t mb3d_render_control_work_used(const MB3DRenderControl* control);
MB3DRenderStatus mb3d_render_control_status(const MB3DRenderControl* control);

/**
 * Destroy after the render and all cancellation/query calls have returned.
 * A control cannot be reset or reused for another job; create a fresh control.
 */
void mb3d_render_control_destroy(MB3DRenderControl* control);

/**
 * Render with optional cancellation/work control. NULL control keeps the
 * original unlimited behavior. terminal_status may be NULL. Cancellation,
 * exhaustion, unsupported execution and errors return success == 0, zero
 * dimensions and NULL pixels/zbuffer; no successful partial surface is returned.
 * The control and options must remain alive for the duration of this call.
 */
MB3DRenderResult mb3d_render_file_with_control(const char* m3p_path,
    const MB3DRenderOptions* opts, MB3DRenderControl* control,
    MB3DRenderStatus* terminal_status);

/** Per-render arithmetic policy. Native trades x87 numerical compatibility
 * for hardware FP64 and native transcendental functions on Apple Silicon.
 * Existing options/result layouts and entrypoints retain their ABI. */
typedef enum MB3DPrecisionMode {
    MB3D_PRECISION_DEFAULT = 0,
    MB3D_PRECISION_COMPATIBLE = 1,
    MB3D_PRECISION_NATIVE = 2
} MB3DPrecisionMode;
MB3DRenderResult mb3d_render_file_with_precision(const char* m3p_path,
    const MB3DRenderOptions* opts, MB3DPrecisionMode precision,
    MB3DRenderControl* control, MB3DRenderStatus* terminal_status);

/**
 * Free memory allocated by the mb3d API (pixels, zbuffer, etc.).
 */
void mb3d_free(void* ptr);

#ifdef __cplusplus
}
#endif
