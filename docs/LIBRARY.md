# Library v1: state and application API

What the library contains, the application API built on it, and the viewer
prototype. [ARCHITECTURE.md](ARCHITECTURE.md) describes the CPU renderer and
formulas in depth, [GPU.md](GPU.md) the Metal renderer.

## What the library is

| Layer | Where | State |
|---|---|---|
| Scene model | `src/core/types.h` | Two packed Pascal disk structs: `TMandHeader10` (840 bytes: camera, lights, effects) and `THeaderCustomAddon` (1136 bytes: six formula slots and hybrid options) |
| Loader | `src/io/m3p_parser.cpp` | `.m3p` of every MB3D version and `.m3i`; file paths only |
| CPU renderer | `src/core/renderer.cpp` | Full MB3D pipeline, see below. Runs on any ARM64/x86-64 target |
| Formulas | `src/core/generated/`, `external_formulas/Formulas/` | 10 internal and about 460 external formulas compiled to C++ (457 external identities in the qualification inventory). No JIT, no x86 code; `.m3f` files are not executed |
| Metal renderer | `src/render/native_metal/`, `comparison/metal_exact/metal_device.h` | Hardware FP32, shaders precompiled into `metal_native_scene.metallib`, pipelines specialised per scene and cached on disk |
| Backend choice | `src/render/backend.cpp` | CPU, Metal or Auto with CPU fallback for unsupported scenes |
| Application API | `include/mb3d/mb3d_scene.h`, `src/render/mb3d_scene_api.cpp` | New in v1, described below |
| Swift wrapper | `swift/` | `MB3DKit` over the xcframework from `scripts/build_xcframework.sh` |

### CPU pipeline against Pascal MB3D

Implemented: ray marching with analytic and numerical DE, normals (inline and
from the Z-buffer), hard and soft shadows, four ambient occlusion types, dynamic
fog, volumetric light, light and background maps, colouring, reflections and
transparency, depth of field, Julia mode, alternating, interpolated and
DE-combinate hybrids, dIFS, planar/fisheye/panorama cameras, axis cut planes,
per-eye stereo shift, tiling, 2D slices and Monte Carlo.

Not ported from Pascal: animation and keyframe interpolation, the navigator,
voxel export, blocky and partial rendering, the formula JIT and parser
(`FormulaParser.pas`, `EM_JIT_M3Formulas`), and most of `ImageProcess.pas`.
ImageScale downscaling and the tiling crop exist only in the CLI, so API
images are the internal surface.

Closeness to Pascal is chosen per render. `MB3D_PRECISION_COMPATIBLE` emulates
x87 80-bit arithmetic with SoftFloat and is the reference mode; `NATIVE` uses
hardware doubles and is what previews and Metal use. Metal is FP32 and falls
back to the CPU when more than 1 % of a frame leaves the FP32 range.

### Metal coverage

Every render stage of the general plan is on the GPU: primary rays including
rays starting inside the surface, normals, hard and soft shadows, both AO
algorithms, colouring, reflections, DOF, volumetric light and maps. The
generated inventory marks all 457 formula shaders as source-generated and not
device-qualified. Output is always read back to CPU memory; frames up to
4096 pixels per side.

## Application API (`mb3d_scene.h`)

