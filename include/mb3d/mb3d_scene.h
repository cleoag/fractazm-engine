#pragma once

#include "mb3d/mb3d.h"
#include <stddef.h>

/**
 * mb3d application API (library v1).
 *
 * An editable in-memory scene, the formula catalog, a navigation camera,
 * per-eye stereo cameras, a clip volume and a progressive renderer. Every call
 * is synchronous and may be made from any thread; one MB3DScene must not be
 * mutated while another thread reads it. Rendering never mutates the scene:
 * clone it (about 2 KB) to edit while a render is running.
 *
 * Functions returning int return nonzero on success unless stated otherwise.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define MB3D_FORMULA_SLOTS 6
#define MB3D_FORMULA_PARAMS 16

/* ---- Scene lifetime ---------------------------------------------------- */

typedef struct MB3DScene MB3DScene;

/** Load a .m3p parameter file (all MB3D versions). NULL on failure. */
MB3DScene* mb3d_scene_load(const char* m3p_path, char* error, size_t error_size);
MB3DScene* mb3d_scene_clone(const MB3DScene* scene);
void mb3d_scene_destroy(MB3DScene* scene);
int mb3d_scene_save(const MB3DScene* scene, const char* m3p_path);

/** Directory holding this scene's maps/backgrounds (M3Maps). Default: cwd. */
void mb3d_scene_set_resource_root(MB3DScene* scene, const char* directory);

/** Authored image size and iteration limit of the scene. */
void mb3d_scene_get_size(const MB3DScene* scene, int* width, int* height);
int mb3d_scene_get_iterations(const MB3DScene* scene);
int mb3d_scene_set_iterations(MB3DScene* scene, int iterations);

/* ---- Formula catalog and hybrid slots ---------------------------------- */

/** Formulas compiled into the library. Names are the MB3D formula names. */
int mb3d_formula_count(void);
const char* mb3d_formula_name(int index);
int mb3d_formula_exists(const char* name);

typedef enum {
    MB3D_HYBRID_ALTERNATE = 0,
    MB3D_HYBRID_INTERPOLATE = 1,
    MB3D_HYBRID_DE_COMBINATE = 2
} MB3DHybridMode;

typedef struct {
    char name[33];      /* empty: unused slot */
    int iterations;     /* repeats of this formula in the hybrid sequence */
    int param_count;    /* 0..MB3D_FORMULA_PARAMS */
    int builtin;        /* nonzero: one of the ten internal MB3D formulas */
} MB3DFormulaSlot;

int mb3d_scene_get_formula(const MB3DScene* scene, int slot, MB3DFormulaSlot* out);

/**
 * Put a formula into a slot from its .m3f description. Only the text [OPTIONS]
 * section is read (parameter names and defaults); the formula itself must be
 * compiled into the library, x86 [CODE] is never executed.
 */
int mb3d_scene_set_formula_m3f(MB3DScene* scene, int slot, const char* m3f_path,
                               int iterations, char* error, size_t error_size);
int mb3d_scene_clear_formula(MB3DScene* scene, int slot);
int mb3d_scene_set_formula_iterations(MB3DScene* scene, int slot, int iterations);

/** Parameter values are always available; names only after set_formula_m3f. */
int mb3d_scene_get_formula_param(const MB3DScene* scene, int slot, int index, double* value);
int mb3d_scene_set_formula_param(MB3DScene* scene, int slot, int index, double value);
const char* mb3d_scene_formula_param_name(const MB3DScene* scene, int slot, int index);

MB3DHybridMode mb3d_scene_get_hybrid_mode(const MB3DScene* scene);
int mb3d_scene_set_hybrid_mode(MB3DScene* scene, MB3DHybridMode mode);

/* ---- Camera ------------------------------------------------------------ */

/**
 * MB3D starts every ray on a rectangle (the start plane, `2.1345 / zoom` scene
 * units wide) and fans the rays out by the field of view. That is a pinhole
 * camera standing behind the rectangle, at half its height divided by
 * tan(fov / 2), with the rectangle as its near clipping plane: where the
 * rectangle is inside the fractal the picture shows a flat cut. Rays end
 * `depth` behind the rectangle, the far limit.
 */
