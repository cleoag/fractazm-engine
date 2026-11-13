# Native Metal GPU renderer

The main CLI renders Mandelbulb3D scenes on Apple GPUs with Native Metal, in
hardware binary32. Build commands are in [BUILD.md](BUILD.md), open work in
[REMAINING.md](REMAINING.md), the application API in [LIBRARY.md](LIBRARY.md).
Reports under `comparison/results/` are evidence, not instructions.

## State

| Item | State |
|---|---|
| On the GPU | Primary rays (also from inside), normals (DE, direct orbit, Z-buffer), hard, positional and soft shadows, DE AO and HiQ AO, colour with light geometry and maps, volume light, iteration fog, reflections (1-8 bounces), transmission, ordered DOF |
| Formulas | 457 external and 10 internal, lowered statically to FP32 Metal; DE combinations (`FormulaType` 1-6); random first step |
| Limits | Frames up to 4096 pixels per side, read back to CPU memory; no Monte Carlo or JIT (excluded by the owner); no explicit `RenderControl` requests |

Static lowering is implementation coverage, not device qualification
([NATIVE_ACCEPTANCE_PLAN.md](NATIVE_ACCEPTANCE_PLAN.md)). Coverage at 160x120,
`--threads 6`, authored RNG (a measure, not qualification): 73 of 77 ordinary
scenes render on the GPU, 4 take `FP32_RANGE` (ranks 25, 30, 50, 73); 447 of 454
`extall_*` scenes, 7; no faults or timeouts. Ordered scenes (ranks 56, 70, 68)
are far slower than CPU (81, 27, 20 s against 1.9, 0.8, 0.3 s); no balanced
original-size speed measurement exists.

## Running

```bash
build-main/mb3d_render <scene.m3p> --backend metal --output <out.png> \
  [--width W --height H] [--threads N] [--deterministic] \
  [--profile-json <profile.json>] [--metal-library <lib.metallib>]
```

- `--backend cpu|metal|auto` (once; CPU is the default). Metal and Auto share one
  policy, imply `--precision native` and reject an explicit Compatible.
- Run from a directory whose `M3Maps/` holds the scene's maps. Missing maps
  leave the legacy inactive state, which does not qualify a scene needing the
  resource. Replacement maps are labelled as selected, never as originals.
- `--metal-library` overrides `metal_native_scene.metallib` beside the real
  executable. `--deterministic` changes the authored primary and AO jitter
  flags: a parity control, not evidence for the unchanged scene.
- `--profile-json` (`mb3d.render-stage-profile.v1`) records backend, fallback
  code, stage times and `metal_execution` counters (`engine`,
  `pipeline_archive` `[hits, misses]`, `fp32_range_*`, work counters).

| Environment variable | Effect |
|---|---|
| `MB3D_METAL_PIPELINE_CACHE` | Archive directory (default `~/Library/Caches/mb3d/metal-pipelines`); `off` disables |
| `MB3D_NATIVE_FP32_RANGE_PERCENT` | Diagnostic: replaces the 1 % range limit (0..100) |
| `MB3D_NATIVE_OUTLINE_DE=all` | Diagnostic: outlines every formula call site; can shift FP32 results |

### Fallback

A scene the GPU cannot take renders as a whole on Native CPU, with a code:

| Code | Cause |
|---|---|
| `unsupported_options` | Shadow-disable override, diagnostics, traces, any `RenderControl` |
| `unsupported_scene` | Unsupported formula, camera, effect or parameter range |
| `metal_not_built`, `metal_unavailable` | No compiled adapter, device or library |
| `fp32_range` | After dispatch: over 1 % of geometry pixels left binary32 |

`fp32_range` is the only post-dispatch CPU fallback. Faults in device, library,
ABI marker or records fail with no image. `allow_cpu_fallback=false` (viewer)
turns every fallback into a failed request. Rejected before dispatch:
reflections with visible lights or a background map inside a material; a
background map with image coordinates (`bDirectImageCoord`); dIFS-only
transmission and transmission from inside rendering; volume light with a
persistent primary owner; integer or raw formula parameters outside binary32;
frames beyond the device budget.

### Cold and warm