| Need | Calls |
|---|---|
| Scene in memory | `mb3d_scene_load`, `_clone`, `_save`, `_destroy` |
| Formula combinations | `mb3d_formula_count/name/exists`, `mb3d_scene_set_formula_m3f`, `_clear_formula`, `_set_formula_iterations`, `_get/_set_formula_param`, `_formula_param_name`, `_set_hybrid_mode` |
| Gestures | `mb3d_scene_rotate_camera`, `_roll_camera`, `_pan_camera`, `_dolly_camera`, `_move_camera`, `_set_navigation_aspect`, `_get/_set_camera` |
| Editor controls for colour and light | `mb3d_scene_get/_set_palette`, `_palette_positions`, `_palette_range`, `_fog`, `_light`, `_ambient`; detail is `_get/_set_iterations` |
| Fast preview | `mb3d_scene_render` with `MB3D_QUALITY_PREVIEW`: passes doubling from `preview_first_edge` to the requested size, effects off |
| Maximum quality | `mb3d_scene_render` with `MB3D_QUALITY_FINAL`: one pass, all effects, Pascal-compatible precision on the CPU |
| Many frames in a row | `mb3d_renderer_create`, `_render`, `_destroy`: a persistent renderer that keeps the Metal device, queue, shader library and passed setup checks between frames |
| Adapting to frame time | `MB3DFrame.render_seconds`, `gpu_seconds`, `gpu_commands`, `pipelines_compiled`, `pipelines_loaded` |
| Learning why the GPU was not used | `MB3DFrame.cpu_fallback_reason` |
| Progress bar of a final render | `MB3DRenderRequest.progress`: fraction of expected time; a frame of 640 px or more is preceded by a quarter-size full-quality pass that calibrates it |
| Telling a person what a render is doing | `MB3DRenderRequest.stage`: each pass reports the stages it begins (preparation, volume light, surface, shadows, ambient occlusion, colour, reflections, depth of field); with `progress` the quarter-size pass shows beforehand which stages the full pass has |
| A final frame that fills in on screen | `MB3DRenderRequest.lattice_stride/_x/_y`, `mb3d_lattice_offset`, `mb3d_lattice_merge`: the frame rendered in parts that each cover the whole picture |
| A scene that starts quickly the first time | `MB3DRenderRequest.quick_pipelines`: GPU programs that call the scene's formulas instead of repeating them, compiled several times sooner |
| Offering only the sizes a device can render | `mb3d_scene_admits`: whether a final frame or a lattice frame of a given size passes the Metal engine's checks, the device's memory budget among them, without rendering |
| Following a gesture between frames | `mb3d_scene_reproject_frame`: a delivered frame redrawn for another camera from its depth |
| Interrupting on edits | `mb3d_cancel_create/request/reset/destroy`: stops a CPU pass at its next poll and a Metal pass before its next GPU command |
| Apple Vision Pro eyes | `mb3d_scene_create_eye`: one scene per eye, parallel axes |
| Rendering inside a volume | `mb3d_scene_set_clip_volume` |

Frames are RGBA8 with alpha 0 where no surface was hit, plus the packed MB3D
depth, so an application can composite the fractal over its own content.

`mb3d_scene_render` is stateless: every call reads and hashes the 18 MB shader
library and repeats the setup marker dispatches. An interactive client renders
through `mb3d_renderer_render` instead, which does that once per renderer and
gives the same image. On an M4 Pro this removes about 11 ms from every preview
frame (for example 23 to 12 ms at 160 px for ABoxPartlyJuliaRoff); the rest of
a frame is GPU time that depends on the scene. A renderer renders one frame at
a time. GPU buffers are still allocated per frame and image maps are decoded
again; the renderer is used by the general Metal engine, which serves every
preview and the frames the first engine does not admit.

### Camera, near and far limits

MB3D has no near plane of its own. Every ray starts on a rectangle, the start
plane, `2.1345 / zoom` scene units wide, and the rays fan out by the field of
view. That is a pinhole camera standing behind the rectangle, at half its
height divided by tan(fov / 2), with the rectangle as its near clipping plane.
A ray whose start point is already inside the fractal is drawn as a flat cut
(MB3D colours it from the iteration count); scenes rendered "in and outside"
switch to the other side of the surface instead, and "inside rendering"
inverts the distance estimate for the whole frame. Both are honoured by the
CPU and Metal renderers. Rays end at the far limit, `dZend`, `MB3DCamera.depth`
behind the start plane; a ray that reaches it shows the background.

The MB3D navigator avoids cuts by measuring the distance estimate at the
camera and making the view, the step and the far limit proportional to it.
The gesture helpers do the same with one difference: they keep the viewpoint
and pull only the start plane towards it (the zoom grows by the same factor),
so the picture stays while less of the near space is clipped.

- After every `mb3d_scene_rotate_camera`, `_pan_camera`, `_dolly_camera` and
  `_move_camera` the distance estimate is taken at the centre of the start
  plane, on the CPU. The plane may be as large for that distance as it was in
  the camera the gestures started from (the loaded scene or the last
  `mb3d_scene_set_camera`), and at least as large as the navigator's 0.53 of
  it; a larger plane is pulled in, and let out again when there is room, not
  past the surface the camera looks at and not past its first size.