typedef struct {
    double eye[3];      /* centre of the start plane */
    /* Orthonormal. `right` points to the right edge of the picture and `up`
     * to its bottom edge: they are the directions of MB3D's pixel columns and
     * rows, and rows are counted downwards. */
    double right[3], up[3], forward[3];
    double fov_y_degrees;
    double zoom;        /* MB3D zoom; the view spans 2.1345 / zoom scene units */
    double pivot;       /* distance ahead of `eye` of the point rotations turn about; 0 turns in place */
    double depth;       /* distance from the start plane to the far limit */
} MB3DCamera;

/** Fails for scenes whose projection the navigation camera does not model. */
int mb3d_scene_get_camera(const MB3DScene* scene, MB3DCamera* out);
/** Sets the camera as given; the gesture helpers then start from it. */
int mb3d_scene_set_camera(MB3DScene* scene, const MB3DCamera* camera);

/**
 * Gesture helpers. Angles in radians; moves in fractions of the view span.
 *
 * Directions, as seen in the picture (`up` is MB3D's row direction, which
 * points to the bottom edge):
 * - mb3d_scene_rotate_camera: a positive yaw turns the view direction to the
 *   right, a positive pitch to the top. Without `fly` the camera goes round its
 *   pivot, so the surface in front of the pivot moves to the right and to the
 *   top; with `fly` the camera turns in place and the whole picture moves to
 *   the left and to the bottom.
 * - mb3d_scene_roll_camera: a positive angle turns the camera clockwise about
 *   its view direction, so the picture turns anticlockwise.
 * - mb3d_scene_move_camera and mb3d_scene_pan_camera: a positive `right` moves
 *   the camera to the right (the picture goes left), a positive `up` moves it
 *   towards the bottom edge (the picture goes up).
 *
 * They keep the start plane out of the fractal, as the MB3D navigator does by
 * sizing the view to its local distance estimate. After the camera moved, the
 * free distance around the centre of the start plane is measured with the
 * scene's distance estimate (on the CPU, well under a millisecond for most
 * scenes). When the plane is too large for it, the plane is pulled towards the
 * viewpoint and `zoom` grows by the same factor: viewpoint, direction and field
 * of view stay, so the picture stays, and less of the near space is clipped.
 * When there is room again the plane goes back, up to the size the gestures
 * started from (the loaded scene or the last mb3d_scene_set_camera).
 *
 * A step that would leave no free start plane (the viewpoint would enter the
 * fractal) is not made: the call returns 0 and the camera stays. Unchecked,
 * as before, are scenes whose own start plane cuts the fractal, panoramas, and
 * scenes with a formula that reads a map.
 */
int mb3d_scene_rotate_camera(MB3DScene* scene, double yaw, double pitch, int fly);
int mb3d_scene_roll_camera(MB3DScene* scene, double angle);
int mb3d_scene_move_camera(MB3DScene* scene, double right, double up, double forward);

/**
 * Sideways, in fractions of what the view shows at the surface the camera
 * looks at: that surface follows a finger that drags it. (The view at the
 * start plane is narrower; mb3d_scene_move_camera moves in fractions of that.)
 */
int mb3d_scene_pan_camera(MB3DScene* scene, double right, double up);

/**
 * Towards (amount > 0) or away from what the camera looks at. The distance
 * from the viewpoint to the surface in the middle of the picture shrinks or
 * grows by e^amount: a pinch that doubles the picture is ln 2, halves the
 * distance and shows that surface twice as large, so the camera comes ever
 * closer without entering. With nothing in the middle of the picture the
 * nearest surface around the camera sets the pace, and where distances are
 * unknown the step is `amount` of the view span the gestures started with.
 * While the pivot is ahead the camera moves towards or away from it and the
 * far limit stays in place; a camera at its pivot takes it along, and backing
 * away never brings the far limit nearer.
 */
int mb3d_scene_dolly_camera(MB3DScene* scene, double amount);

/**
 * Shape (width / height) of the frames the application renders when it is not
 * the scene's own and the application fits the camera to it; the gesture
 * helpers then keep the start plane of that frame clear. 0: the scene's own.
 */
int mb3d_scene_set_navigation_aspect(MB3DScene* scene, double aspect);

/* ---- Stereo ------------------------------------------------------------ */

typedef enum { MB3D_EYE_LEFT = 0, MB3D_EYE_RIGHT = 1 } MB3DEye;

/**
 * A new scene for one eye: the camera is shifted along its right axis by half
 * of `separation` (a fraction of the view span; about 0.03 is comfortable).
 * Both eyes keep parallel axes, so the application sets convergence by shifting
 * the two images. Works on every backend. Destroy with mb3d_scene_destroy.
 */
MB3DScene* mb3d_scene_create_eye(const MB3DScene* scene, MB3DEye eye, double separation);