The first render after a new library, GPU or macOS version compiles pipelines,
usually under a minute. A pipeline the driver cannot compile ends the render
with the compiler's error. Warm the archive on the rendering machine; a 64x48
warm-up serves every size and thread count
(`python3 scripts/warm_metal_pipelines.py --binary B --library L --runtime
<root>`). Every GPU timing states cold or warm.

## Architecture

### Engines

`RenderWithBackend` (`src/render/backend.*`) wraps rendering for the CLI;
`Render`, `PaintFromSiLight` and the C API stay CPU. Order: restricted adapter,
then the general engine (after a pre-dispatch `Unsupported` or a valid result
with an initial-inside hit, handed over before surface packing; preliminary work
is counted, faults still fail), then Native CPU. `prefer_general_metal` in
`BackendOptions` skips the restricted adapter.

| Engine | Where | Scope |
|---|---|---|
| Restricted | `src/render/metal_renderer.mm`, `comparison/metal_exact/native_*` | Internal Integer Power 8; shadows, AO, opaque reflections (1-2 bounces) |
| General | `src/render/native_metal/` (`general_renderer.mm`) | Registered formulas and hybrids, every stage above; 188-byte task state, 8192-byte pointer-free formula program |

Host preparation binds Native binary64 and libmath on copies of the effective
header and addon and uploads constants and tables; no pixel result is computed
on the CPU inside a Metal render. Output is RGB plus the imported `TsiLight5`
surface. CMake: `MB3D_ENABLE_METAL_RENDERER` defaults ON only for Apple ARM64
and builds `metal_native_scene.metallib` (target `mb3d_metal_native_shaders`)
beside `mb3d_render`; `MB3D_BUILD_METAL_EXACT_SCENE`,
`MB3D_BUILD_METAL_FORMULA_PROBE`, `MB3D_BUILD_METAL_PROBE` and
`MB3D_BUILD_GENERAL_NAVIGATION_EXPERIMENT` (all OFF) add optional targets.

### Render plan and stages

`native_metal/render_plan.{h,cpp}` prepares every stage once for runtime and for
host-only admission (`mb3d_native_formula_host`, no Metal link). Requested,
previously calculated and effective effects are separate fields, and requested
maps are captured before a failed load clears a flag (`resource_requirements.h`).

- Shadows: a multi-light kernel (mask 1 keeps the single-light one); positional
  lights march there only. The last selected light sets the soft six-bit
  visibility; soft reflection shadows need the primary soft pass.
- Reflections are deferred: trace kernels write variable-length shade records,
  `native_camera_inside_reflection_shade` evaluates them in SR_CalcRay order.
  Transmission is an explicit depth-first stack. Secondary rays end as open air
  after `kSecondaryRayStepLimit` (100000) marches or on NaN.
- Colour payloads (`NativeHitColorPayload`) are captured after the normals and
  before shadow and AO. Numerical DE restores `Rout` but keeps `Rold` of the
  last gradient orbit; that pair is intentional.
- Every record is versioned, validated before an image is published and carries
  work counters; `execution_plan` in the profile is configuration, not activity.
- Lattice frames (`RenderPlanRequest.lattice`, CLI `--lattice s,x,y`): the
  allocated pixels are every `s`-th pixel (2 to 4) of the full frame from the
  offset. `mb3d_camera::Params.stride_extra` (`s-1`, 0 for a dense frame)
  scales the pixel position in the ray origin, the paint position and the
  shadow length; the host builds the axis tables and the colour rows and
  columns for the same positions. Geometry, shadows, reflections and colour
  are those of the same pixels of the dense frame. Screen AO is prepared for a
  header of the lattice's size and reads the lattice's own depth; DE AO
  dithering follows the lattice's rows. Depth of field, Z-buffer normals,
  tiles, the CPU and the restricted engine refuse a lattice. With
  `stride_extra` 0 every expression has its earlier value: 21 scenes at
  960x1280 gave byte-identical images before and after the change
  ([record](../comparison/results/lattice-frames-20261111/REPORT.md)).

### Formula shaders