- `mb3d_scene_dolly_camera(amount)` moves by the distance to the surface in the
  middle of the picture, walked along the central ray with the distance
  estimate: that distance shrinks or grows by e^amount, so the camera comes
  ever closer without entering. Twelve steps of ln 2 on the test scene raise
  the zoom from 1 to 4630 with no cut pixel. With a pivot ahead the camera
  moves towards or away from it and the far limit stays in place; a camera at
  its pivot takes it along; backing away never brings the far limit nearer.
- `mb3d_scene_rotate_camera` turns about a point three times as far as the
  surface ahead when the pivot is further than that, and keeps it as the
  pivot. `mb3d_scene_pan_camera` moves in fractions of what the view shows at
  that surface.
- Directions follow MB3D's camera matrix, whose second row is the direction of
  growing pixel rows: `up` points to the bottom edge of the picture. A positive
  `up` of `_pan_camera` or `_move_camera` therefore moves the picture up, a
  positive pitch turns the view to the top, a positive yaw to the right, and a
  positive `mb3d_scene_roll_camera` angle turns the camera clockwise (the
  picture anticlockwise). `mb3d_scene.h` lists them; the scene API test pins
  them on rendered frames.
- A step is refused (the call returns 0, the camera stays) when no start plane
  of the new viewpoint is free, when the viewpoint would enter the fractal
  from free space, and when it would go further from where the gestures
  started than 1.2 times the first depth or three times the first distance to
  the surface. A dolly step is first tried at a half, a quarter and an eighth.
- One call costs 0.02 to 0.5 ms on an M4 Pro over the 21 gallery scenes of the
  application (a few hundred evaluations of the distance estimate).
- `mb3d_scene_set_navigation_aspect` gives the shape of the frames when the
  application fits the camera to another shape than the scene's own.
- Left unchecked: scenes whose own start plane cuts the fractal (BulboxCut of
  those 21), spherical panoramas, and scenes with a formula that reads a map.

`mb3d_scene_reproject_frame` moves every surface pixel of a delivered frame to
where another camera of the same scene sees it, on the CPU and without a
render: about 1 to 5 ms for a 320x240 frame on an M4 Pro. It knows the MB3D
camera (rays start on a plane and fan out) and the packed depth, which an
application cannot reconstruct from `MB3DCamera`. The background stays in
place, as MB3D draws it in screen space; uncovered pixels take the farthest
neighbouring content. Shading does not change with the view and pixels land
on whole positions, so it is a stand-in until the next rendered frame, not a
replacement for it.

A final frame can be rendered in parts so that an application shows it
filling in (`lattice_stride` 2 to 4 in the request, Metal only). A part, a
lattice frame, is every `stride`-th pixel of the frame along both axes from
the offset (`lattice_x`, `lattice_y`), delivered as a small frame of its own:
the rays, shadows and reflections are those of the same pixels of the whole
frame, and screen ambient occlusion is computed on the part's own depth as
for a frame of the part's size. `mb3d_lattice_offset` gives the offsets in an
order where each part lies as far as possible from those before it (the first
four of stride 4 are every second pixel), and `mb3d_lattice_merge` puts a
part into a full-size picture and fills the pixels not rendered yet from
those around them, so the picture is complete after every part. In Swift:
`RenderSettings.lattice`, `Lattice.sequence(stride:)` and `LatticePicture`.

- The parts together are close to the whole frame, not identical to it:
  ambient occlusion sees a coarser depth picture, and its dithering follows
  the part's rows. The whole frame stays the reference; an application
  renders pictures for export whole.
- Over the 21 gallery scenes at 960x1280 on an M4 Pro the first part of
  stride 4 arrived after 7 to 12 % of the whole frame's time, the sixteen
  parts took 1.17 to 1.81 times the whole frame (median 1.38) and the four
  parts of stride 2 took 1.06 to 1.20 times; one scene with a volume stage,
  which every part repeats, took 3.26 and 1.51 times
  ([record](../comparison/results/lattice-frames-20261111/REPORT.md)).
- Refused: the CPU backend, previews, depth of field, normals from the depth
  buffer, tiled renders. A part always renders in the general Metal engine,
  also for scenes whose whole frame the first engine takes.
- A part needs the buffers of a frame of its own size, so a picture too large
  for a device's memory as a whole frame can still be rendered in parts.

