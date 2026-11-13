# Build, test and run

Runbook for building, checking and running the renderer. See also [LIBRARY.md](LIBRARY.md),
[GPU.md](GPU.md), [PARITY_TRACE.md](PARITY_TRACE.md) and
[REMAINING.md](REMAINING.md).

Rules: `mb3d/` is immutable; never build or edit it. Monte Carlo/Halton and
JIT are out of scope, so never run unfiltered CTest. Write each run to a new
output directory (`comparison/runs/` is the git-ignored convention). Keep 12 GiB
free. Do not edit sources or rebuild while a job uses the build directory.

## Default: small release check

For a release candidate: build the library and CLI, run CTest label `release`
and render three tracked 64x48 scenes (built-in formula, external formula,
height map) on each enabled backend.

```sh
# Apple Silicon: CPU + Metal. Add --cpu-only for a CPU-only build.
python3 -B scripts/bootstrap_arm64.py --output comparison/runs/my-release-check
# Windows x64, PowerShell, native environment.
python scripts/bootstrap_windows.py --jobs 4
```

The `release` label is assigned in `cmake/ReleaseChecks.cmake` and built by
the `mb3d_release_checks` target. It covers context and control-handle
lifecycle, the render and scene APIs, `.m3p` parsing, formula registry and
guards, native precision isolation, CLI cancellation, budget and profile checks,
and Metal scheduling. The smoke step fails on a failed render, an invalid PNG,
an exceeded deadline or a CPU fallback in a Metal check. A pass is
`PASS_RELEASE_CHECK` for the host that ran it, not formula, scene or Pascal
qualification.

`scripts/bootstrap_arm64.py` needs native Apple Silicon Python (Rosetta is
rejected). `--output DIR` is required and must not exist; it receives logs,
`bootstrap.json`, `ctest.xml` and the smoke images.

| Option | Meaning |
|---|---|
| `--scope` | `release` (default), `primitives` (CPU runtime tests, label `native_cpu`), `qualified` (`arm64_qualified`), `ordinary` (`ordinary`) |
| `--cpu-only` | Metal off; CPU smoke renders only |
| `--trace ON\|OFF` | `MB3D_ENABLE_PARITY_TRACE`, default ON |
| `--lto` | `CMAKE_INTERPROCEDURAL_OPTIMIZATION=ON` |
| `--sanitizer none\|address\|thread` | Instrumented build; test timeouts scale 2x or 10x |
| `--jobs N`, `--build-dir DIR` | Default 4; default `build-native-arm64-<scope>-<trace>-<sanitizer>` |

`scripts/bootstrap_windows.py` needs native Windows x64. Options:
`--scope release|ordinary`, `--jobs N`, `--build-dir` (`build-vs-bootstrap`),
`--output-dir` (new; default `comparison/runs/bootstrap-<UTC>`), `--venv-dir`
and `--skip-install` (`ordinary` only), `--zlib-source DIR`.

For a configured build directory:

```sh
cmake --build <build> --target mb3d_release_checks --parallel 4
ctest --test-dir <build> -L '^release$' --output-on-failure --timeout 180
python3 -B scripts/release_smoke.py --build <build> --output <new-dir> --backend all
```

On Windows add `-C Release` to `ctest` and use `--backend cpu`.

## Build matrix and CMake options

| Profile | Toolchain | Notes |
|---|---|---|
| Apple ARM64 CPU + Metal | Apple Clang, Xcode Metal toolchain | Default on native arm64 macOS; `-fno-fast-math -ffp-contract=off` is applied |
| Apple ARM64 CPU only | same, `-DMB3D_ENABLE_METAL_RENDERER=OFF` | Bootstrap `--cpu-only` |
| Windows x64 CPU | VS 2022 (MSVC v143), MASM | `Visual Studio 17 2022` generator, `-A x64` |
| iOS, visionOS, simulators | `scripts/build_xcframework.sh` | Static and Metal library per platform |
| Pascal i386 reference | External Lazarus/FPC copy | Oracle only, see below |

CMake 3.28 or newer, C++20. zlib is found or fetched (1.3.1, SHA-256 pinned).
Metal builds need the Metal compiler: check `xcrun metal --version`; install
with `xcodebuild -downloadComponent MetalToolchain` if missing.

