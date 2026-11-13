# Parity contract and verification runbook

The CPU/Pascal parity program: what "matches Pascal" means, acceptance rules,
the trace format, diagnosis and qualification state. It is optional: default
release checks ([BUILD.md](BUILD.md#default-small-release-check)) need no Pascal
build, trace or parity certificate, and an ordinary fix does not run it. When
parity is claimed every rule below applies and known failures stay visible.

Related: [ARCHITECTURE.md](ARCHITECTURE.md), [BUILD.md](BUILD.md#pascal-reference),
[GPU.md](GPU.md), [NATIVE_ACCEPTANCE_PLAN.md](NATIVE_ACCEPTANCE_PLAN.md),
[REMAINING.md](REMAINING.md). Native Metal is FP32 and never CPU/Pascal parity.

## What parity means

Parity is a claim about one frozen C++ candidate, one arithmetic profile and one
reference identity over a declared finite case set. Exact state and trace, point
results, image results and behavioral rejection never substitute for each other,
and scene completion, equal RGB across thread counts or GPU-versus-CPU agreement
is different evidence. Monte Carlo and the formula JIT are excluded from every
denominator (do not run `monte_carlo_tests`, `halton_tests`, the MC Python
modules or the three JIT scenes); GUI, editors and animation are out of scope.

## Acceptance rules

Report outcomes separately:

- **Exact RGB:** decoded 8-bit pixels match (not PNG bytes). **Default image
  PASS:** every gate of `comparison/tools/image_diff.py`: max
  channel delta <= 255, mean <= 2, p99 <= 10, fraction of pixels with a channel
  delta > 2 <= 0.6, PSNR >= 40 dB, luminance SSIM >= 0.98; keep all metrics.
- **Numerical FAIL:** a required gate fails; a relaxed diagnostic profile never
  converts it. **Error, unverified, rejected:** keep the reason; an expected
  rejection can pass its behavioral test but is never a numerical PASS.

Point checks (`check_formula_scene_points.py`) accept a finite lane when
`abs(actual - reference) / max(1, abs(reference)) <= 2e-12`; NaN matches by
classification, infinity by equality. Dedicated fixtures may demand exact state
bits, signed zero or NaN behavior, and their declared policy wins. Analytic map
callbacks test the callback contract, not map sampling.

Crashes, timeouts, missing assets and partial outputs are errors. Do not drop
cases, lengthen timeouts, accept an empty formula substitution or relax a
threshold; keep failed attempts, and give replacements new identities.

## Precision and candidate identity

Compatibility gates pass `--precision compatible` explicitly, independent of CPU
architecture. Record the effective policy with source commit and dirty state,
binary, compiler and flags, scene and asset hashes, settings and worker count.
`native` results are a separate profile: exactly repeatable within the policy,
no Pascal claim, no relaxed threshold. The numerical-DE and camera fixtures
(frozen rows, the camera one against a literal x87 assembly mirror run under
Rosetta) are finite arithmetic checks, not image parity. Keep native Intel x64,
Windows x64 under ARM translation, Rosetta and Apple ARM64 identities apart. A
frozen report cannot supply counts for a new candidate; a change to shared
camera, DE or formula arithmetic needs fresh renders.

## Reference and runtime identity

`mb3d/` is immutable. Pascal is built or instrumented only in an external tree
made by `scripts/prepare_pascal_work.py` ([BUILD.md](BUILD.md#pascal-reference)).
Keep three profiles distinct: extracted original-CODE oracles, the patched
portable-math application (it uses `portable_math.dll` where original code used
x87 FSINCOS, so matching it does not prove original loader arithmetic), and the
guarded application. `patches/PasLcg.pas` mirrors `src/core/pas_lcg.h`, the
deterministic replacement for Pascal `Random`.

Every comparison retains source commit and dirty state; executable, compiler and
tool hashes; scene, formula CODE and map hashes; effective options and caller
profile; commands, worker count, timeouts, metrics, failures and output hashes.
Use the qualified Pascal random stream and matching worker counts (the 343-scene
ordinary corpus uses 16). Reproduce a divergence at one worker, then at fixed 1,
4 and 16; equality across thread counts is not assumed when logical RNG lanes
depend on scheduling. Concurrent processes need isolated runtimes and outputs.

Verify the formula that actually loaded and its prepared parameters before
claiming selector coverage: short saved counts, compound option widths and
loader migrations can replace a request by a default
(`external_validation.py`: `prepare_external_options`). Shared C++ expansion in
two probes is not an independent loader oracle. The 343-scene ordinary corpus is
342 unchanged inputs plus one `transformIFS` replacement for the invalid
standard-Rotate/dIFS input (`comparison/fixtures/invalid_difs_standard_transform.m3p`);
its [preflight snapshot](../comparison/results/stage-a-20261018/final343-reference-preflight-v1.json)
hashes were observed later, not retroactive proof.

## Trace contract

`comparison/tools/parity_trace_schema.json` (version 2) is the stage and field
inventory. Emitters are `src/core/parity_trace.cpp` and `ParityTrace.pas` in the
external Pascal tree. A line is:

```text
stage=<name> x=<pixel-x> y=<pixel-y> key=value ... checksum=0x0123456789ABCDEF
```

- Normalize CRLF to LF; values contain no whitespace. Floating fields carry text
  plus `<name>_bits`, and the bits are authoritative. Boolean-builder fields use
  0 and -1; pre-render coordinates are -1,-1.
- The checksum is FNV-1a-64 (offset `0xCBF29CE484222325`, prime
  `0x100000001B3`) over emitter-ordered lines joined with LF, no trailing LF. A
  floating field contributes `name=0xBITS` and `name_bits=0xBITS`, others one
  textual line. Stages repeat; `stages_order` is an inventory, not a grammar.
- `_canary` comes first and must match: fixed values plus sin, cos and log
  probes. A mismatch can be serialization, math or FPU-state drift; investigate
  it before reading later stages. C++ emits only its checksum, Pascal may add
  the payload.
- Stages: `_canary`, `ray_ready`, `calcde_enter`, `calcde_after_orbit`,
  `calcde_result`, `calcde_gradient`, `normals_raw` and the HiQ AO stages
  `ao_hiq_r_begin`, `_dir0_step`, `_dir0_eval`, `_sum`, `_finish`.
- CLI: `--parity-trace <path>`, `--parity-trace-schema`, `--parity-trace-level
  checkpoint|detail|all`, `--parity-trace-detail`, `--parity-trace-start-stage`,
  `--parity-trace-end-stage`, and exactly one `--diag-points x,y` for schema
  traces. CMake: `MB3D_ENABLE_PARITY_TRACE`.

`comparison/tools/trace_diff.py` validates fields and types, duplicate and
reserved keys, paired bits, emitter order and recomputed hashes. `--mode bits`
compares validated checksums. `--mode tolerance --ulp 4` needs complete detail,
equal nonfloating values and ordered signed ULP distance; nonfinite values match
only by identical bits. `--mode text` and legacy logs are diagnostic, never
bitwise evidence. Never relabel tolerant as exact.

Legacy diagnostic writers share one append lock. A clean run does not certify
trace completeness; post-AO, normal or shadow claims need live siLight buffers.

## Diagnosing a mismatch

1. Validate hashes, loaded formula and parameters, and the canary.
2. Find the first divergence along: setup, camera ray, formula inputs, orbit,
   DE and ray decision, depth and normals, shadows and AO, reflection, color.
3. Test causes (rounding, x87 versus SSE order, integer conversion, NaNs, state
   lifetime, RNG, assets); change shared math only for a measured shared cause.
4. Fix the common cause, add independent evidence, test the family, then run one
   broad qualification.

Single-pixel comparison (`$referenceExe` qualified reference, `$candidateExe`
frozen C++ candidate):

```powershell
& $referenceExe --render $scene --output "$out/pascal.png" --threads 1 --parity-trace "$out/pascal.trace" --diag-points 100,75 --parity-trace-level all
& $candidateExe $scene --output "$out/cpp.png" --threads 1 --deterministic --precision compatible --parity-trace "$out/cpp.trace" --diag-points 100,75 --parity-trace-schema --parity-trace-level all
python comparison/tools/trace_diff.py --pascal "$out/pascal.trace" --cpp "$out/cpp.trace" --mode bits --report "$out/diff.json"
```

Choose an in-bounds pixel; decoded RGB of plain, repeated, legacy-diagnostic and
schema runs must agree at each worker count before instrumentation is trusted.
Never run the unguarded original on known nonterminating inputs; use the guarded
profile with `check_formula_hang_guards.py --cpp $candidateExe --patched-pascal
$guardedPascalExe --output "$out/guard-checks"`, which certifies error handling
(expected exit codes, diagnostics, no completed image), not numeric answers.

## Final candidate qualification

Freeze one candidate directory (renderer, probes, tests, source snapshot,
compiler metadata), write to new output directories and never overwrite a
checkpoint. Run an explicit ordinary CTest and Python selection, not blanket
discovery. Image runs require `--precision compatible` and a CLI profile
confirming it; runners fail rather than use a default.

| Work | Tool (all in `comparison/tools/`) |
|---|---|
| External image plan | `audit_stage_a_coverage.py --output LEDGER --prepare-replay DIR --cpp-exe EXE` |
| External image replay | `run_stage_a_replay.py --plan DIR/plan.json --workers 1` |
| External point replay | `run_stage_a_points.py --cpp PROBE --oracle ORACLE --ledger LEDGER --output DIR` |
| Ordinary 343 | `prepare_final343.py --verify-snapshot`, then `replay_parity.py --reference REF --cpp-exe EXE --output DIR --checkpoint-every 16` |
| New original captures | `sweep.py --scene-dir SCENES --cpp-exe EXE --pascal-exe PAS --threads 16 --report-dir DIR` |
| Supplementary seal | `audit_final_supplementary.py --manifest MANIFEST --output SEAL` |
| Consolidation | `audit_stage_a_release.py` records and verifies; it renders nothing and does not close Stage A |

- Scope is fixed by
  [coverage-ledger-v1.json](../comparison/results/stage-a-20261018/coverage-ledger-v1.json):
  457 groups, 5,506 point cases, 5,478 scheduled images, 28 image exclusions. A
  changed scope needs a versioned ledger, not an edited denominator.
- Reuse a case only when scene hash, profile, candidate identity and outputs
  match; supplementary overlaps never enlarge primary totals.
- The primary collector needs exact case identities and scene hashes, rejects
  duplicates and allows only the recorded 19 generic point exceptions and two
  undefined selector cases. Its `--supplementary` arguments are historical
  links: validate supplementary evidence with an explicit manifest and combine
  the seal with the primary checkpoint. Collectors refuse to overwrite.
- Declare the ordinary test set exactly, adding `--additional-ordinary-test`
  for `m3i_parser_tests`, `legacy_loader_tests`, `formula_work_guard_tests` and,
  for the Manowar candidate, `manowar_hybrid_tests` and
  `manowar_sse_producer_tests`. `replay_parity.py --case-list` excludes
  `--resume` and `--formula`; resume only from a hash-verified checkpoint.
- Raw PNGs, traces and logs go under `comparison/runs/` (git-ignored, local);
  compact summaries go to `comparison/results/`. A final report states
  candidate identity, counts, exact versus tolerant results, exclusions and
  remaining obligations.

The exact compatible Metal host's scoped `PASS_SCOPED` gate is defined in
[GPU.md](GPU.md); it is not CPU/Pascal parity.

## Current qualification status

Records are bound to the build that produced them; raw runs are not in the repo.

| Candidate | Result | Record |
|---|---|---|
| Last full-corpus Windows/MSVC baseline (compatible) | 343/343 ordinary images (61 exact RGB); 5,478/5,478 scheduled external images (5,246 exact); 5,487 point PASS and 19 unverified of 5,506; point and image PASS for 446 of 457 groups | [checkpoint](../comparison/results/compatibility-final-20261018/checkpoint.json), [formula status](../comparison/results/compatibility-final-20261018/formula-status.json), [seal](../comparison/results/compatibility-final-20261018/supplementary-qualification.json) |
| Later reference-gap candidate | Fixes Manowar interpolation caller state, ABoxMod4d precision and `_IntPowRotateC` outgoing SSE state; 447 of 457 groups with the Manowar profile; scoped regression of the 39 affected of 5,849 scenes passes | [qualification](../comparison/results/reference-gaps-20261018/qualification.json) |
| Apple ARM64 native, compatible | `arm64_qualified` and broader `ordinary` numerical gates over the tracked fixtures with zero numerical failures; both Manowar chain suites and 34 byte-exact angle-loader rows pass | CTest labels; [BUILD.md](BUILD.md#native-apple-silicon-runtime-checks) |

The baseline is frozen and not relabelled as a run of a later binary; ratios such
as 446/457 are finite coverage, not completion. Windows and ARM64 results do not
inherit from each other, and ARM64 evidence is finite fixtures and smoke runs.
Supplementary ledgers: selectors (589 PASS plus two original-undefined), INTEGER
(166 boundary cases), the
[direct-header contract](../comparison/results/stage-a-20261018/direct-header-contract-v20-final.json),
[integer-domain ledger](../comparison/results/stage-a-20261018/integer-domain-ledger-v6.json)
and [M3I evidence](../comparison/results/stage-a-20261018/m3i-source-qualification-v20.json).

### Remaining reference limitations

Strict full parity is open because the original cannot be evaluated here:

- Generic point gaps: 13 Manowar caller-context cases, four odd-Flip
  gnarly/ducks cases, two zero-power Trifox cases.
- 28 excluded original images: four odd-Flip, two Trifox, 22 conditional. Bounded
  witnesses show 18 nonterminating orbits and six excessively long finite ones,
  not that every pixel of an excluded image diverges.
- Two undefined `_GenCWarp` selector angles (valid selectors are 0 to 23).
- Odd Flip reads saved EDI/ESI as a Double; strict C++ rejects the four
  originals. The Manowar caller profile is finite (13 cases, selected chains).

Rejection tests and bounded-execution guards ([ARCHITECTURE.md](ARCHITECTURE.md#bounded-execution))
are error handling, not numerical results.

## Completion gates and remaining work

**Stage A** (CPU/Pascal parity of the formula and ordinary feature scope), as
reported by `audit_stage_a_coverage.py`:

- **A1, scope:** every active formula and reachable ordinary feature is linked
  to source and supported modes. Open: 457 groups are mapped, but branch and
  state classification is incomplete.
- **A2, semantics:** selectors, boundaries, Julia and C, derivatives and state,
  map and hybrid conventions are covered by tests or bounded source
  equivalence. Open: explicit gaps remain; the
  [75-axis reconciliation](../comparison/results/stage-a-20261018/ordinary-axis-reconciliation-v20.json)
  is scoped evidence, not a branch enumeration.
- **A3, numerical:** all supported well-defined cases pass the unchanged point,
  state and image gates on a new candidate; historical PASS does not count.
- **A4, reference defects:** each is handled by tested rejection, a verified
  caller profile or a named approximation; rejection is not a numerical PASS.
- **A5, frozen candidate:** A1 to A4 hold on one frozen final candidate. Not run.

Stage A and strict full parity are open (`stage_a_complete` is false in the
records). Release closure with stated limits is a different claim; changing the
compatibility scope needs an explicit user decision.

**Stage B** (execution model): jobs own their resources and outputs, and
cancellation, work accounting and terminal results exist
([ARCHITECTURE.md](ARCHITECTURE.md#render-state-ownership-and-threading)). Open:
concurrent-versus-serial qualification of different scenes at 1, 2, 4 and 16
workers (cold, warm and missing resources, cancellation, recovery) and
race, memory and UB checking beyond the native runtime subset.

**Apple and GPU:** Apple CPU orbit, stage and image qualification on real
hardware is open. GPU claims need a declared numerical profile, device and
compiler identity and explicit fallback accounting; see [REMAINING.md](REMAINING.md).