`mb3d_scene_admits` answers before a render whether the device takes a frame:
it runs the Metal engines, in the order a render tries them, up to the first
GPU command (capabilities, options, the buffers of the frame against the
limits the device reports, `recommendedMaxWorkingSetSize` and
`maxBufferLength`) and returns there. It
reads the width, height, lattice fields and shader library of a request,
needs no renderer and waits for none. `MB3D_RENDER_UNSUPPORTED` carries the
engine's reason; "exceed device budget" in it means memory, and the same
size may still be admitted as a lattice frame of stride 2 or 4. Admission is
not a promise: a pipeline can still fail to compile and the system can still
interrupt a frame. In Swift: `Renderer.refusal(of:settings:)`.

A formula is placed in a slot from its `.m3f` file: only parameter names and
defaults are read from the text `[OPTIONS]` section, following
`CustomFormulas.pas` for angle triples and sextets. Applications ship the
`mb3d/M3Formulas` folder as a resource.

The colour and light calls let an editor work without the MB3D structs: the
ten surface colours and their positions, the part of the colouring value the
palette covers, depth fog, the first global light (a pad position, brightness
and hard shadows) and the ambient strength. They write the fields the MB3D
lighting window writes, with its slider mappings (`LightAdjust.pas`), so both
backends show an edit and a value is stored on the MB3D slider grid; the
header gives the range and the step of each. A setter given what its getter
returned leaves the scene bytes unchanged, so opening an editor does not
alter the picture. The light pad is in view axes also for a light fixed to
the object.

### Quick pipelines

A scene's GPU programs are specialized when the scene is first rendered and
kept on disk afterwards; a changed shader library (every update of an
application) starts again. By default the formula program is inlined at every
place a kernel uses it, which is slow to compile. With
`MB3DRenderRequest.quick_pipelines` (Swift: `RenderSettings.quickPipelines`)
every kernel of the general Metal engine calls it instead, as the reflection
and shadow kernels always do.

- Measured on 2026-11-12
  ([record](../comparison/results/quick-pipelines-20261112/REPORT.md)): the
  first frame of a new scene in the application on an iPad Pro M1 after a
  median of 3.4 s instead of 25.8 s (99 scenes); cold `mb3d_render` at 320 px
  on an M4 Pro in 1.8 to 3.7 s instead of 11 to 30 s (four scenes); whole
  frames at authored size as fast as before (four scenes).
- All 140 gallery scenes rendered at 480 px both ways are byte-identical. The
  call boundary can change how the compiler contracts multiply-add, so
  identity is measured for those scenes, not promised for every scene.
- The two kinds of program are stored apart: a scene rendered both ways
  compiles twice. An application keeps to one value.
- It has no effect on the CPU or on the first Metal engine, which takes whole
  final frames of the scenes it admits.

### Limits of v1

- Clip volume: box faces the eye looks at from outside cut the fractal exactly,
  and identically on both backends, because they are MB3D cut planes. The far
  side is a plane through the farthest corner. Geometry beside the box is not
  masked, and shadows see geometry behind the box.
- Stereo is two renders with parallel axes; convergence is an image shift in the
  application. There are no off-axis projection matrices, so head-tracked
  immersive rendering needs a new camera path in both renderers.
- A Metal pass is cancelled before its next GPU command; a command already
  running finishes first. On an M4 Pro a cancel request during a 2048 px
  FINAL frame or a 1280 px PREVIEW frame of TreePlanet and MarkJayBee -
  Curvichrome was answered in 5 to 570 ms. Commands are not split to bound
  that time. CPU passes, including the CPU fallback of a Metal request, stop
  cooperatively.
- The first Metal render of a new formula combination compiles pipelines
  (13 s for the test scene on an M-series Mac, then 0.04 s per 480x360 frame).
- Navigation trusts the scene's distance estimate, scaled by the share of it
  the scene's ray marcher steps. Where the estimate is poor a start plane can
  still touch the fractal (a patch of cut pixels at the frame edge was seen in
  TreePlanet close to a branch), and a start plane smaller than 2e-5 of the
  scene coordinates is refused because the Metal renderer leaves the binary32
  range there, which ends a fly-in at a zoom of about 1e4 to 1e5 times the
  authored one. Iterations are not raised with the zoom.