| Option | Default | Effect |
|---|---|---|
| `MB3D_ENABLE_METAL_RENDERER` | ON on native Apple arm64, else OFF | Native Metal in the main renderer; CPU stays the default backend. Fatal on any other platform or with `MB3D_NATIVE_TESTS_ONLY` |
| `MB3D_DEFAULT_PRECISION` | `compatible` | `compatible` (software x87) or `native`; only changes the CLI/API default |
| `MB3D_ENABLE_PARITY_TRACE` | ON | Parity-trace instrumentation, see [PARITY_TRACE.md](PARITY_TRACE.md) |
| `MB3D_BUILD_DIAGNOSTICS` | OFF | Test and diagnostic executables in the default build; explicit targets and `mb3d_release_checks` work either way |
| `MB3D_NATIVE_TESTS_ONLY` | OFF | Portable CPU runtime tests only, without the renderer, zlib or Metal (`mb3d_native_tests`, label `native_cpu`) |
| `MB3D_METAL_SDK` | `macosx`; `iphoneos`, `xros` or their simulators for those targets | `xcrun` SDK used to compile shaders |
| `MB3D_TEST_TIMEOUT_SCALE` | 1 | Integer multiplier for CTest deadlines in slow instrumented builds |
| `MB3D_BUILD_GENERAL_NAVIGATION_EXPERIMENT` | OFF | Experimental unqualified sparse navigator; needs Metal. Used only by `apps/macos_viewer` |
| `MB3D_BUILD_METAL_PROBE`, `MB3D_BUILD_METAL_FORMULA_PROBE` | OFF | Isolated Metal postprocess probe (native Apple ARM64 only); experimental FP32 Metal formula renderer |
| `MB3D_BUILD_METAL_EXACT_SCENE` | OFF | Exact-arithmetic Metal stage and restricted M3P primary stage; not with `MB3D_NATIVE_TESTS_ONLY` |

Aggregate targets: `mb3d_release_checks`, `mb3d_native_tests`, `mb3d_ordinary`,
`mb3d_arm64_qualified` (Apple ARM64), `mb3d_render` (CLI).

## Native Apple Silicon runtime checks

By hand, without the bootstrap:

```sh
cmake -S . -B build-arm64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build-arm64 --target mb3d_release_checks --parallel 4
ctest --test-dir build-arm64 -L '^release$' --output-on-failure --timeout 180
```

Other labels, always selected explicitly: `native_cpu`, `ordinary` (target
`mb3d_ordinary`), `arm64_qualified`, `metal_native` and `render_backend` (GPU
fixtures; run serially). `mb3d_render` and `metal_native_scene.metallib` land
in the build directory root.

## Application framework and Swift package

`scripts/build_xcframework.sh [macos] [ios] [ios-simulator] [visionos]
[visionos-simulator]` (all by default) builds `xcframework/MB3D.xcframework`
and the shader libraries in `xcframework/metallib/<platform>/`, both git-ignored.
The script sets the minimum OS of each slice and of its shader library: macOS
14.0, iOS 17.0, visionOS 1.0, the versions `swift/Package.swift` declares. The
shader flag is the CMake variable `MB3D_METAL_MIN_OS_FLAG`; it is empty in
ordinary builds, which keep the SDK default. The framework holds only the
platforms named on the command line, so a subset build drops the other slices.
`swift/` is the `MB3DKit` package over it. API, checks and limits:
[LIBRARY.md](LIBRARY.md).

## Fresh Windows machine

Supported: native Windows x64, MSVC 2022, CPU only. No Lazarus or Pascal.

Install, then reopen PowerShell (no Developer Command Prompt needed):

- Visual Studio 2022 or Build Tools 2022: Desktop development with C++, MSVC
  v143 x86/x64 tools and a Windows 10/11 SDK. Newer Visual Studio does not
  satisfy the `Visual Studio 17 2022` generator.
- CMake 3.28 or newer, Git for Windows, 64-bit CPython 3.12 to 3.14, all on
  `PATH`. The release scope uses only the Python standard library.

```powershell
git clone --config core.autocrlf=false https://github.com/cleoag/mb3d_cpp_port.git
cd mb3d_cpp_port
python scripts/bootstrap_windows.py --jobs 4
```

Keep `core.autocrlf=false`; `.gitattributes` holds `mb3d/M3Formulas/*.m3f` in
CRLF (audited hashes include it). Do not normalize them.

The script fetches zlib, builds `mb3d_release_checks`, runs the `release`
label and renders three smoke scenes. Results: `build-vs-bootstrap/Release/
mb3d_render.exe` and `comparison/runs/bootstrap-<UTC>/` (`bootstrap.json`, logs,
`ctest.xml`, `smoke/`). Lower `--jobs` if compilation runs out of memory.

Offline: unpack zlib 1.3.1 (archive SHA-256 `50b24b47bf19e1f35d2a21ff36d2a366
638cdf958219a66f30ce0861201760e6`, checked by hand) and pass `--zlib-source`.

CLI only: `cmake -S . -B build-vs-local -G "Visual Studio 17 2022" -A x64`,
then `cmake --build build-vs-local --config Release --target mb3d_render`.

Windows on ARM: an x64 build there only checks compilation. The x64 translator
rounds x87 intermediates to binary64, so `compatible` results from a translated
run are not x64 evidence.