/* ---- Clip volume ------------------------------------------------------- */

typedef struct { double min[3], max[3]; } MB3DVolume;

/**
 * Confine rendering to an axis-aligned box in scene coordinates, for example a
 * visionOS volume. NULL removes the volume. The camera-facing faces cut the
 * fractal exactly; the far side is limited by a plane through the farthest box
 * corner. Rays passing beside the box are not masked in v1.
 */
int mb3d_scene_set_clip_volume(MB3DScene* scene, const MB3DVolume* volume);
int mb3d_scene_get_clip_volume(const MB3DScene* scene, MB3DVolume* out);

/* ---- Colours, fog and light --------------------------------------------- */

/**
 * Appearance controls for an editor. A setter changes only the scene, so both
 * backends show it. Values outside the stated range are clamped; a non-finite
 * value or a NULL argument fails and leaves the scene unchanged. A setter
 * given what its getter returned changes nothing in the scene. Any other value
 * is stored on the MB3D slider grid stated with each call, so the getter then
 * returns the nearest stored value.
 */

#define MB3D_PALETTE_COLORS 10

typedef struct { unsigned char r, g, b; } MB3DColor;

/**
 * The ten surface colours in palette order. Positions, specular colours and
 * transparency of the entries are kept. A surface coloured by a diffuse colour
 * map shows the map instead, and the inside of a cut fractal has four colours
 * of its own.
 */
int mb3d_scene_get_palette(const MB3DScene* scene, MB3DColor colors[MB3D_PALETTE_COLORS]);
int mb3d_scene_set_palette(MB3DScene* scene, const MB3DColor colors[MB3D_PALETTE_COLORS]);

/**
 * Where each colour sits along the palette, 0 (its start) to 1 (its end); the
 * surface blends between neighbours. Many scenes use three or four colours and
 * pack the others just before 1, so a palette of ten different colours needs
 * new positions: {0, 0.1, ... 0.9} spreads them evenly. The setter needs
 * ascending values and puts the first colour at 0. Stored in steps of 1/32767.
 */
int mb3d_scene_get_palette_positions(const MB3DScene* scene, double positions[MB3D_PALETTE_COLORS]);
int mb3d_scene_set_palette_positions(MB3DScene* scene, const double positions[MB3D_PALETTE_COLORS]);

/**
 * The part of the colouring value the palette is laid over. The colouring
 * value of a surface point (its smoothed iteration count, or its orbit trap in
 * scenes that colour by it) runs from 0 to 1. `start` gets the first palette
 * colour and `end` is where the palette ends: moving both shifts the colours
 * over the surface, `end - start` stretches them and `end < start` reverses
 * them. Outside the range the palette repeats in scenes with colour cycling
 * (most) and holds its first or last colour in the others.
 *
 * Both values are limited to MB3D_PALETTE_RANGE_MIN..MAX. They are stored as
 * the MB3D "Color start" and "Color end" sliders with fine adjustment, each
 * end within 1 % of the width of the range plus 0.0003.
 */
#define MB3D_PALETTE_RANGE_MIN (-10900.0 / 32767.0)             /* about -0.333 */
#define MB3D_PALETTE_RANGE_MAX (16.0 / 9.0 - 10900.0 / 32767.0) /* about 1.445 */
int mb3d_scene_get_palette_range(const MB3DScene* scene, double* start, double* end);
int mb3d_scene_set_palette_range(MB3DScene* scene, double start, double end);

/**
 * Depth fog. Distant surface fades into the background colour, which runs
 * from `color` at the top of the image to `color_bottom` at the bottom; give
 * both the same value for one fog colour. Pixels without surface have that
 * colour too (and alpha 0 in MB3DFrame). `amount` is 0 (no fog) to 1 (the
 * MB3D "Depth" slider at its maximum), stored in steps of 1/240: at about 0.19
 * the far end of the scene is solid fog, and more brings the fog nearer. The
 * fog begins at a fixed depth of the scene's own depth range, so a fractal in
 * the nearest part of that range shows little of it at any amount. MB3D's
 * dynamic fog is another effect and is not changed.
 */
typedef struct {
    MB3DColor color, color_bottom;
    double amount;
} MB3DFog;

int mb3d_scene_get_fog(const MB3DScene* scene, MB3DFog* out);
int mb3d_scene_set_fog(MB3DScene* scene, const MB3DFog* fog);