- Reprojection does not model the spherical panorama camera, moves a
  background image map with the screen instead of the view, and has no
  sub-pixel filtering.
- No `MTLTexture` output and no caller-supplied `MTLDevice`.
- Scenes load from paths, not memory buffers.
- The colour and light setters reach the surface palette, depth fog, one
  global light and the ambient strength. Everything else comes from the loaded
  `.m3p`: the other lights, light colour and type, positional lights and light
  maps, dynamic fog, diffuse and specular strength, the specular colours of
  the palette, interior colours, colour maps, the background image, ambient
  occlusion, reflections and depth of field.
- A control can be without effect in a scene: the palette where a diffuse
  colour map colours the surface, depth fog where the fractal sits in the
  nearest part of the scene's depth range, the ambient strength where the
  ambient light comes from a background image, and hard shadows in previews.
  The palette range is in units of the colouring value, whose spread over the
  surface differs from scene to scene.
- The sparse navigation previews (`navigation_preview`, the experimental
  `general_navigation`) are used only by `apps/macos_viewer`.

Open work on these limits is listed in [REMAINING.md](REMAINING.md).

## macOS viewer prototype

`apps/macos_viewer` is an experimental AppKit and Objective-C++ client, not part
of the library and not a qualification tool. It links the static engine
archives and a packaged Metal library, and shows the RGB the GPU computed; there
is no direct Metal texture output. [BUILD.md](BUILD.md#macos-viewer-prototype)
has the build and launch commands.

| Mode | Work while the camera moves | Approximation |
|---|---|---|
| Fast shading | Full geometry on the general Metal renderer | AO, hard shadows, reflections and DOF off |
| Grid | One ray per 2×2 block | Neutral lighting, the ray's colour fills the block |
| Points (default) | One pseudo-random position per 4×4 block | Same lighting; a stable seed prevents flicker |
| Full effects | The ordinary Metal render | None |

- Navigation is Orbit or Fly with the mouse. Moving previews adapt between 160
  and 480 pixels on the long edge; 640 pixels are restored when the camera
  rests.
- After 300 ms without input the remaining sparse phases run (16 for Points,
  4 for Grid), each adding new pixels and keeping the computed ones, so the
  finished preview has every pixel computed exactly once. Full effects then
  start automatically for the same camera. A camera change resets accumulation;
  there is no temporal reprojection.
- Sparse sampling exists for the restricted primary engine (integer bulb, sphere
  IFS, box IFS). Other scenes use the approximate general sparse navigator,
  which the GUI enables by default and headless services enable with
  `MB3D_VIEWER_GENERAL_PREVIEW=1`; it uses a 64-iteration geometry profile.
- The window runs two persistent services of the same binary, one for previews
  and one for full effects, talking over pipes with a private protocol. Starting
  to move cancels the full-effects service at once and keeps the warm preview
  device, pipelines and buffers. A GPU command already submitted may still
  finish.
- Metal only: a CPU fallback is refused, and a GPU fault is an error, not a
  retry by another path. A request stops at the 180 s diagnostic threshold.
- Mono scenes with the ordinary, planar and spherical cameras are supported;
  stereo scenes are refused. The camera is not written back to the scene file,
  and there is no final export.
- The package is ad hoc signed for the local Apple Silicon Mac; distribution and
  notarization are untested.

The viewer's resolution ladder, request epochs and service protocol
(`presentation.h`, `service.h`) are the model for what `mb3d_scene.h` still
lacks: prepared scenes that survive a camera change, capability queries and
job handles.

## Building

`docs/BUILD.md` owns the CMake commands. For applications:

```sh
scripts/build_xcframework.sh            # macOS, iOS, visionOS and simulators
scripts/build_xcframework.sh macos ios  # a subset
```

The result is `xcframework/MB3D.xcframework` (one merged static library
and the C headers per platform) and `xcframework/metallib/<platform>/
metal_native_scene.metallib`, which the application adds to its bundle and
passes as `metal_library`. `swift/` is a Swift package over that framework.

The contract is checked by `scene_api_tests` (label `release`). With Metal
built it also renders through a persistent renderer on the GPU and requires
the stateless image, byte for byte, across camera changes. For the colour and
light calls it requires, on the CPU, an unchanged picture after writing back
what was read, a changed picture for every control, the documented pad
orientation, and clamping or refusal of input outside the range.
