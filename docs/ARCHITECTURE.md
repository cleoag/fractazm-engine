# CPU architecture

How the C++ CPU renderer is built and how external formulas are compiled into
it, checked against the code. `mb3d/` is the immutable Pascal reference. Related
documents: [GPU.md](GPU.md) (Metal), [LIBRARY.md](LIBRARY.md) (application API),
[BUILD.md](BUILD.md) (build and tests), [PARITY_TRACE.md](PARITY_TRACE.md)
(parity) and [REMAINING.md](REMAINING.md) (open work).

## Product boundary

The maintained product is the C++ CPU renderer on x86-64 and ARM64 and the
Native Metal FP32 renderer on Apple Silicon. The CPU renderer is a headless
engine: a CLI (`src/cli/main.cpp`), a C API (`include/mb3d/mb3d.h`) and the
application scene API (`include/mb3d/mb3d_scene.h`). GUI editors, animation,
the navigator, voxel export and the formula JIT are not ported. The Monte Carlo
code (`calc_monte_carlo.*`, `monte_carlo_renderer.cpp`, `--mc-samples`) exists
but is outside parity qualification and active verification work. A passing
render is not a numerical result.

## Source map

| Path | Role | Kind |
|---|---|---|
| `src/core/` | Renderer, formulas, math, arithmetic kernels, context and control, trace | live code |
| `src/core/generated/` | Registration wrappers for external formulas | generated code, do not edit |
| `src/io/` | `.m3p`/`.m3i` parser, image and buffer writers | live code |
| `src/render/` | Backend selection (`backend.*`), scene API (`mb3d_scene_api.cpp`), Metal (`native_metal/`, `metal_renderer.mm`) | live code |
| `src/cli/` | `mb3d_render` and two probe tools | live code |
| `include/mb3d/` | Public headers `mb3d.h` (render, control, precision) and `mb3d_scene.h` | live code |
| `external_formulas/Formulas/` | Translated formula bodies, `.inc`, by category `3D 3Da 4D 4Da Ads dIFS dIFSa` | live code (compiled in) |
| `external_formulas/asm/` | Capstone listings of the original `.m3f` machine code | input, reading aid only |
| `external_formulas/vendored/` | Berkeley SoftFloat 3e subset and `portable_math` | vendored code |
| `tests/` | Unit tests; `tests/fixtures/*.h` hold independent original-code expected rows | live tests |
| `comparison/tools/` | Python and C++ tools for parity runs, audits and Metal evidence | tooling |
| `comparison/scenes*`, `comparison/fixtures/` | Scene corpora and fixtures | inputs |
| `comparison/results/` | Compact manifests, qualification records and reports | evidence |
| `mb3d/` | Pascal reference tree | immutable |