/**
 * The first global (directional) light that is switched on.
 *
 * `x` and `y` place the light on a pad as the viewer sees the picture, each
 * -1..1: (0, 0) shines from the camera straight onto the scene, +x moves the
 * light to the viewer's right and +y upwards. At 0.5 on either axis the light
 * comes exactly from the side (or from above); beyond that it moves behind the
 * scene, and (1, 0) or (0, 1) is opposite the camera. A pad limited to
 * -0.5..0.5 on both axes reaches every direction in front of the scene. The
 * values are the two MB3D light angles divided by 180 degrees. A light fixed
 * to the object (MB3D "Rel. to object") is read and set as it stands for the
 * current camera.
 *
 * `brightness` is 0..100 and multiplies the light colour; 1 is the MB3D
 * default. Two significant digits are stored.
 *
 * `hard_shadow`: the light casts hard shadows. Only MB3D_QUALITY_FINAL renders
 * them.
 *
 * The getter fails when no global light is on, that is when the scene is lit
 * only by positional lights and light maps. The setter then switches on the
 * first light that is off, as a white MB3D default light, and fails when all
 * six lights are in use.
 */
typedef struct {
    double x, y;
    double brightness;
    int hard_shadow;
} MB3DLight;

int mb3d_scene_get_light(const MB3DScene* scene, MB3DLight* out);
int mb3d_scene_set_light(MB3DScene* scene, const MB3DLight* light);

/**
 * Strength of the ambient light: 0 is none, 1 the MB3D default, 3 the maximum
 * (the MB3D "Ambient" slider divided by 90, stored in steps of 1/90). The two
 * ambient colours are kept. No effect in scenes that take the ambient light
 * from a background image.
 */
int mb3d_scene_get_ambient(const MB3DScene* scene, double* amount);
int mb3d_scene_set_ambient(MB3DScene* scene, double amount);

/* ---- Rendering --------------------------------------------------------- */

typedef enum { MB3D_BACKEND_AUTO = 0, MB3D_BACKEND_CPU = 1, MB3D_BACKEND_METAL = 2 } MB3DBackend;

typedef enum {
    /* Coarse-to-fine draft passes without AO, shadows, reflections and DOF. */
    MB3D_QUALITY_PREVIEW = 0,
    /* One pass at the requested size with every authored effect. */
    MB3D_QUALITY_FINAL = 1
} MB3DQuality;

/** Shared cancellation for one or many renders; cancel is thread-safe. A CPU
 *  pass stops at its next poll, a Metal pass before its next GPU command. */
typedef struct MB3DCancel MB3DCancel;
MB3DCancel* mb3d_cancel_create(void);
void mb3d_cancel_request(MB3DCancel* cancel);
void mb3d_cancel_reset(MB3DCancel* cancel);
void mb3d_cancel_destroy(MB3DCancel* cancel);

/** Progress of the full-size pass of a FINAL render as a fraction of its
 *  expected time, 0 to 1, never decreasing; it stays 0 during the quarter-size
 *  pass. Called from a library thread while the render runs; it must return
 *  quickly and must not call the library. */
typedef void (*MB3DProgressCallback)(double fraction, void* user);

/** What a render is doing, for telling a person. A frame goes through the
 *  stages its scene needs, in this order on the CPU; Metal may return to
 *  PREPARATION between stages, come back to SURFACE once after SHADOWS and run
 *  COLOR after REFLECTIONS. A stage the scene does not need is not reported. */
typedef enum {
    MB3D_STAGE_PREPARATION = 0,   /* buffers, tables, GPU programs */
    MB3D_STAGE_VOLUME = 1,        /* volumetric light */
    MB3D_STAGE_SURFACE = 2,       /* rays find the fractal's surface */
    MB3D_STAGE_NORMALS = 3,       /* surface directions from the depth picture */
    MB3D_STAGE_SHADOWS = 4,
    MB3D_STAGE_AMBIENT_OCCLUSION = 5,
    MB3D_STAGE_COLOR = 6,
    MB3D_STAGE_REFLECTIONS = 7,
    MB3D_STAGE_DEPTH_OF_FIELD = 8
} MB3DRenderStage;

/** A pass (0-based, of pass_count) has begun a stage. Called on the rendering
 *  thread; it must return quickly and must not call the library. */
typedef void (*MB3DStageCallback)(MB3DRenderStage stage, int pass, int pass_count, void* user);