## Running the CLI

`mb3d_render <scene.m3p> [options]`. Run from a directory holding the scene's
`M3Maps` and background assets. Formulas are compiled in. `--help` lists all.

| Option | Meaning |
|---|---|
| `--backend cpu\|metal\|auto` | Default `cpu`. `metal` and `auto` use Native Metal when the scene is supported, else Native CPU with the reason in the profile |
| `--metal-library <path>` | Default `metal_native_scene.metallib` beside the executable |
| `--precision compatible\|native` | CPU arithmetic: `compatible` is software x87, `native` is hardware FP64. Metal always uses hardware FP32. Default from `MB3D_DEFAULT_PRECISION` |
| `-t, --threads N`, `-w`, `-h`, `--max-iter` | Threads (default hardware concurrency; match them across backends); size and iteration overrides |
| `-o, --output <path>` | Default `<input>.png`. `-f png\|bmp`; `--zbuffer`, `--zbuffer-format bmp\|png8\|png16`, `--normalmap` add outputs |
| `--profile-json <path>` | Write stage timings, backend, fallback code and `metal_execution` to a new file; an existing file is an error |
| `--lattice s,x,y` | Metal only: render every `s`-th pixel (2 to 4) of the frame along both axes from offset `x,y`; the output has the lattice's size (see [LIBRARY.md](LIBRARY.md)) |
| `--no-shadows --no-ao --no-dof --no-reflections --no-volume --no-postprocess` | Disable effects |
| `--work-budget N`, `--info` | Shared work limit (0 unlimited); print scene parameters without rendering |
| `--deterministic` | Parity-work mode; see [PARITY_TRACE.md](PARITY_TRACE.md) |

Exit codes: 0 success, 1 argument error, 2 `.m3p` read error, 3 render error,
4 file write error, 5 cancelled, 6 work limit exhausted.

After dispatch Metal falls back to Native CPU only when more than 1 % of
geometry pixels leave the binary32 range; any other GPU fault ends without an
image ([GPU.md](GPU.md)). Native results are never x87/Pascal parity.

A measurement states backend, precision, build identity, scene, size, threads
and whether the Metal pipelines were cold or warm.

## Metal pipeline archive: cold and warm renders

Specialized pipelines are archived on disk, keyed by shader library bytes,
GPU, macOS version and scene specialization; a new library, GPU or OS update
starts cold. `MB3D_METAL_PIPELINE_CACHE` selects the directory (default
`~/Library/Caches/mb3d/metal-pipelines`, pruned to 2 GiB); `off` disables it.

Warm the archive after such a change, on the machine that will render:

```sh
python3 scripts/warm_metal_pipelines.py \
  --binary <build>/mb3d_render --library <build>/metal_native_scene.metallib \
  --runtime <resource-root> [scene.m3p|dir ...]
```

Always pass `--binary` and `--library`; their defaults name a missing
directory. Without scene arguments it uses `mb3d/M3Parameter`. Other options:
`--jobs` (4), `--width`/`--height` (64x48), `--threads` (6), `--timeout`
seconds per scene (300). Each scene renders once, small, with `--backend metal
--precision native`; the pipeline set does not depend on size or thread count,
so one warm-up serves every size. It prints hits and compiles per scene and
exits 1 if a render failed. It is not a benchmark.

Telling compile cost from slow rendering:

- A profile from `--profile-json` reports `metal_execution.pipeline_archive`
  as `[hits, misses]`. Any miss means backend compilation is inside that frame's
  time. Never compare a cold wall time on one side with a warm one on the other.
- A render that sits at start for minutes, or ends with a pipeline error and no
  image, is a compile problem until shown otherwise. Read the profile's `status`
  and `error` and sample the process; the compiler runs as `MTLCompilerService`,
  whose unified-log entries show what it is doing.
- To test inlining as the cause, render 64x48 with `MB3D_NATIVE_OUTLINE_DE=all`
  and a separate empty `MB3D_METAL_PIPELINE_CACHE`. Outlining can change FP32
  contraction, so a code change still needs an image and work comparison.
- Report compile time separately from frame time; frame limits are in
  [AGENTS.md](../AGENTS.md).

GPU slot: the bootstrap, `release_smoke.py` and `run_m3parameter_gpu.py` lock
`comparison/runs/.gpu-execution.lock` for Metal work; run other GPU jobs one
at a time.

## Optional deep qualification

Not part of the release path. Run these only on an explicit request or to
diagnose a concrete failure.

- `bootstrap_arm64.py --scope ordinary` or `qualified`, and
  `bootstrap_windows.py --scope ordinary` (adds the Python suites).