`comparison/results/` records are evidence bound to the build that produced
them, not statements about current code. Pascal changes are made only in an
external working tree (see
[PARITY_TRACE.md](PARITY_TRACE.md#reference-and-runtime-identity)).

Pascal unit to C++ module:

| Responsibility | Pascal | C++ |
|---|---|---|
| CLI | `HeadlessRender.pas`, `Mand.pas` | `src/cli/main.cpp` |
| Records and loading | `TypeDefinitions.pas`, `FileHandling.pas` | `src/core/types.h`, `src/io/m3p_parser.cpp` |
| Runtime setup | `HeaderTrafos.pas` | `src/core/header_trafos.cpp` |
| Math and callers | `Math3D.pas`, `formulas.pas` | `src/core/math3d.*`, `src/core/formulas.*` |
| External formulas | `CustomFormulas.pas`, `M3Formulas/` | `src/core/external_formulas.*`, `src/core/generated/`, `external_formulas/Formulas/` |
| Rays | `Calc.pas`, `CalcThread.pas`, `CalcPart.pas`, `CalcSR.pas` | `ray_marcher.cpp`, `renderer.cpp`, `reflection.cpp` |
| Shadows and AO | `CalcHardShadow.pas`, `CalcAmbShadowDE.pas`, `AmbHiQ.pas` | `shadow.cpp`, `ambient_occlusion.cpp` |
| Shading and effects | `PaintThread.pas`, `ImageProcess.pas` | `colorizer.cpp`, `dof.cpp`, `image_process.cpp` |
| Maps | `maps/MB3DMaps.pas` | `lightmap.cpp`, `lightmap_resources.cpp` |

CMake targets: `mb3d_runtime` (context, control, thread pool, trace, precision),
`mb3d_core` (`src/core` plus `generated/`), `mb3d_io`, `mb3d_render_backend` and
`mb3d_render`. CPU-relevant options: `MB3D_DEFAULT_PRECISION=compatible|native`,
`MB3D_ENABLE_PARITY_TRACE` (default ON), `MB3D_ENABLE_METAL_RENDERER`,
`MB3D_BUILD_DIAGNOSTICS`, `MB3D_NATIVE_TESTS_ONLY`. `CalcMissed` has a Pascal
declaration but no audited call site; its scope is undecided.

## Scene model and pipeline

Scenes are two packed Pascal disk structs in `src/core/types.h`, with
`static_assert` size checks: `TMandHeader10` (840 bytes: camera, lights,
effects, hybrid header) and `THeaderCustomAddon` (1,136 bytes: six `THAformula`
slots of 188 bytes with their option values). `m3p_parser.cpp` reads `.m3p` of
every MB3D version and `.m3i` into these records and records legacy migrations.
Runtime records keep the packed layout; code that calls ordinary C++ functions
copies values into aligned locals and writes them back at the original callback
boundaries, and legacy file stores use `memcpy`.

File loading reproduces the GUI read-back; direct-header setup keeps its own
Pascal semantics (for example hybrid mode 3). They are not normalized together.

`Render(header, addon, RenderOptions)` and `PaintFromSiLight` in `renderer.h`
run the stages in this order, each consuming completed earlier state:

1. Preparation (`HeaderTrafos`, formula constants, maps, camera), then volume
   light map preparation (also sampled inside ray stages).
2. Primary rays: formula orbit, analytic or numerical DE, inline normals,
   dynamic fog. Then optional Z-buffer normals.
3. Hard shadows, then ambient occlusion, then color.
4. Secondary rays (reflections, transparency), then ordered depth of field.

`PaintFromSiLight` repaints from a supplied siLight5 buffer with its own shorter
plan. `RenderResult` holds RGB, packed depth, siLight buffers, effective
precision, work units and profile; the CLI crops, anti-aliases and encodes after
the render returns.

Logical workers own their RNG and scratch state. The RNG is `PasLcg`
(`src/core/pas_lcg.h`), a deterministic replacement for Pascal's `Random`
shared with the patched Pascal build. Physical scheduling must not change a
lane's identity: equal inputs and worker count give identical RGB, and parity
runs fix the worker count.

## Render-state ownership and threading

Each `Render` and `PaintFromSiLight` call creates a fresh noncopyable
`RenderContext` (`render_context.*`). It owns the six formula constant buffers,
the three-slot formula map cache with missing-map state, map frame and sequence
settings, lighting, background and diffuse maps, the volume light map, the
resolved precision, the optional `RenderControl` and the diagnostic
configuration. Packed runtime records borrow pointers from it; layouts do not
change. `RenderResult` owns the image and surface buffers.

Workers capture a shared owning handle, and a scoped thread binding routes the
legacy map and formula callbacks to that job, so callbacks need no process-wide
state. Scopes restore the caller on success and on exception; the last owner
frees maps after workers have joined. Low-level probes bind an explicit
`ScopedRenderContext`.

`RenderOptions::resource_root` is the base directory for relative maps and
backgrounds (empty: the current directory at entry). Numbered-map search, the
three-map limit, lazy loading and missing-map fallback follow Pascal; background
basenames try the resource root, then the map directories. Metal shares this
preparation. Cache pixels, misses and volume data are never inherited across jobs.

Thread safety rules:

- Concurrent calls need independent mutable headers and addons, stable
  resources and distinct output files. Do not change the process directory or
  environment, map sequences or frame settings while a render runs.
- `MB3D_VLM_DUMP` selects one process-wide path; use it in isolated runs only.
- Stage barriers propagate worker failures to the CLI and C API; the external
  registry initializes under `call_once`.
- Isolation tests compare concurrent and serial jobs at 1, 2, 4, 16 workers;
  Stage B is open (see [PARITY_TRACE.md](PARITY_TRACE.md)).

## Cancellation and work budget

`RenderOptions::control` is an optional shared, single-job `RenderControl`
(`render_control.*`); null keeps the legacy uncounted path. `RenderControl(0)`
has no limit but counts work and supports asynchronous cancel; a positive limit
caps one counter shared by all workers and stages. Create a new control per
call.

The work unit is an admitted DE invocation, formula callback or instrumented
image element, sample or tap; nested events count separately. It is not a
Pascal iteration, an instruction, a finished pixel or elapsed time. Admission
never exceeds the limit, and a batch larger than the remaining allowance is
rejected, so the counter can end below it (`RenderResult::work_units`).

`RenderStatus` goes `Prepared`, `Running`, then `Completed`, `Cancelled`,
`WorkExhausted`, `Failed` or `Unsupported`. The first terminal transition wins,
and a started control cannot be reused. Workers join before the call returns;
cancellation and exhaustion discard partial buffers and return
`success == false` with a distinct status.

The C API has `MB3DRenderControl` (`mb3d_render_control_create`, `_cancel`,
`_work_used`, `_status`, `_destroy`) and `mb3d_render_file_with_control`. The CLI
has `--work-budget` and exits 5 on cancel and 6 on exhaustion. Control is
cooperative, not a wall-time bound: map decoding, I/O, allocation and bulk
copies delay polling. Loading runs before the lifecycle; C API output copies and
CLI cropping and saving run after it. With `--backend metal` or `auto`, a
control forces the Native CPU fallback because Metal has no accounting.

## Arithmetic profiles and platforms

`RenderOptions::precision` is `PrecisionMode::Compatible` or `Native`. The
result and CLI profile record the effective choice. CMake
`MB3D_DEFAULT_PRECISION` sets only the default value; both modes are always
compiled. `RenderContext` holds the choice and `ScopedNumericPrecision` follows
the worker binding, so concurrent renders never share a process-wide setting.

- `compatible`: software-extended x87 80-bit arithmetic through the vendored
  SoftFloat (`extended80.h`), the original exponent range, transcendental
  sequences and Single/Double stores. This is the only mode that can claim
  parity with Pascal.
- `native`: hardware binary64 and platform `std` math in the same
  software-extended paths, including external formulas, camera and numerical DE.
  RGB may differ; it inherits no Pascal or compatible claim. It does not enable
  fast-math or reorder formulas.

C API: `mb3d_render_file_with_precision` (`MB3D_PRECISION_DEFAULT`,
`_COMPATIBLE`, `_NATIVE`); older entry points use the configured default. CLI:
`--precision compatible|native`; `--backend metal` or `auto` requires `native`.

Backend selection (`RenderWithBackend`, `src/render/backend.*`): CPU is the
default and keeps the requested precision. Metal and Auto need native precision
and an ordinary full render; they try the restricted adapter, then the general
Native engine, else run the whole job on Native CPU with a recorded reason.
`Render`, `PaintFromSiLight` and the C API are CPU entry points. Fallback codes
are in [GPU.md](GPU.md).

Shared kernels:

- `numerical_de.cpp`: ordered binary64 `sqrt((dz^2 + dy^2) + dx^2) + offset`,
  then extended `ln2 * log2(radius)`, products and division, then the Double
  store; one software implementation on Apple ARM64 and MSVC x64.
- `numerical_camera.cpp`: extended sine and cosine, normalization in
  `(x*x + z*z) + y*y` order, Double vector stores, extended matrix products.
- `extended_*.h`: selected extended atan, log, power, sine and cosine with
  Intel-style two-limb Pi reduction. Translated contracts, not x87 execution.
  `ExtendedSeriesContext` must not outlive its scope or cross a thread.
- `sse_estimates.h`: native SSE on Intel; on ARM an integer lookup that
  reproduces the legacy Intel reciprocal and rsqrt estimate (signed zero,
  denormals, NaN payloads, reciprocal underflow).

Platforms:

- Windows x64 (MSVC): the original reference platform. x87 formula bodies use
  MASM (`win64_de_x87_helpers.asm`); `/fp:strict` applies only to
  `portable_math.c` and `numerical_de.cpp`. Native mode leaves the MASM bodies
  unchanged and bypasses only the shared helpers.
- Apple ARM64: `long double` is `double`, so x87 intermediates use SoftFloat,
  compiled to plain ARM64 integer code (no x86 instructions, no Rosetta). The
  build uses `-fno-fast-math -ffp-contract=off` and assumes nearest-even
  rounding with gradual underflow. SoftFloat rounding and exception state is
  thread-local and restored by compatible scopes.
- Metal has no `double`; GPU work is FP32 and a separate profile. Keep CPU
  `compatible`, CPU `native` and GPU numerics apart in every record.

CMake test labels: `native_cpu` (portable runtime suites, also built by
`MB3D_NATIVE_TESTS_ONLY`), `ordinary`, `arm64_qualified` (selected formula and
kernel subset) and `release`. Commands are in [BUILD.md](BUILD.md).

## Stage profiling

`RenderOptions::profile_timing` (CLI `--profile-json`) records job-owned wall
times in `RenderResult::profile`: Preparation, Volume, Primary, Normals,
Shadows, AO, Color, Reflections and DOF, each `completed`, `interrupted` or
`not_run`. The coordinating thread records after workers join; arithmetic, RNG
use and work accounting do not change. `benchmark_gallery_arm64.py` adds process
CPU time and peak RSS.

## External formulas

### How they are compiled in

Pascal loads `.m3f` files (metadata plus x86 CODE) from `M3Formulas` at run
time. C++ does not execute `.m3f` bytecode and has no JIT. Every supported
formula is a static C++ translation of the original CODE:

- `external_formulas/Formulas/<category>/<name>.inc` holds the body as an
  `execute_formula<ID>` specialization plus a metadata block.
- `src/core/external_formula_table.cpp` holds the handwritten registrations;
  `src/core/generated/external_formula_table_<category>.cpp` holds one
  generated wrapper per remaining `.inc`. `external_formulas.h` carries the
  shared types, `.inc` helpers and wrapper macros.
- `scripts/gen_bulk_formula_table.py` writes the generated files from the `.inc`
  sources and the `.m3f` option sections:

```text
python3 scripts/gen_bulk_formula_table.py --formulas-dir external_formulas/Formulas \
  --m3f-dir mb3d/M3Formulas --existing-table src/core/external_formula_table.cpp \
  --output-dir src/core/generated
```

- `ExternalFormulaInfo` records id, name, DE option and scales, `SIpow`,
  `RStop`, wrapper, parameter map, `.m3f` option types, map-value mask and
  constant slots; `ExternalFormulaAt` and `GetExternalFormula` look it up.
- Registration is published once under `std::call_once`; failure clears the
  partial table before a retry, recursive or late registration is rejected and
  published metadata is immutable. This is not a plugin API.
- `.m3f` text is read by the generator and, in the application API, for
  parameter names and defaults ([LIBRARY.md](LIBRARY.md)).

Scope: about 460 registered names in seven categories plus ten internal
formulas. The registry and `comparison/tools/formula_coverage.py` own the list;
per-formula CPU status is in
[formula-status.json](../comparison/results/compatibility-final-20261018/formula-status.json).

- All registered formulas run on the CPU in both precision modes. x87-sensitive
  bodies use the software-extended type on ARM64 and MASM or x87 code on MSVC
  x64. The CMake `arm64_qualified` subset lists those checked on Apple ARM64.
- Some originals are not numerical parity because they read caller registers or
  never terminate: odd `Flip_XZ` in `gnarlyIFS`, `gnarly2IFS`, `gnarly3IFS`,
  `ducksIFS`, zero-power `TrifoxComplexAngles`, two invalid `_GenCWarp`
  selectors and some conditional formulas. Strict C++ rejects them; see
  [PARITY_TRACE.md](PARITY_TRACE.md#remaining-reference-limitations).
- The Metal lowering is generated by `scripts/gen_native_metal_formulas.py`
  ([GPU.md](GPU.md)).

### Parameter and constant ABI

A saved option declaration, a saved scalar slot and a prepared runtime offset
are three different indices, and compound options expand into several values.
Stale payload beyond the saved option count takes metadata defaults and never
becomes extra angles or selectors. Tests check the values actually loaded, not
the bytes a generator wrote.

| Contract | Rule |
|---|---|
| Storage | Keep negative variable offsets from `PVar`, positive constants and alignment. `FormulaParamMap` holds offset and type per entry; do not infer layout from declaration count. |
| Defaults and boundary | `PVar-8` defaults to 0.5 where required (including MengerKochV2). The wrapper parameter count fixes where the constant (C) region starts; `_CelticMode` uses its four-option boundary. |
| Compound angles | Keep the original sine and cosine pairs, matrix orientation and stored values. Scalar-angle and matrix expansion are not interchangeable rotations. |
| INTEGER conversion | Pascal `Round` is nearest-even; keep int32 overflow, NaN and Inf results and the signed, unsigned, byte or low-word readings. Do not use `std::round`. |
| Constant parser | Invalid Single or Double text, including trailing semicolons, reserves a zero slot; unsupported types may be skipped (`_IFS_tess`'s `Integer = 50` is not stored). Do not share one skip policy. |
| Legacy migration | `_FlipXY`, `_FlipXZ`, `_FlipYZ` migrate to the `*c` names with option 0 set to zero; `FlipXYc` has DE option -1; ABoxMod1 uses ADE scale 1 and persistent first-iteration state. Record migrations instead of equating original and effective identity. |

### Caller state and arithmetic contracts

A formula is not a pure `XYZ -> XYZ` function: its contract can include XYZW,
Julia and C, derivatives, stored radius, orbit trap, varying scale, first and
counting flags, extra SSE lanes, map callbacks and hybrid counters. The
`FormulaSse01` lanes of `TIteration3Dext` sit outside the Pascal-compatible
prefix; `ExternalFormulaUsesSse01` selects the Manowar caller behavior
(interpolation drivers copy the original `CopyVec4SSE2` output before each
callback; `_IntPowRotateC` publishes its outgoing XMM0 and XMM1). This is a
finite chain contract, not an implicit-register ABI for every formula.

| Area | Rule to keep |
|---|---|
| Driver | Standard and dIFS calls differ. A standard transform such as `badRotate` inside a dIFS chain can crash the original; use `transformIFS` for positive controls (`comparison/fixtures/invalid_difs_standard_transform.m3p` is the invalid case). |
| Counters and state | `ItResultI` and `bFirstIt` are independent; `_RotatePlaneOnIts`, `_RotateXYonIts`, `_RotateXYonItsP` wrap the first counter at 32 bits. Aexion1 initializes at `bFirstIt == 0`; `Rout`, `Rold`, varying scale, derivatives and IFS `Dfree2` survive calls. |
| Integer loops | Byte counters can run 256 bodies for zero (Pine variants); signed-byte compares and bottom-tested loops are intentional. Metadata minima are not clamps. |
| Julia and C | Many bodies add C before their final Double stores (MsltoeSym4, TalisTpx, ABoxPlatinum, ...) and the wrapper must not add it again; the set is `NO_WRAPPER_C_FORMULAS` in `gen_bulk_formula_table.py`. `_lorenz`, `_rossler`, `_Rotate4d`, `_vanderpol*` keep W without a wrapper add; `_PartlyJulia*` separates the saved point from C. |
| Local coordinates | With Apply=0 a formula can keep orbit XYZ yet compute distance in transformed local coordinates (CommaIFS, cubetubeIFS, gear2IFS, KnotsIFS, hextgrid family). Keep VaryScale, color and derivative writes separate. |
| Order and ties | Menger4ex uses a six-comparison network, not a sort; keep signed `Deriv1`, unordered comparisons and NaN behavior in folds and minima. |
| Widths and stores | Extended intermediates, Single products, Double spills, operand order, FIST and FRNDINT conversion and the timing of the Julia add are observable; `long double` is not a substitute. Signed zero is a floating literal, never integer `-0`; Amazing Surf 2 uses IEEE64 fold masks. |
| Random state | `_RotateRnd` and `_TranslateRnd` keep signed 32-bit counters and low-word sequences; keep skip boundaries and consumption, not just the seed. |

### Maps and resources

Map callbacks are part of the formula ABI: argument order, map ID, channel,
aliases and output writes. `_MapTranslate`, ABoxMapSFold, SphereHeightMap,
HeightMapIFS and related formulas need per-call resource context; HeightMapIFS
keeps matrix axis order, early clipping and masked channel-3 sampling. Cube-face
signs, nearest-even texel conversion, projection and interpolation need their
own resource checks; a callback mock cannot qualify them.

`ExternalFormulaInfo::mapValueMask` is a legacy CPU prefetch hint from option
labels containing MAP (including channel selectors and Tglad Log2MapPower), not
a list of mandatory assets. Missing real callback resources reject preparation.
Real-map comparisons use identical asset bytes, resolved paths and frame
selection; the example is `comparison/scenes_validation/heightmap_real_asset.m3p`
with `comparison/fixtures/M3Maps/9001.bmp`.

### Bounded execution

`_e_3Dtess`, `_IFS_tess` and `Knots2IFS` cap each callback at 1,048,576
actually executed loop bodies, in native and portable paths
(`FormulaWorkPolicy::maxInnerStepsPerCall`, `formula_work_guard.h`). Large
requested depths that exit early stay valid. Exhaustion fails before another
body runs and never returns a truncated orbit as success.

Preserved behavior: `TrifoxComplexAngles` rejects a zero prepared integer power
(the original shift loop never ends) and keeps signed nonzero powers;
`_e_3Dtess` keeps signed depth and its geometric early exit; `_IFS_tess` keeps
unsigned depth and the original constant layout (no invented limit of 50);
`Knots2IFS` keeps its bottom-tested signed loop (counts at most one, including
`INT_MIN`, run one body).

On ARM64 the sector-loop transforms `_BPolygonFromCircle`, `_BPolygonToCircle`,
`_BPolyhedronToSphere` and `_BStellahedron_F_Sphere` use the same limit across
their repeated extended subtractions. Selected dispatchers validate slot walks
and bound calls that make no counted progress. Workers keep the first failure
and stop; the main thread reports it and no image is published. Portable loops
also poll the shared `RenderControl`; native assembly is polled at callback
boundaries. Other callbacks may contain unbounded loops, so admission does not
guarantee a return. A cap is policy, not proof of nontermination, and never
turns an original nonterminating input into a numerical pass. Tests:
`formula_work_guard_tests`, `formula_iteration_guard_tests`.

### Adding or fixing a formula

1. Read the original CODE (`external_formulas/asm/<name>.asm`), the `.m3f`
   `[OPTIONS]` and `[CONSTANTS]` and the Pascal caller (`CustomFormulas.pas`,
   `HeaderTrafos.pas`, `formulas.pas`) to settle layout, DE option, caller state
   and arithmetic widths.
2. Change the `.inc` body in `external_formulas/Formulas/<category>/`, keeping
   operation order, stores and conditional writes.
3. Regenerate wrappers and defaults together; never edit `src/core/generated/`
   by hand. Add the name to `NO_WRAPPER_C_FORMULAS` or an `ExternalFormulaUses*`
   trait only if the original body adds C or keeps state.
4. Add expected rows to `tests/fixtures/` taken from the original code, not the
   translation, plus a focused test; run `external_registry_tests`,
   `formula_validation_tests` and the release check in [BUILD.md](BUILD.md).
5. A parity claim follows [PARITY_TRACE.md](PARITY_TRACE.md); the Metal lowering
   of the changed formula is regenerated and checked per [GPU.md](GPU.md).

Coverage inventories (`formula_coverage.py`, `audit_backend_readiness.py`) map
registrations, sources and scenes; they are not executed-branch coverage or
numerical parity. A non-blank image proves no formula behavior.