typedef struct {
    int width, height;         /* 0: authored size. Aspect follows the request. */
    MB3DBackend backend;       /* AUTO: Metal when a library is given, else CPU */
    MB3DQuality quality;
    int thread_count;          /* CPU workers, 0: hardware concurrency */
    int preview_first_edge;    /* long edge of the first preview pass, 0: 160 */
    /* Path to metal_native_scene.metallib (in the app bundle). NULL: CPU. */
    const char* metal_library;
    /* FINAL on CPU only. DEFAULT is the build default (Pascal-compatible). */
    MB3DPrecisionMode precision;
    /* FINAL only, optional. With it, a frame whose long edge is at least 640 px
     * is preceded by the same frame at a quarter of the size in full quality
     * (pass 0 of 2); that pass also calibrates the reported fraction. */
    MB3DProgressCallback progress;
    void* progress_user;
    /* FINAL on Metal only; 0: the whole frame. A lattice frame: every
     * lattice_stride-th pixel (2 to 4) of the width x height frame, starting at
     * pixel (lattice_x, lattice_y), both below the stride. See "Lattice frames". */
    int lattice_stride, lattice_x, lattice_y;
    /* Optional: told the stages of every pass as they begin. With `progress`
     * the quarter-size pass goes through the stages of the full pass first,
     * which tells a caller what the full pass will consist of. */
    MB3DStageCallback stage;
    void* stage_user;
    /* Metal only. Nonzero: the GPU programs of this render call the scene's
     * formulas instead of repeating them at every place of use. A scene seen
     * for the first time then starts several times sooner (its programs
     * compile faster) at the same rendering speed. The programs are kept apart
     * from those of a request without it, so keep to one value for a scene.
     * See "Quick pipelines" in docs/LIBRARY.md for what was measured. */
    int quick_pipelines;
} MB3DRenderRequest;

typedef struct {
    const unsigned char* rgba; /* width*height*4; alpha 0 where nothing was hit */
    const unsigned short* depth; /* packed MB3D Zpos, > 32767 is background */
    int width, height;
    int pass, pass_count;      /* pass is 0-based; the last pass is the result */
    MB3DBackend backend;       /* backend that produced this pass */
    double render_seconds;     /* wall time of this pass, without the callback */
    /* Metal pipelines this pass had to compile (cold) or load from the disk
     * cache. Both are 0 once the scene's formula combination is warm. */
    int pipelines_compiled, pipelines_loaded;
    /* Time the GPU spent executing this pass's commands and their count; the
     * rest of render_seconds is CPU work and waiting. 0 on the CPU. */
    double gpu_seconds;
    int gpu_commands;
    /* Why a pass requested with AUTO and a Metal library ran on the CPU;
     * NULL when the pass ran where it was requested. Dies on return. */
    const char* cpu_fallback_reason;
} MB3DFrame;

/* ---- Lattice frames ------------------------------------------------------
 *
 * A FINAL frame can be rendered in parts that each cover the whole picture: a
 * request with `lattice_stride` s renders pixels (x + i*s, y + j*s) of the
 * frame and delivers them as one frame of ceil((width - x) / s) by
 * ceil((height - y) / s) pixels, in one pass and without the quarter-size pass.
 * The s*s requests with every (x, y) below s render every pixel of the frame
 * once, each in about 1/(s*s) of the frame's time and memory, so the picture
 * can be shown while it fills and a frame too large for the device's buffers
 * can still be rendered.
 *
 * Each pixel is computed as in the whole frame (same ray, thresholds and step
 * sizes), with two differences that make the assembled picture close to the
 * whole frame but not identical to it: screen-space ambient occlusion compares
 * a pixel with its neighbours on the lattice, s pixels apart, and stages that
 * carry state from pixel to pixel (hard-shadow shortcuts, dithering, the
 * random first step) carry it along the lattice. A frame for export is
 * rendered whole.
 *
 * Refused (MB3D_RENDER_UNSUPPORTED): the CPU backend, PREVIEW, scenes with
 * depth of field or Z-buffer normals (both read neighbours of the full frame),
 * and scenes the Metal renderer does not take.
 */

/**
 * The `index`-th lattice of a stride, 0 <= index < stride*stride, in an order
 * that covers the frame ever more finely: each next lattice lies as far as
 * possible from those before it. Returns 0 for a stride outside 2..4 or an
 * index outside the range.
 */
int mb3d_lattice_offset(int stride, int index, int* x, int* y);