`scripts/gen_native_metal_formulas.py` reads the registered ARM source bodies
and writes `src/render/native_metal/generated/` (`formula_<id>.metal.h`,
`family_*`, `formula_indefinite.metal.h` for the 13 bodies returning the x87
indefinite NaN, `formula_state_effects.h`). A changed source pattern fails
generation; `--check` regenerates and compares every output, and CMake runs it
inside the metallib build. No runtime formula compiler, M3F machine code or CPU
callback exists on this path. Selectors and the orbit-driver constant must match
the uploaded program; typed integer parameters decode from binary64 bits, never
through FP32; counters wrap as int32.

### Libraries and caches

`metal_native_scene.metallib` is built from separate AIR units: scene and
general kernels with `-ffp-contract=fast`; DOF, Z-buffer, screen AO and DE AO
with contraction off. Its profile marker `[2,4,32]` is checked on every render.

- **Pipeline archive.** Specialized pipelines persist as `MTLBinaryArchive`
  files, newest kept within 2 GiB, keyed by library SHA-256, device, OS version
  and every specialization. A compile failure is not retried without the archive;
  only an archive fault falls back to a plain compile. It cannot be shipped.
  The iOS and visionOS simulators run without it, because Metal aborts the
  process there when an archive is serialized; pipelines compile plainly. In
  memory: 4 libraries of 64 pipelines; compiled code is reused, never results.
- **Outlining.** Function constant 21 outlines every formula call site in the
  reflection kernel, every shadow pipeline and every pipeline of a program with
  `_MapTranslate`, HeightMapIFS or HeightMap2IFS: inlined copies took the
  backend compiler minutes or ran `MTLCompilerService` out of address space
  (rank 6). Images stayed bit-identical; the shadow stage is about 8 % slower
  ([record](../comparison/results/native-de-outline-20261029/REPORT.md)).
  `MB3DRenderRequest.quick_pipelines` sets the constant for every pipeline of
  a request, as the diagnostic variable does: cold pipelines compiled 6 to 9
  times sooner and 140 gallery scenes stayed byte-identical at 480 px
  ([record](../comparison/results/quick-pipelines-20261112/REPORT.md)). Other
  constants: 17 smoothing, 18 wrapper policy, 19 and 20 combinations, 22 random
  first step.

### Dispatch sizes and budgets

| Work | Per command |
|---|---|
| Primary, speculative shadow passes, volume cells, soft-shadow and reflection pixels; DE AO | `kPixelDispatchBatch`, `kDEPixelBatch` = 262,144 pixels (reflections also by a 256 MB shade-slot budget) |
| Non-random HiQ AO | 16,384 pixels; image segments: at most 8 encoders and 65,536 invocations in one AO phase |

- Per-pixel cost has a long tail: at 16,384 pixels per command reflections ran
  at a quarter of throughput. Every command commits and waits; cancellation is
  checked between commands. A failed single-stage command reports its kernel,
  thread count and waiting time in front of Metal's text
  (`[native_camera_general_primary, 71680 threads, 149 ms] ...`). A failed segment is poisoned; failure forbids
  publication and CPU fallback.
- `NativeDeviceBudget` is half the recommended working set (1 GiB if none
  reported); a frame that does not fit goes to CPU before GPU work.
- Step limits: secondary rays 2^22, DE calls 2^24 per record, primary and
  shadow passes 65,536, formula work 2^28 per record. A worst-case geometric
  bound is not an admission criterion. An exhausted shadow or AO march faults.

### Ownership and parallelism

Pixels, rows and rays run in parallel only where an audit proves independence;
the rest keeps the CPU's logical task order.

| Stage | Parallel unit | Stays ordered |
|---|---|---|
| Primary | Pixels under the independent state contract (Hybrid3D DE0, Hybrid3DAnalytic DE2/11, conditional HybridDifs DE20, single-part Hybrid4D DE4) or a closed chain | Other 4D and interpolated drivers; ranks 56, 70; conditional dIFS frames with carried reads (rank 68) re-render in task order |
| Hard shadows | Speculative pixels, then a per-row scan replays the ordered shortcut caches (`WholeRow`); a pass-0 or scan fault selects the ordered row kernel | Persistent formulas (`ContiguousRows`) |
| Soft shadows, DE AO, reflections | Pixels wherever each starts a fresh lane; bounces in order | Persistent tasks and formulas |
| Screen AO | 16,384-pixel commands | Axes, levels, repeats; legacy RNG grouping |
| DOF | Tiles | Passes, equal-depth ties, per-pass quantization |

