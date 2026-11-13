# AGENTS.md

Working rules for `mb3d_cpp_port`. Start at [README.md](README.md) for the
status table and document index.

## Release workflow

- Use the minimum verification the actual change needs. Prefer existing tests
  and reuse a successful result while its inputs are unchanged.
- Add a test only for a demonstrated bug or a significant changed contract that
  no existing check covers. No tests mirroring the implementation, duplicate
  checks, speculative edge-case matrices, new test frameworks or audit
  infrastructure.
- The default scope is `release`: the `mb3d_release_checks` target, the CTest
  label `release` and three small real scenes per enabled backend, plus a short
  regression for the behaviour actually changed. Run it for a release candidate,
  not after every small edit. [docs/BUILD.md](docs/BUILD.md) owns the commands.
- Full `ordinary` runs, corpus matrices, Pascal oracles, repeated independent
  audits, AIR/dependency proofs, three-process repeats and effect-off matrices
  are optional deep qualification: run them only on an explicit request or to
  diagnose a concrete failure.
- Release checks do not claim full formula, scene or Pascal qualification.
  Report `CHECKED_FOR_RELEASE` separately from historical `VERIFIED` totals.
- Arithmetic, tolerances, the public API, cancellation, resource/fault/range
  guards, the disk reserve and the frame limits stay in force.
- Once the relevant checks pass, commit and continue. Report each finished
  package and go on without a routine pause; stop for an explicit pause, a real
  blocker or a decision that belongs to the user
  ([docs/REMAINING.md](docs/REMAINING.md#standing-decisions)).

## Documents

The maintained documents are listed in [README.md](README.md#documentation).
When work changes the state, update them and
[docs/REMAINING.md](docs/REMAINING.md) in the same change. Keep dated progress
narratives out of them: a dated investigation is a record under
`comparison/results/<record>/`. Records are evidence of their day, never
alternative instructions. Read [docs/GPU.md](docs/GPU.md) before changing GPU
code, [docs/PARITY_TRACE.md](docs/PARITY_TRACE.md) before Pascal-versus-C++
work, and [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) before touching external
formulas.

## Repository

- `main` is the single integration branch. Autoresearch is retired: do not
  restart its loop, create an orchestrator or depend on a user skill.
- The maintained scope is the C++ CPU renderer on x86-64 and ARM64, Native
  Metal, the application library, the required checks and tools, the immutable
  Pascal reference, and the scenes, maps and formulas.
- Close each change with verification, updated documents and a commit. Keep
  generated builds, virtual environments, logs and temporary reports out of
  Git. Do not leave dirty detached worktrees or silently merge an unqualified
  experimental renderer.
- A rebuild creates a new binary and library identity. Historical acceptance
  counts and timings stay attached to the frozen artifacts that earned them.
- Dependencies and fixtures of the bootstrap entry points must be in Git or in
  the pinned dependency declarations. Never copy from another developer's
  build tree, `comparison/runs` or site-packages.

## Source of truth

- `mb3d/` is the immutable Pascal reference tree. Never modify or build it.
- Editable Pascal lives in an external working copy with its own Git history;
  see [docs/BUILD.md](docs/BUILD.md).
- Keep original and guarded Pascal executable identities separate. A guarded
  rejection does not turn an original nonterminating case into parity PASS.

## Scope and priorities

- The current priority is a working library release for Apple applications:
  [docs/LIBRARY.md](docs/LIBRARY.md) and the first section of
  [docs/REMAINING.md](docs/REMAINING.md).
- The longer goal is Native Metal rendering of every ordinary scene and every
  external formula within the binary32 limit, with maximal GPU parallelism.
  - Phase one is the 77 ordinary scenes of `mb3d/M3Parameter`, ordered in
    `comparison/scene-queue.json`. `Test8.m3p` was removed by the user, and the
    Monte Carlo/JIT scenes are excluded.
  - Preserve each scene's formulas, hybrid order, settings and effects. Use
    original resources or the pinned map selections the user approved; label
    them as selected substitutes, never as authenticated originals.
  - Count completion by qualified full scenes, not by generated formulas, host
    preparation or previews. The 454 single-formula `extall_*` scenes are a
    coverage measure.
- **Monte Carlo and JIT are excluded by default** from analysis, implementation,
  tests, benchmarks and percentages. Never run `monte_carlo_tests`,
  `halton_tests` or unfiltered test discovery. Resume only on the user's
  explicit request.

## Limits

- **Frame limits.** A complete frame, including preparation and effects, has a
  180 s diagnostic threshold and a 300 s hard stop. Investigate a slow frame
  with small diagnostics; do not raise the deadline or launch more full
  repeats. Defer persistently slow scenes to the end of the queue with their
  original settings.
- **Pipeline compilation** is measured and reported separately from the frame
  limit. A render that sits at start for minutes or ends with a pipeline error
  is a backend compile problem until shown otherwise; see
  [docs/BUILD.md](docs/BUILD.md#metal-pipeline-archive-cold-and-warm-renders).
- **Disk.** Keep 12 GiB free. Remove superseded task-owned builds, dumps and
  logs after keeping compact evidence. Preserve frozen references and final
  user-visible images.
- **Builds versus runs.** Never edit sources or rebuild while a build, ctest or
  GPU run uses the build directory. Run long GPU jobs on frozen copies of the
  binary and library.

## Arithmetic and evidence

- **CPU arithmetic is selectable per render.** `compatible` preserves the
  software x87 contract; `native` uses hardware FP64. Keep both and record the
  effective mode in evidence. Native differences are allowed but never count as
  x87/Pascal or cross-platform PASS. Exact repeatability within a mode is
  required.
- **Native Metal** uses hardware binary32. Its differences from CPU are allowed
  only when explained and recorded in
  [docs/GPU.md](docs/GPU.md#fp32-limits).
  - The FP32 range policy is the only post-dispatch CPU fallback: more than 1 %
    of geometry pixels leaving binary32 sends the frame to Native CPU
    (`fp32_range`). A whole Native CPU frame caused by proven genuine FP32
    overflow is an accepted qualification outcome, reported separately from
    GPU coverage; an unexplained range exit is OPEN.
  - Every other GPU fault fails without an image.
  - No CPU per-pixel work is allowed inside a reported Metal render.
- **Optional full qualification.** Compilation, admission and previews are not
  numerical qualification. A scene claimed as qualified needs original-size
  frames with active effects, same-engine on/off controls and three exact
  repeats on the final library and host identity. An ownership or policy change
  needs an old/new comparison with byte-identical RGB and work.
- **Comparisons.** CPU and GPU comparisons use the same `--threads`. Every GPU
  timing states whether the pipeline archive was cold or warm.
- **Evidence placement.** Compact evidence goes to
  `comparison/results/<record>/`; raw runs go to the git-ignored
  `comparison/runs/<run>/`. Never overwrite frozen references or reuse
  historical totals for a new binary. Synthetic runner fixtures prove validator
  behaviour only. Missing archive evidence is not permission to regenerate or
  relabel it.
- **Reports.** Report the source commit, commands, suite counts, smoke images
  and any failures. A clean-clone check on an existing host is not a test on a
  fresh OS.
