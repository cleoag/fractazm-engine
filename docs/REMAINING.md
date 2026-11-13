# Remaining work

The priority is a working library release for Apple applications. The default
path for any change is a build, the `release` test label, three smoke scenes per
enabled backend and a short regression for the behaviour changed
([BUILD.md](BUILD.md#default-small-release-check)). Deep qualification is a
separate, optional claim and never a release blocker.

## 1. Application library (v1)

[LIBRARY.md](LIBRARY.md) records the API, its checks and limits. Open, in order
of value for the planned macOS, iOS and visionOS applications:

1. Exact clip volume: mask geometry beside the box and stop rays at its back
   faces in the CPU and Metal ray setup.
2. Off-axis per-eye projection and `MTLTexture` output for head-tracked
   visionOS rendering; today each eye is a parallel-axis readback render.
3. Splitting long GPU commands so that a cancel request (now honoured before
   every command) is answered within a bound, and pipeline warm-up when a
   formula changes.
   The persistent renderer (`mb3d_renderer_*`) keeps the device, queue, library
   and setup checks; reusing GPU buffers and decoded maps between its frames
   is open.
4. Setters beyond the editor controls now in `mb3d_scene.h` (surface palette
   and its range, depth fog, the first global light, ambient strength): the
   other five lights, light colour and type, dynamic fog, diffuse and specular
   strength, interior colours, ambient occlusion, reflections and depth of
   field. The release check renders the colour and light edits on the CPU;
   on Metal they were compared with the CPU by hand on one scene
   (`test_01_intpow2`, 160x120) and on no device. Also a memory-buffer scene
   loader, and bundled formula metadata instead of reading `.m3f` files.
5. Navigation (`mb3d_scene_dolly_camera`, `_pan_camera`, the plane kept
   clear by every gesture helper): checked by the release test on one scene on
   the CPU and by contact sheets of four gallery scenes on Metal on an M4 Pro
   (fly in, turn, pan, back out). Open: the cost of the CPU distance estimate
   on iPhone and iPad; a start plane tested at more than its centre; raising
   iterations with the zoom; scenes with map formulas and scenes authored with
   a cut, which the helpers leave unchecked; the depth fog, which is tied to
   the depth range and so changes when backing away extends the far limit.
6. Device runs. Through the Swift wrapper, 30 package scenes ran on macOS and
   25 each in the iOS simulator, on an iPhone 17 Pro and on an iPad Pro M1
   (iOS 27.0); nothing has been run on Vision Pro. Open from those runs
   (numbers in the application repository,
   `docs/measurements/2026-11-stage0/REPORT.md`):
   - iPhone 17 Pro: PREVIEW ends with `Native general invalid GPU record` for
     Aexion 10bulbs at 320 px and for QuatP4hybridJulia and Theli-At - Black
     Mandelbulb at 240 px; 160 px renders. Mac and iPad render all sizes.
   - iPhone 17 Pro: the 1280 px FINAL frame of LightBulbMoon - A Fabulous
     Excuse ends with Metal's `Caused GPU Hang Error`.
   - iPad Pro M1: the dense 160 px preview frame takes 7.7 times the M4 Pro
     time and 4.6 times the iPhone 17 Pro time (medians of 25 scenes); the
     cause is not known.
   - The first render of a scene compiles pipelines for 5 to 139 s on these
     devices; the application API has no warm-up call.
   - Scenes whose primary pass is ordered (`primary_state_reason` 3) take 3 to
     105 s per 160 px preview frame on an M4 Pro and are unusable for
     navigation.
   - In the simulator 15 of 24 FINAL frames at 1280 px fell back to the CPU
     with `Native general buffers exceed device budget`; no device did.
   - The progress fraction of a FINAL render (`MB3DRenderRequest.progress`)
     follows the real share of time within about 25 % on an M4 Pro (four
     scenes at 1280 px): stages weigh differently in the quarter-size
     calibration pass and in the full pass. When the full pass runs on another
     backend than the calibration pass, the fraction is a time estimate only.
   - A larger ray step for motion frames was measured and not added: doubling
     `mZstepDiv` shortens a 160 px preview frame on an M4 Pro by a median of
     1.33 times over 25 scenes, but two of them (JuliaSetSample, Theli-At -
     Black Mandelbulb) then leave Metal with `Native FP32 range` and three
     change visibly (Hyperben2 - Ozosphere tweak, Theli-At - Bone Menger using
     MH, Dainbramage - Hydra). Tripling gains 1.5 times, and
     QuatP4hybridJulia leaves Metal as well.
   - `mb3d_scene_reproject_frame` has run on macOS only, and on three scenes.
   - iPad Pro M1, the application (2026-11-11): PREVIEW frames can end with
     Metal's `Impacting Interactivity`
     (`kIOGPUCommandBufferCallbackErrorImpactingInteractivity`): the system
     stops a command buffer it could not preempt in time for another user of
     the GPU. Seen on the first frame of a launch in 9 of 18 launches (320 px,
     about 20 ms per command, while the launch animation runs), in some
     launches on every 900 px frame for seconds (about 25 ms per command on
     average), and in one run on 6 of 7 frames of 160 px rendered by a second
     renderer over three minutes. The same frame renders when requested again.
     FINAL frames, which are thousands of commands of under a millisecond,
     never ended this way; one ended with `Discarded (victim of GPU
     error/recovery)` (`kIOGPUCommandBufferCallbackErrorInnocentVictim`).
     Which other user of the GPU triggers it is not established: the
     interruptions come in periods of seconds to minutes, also with a still
     screen and one renderer, and the same 20 frames in a row passed in two
     runs and lost 12 in a third. The photo analysis daemon was as busy in a
     clean run as in a bad period. The environment variable
     `AGX_RELAX_CDM_CTXSTORE_TIMEOUT=1` changed nothing (1 of 3 launches
     interrupted with it, 1 of 3 without). On an iPhone 17 Pro one launch with
     14 frames showed none. The library reports
     the error like any other Metal failure and does not retry; the
     application renders the frame again after a pause and does not start a
     frame on its second renderer while the first one draws.
   - The same iPad over the 140 gallery scenes (2026-11-12,
     [record](../comparison/results/quick-pipelines-20261112/REPORT.md)):
     every failed command was the primary stage, one command for a whole
     320 px preview frame (stopped after 145 to 750 ms in heavy scenes) or for
     a lattice part (`Caused GPU Hang Error` after 2 and 4 s). After two hangs
     in a row the system refused every command of the process (`Ignored (for
     causing prior/excessive GPU errors)`); the application now asks nothing
     more of a scene after its first hang. Lattice parts do end with
     `Impacting Interactivity`, unlike the whole frames noted above. Shorter
     primary commands, by a fixed batch or a time target with retry, did not
     help and were removed: a command of 64 threads still ran for 24 to
     253 ms, so single rays are that slow there. Open: a primary kernel that
     stops after a number of steps and continues in the next command; what
     the system's limit for a command is; whether the application's own
     animations are the other user of the GPU.
     What is published on these limits, a proposed design and twelve
     checks on devices are in `comparison/results/gpu-command-limits-20261112`
     (a survey of sources, nothing measured).
     The owner scheduled this work for the application's version after
     the first release (2026-11-12; `docs/ROADMAP.md` of the application,
     version 1.1): nothing of it is done for the first release.
   - `quick_pipelines` is a choice of the caller. Whether outlining becomes
     the default of the general engine is the owner's decision: 140 scenes at
     480 px and four at authored size are byte-identical, the qualification
     corpus was not compared. On the iPad Pro M1 nine scenes rendered their
     frame with effects outlined in 0.76 to 1.11 of the inlined time (record,
     section 5).
   - An iPhone 17 Pro ran the gallery with no command stopped for
     interactivity; its failure is a hang of a command of 1 to 4.5 s (six
     scenes, record section 6). Not tried there or on the iPad: whether a new
     command queue lifts the refusal that follows two hangs.
   - A pipeline archive directory the process may not write to ends the
     process inside Metal's archive serialization (seen in the sandboxed macOS
     application with `MB3D_METAL_PIPELINE_CACHE` set to a foreign path); the
     library does not check the directory first.
   - `scene_api_tests` failed once in nine runs at its cancellation check,
     which races a 1024x768 render against the cancel; the run followed one
     that had filled an empty archive.
   - iPad Pro M1, the application: MengerSpheres PREVIEW takes 2.7 s at
     224x320 and 0.7 s at 112x160, against 0.17 s at 224x320 for
     ABoxScale2Start; too slow to turn by hand.
   - Two renderers in one process (the application prepares gallery scenes with
     a second `MB3DRenderer` while the first draws the screen) ran on an M4 Pro
     over 21 scenes without a fault, and on the iPad Pro M1 with the
     interruptions above; this use has no test in the release set.
7. Record the release scope on Windows and on native x64 hardware.
8. Lattice frames (a final frame rendered in parts, `lattice_stride` in
   `mb3d_scene.h`): the release test renders one scene in four parts, and 21
   gallery scenes were compared with their whole frames on an M4 Pro
   ([record](../comparison/results/lattice-frames-20261111/REPORT.md)). Open:
   - The change touched the camera shaders that every Native frame uses. Dense
     frames of those 21 scenes are byte-identical before and after it; the
     frozen full-corpus images were not rendered again.
   - No lattice frame has been rendered on an iPhone, an iPad or in the
     simulator by the library's own checks.
   - Ambient occlusion is computed per part, so the merged picture differs
     from the whole frame where occlusion varies from pixel to pixel; DE AO
     dithering follows the part's rows instead of the frame's. AO computed
     once on the merged depth would remove both and would need a stage of its
     own.
   - Reflections in Curvichrome differ in blocks between the parts and the
     whole frame; the cause was not isolated.
   - A part always renders in the general engine. For scenes whose whole
     frame the restricted engine takes, parts and whole frame come from
     different engines.
   - Depth of field and Z-buffer normals refuse a lattice; so does the CPU.
   - Every part repeats what does not depend on the frame's size: the
     preparation (buffers, maps, pipeline lookup) and the `volume` stage,
     which made the sixteen parts of KochSurf Sample 6 cost 3.26 times its
     whole frame. Keeping these between the parts of one picture is open (see
     item 3).
9. The stage callback (`MB3DRenderRequest.stage`) is checked by the release
   test on one scene on the CPU, and by the same test run by hand with the
   shader library on an M4 Pro. The engines enter stages in different orders
   (documented in the header, not unified), and no stage carries an expected
   duration: a client derives that from the quarter-size pass.
10. Admission (`mb3d_scene_admits`): the release test checks the call's
    refusals on the CPU-only run, and by hand with the shader library on an M4
    Pro a 640x480 frame, its lattice part and 4096x4096 are admitted (the
    restricted engine takes that scene; the frame then rendered there in 1.9
    s) and 8192x6144 is refused for its size. A refusal for the budget is not
    in the test: it depends on the device. Open:
    - The budget is the one figure the device reports for the process
      (`recommendedMaxWorkingSetSize`); memory the application or another
      renderer already holds is not subtracted, so an admitted frame can still
      fail to allocate.
    - A whole frame the restricted engine admits can still need the general
      engine after its first pass (a camera inside the fractal); the general
      engine's budget is then checked only by the render.
    - Not run on an iPhone, an iPad or in the simulator by the library's own
      checks.
11. Community scenes. For the application's gallery, 1 282 scenes of a
    Pascal-rendered archive of about 16 000 were rendered on Metal on an M4 Pro
    and compared with Pascal's pictures
    ([record](../comparison/results/gallery-candidates-20261111/REPORT.md)):
    37 % ran without fallback and looked like Pascal's picture, 35 % ran on
    Metal but looked visibly different, the rest fell back to the CPU, failed
    or exceeded the time limit. The comparison is a statistic on reduced
    pictures, looked at by eye for a sample only. Open:
    - The cause of the visible differences was not investigated per scene.
      Eighteen formulas passed in at most 6 % of their scenes (listed in the
      record).
    - Scenes with Z-buffer normals refuse a lattice frame, so an application
      cannot draw their final frame in parts (24 of the graded scenes).
    - A background picture can send a scene to the CPU ("Unsupported Native
      background map image coordinates"); map files are not in the checkout.
    - One dIFS scene took 0.5 times `ABoxScale2Start` at a 320x240 area and 46
      times `MengerSpheres` at 640x480; a second one did not finish a cold
      render in 240 s. Neither was investigated.
    - `in a Celtic box.m3p` rendered on Metal with `mb3d_render` in that
      record, but through `mb3d_scene.h` in the application it is refused
      ("Native Metal general scene pass is not implemented") and would render
      on the CPU; the application's gallery leaves it out. The difference
      between the two paths was not investigated.

## 2. Interactive navigation

The viewer prototype and its sparse previews are described in
[LIBRARY.md](LIBRARY.md#macos-viewer-prototype). They are optional and not a
prerequisite of the library release. Open:

- **Presentation timing.** Input, service and draw events are traced; actual
  display acknowledgment and input-to-present latency are not measured on the
  AppKit path.
- **Nested GPU continuations** for long dependent rays: resumable ray, orbit,
  formula and normal state with state and RNG proofs. Source-derived callers,
  refinements, normals and inside retries have bounded exact controls; the
  [last checkpoint](../comparison/results/p2-prepared-rays-bounded-20261104/REPORT.md)
  keeps an unresolved ABoxScale2Start normal/trap/radius mismatch. Production
  owner and pool integration, pause/cancel/release, telemetry and corpus gates
  remain.
- **Adaptive quality and output:** a latency budget per command, GPU
  accumulation and reprojection, texture output instead of RGB readback.
- **Bounded full effects** after accumulation.
- **Numerical refusals.** The general preview admits all 77 cameras, but four
  scenes refuse on FP32 range (six with the fast geometry profile), and
  dependent scenes still take seconds per command
  ([record](../comparison/results/viewer-general-navigation-20261102/REPORT.md)).
  An all-scene GPU preview needs a concrete wider-range proposal (decision U5).
- **One public interactive API** shared by two clients. `mb3d_scene.h` covers
  scenes, cameras and progressive passes; prepared scenes that survive a camera
  change, capability queries and job handles are internal to the viewer.
- Clipboard paste into the viewer's Open/Maps path field is unconfirmed.

## Standing decisions

| Rule | Decision |
|---|---|
| U1 — pauses | Work continues without routine approval pauses; report after each finished package. |
| U2 — FP32 CPU acceptance | A whole Native CPU frame is accepted only for proven genuine binary32 overflow, with pinned approved resources, active effects and three exact repeats. It is counted separately from GPU coverage. |
| U3 — speed gate | At least 1.25× geometric-mean full-render speedup, 2× as an aspiration, and no confirmed regression over 5 %. |
| U4 — cold compilation | Cold pipeline compilation is measured separately from the 180 s diagnostic / 300 s hard frame limit. A compiler error is never a compile measurement. |
| U5 — wider GPU range | Unchanged: a proven range exit renders the whole frame on Native CPU, or fails in a client that forbids CPU. A wider-range or higher-precision GPU profile is proposed, not implemented or accepted. |
| U6 — formula scope | The 77 ordinary scenes come first. The 454 `extall_*` scenes measure coverage; all 467 registered formulas are part of final acceptance. |

## 3. Optional full GPU qualification

Run only on an explicit request. The checklist and the per-scene ledger are in
[NATIVE_ACCEPTANCE_PLAN.md](NATIVE_ACCEPTANCE_PLAN.md); the rules are in
[GPU.md](GPU.md). On the frozen `build-main` build the gate is OPEN with 57
scene, 438 formula, 10 pipeline and 1 binding obligations outstanding.

- **Formulas.** 29/457 external proofs are admitted, 28 of the 95 identities
  used by the 77 scenes. The planner
  `comparison/tools/plan_native_formula_verification.py` splits the 457 formulas
  into 38 groups; work resumes at group05. For each group pin the original
  semantics, parameters and caller state, run the independent reference and the
  device observer, and require three separate process repeats. Keep a mismatch
  OPEN and fix the implementation or document the arithmetic contract; never
  adjust the oracle to the device.
- **Scenes still unqualified on any build** (17), by cause:
  - *Reference evidence:* Torii temple needs CPU-reference provenance; Misty,
    ApolloBalloons and DEcomb1 need acceptable original-size CPU references
    (Misty's exceeded 300 s).
  - *Numerical attribution:* ABoxSOff4d, Nothing left but bones, Getting Loopy,
    Kiosk, Lenord — Between Dark and Light, Makin, Ringlets and Peacock need an
    explained FP32 difference or a correction.
  - *State and ownership:* cutted sphere over carpet needs an exact repair of
    carried dIFS state; genQuat needs its finite-only 4D change separated from
    the unqualified Housing first-use rule
    ([evidence](../comparison/results/native-state-entry-20261031/REPORT.md));
    Housing needs its signed shading bias explained.
  - *Slow tail:* ImpossibleWorld took 210.7 s, mostly reflections; spineJulia's
    CPU fallback exceeded 300 s and lacks an overflow proof.
- **Requalification.** The 60 scenes accepted on the earlier frozen build must
  be rerun or rebound on the final identity.
- **Pipeline and binding.** Ten pipeline obligations (scheduling, reuse,
  ownership, fault/commit behaviour, guards, held-out cases, control backends)
  and build binding remain: a compatible/control library and eight observer
  roles — `dependency_analysis`, `guard_failures`, `independent_heldout`,
  `metal_compatible`, `oracle`, `resource_reuse`, `result_reuse`, `scheduling`.
  The contract is `comparison/contracts/native-gpu-acceptance-v4/acceptance-contract.json`,
  checked by `comparison/tools/qualify_gpu_parallel_pipeline.py`.
- **Range gaps** in the `extall` coverage scenes: Fuzzy, Sin4Dplus, IFS_tess and
  the surface-dust group. A preview range exit is not a full-frame overflow
  proof.
- **Ordered scenes** (queue ranks 56, 70, 68) render far slower on the GPU than
  on the CPU at coverage size; see [GPU.md](GPU.md#state).
- **Speed and publication.** Balanced original-size baseline/candidate
  measurements with identical thread counts, cold compilation reported
  separately, then an independent final review with `PASS remaining=0`.

Resources: all 77 scenes have pinned maps. Six numbered maps are approved
available selections, not authenticated originals, and the missing original
1633 uses approved 3038 bytes in a distinct profile
([record](../comparison/results/native-resource-substitution-20261030/REPORT.md)).

## 4. CPU and Pascal parity

The CPU paths are part of the product and their acceptance is independent of
Metal; see [PARITY_TRACE.md](PARITY_TRACE.md).

- **Strict Stage A is open:** 13 Manowar caller-context cases, four odd-Flip
  gnarly/ducks cases and two zero-power Trifox cases; 28 excluded images, 18
  nonterminating orbits, six excessively long finite cases and two undefined
  `_GenCWarp` angle classes. Keep original and guarded Pascal identities apart.
- **Cross-platform images:** compatible camera and DE changes need fresh
  ARM64-versus-Windows comparisons on final binaries. x86-to-ARM image identity
  is not certified.
- **Stage B:** concurrent-versus-serial qualification at 1/2/4/16 workers,
  cold/warm/missing resources, cancellation and recovery. There is no hard
  cancellation latency guarantee; TSan covers the native subset only.
- **Windows:** the MSVC x64 build compiles under Windows ARM/Prism with an
  ordinary gate of 39/42 — `manowar_hybrid_tests` and `math3d_tests` fail
  x87-sensitive checks and `renderer_tests` was stopped externally. Validate
  the full-80-bit Prism setting or run on native Intel/AMD x64, with the
  original thresholds.
- **Apple CPU:** orbit, stage and image qualification on Apple Silicon is open.
- `CalcMissed` has a Pascal declaration but no audited call site; whether to
  port it is undecided.

## 5. Deferred unless the user asks

- Monte Carlo and JIT, including their tests, fixtures and comparison runs.
- GUI editors, animation and export.
- Extending the standalone exact-compatible Metal pipeline beyond what
  production and acceptance controls require.