- `pixel_state_contract.h` keeps reads before writes, must-writes, possible
  writes, entry initialization and immutable live inputs as separate masks
  (Julia values and hybrid counts are live inputs). Slot masks come from a hand
  audit or `formula_state_effects.h`; carried `derivative[0..2]` lanes are exempt
  only under 3D drivers when every reading body is compiler-proven
  self-contained. Closed chains in `pixel_independence.h` bind namespace, ID,
  parameters, traits, wrapper and signed counts.
- Conditional dIFS: a HybridDifs part reads carried state only when no
  iteration improves the 65535 entry minimum, and counts `difs_carried_reads`.
  Any counted read discards the frame, checked right after the primary stage,
  and renders it again in CPU order on the GPU (`ordered_carried_state`).
- Random first step (`iOptions` bit 0): a probe kernel marks marching pixels, a
  bounded GPU prefix scan with exact integer LCG jumps (`seed_scan_*`) derives
  each entering seed in task order, and the device faults unless each exit seed
  equals its successor's entering seed. Task `t` starts at
  `PasLcgInitFromThread(t+1)`; CPU and GPU comparisons use the same `--threads`.

### Navigation preview sessions

For the optional viewer (`apps/macos_viewer/`); neither session extends
full-scene or formula acceptance. `NavigationPreviewSession`
(`navigation_preview.*`, own metallib) serves closed fresh-state capabilities
with neutral lighting; phases are consecutive and disjoint, so full accumulation
runs exactly width*height rays. `GeneralNavigationSession`
(`general_navigation.*`, own metallib) samples general formulas with an
approximate 64-iteration headlight profile; conditional dIFS and dependent
traversal stay ordered; a GPU display pass interpolates only unmeasured pixels
into a separate buffer; a failed or cancelled phase publishes no RGB. The GUI
sets `MB3D_VIEWER_GENERAL_PREVIEW=1` (`0` opts out);
`MB3D_VIEWER_ORDERED_CONTROL=1` forces ordered navigation.

### Exact compatible Metal pipeline

`comparison/metal_exact/` holds the shared compatible and native scene pipeline,
a dependency of the restricted adapter; its standalone host
`mb3d_metal_exact_scene` (`comparison/tools/metal_exact_scene.mm`) is separate
from the CLI. The pinned catalog is `scene_manifest.inc`; admission reads raw M3P
flags before loader normalization (`--describe` exports the descriptor without
GPU work); `--full-shading` adds surface packing, one-light hard shadows, HiQ AO
and colour. Device arithmetic is integer-backed nearest-even binary32, binary64
and extended80; no soft shadows, volume, reflections, DOF or CPU compute
fallback. Independent CPU adapters call production code and every result byte
must match ([PARITY_TRACE.md](PARITY_TRACE.md)). The host takes
`--precision compatible|native` (default compatible; Native only with
`--benchmark-render --full-shading` or `--describe`); library marker `[1,4,80]`
(compatible) or `[2,4,32]` (native), a mismatch fails.

### Isolated probes

`mb3d_metal_formula_probe`, `mb3d_metal_downscale_probe` and
`mb3d_native_formula_probe` are unqualified feasibility tools.

## Rules for changing GPU code

