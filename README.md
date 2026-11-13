# Mandelbulb3D C++ renderer

A C++ port of the Mandelbulb3D (MB3D) rendering engine: a library with a C API,
a command-line renderer, and a native Metal renderer for Apple Silicon. It loads
`.m3p` scenes and renders them with the built-in and statically translated
external formulas, hybrids, lighting, shadows, ambient occlusion, reflections,
transparency, volumetric light, fog, depth of field, stereo, panorama and
slicing. The Pascal GUI, editors, animation and export are not ported.

| Part | Where | Purpose |
|---|---|---|
| CPU renderer | `src/core`, `src/io` | Full MB3D pipeline on x86-64 and ARM64 |
| Metal renderer | `src/render` | Hardware FP32 renderer for Apple Silicon |
| Formulas | `external_formulas/`, `src/core/generated` | Built-in and external MB3D formulas compiled as C++ and as Metal shaders |
| C API | `include/mb3d/` | `mb3d.h` (render a file), `mb3d_scene.h` (editable scenes for applications) |
| CLI | `src/cli` | `mb3d_render` |
| Apple packaging | `scripts/build_xcframework.sh`, `swift/` | `MB3D.xcframework` and the `MB3DKit` Swift package for macOS, iOS and visionOS |
| macOS viewer | `apps/macos_viewer` | Experimental AppKit client with mouse navigation |
| Pascal reference | `mb3d/` | Immutable original sources, scenes, maps and formulas |
| Checks and evidence | `tests/`, `comparison/` | Tests, scenes, verification tools and recorded results |

## Quick start

Apple Silicon, CPU and Metal:

```bash
python3 scripts/bootstrap_arm64.py --output comparison/runs/my-check
```

Windows x64, from a Visual Studio developer environment:

```powershell
git clone --config core.autocrlf=false --branch main https://github.com/cleoag/mb3d_cpp_port.git
cd mb3d_cpp_port
python scripts/bootstrap_windows.py --jobs 4
```

Both build the `mb3d_release_checks` target, run the CTest label `release` and
render three small scenes per enabled backend. Render a scene:

```bash
<build>/mb3d_render comparison/scenes/test_01_intpow2.m3p --output out.png \
    --width 320 --height 240 --threads 4
```

Scenes that use image maps need an `M3Maps` directory in the working directory.
The maps approved for the 77 reference scenes are tracked in
[comparison/resources/phase1-selected](comparison/resources/phase1-selected/README.md).
[docs/BUILD.md](docs/BUILD.md) has every build, test and run command, and
[docs/LIBRARY.md](docs/LIBRARY.md) describes the application API.

## Arithmetic

| Mode | Arithmetic | Use |
|---|---|---|
| CPU `compatible` (default) | Software x87 80-bit emulation | Reference mode for comparison with Pascal MB3D |
| CPU `native` | Hardware double | Fast CPU renders and previews |
| Metal | Hardware FP32 | Fastest; a frame that leaves the FP32 range on more than 1 % of its pixels is rendered on native CPU |

`mb3d_render --precision compatible|native` and `--backend cpu|metal|auto`
select the mode for one render; `--backend metal|auto` implies native precision.
Native and Metal images may differ from the compatible mode and never count as
Pascal parity.

## Status

| Area | State |
|---|---|
| Release checks | `ctest -L release` passes 13/13 on Apple ARM64. The same scope is the Windows default; it has not been recorded on native x64 hardware. |
| Formulas | 467/467 formulas (457 external, 10 internal) are checked for release: 222 external ones against numerical references, the rest functionally. [Records](comparison/results/formula-campaign-004/RELEASE.md). |
| Application library v1 | Scene editing, formula slots, camera gestures, progressive preview, final render (whole or in parts that fill in on screen), per-eye stereo and clip volume through `mb3d_scene.h`; xcframework slices for macOS, iOS and visionOS build. Runs on macOS, in the iOS simulator, on iPhone 17 Pro and iPad Pro M1, with open device faults listed in docs/REMAINING.md; nothing has been run on visionOS. |
| CPU, compatible | The last full-corpus Windows checkpoint passes 343/343 ordinary scenes and 5,478/5,478 scheduled formula images. Strict Pascal parity (Stage A) is open; see [docs/PARITY_TRACE.md](docs/PARITY_TRACE.md). |
| Metal | Every render stage and all 457 external formula bodies are implemented. Optional full qualification on the frozen `build-main` build stands at 20/77 scenes (19 GPU + 1 proven FP32 CPU) and 29/457 formula proofs; an earlier frozen build holds 60/77. See [docs/NATIVE_ACCEPTANCE_PLAN.md](docs/NATIVE_ACCEPTANCE_PLAN.md). |
| Windows under ARM/Prism | Compiles; the ordinary gate is 39/42 with two x87-sensitive failures and one stopped suite. |
| Monte Carlo and JIT | Deferred: excluded from work, tests and percentages. |

Release checks are scoped: they do not claim full formula, scene or Pascal
qualification. Qualification counts belong to the frozen builds that earned
them and do not transfer to a new build. Open work is in
[docs/REMAINING.md](docs/REMAINING.md).

## Documentation

| Document | Owns |
|---|---|
| [AGENTS.md](AGENTS.md) | Working rules for people and coding agents |
| [docs/LIBRARY.md](docs/LIBRARY.md) | Code inventory, application API, Swift package, viewer prototype, v1 limits |
| [docs/BUILD.md](docs/BUILD.md) | Build, test, run and Pascal reference runbooks |
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | Source map, render pipeline, CPU arithmetic profiles, external formulas |
| [docs/GPU.md](docs/GPU.md) | Metal renderer: running, architecture, change rules, FP32 limits |
| [docs/PARITY_TRACE.md](docs/PARITY_TRACE.md) | CPU/Pascal parity contract and verification runbook |
| [docs/NATIVE_ACCEPTANCE_PLAN.md](docs/NATIVE_ACCEPTANCE_PLAN.md) | Optional full GPU qualification: checklist and per-scene ledger |
| [docs/REMAINING.md](docs/REMAINING.md) | Open work and standing decisions |

Records in `comparison/results/` are evidence of their day, not instructions.
`comparison/runs/` is the git-ignored directory for local raw runs; a clone does
not contain it, and historical records that link into it describe archives that
are no longer distributed.

## Licence

The library is a work based on Mandelbulb 3D and is distributed under the same
licence, the GNU Lesser General Public License, version 2.1:
[LICENSE](LICENSE). The origin of every part, the third-party code and where
the source is published are in [NOTICE.md](NOTICE.md).

## Reference safety

- `mb3d/` is the immutable Pascal reference. Never modify or build it in place;
  editable Pascal lives in an external working copy described in
  [docs/BUILD.md](docs/BUILD.md).
- `build-main` and `build-qualified-reference` are local aliases of frozen
  builds. Never rebuild through them.
- Keep binary, source, scene and asset identities intact, diagnose the first
  divergent computation, and keep existing acceptance thresholds.