- GPU scene acceptance follows
  `comparison/contracts/native-gpu-acceptance-v4/acceptance-contract.json`.
  `python3 -B comparison/tools/qualify_gpu_parallel_pipeline.py --contract
  <contract> --evidence <evidence-dir> --output <new-dir>` validates evidence
  without rendering and exits nonzero (`OPEN`) while obligations remain. No
  evidence directory is kept in the repository.
- [NATIVE_ACCEPTANCE_PLAN.md](NATIVE_ACCEPTANCE_PLAN.md) owns the plan;
  [GPU.md](GPU.md) the GPU diagnostics and benchmark commands.

## Pascal reference

`mb3d/` holds the original sources, scenes and formulas and is immutable.
Pascal is built only from an editable copy outside this repository, on Windows
with Lazarus (LCL) and Free Pascal 3.2.2 i386/Win32. A 64-bit FPC cannot
assemble the original i386/x87 formula code. Pascal results are the oracle for
[PARITY_TRACE.md](PARITY_TRACE.md); a rebuilt executable is a new binary
identity, so record its hashes before comparing.

Restore the source into a new directory outside the repository:

```powershell
python scripts/prepare_pascal_work.py --destination <new-dir-outside-repo>
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build_portable_math_i386.ps1 -OutputDirectory <that-dir>
cd <that-dir>
<lazarus>\lazbuild.exe --build-all --cpu=i386 --os=win32 "--compiler=<fpc-i386-win32>\fpc.exe" Mandelbulb3D.lpi --opt=-O2 --opt=-g- --opt=-gl-
```

`prepare_pascal_work.py` exports `mb3d/` from `git archive HEAD`, checks it
against `patches/pascal_guarded_v3_source_manifest.json`, applies
`patches/pascal_guarded_v3_source.patch` and writes `bootstrap-receipt.json`; it
refuses an existing destination or one inside the repository.
`build_portable_math_i386.ps1` builds `portable_math.dll` into that tree
(finds VS 2022 through `vswhere`, optional `-VisualStudioPath`) and will not
overwrite one. Supply image maps beside the executable; Pascal resolves
`M3Formulas` and `M3Maps` next to it.

The other two files in `patches/` (`pascal_working_copy.patch`,
`pascal_hang_guards_20261018.patch`) are older snapshots; do not apply them to
a restored tree.

Keep the external copy in its own Git repository. Back it up with `git -C
<work> bundle create <file>.bundle <refs>` plus `bundle verify` (committed
state only); restore with `git clone --config core.autocrlf=false <file>.bundle
<new-dir>` and rebuild fully. `comparison/tools/build_pascal_windows.ps1` takes
the work location from `MB3D_PASCAL_WORK_ROOT` or
`comparison/tools/windows_workspace.local.json` (the file wins).

## macOS viewer prototype

`apps/macos_viewer/` is an optional Apple Silicon prototype client. It links an
existing engine build and never rebuilds it. First build the engine with
Metal on (release check or `mb3d_render_backend`) so the build directory holds
`libmb3d_{render_backend,native_formula_host,io,core,runtime,softfloat}.a` and
`metal_native_scene.metallib`. Then:

```sh
python3 apps/macos_viewer/build.py --engine-build <engine-build-dir> \
  --engine-source <engine-commit> [--output <dir>]
open "build/macos-viewer/Mandelbulb Viewer.app"
```

`--engine-source` is the commit the engine build came from; interface headers
are taken from it, overlaid with the current backend and preview headers. The
default output `build/macos-viewer` needs 13 GiB free and must be separate from
the engine build. The script compiles the viewer, adapters and two preview
shaders, ad hoc signs the app and writes `build-record.json`. Do not rebuild a
running bundle.

The app opens a demo scene. Drag rotates, Shift-drag or right-drag
pans, the wheel and W/S/A/D/Q/E move, Escape cancels, **Maps** selects a
resource root containing `M3Maps`. The general sparse preview is on by default;
`MB3D_VIEWER_GENERAL_PREVIEW=0` in the environment of
`.../Contents/MacOS/MandelbulbViewer` turns it off. Previews are internal,
unqualified approximations: no export, no stereo navigation, no CPU fallback
for Metal requests.

Check a copy of the built app with `python3 apps/macos_viewer/smoke.py --app
<app> --output <new-dir>`; the other `test_*.py`/`check_*.py` scripts take `--help`.

## Local aliases

Two git-ignored symlinks in the repository root name frozen builds under
`comparison/runs/`:

- `build-main`: the currently selected Apple ARM64 CPU + Metal build, the
  binary and library for GPU runs and warm-up.
- `build-qualified-reference`: the previous accepted build, kept as reference.

They are artifacts, not build trees: never configure, build or run tests that
write into them. After a source change build a new directory with the
bootstrap; repoint an alias only on the owner's decision. Run long jobs against
a frozen copy of the executable and library.