"Owner rule" marks a decision the owner made explicitly; change it only when
asked. Routine releases use the
[small release check](BUILD.md#default-small-release-check) and a focused
regression; full-corpus and original-Pascal requirements apply to an explicit
full-qualification claim. Arithmetic, fault and fallback rules are never
weakened to pass a smoke check.

### Scope and status words

- **Goal (owner rule).** Render every ordinary scene and external formula on the
  GPU in parallel within binary32, preserving formulas, hybrid order, settings
  and effects; never disable an effect to claim completion. Use original
  resources or owner-selected maps pinned in the contract; selected maps never
  authenticate original appearance.
- **Corpus.** The 77 scenes of `mb3d/M3Parameter` (`comparison/scene-queue.json`)
  and 454 `extall_*` scenes. **Monte Carlo and JIT are excluded (owner rule).**
- Native GPU is never reported as CPU/Pascal parity. A compilation, admission,
  64x48 plan, preview or coverage count is a step, not qualification; the
  contract `comparison/contracts/native-gpu-acceptance-v4/acceptance-contract.json`
  stays OPEN until every obligation passes.
- Coverage status words: `GPU`, `FP32_RANGE` and `FALLBACK` (Native CPU by
  policy or rejection), `GPU_FAULT` (no image), `TIMEOUT`, `QUALIFIED_FP32_CPU`
  (accepted whole-frame CPU outcome; never GPU coverage, parity or a speed
  sample).

### Execution ownership and arithmetic

- **No CPU per-pixel work inside a reported Metal render.** The host prepares
  constants, validates records and never evaluates pixel formulas; whole-job CPU
  fallback is decided before dispatch (`fp32_range` is the one exception). A
  mixed frame cannot be attributed or qualified.
- **Parallel by proof.** Regrouping grants nothing and keeps logical task and row
  ownership; a lane using group memory, barriers or SIMD collectives needs a new
  audit. Dependent bounces, normal samples and stateful samples stay ordered.
- **Primary state contract (owner rule).** An independent contract selects pixel
  ownership; there is no per-chain list. A union of body must-writes is not a
  query must-write (an early escape skips bodies). A primary proof never
  authorizes shadow, AO or reflection ownership.
- **Owner change acceptance.** RGB and every non-timing work counter are
  byte-identical to the old ownership for each affected scene. A pure FP32 kernel
  difference needs a fresh-lane control (lanes reset before each pixel or row)
  byte-identical to the old frame (`native_primary_admission.py
  --control-library`) and is recorded as `FP32_KERNEL_DIFFERENCE`, never a match.
- **No memoization** of DE or orbit results by position; no single unbounded
  kernel per frame.
- **Arithmetic.** MSL has no `double`, so no x87 path exists on the GPU. The
  compiler places FMA contractions per kernel: the same source can round
  differently in two kernels or outlined versus inlined. The same binary, library
  and `--threads` repeat byte for byte. A changed frozen Native image needs an
  isolated cause, kept before/after images and review; never just update the
  reference. Compatible-library support is never inferred from a main-renderer
  result.

### FP32 range policy (owner rule)

- **Range exit.** A formula step whose state leaves binary32 is not a fault: the
  orbit restores the state before that step and escapes. At most
  `kNativeRangeExitsPerDistance` (9) per DE query plus one per direct-normal
  orbit are counted. Single-step probes keep the strict fault. Exits never
  support parity.
- **The 1 % rule.** A frame stays on the GPU while at most 1 % of geometry pixels
  take an exit; beyond that the validated frame is discarded and Native CPU
  renders it (`fp32_range`, console note). Volume light applies the rule to map
  cells; DE AO exits are reported, not counted. The decision falls right after
  the primary pass if primary pixels alone exceed 1 % (this avoids compiling
  large secondary kernels), otherwise after all records validate.
- **Parameter range.** A finite binary64 real parameter beyond binary32 uploads
  as +-infinity (BOXSCALE, reciprocal options of a zero input); a pass that
  consumes it takes a counted exit. Integer and raw lanes and other prepared
  scalars keep the strict check (`unsupported_scene`). A radius stop beyond
  binary32 clamps to `FLT_MAX`.
- **Box inner scale is never clamped:** the test runs on binary64 `MinR^2` bits;
  a finite clamp would invent geometry.
- **dIFS inert truncation.** A dIFS step leaving binary32 after the minimum
  stopped improving ends the search uncounted; a falling minimum still counts.
- **Gradient recovery needs a proof.** An overflowing radius difference may
  certify the existing minimum only through an explicit conservative bound at an
  executed radius update (numerical-gradient policy 4), per orbit, counted for
  successful queries (`gradient_range_clamps <= de_calls`), never carried on. An
  infinite denominator alone proves nothing; the derivative is never saturated.
- **Genuine overflow stays on CPU** (see the classes below); a GPU path needs a
  new numerical method, an owner decision.
- **Accepted FP32 CPU outcome (owner rule).** A whole Native CPU frame is
  `QUALIFIED_FP32_CPU` only with an original-size frame, pinned approved
  resources, active effects, three exact repeats and an independent
  exact-arithmetic proof, on recorded finite operands, that over 1 % of distinct
  geometry pixels overflow. Publish proven and unattributed counts; a point probe
  or counter label is no proof. Such scenes stay out of the GPU speedup
  denominator. Qualified on their frozen build: ExcludeBulbMeng, HalTenny -
  Freak Of Nature, Oxnot - Shells
  ([records](../comparison/results/native-fp32-acceptance-20261030/REPORT.md),
  [Shells](../comparison/results/native-fp32-shells-20261031/REPORT.md)); a new
  build needs its own checks (`fp32_overflow_witness.py`,
  `verify_fp32_cpu_acceptance.py`).

### Formula lowering and effects

- Specialization binds uploaded namespace and ID, orbit driver and wrapper
  policy; FP32 permission never changes an integer branch. Translation defects
  (source order, signed zero, integer parameter, caller transport) are fixed in
  the lowering, never classified as FP32 noise.
- **Effect masks** come from a hand audit or optimized AIR
  (`native_formula_effects.py`, `native_formula_effects_header.py`). CMake fails
  on stale masks: after any formula body, wrapper or `formula_math.metal.h`
  change regenerate both.
- **Wrapper policy.** Production policy 1 covers only the closed Sierpinski4ex /
  reciprocalY / MixPinski4 chain; any enabled Makin slot forces policy 0. A
  wrapper change is compared against policy 0 and a frozen library on full
  device state; after structural shader edits requalify full scenes.
- **Effects.** AO uses exact per-pixel seeds in logical order and ordered wavelet
  axes, levels and repeats; screen AO keeps the legacy RNG grouping. DOF is
  ordered, ties kept, RGB quantized after every pass. Saved calculated soft
  masks differ from an automatic soft-pass request.
- **Materials.** Keep the paint matrix Y column as is. Initial-inside hits keep
  the interior branch; never drop finite and ABI checks to admit a scene. ABI
  and readback faults reach the publication failure gate, not only a counter.
- **Effect evidence.** An effect counts only with GPU activity counters and a
  same-engine on/off control; a flag or engine switch is not evidence.

### Verification

- **Frame limit.** 180 s diagnostic threshold and 300 s hard stop for a complete
  frame including preparation and effects; investigate slow frames, never raise
  the deadline.
- **Frozen inputs.** Use frozen copies of binary and library for long runs; never
  rebuild while a GPU run reads the build directory. Delete
  `metal_native_scene.metallib` before a shader rebuild (Make 3.81 can miss an
  edit within a second); give library variants unique paths.
- **After a structural change:** compare buffer sizes, rejection reasons and
  existing RGB and work; run active effect controls; three exact full repeats for
  newly admitted scenes.
- **Speed claims** use balanced A/B and B/A fresh processes with warm archives and
  thresholds fixed beforehand. Evidence goes to `comparison/results/<record>/`.

### Rejected alternatives

- CPU per-pixel fallback inside Metal (unattributable mixed frames); finite clamp
  of box `Scale/MinR^2` (invents geometry); derivative `w` saturation (no
  effect, the overflow is in the coordinates).
- Per-chain independence allowlists (replaced by the state contract); owner-local
  dIFS carried-read count (all carried reads of rank 68 precede an improving
  orbit); DE or orbit memoization by position (no complete state proof); fixed
  MixPinski4 / Makin wrapper (changed rounding, missed full-frame differences);
  screen AO `IndependentOwners` (missed its speed threshold).
- Loop-unroll pragmas, outlining chosen by source size (compiler still ran out of
  space); one giant kernel (compile time); CUDA first (Metal is the target).

## FP32 limits

Native CPU uses binary64, Native Metal binary32; a difference is allowed only
when explained, and nothing here is CPU/Pascal parity.

### Classifying a difference

1. **Exact repeatability:** the Metal frame repeats byte for byte, or it is a
   defect. Use the **same `--threads`** for CPU and GPU frames.
2. **Camera-shift sensitivity.** Shift the camera centre (`dXmid/dYmid/dZmid`,
   header offsets 36/44/52) by `2^-24*max(|v|,1)` and compare CPU with CPU. If
   CPU moves about as much as GPU differs from CPU, the scene is chaotic at
   binary32 resolution; if GPU-vs-CPU is far larger, look for a deterministic
   defect. The test can clear a scene, not convict it.
3. **Buffer split.** Dump `TsiLight5` on both backends and compare hits, `Zpos`,
   normals, roughness byte, AO, `SIgradient`, orbit trap. Equal geometry with
   noisy normals is the gradient class; a regional shift is not.
4. **Signed bias.** Noise has a near-zero signed mean of GPU-CPU per channel.
5. **Fresh-lane control** separates a kernel difference from an ordering defect.

### Accepted classes

| Class | Criterion | Examples |
|---|---|---|
| Coordinate or squared-radius overflow | A required orbit value exceeds binary32 before escape; Native CPU (`fp32_range`) | spineJulia 73, HalTenny 30, ExcludeBulbMeng 25, Shells 50; extall Tglad scenes (GPU `Extended80` is `float`), Riemann2, gyroidIFS (CPU yields NaN) |
| Normal noise of a chaotic DE | Hit mask and `Zpos` equal; normals differ with a small median and a long tail; no signed bias | Quad3Db, TrifoxComplexAngles, corpus 11, 14, 17, 45 |
| Within CPU's own sensitivity | GPU-vs-CPU about the size of CPU-vs-shifted-CPU | Mandalay-KIFS, Mandalex, corpus 67, KaliDucks, Kalisets1, Home of the Ancients, Hydra |
| Kernel and compiler | Same source in different kernels places FMA differently; `-ffp-contract=off` makes them identical | KentPress, Test for SurfMod, Virtual tubes world |
| Hardware sine | MSL permits 4 ULP for sine; an isolated sine returns `0xb55110b3` where ideal RN32 is `0xb55110b4` (about 1.38 ULP); stays an ideal-RN32 mismatch, no tolerance | PolyFold-sym |

The classes cover only the scenes named. Housing has identical hit masks but a
signed RGB bias (-3.59 against 0.001 under the CPU camera intervention); Lenord
(490 hit-mask disagreements, median normal angle 13.9 degrees) fails every
sensitivity gate. Both are outside every class. A scalar FP32 reference
declares its FMA policy (general AIR contracts, ordered AO, DOF, Z-buffer do not).

### Open FP32 questions

- Sin4Dplus (34 % of pixels) and Fuzzy (61 %), x87-indefinite formulas that leave
  the range: GPU behaviour not diagnosed. IFS_tess (extall 73): mean 4.06 while
  CPU is insensitive to the camera shift.
- Ringlets (51) and Peacock in blue (64): GPU-CPU means 3.309 and 3.021 against
  CPU shift means 0.103 and 0.002; Peacock has a directional bias.
- Deep zooms (corpus 20, 27, 44, Getting Loopy: shift sensitivity 0.27-0.85
  against GPU differences of 15-27) are plausibly FP32 position resolution, but
  the argument is incomplete. The extall surface-dust group (Msltoe_Sym,
  Ruckerbulb, Exp4D, MsltoeFoldQuat, Beth651) and ABoxSOff4d (59.7 % of pixels)
  are unexplained.

## Measuring and diagnosing

| Task | Tool |
|---|---|
| Corpus coverage | `python3 -B comparison/tools/native_gpu_coverage.py --runtime <root> --output <new-dir> [--random] [--compare-cpu] [--baseline <coverage.json>] [--queue Q] [--binary B --library L]`; exit 1 on a baseline regression |
| FP32 sensitivity | `comparison/tools/native_fp32_sensitivity.py --scene S --reference CPU.png --runtime R --output DIR [--shift 24]` |
| GPU vs CPU buffers | `--silight5-dump` on both backends, then `compare_silight5.py` |
| Primary ownership old/new | `native_primary_admission.py --baseline B --candidate C --library L --runtime R --output DIR [--control-library L]` |
| Generator, masks, poison | `scripts/gen_native_metal_formulas.py --check`; `native_formula_poison.py` |
| Benchmarks | `mb3d_backend_render_benchmark ... --rng-mode scene\|parity --backend cpu\|metal` (full benchmarks use scene RNG) |

A render that hangs at start is a compile problem until shown otherwise: `sample`
the process, read the `MTLCompilerService` log, retry a 64x48 frame with
`MB3D_NATIVE_OUTLINE_DE=all` in a separate cache. Tests: [BUILD.md](BUILD.md).