/**
 * Puts a lattice frame into the full frame and fills the pixels that are not
 * rendered yet. `rgba` (width*height*4), `depth` (width*height) and `known`
 * (width*height, all zero before the first call) belong to the application and
 * are kept between calls; `part_rgba` and `part_depth` are a frame delivered
 * for (stride, x, y), `part_width` by `part_height` pixels.
 *
 * Rendered pixels are copied and marked in `known`. Every other pixel gets the
 * colour interpolated from the rendered pixels around it and the depth and
 * alpha of the nearest one, so after any number of parts the buffers hold a
 * complete picture, and after all of them exactly the rendered pixels.
 *
 * Returns how many of the stride*stride lattices are in the frame, or -1 for
 * sizes that do not belong together.
 */
int mb3d_lattice_merge(unsigned char* rgba, unsigned short* depth, unsigned char* known,
                       int width, int height,
                       const unsigned char* part_rgba, const unsigned short* part_depth,
                       int part_width, int part_height, int stride, int x, int y);

/** Called on the rendering thread after each pass. Buffers die on return. */
typedef void (*MB3DFrameCallback)(const MB3DFrame* frame, void* user);

/**
 * Render the scene. PREVIEW delivers several frames of growing resolution,
 * FINAL delivers one. `cancel` may be NULL. Returns MB3D_RENDER_SUCCESS,
 * MB3D_RENDER_CANCELLED, MB3D_RENDER_UNSUPPORTED or MB3D_RENDER_FAILED.
 */
MB3DRenderStatus mb3d_scene_render(const MB3DScene* scene, const MB3DRenderRequest* request,
                                   MB3DCancel* cancel, MB3DFrameCallback callback, void* user,
                                   char* error, size_t error_size);

/* ---- Persistent renderer ------------------------------------------------ */

/**
 * A renderer for many frames, for example interactive navigation. It keeps
 * the Metal device, a command queue, the shader library and the setup checks
 * that passed, so a frame does not read and hash the library again. Images
 * are identical to mb3d_scene_render.
 *
 * `metal_library` is read once here and replaces the request's metal_library;
 * NULL makes a CPU renderer. One renderer renders one frame at a time (calls
 * are serialized); use one renderer per concurrent stream. Any scene may be
 * rendered with it. NULL on failure.
 */
typedef struct MB3DRenderer MB3DRenderer;

/** Whether this device can render the request's FINAL frame (or lattice frame)
 *  on Metal: the scene's capabilities and the frame's buffers against the
 *  device's memory budget are checked as a render checks them, by the same
 *  engines in the same order, and nothing is dispatched to the GPU. It costs
 *  the host-side preparation of a frame and can run beside a render. SUCCESS:
 *  the frame is admitted (a render can still fail later, for instance when a
 *  pipeline does not compile). UNSUPPORTED: the reason is in `error`; "exceed
 *  device budget" in it means the frame is too large for the memory. Only
 *  width, height, the lattice fields and metal_library of the request are
 *  read. */
MB3DRenderStatus mb3d_scene_admits(const MB3DScene* scene, const MB3DRenderRequest* request,
                                   char* error, size_t error_size);

MB3DRenderer* mb3d_renderer_create(const char* metal_library, char* error, size_t error_size);
void mb3d_renderer_destroy(MB3DRenderer* renderer);
MB3DRenderStatus mb3d_renderer_render(MB3DRenderer* renderer, const MB3DScene* scene,
                                      const MB3DRenderRequest* request, MB3DCancel* cancel,
                                      MB3DFrameCallback callback, void* user,
                                      char* error, size_t error_size);

/* ---- Motion between rendered frames ------------------------------------- */

/**
 * Redraw a delivered frame as another camera would see the same surface, so an
 * application can follow a gesture at display rate while the next frame is
 * still rendering. `rgba` and `depth` are a frame of `from` (width*height*4 and
 * width*height); `out_rgba` receives width*height*4 bytes. `from` and `to` may
 * differ only in the camera. The background stays where it is; pixels nothing
 * lands on take the farthest neighbouring content, up to 64 pixels away, and
 * stay transparent black beyond that.
 *
 * Returns the fraction of output pixels a frame pixel landed on (1 for an
 * unchanged camera, lower as the camera uncovers what the frame did not see),
 * or a negative value for a projection the navigation camera does not model
 * and for the spherical panorama.
 */
double mb3d_scene_reproject_frame(const MB3DScene* from, const MB3DScene* to,
                                  const unsigned char* rgba, const unsigned short* depth,
                                  int width, int height, unsigned char* out_rgba);

#ifdef __cplusplus
}
#endif
