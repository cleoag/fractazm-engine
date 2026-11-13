# Native GPU acceptance plan

This is the optional full numerical qualification program, run only when the
user explicitly asks for it. The default library release follows
[the small release check](BUILD.md#default-small-release-check) and does not
require these proof totals. Frozen qualification claims keep their original
acceptance rules; the rules themselves are in [GPU.md](GPU.md).
`comparison/tools/report_gpu_acceptance.py` rewrites the marked scene list
below, so keep its markers and headings.

The checked main build uses native source **fc00c78e** and library **9c8bd7e9**.
Its gate admits **20/77 full scenes (25.97%): 19 GPU + 1 proven FP32 CPU**,
**29/457 external formula proofs (6.35%)**, and the ordinary-suite proof:
**OPEN506 = 57 scene + 438 formula + 10 pipeline + 1 binding obligations**.
Priority formula coverage is **28/95 (29.47%)**. [The promotion](../comparison/results/native-formula-group03-20261101/promotion.json)
pins its frozen evidence and qualification. `build-main` selects this build.
The [cb5897ef package](../comparison/results/native-formula-group02-tail-20261101/REPORT.md)
retains its 14-scene/20-formula OPEN521 snapshot.

The separate retained production identity has **60/77 scenes: 57 GPU + 3 FP32
CPU**, with **OPEN495**. Its [promotion record](../comparison/results/native-ao-promotion-20261031/REPORT.md)
and historical checklist below retain that scope. Those results do not transfer
automatically to a new executable or library. Earlier consolidation OPEN555
and initial affine OPEN553 are snapshots, not the selected build's gate.

Full binding still requires a compatible/control library and eight remaining
observer/oracle roles, listed in [REMAINING.md](REMAINING.md). `formula_gpu` and
`cpu_ordinary` are bound. Preserve historical scene evidence and requalify the
remaining corpus on the final new identity.

## Ordered execution checklist

This is the saved six-item resume order, expanded only with its existing
acceptance conditions. Current item: **6**. Check an item only when all its
conditions pass; then report the remaining numbered items in chat.

Items 1–5 are complete. The current build admits 29 bounded external formula
proofs after 497 exact point/orbit cases, including all 16 group02 and nine
group03 identities. Nineteen GPU scenes and Shells' fresh FP32-CPU outcome pass
the common gate. The [current review and promotion](../comparison/results/native-formula-group03-20261101/REPORT.md)
record the limits; no full-contract PASS is claimed. Release-scope formula
checks (`CHECKED_FOR_RELEASE`, 467/467) are a separate, weaker claim and add no
proof credit here; one closing candidate must eventually carry all 467 formula
decisions, without summing incompatible libraries.

- [x] **1. Finish reciprocal verification.** Preserve frozen build 508d1d30
  and the interrupted collection; collect the unchanged 75 cases in a new
  directory with three separate process repeats. No rebuild is needed.
- [x] **2. Recheck earlier formula groups and admit reviewed results.** Run
  affine40 and fold61 on the same library, independently review all three
  groups and admit successful proofs into acceptance-candidate.
- [x] **3. Verify full scenes.** Check Fishbowl, Hal-Tenny-Resistance,
  Curvichrome and Melting spot bloxx at original size with effects, three
  exact repeats and on/off controls. Compare saved old frames and validated
  CPU references. Recheck the six previously accepted scenes before counting them.
- [x] **4. Publish and promote.** Publish independently reviewed evidence,
  update live status and select the new main build only after required controls pass.
- [x] **5. Finish group02.** First verify updateC/Rotate/RotateC
  (7055,7060,7062), then the remaining five (4522,7010,7065,7076,7081).
  All 16 identities are independently admitted on cb5897ef; the corrected build
  and required full scenes are reviewed, published and promoted.
- [ ] **6. Finish the remaining acceptance plan.** Execute the following
  existing obligations in order. Current subitem: **6.1**. Counts below refer
  to the selected checked build; a new candidate must earn its own credit.
  - [ ] **6.1. Finish the 95 priority external formulas.** Currently 28/95
    (29.47%) on the selected build; 67 remain on that identity. Group04 has
    eight additional reviewed proofs on its separate candidate.
    Each group includes its existing independent
    state/parameter checks, three exact process repeats and required affected
    full-scene/effect controls before acceptance.
    Per the user's reduced-check instruction, run only changed-formula/state
    checks, the closest necessary regression controls and directly affected
    scenes in each package. Do not rerun all earlier formula groups, unchanged
    CPU suites or unaffected scenes after every change. Preserve their frozen
    evidence; perform the complete final-identity regression once for final
    acceptance. Do not transfer historical credit to a new library untested.
  - [ ] **6.2. Finish all remaining formula identities.** Complete the full
    457-external/10-internal scope after priority formulas. Currently 29/457
    external and 0/10 internal proofs are admitted; 438 formula obligations
    remain in total, including the priority work in 6.1.
  - [ ] **6.3. Complete acceptance of all 77 full scenes.** Currently 20/77
    (25.97%): 19 GPU and one proven FP32-CPU outcome; 57 remain. Preserve
    original settings, effects and approved resources, exact repeats and
    numerical explanations. Requalify on the final artifact identities.
  - [ ] **6.4. Close pipeline proofs and artifact binding.** Currently 1/11
    pipeline obligations passes; ten remain. Complete the compatible/control
    library and eight remaining observer/oracle roles, keeping every proof
    bound to the actual final binaries and libraries.
  - [ ] **6.5. Complete balanced full-render speed checks.** OPEN. Compare
    matched baseline/candidate runs, report cold compilation separately,
    achieve at least 1.25x geometric-mean warm speedup and no confirmed
    slowdown above 5%, with all effects preserved.
  - [ ] **6.6. Obtain independent final acceptance and publish.** Require
    the full gate to report **PASS remaining=0**, publish actual evidence,
    update live documentation, and commit/push the final result.

After each subitem is completed, report its result and the remaining **6.x**
subitems in chat. This nesting only expands item 6; it adds no new work.

Windows/Prism 39/42 remains separately open and does not expand or interrupt
this GPU checklist. Preserve historical evidence and all existing limits.

Implementation work follows [REMAINING.md](REMAINING.md); it does not require
an orchestrator, cycle budget or research skill.

Current target: all 77 ordinary scenes, all 467 registered ordinary formulas,
and the 11 full-pipeline obligations. Monte Carlo/JIT remain excluded. Continue
without routine pauses; respect the 180 s diagnostic / 300 s stop and 12 GiB
reserve. Check marks mean verified work, not preparation or previews.

| Stage | Progress | Completion condition |
|---|---|---|
| Resource selection | Done: 16/16 map IDs supplied, 77/77 scenes | Pin selected replacements separately; preserve prior resource profiles and document original-identity limits |
| Common evidence bundle | Selected main partial package reviewed; full binding remains OPEN | One manifest with actual artifacts; preserve all missing obligations as OPEN |
| Import FP32 CPU outcomes | Shells freshly proved on current main; two other historical outcomes need fresh checks | Re-evaluate all full scenes through the common gate |
| Full ordinary scenes | Current main 20/77 (25.97%): 19 GPU + 1 FP32 CPU; 57 remain | Original sizes/effects, three exact repeats and on/off controls on final identities |
| Formula coverage and state | 29/457 external proofs admitted (6.35%); 428 external and ten internal remain OPEN | All 467 identities, device fixtures, mixed chains and independent state/parameter proofs |
| Pipeline obligations | 1/11: ordinary suites admitted | Actual scheduling, reuse, provenance, ordinary CPU, compatible Metal, guards, heldout and ownership evidence |
| Full-render speed | OPEN | Matched baseline/candidate, cold reported separately, >=1.25 geometric mean, no confirmed >5% regression |
| Final acceptance and publication | OPEN | Full gate PASS remaining=0, docs/evidence committed and pushed |

Current main milestones:

- [x] Admit all four affine point/orbit proofs against the unchanged independent reference.
- [x] Admit the four folding/coordinate proofs: 61/61 independent device cases.
- [x] Admit Sphere (19/19) and X2 (16/16) against the fixed reciprocal reference.
- [x] Qualify the X3b/Y3b native correction with independent device and full-scene checks.
- [x] Admit all 16 group02 and nine group03 identities; replay all 497 formula cases on the final library.
- [x] Qualify 19 full GPU scenes and Shells as proven FP32 CPU with exact repeats and controls.
- [x] Review all 20 old/new GPU full-scene comparisons: exact RGB and logical work; Lenord remains outside admission pending resolution of its numerical discrepancy.
- [ ] Requalify the remaining corpus and close all formula/pipeline/binding obligations.

Retained production milestones below preserve their original artifact scope:

- [x] Preserve both CPU arithmetic profiles and the native Metal FP32 boundary.
- [x] Prove full-frame FP32 overflow and exact CPU repeats for ExcludeBulbMeng, Freak Of Nature and Shells.
- [x] Verify separate CPU classification and strict rejection of unrelated fallbacks.
- [x] Pin available maps under the latest user selection.
- [x] Assemble and validate the common evidence manifest.
- [x] Include the three FP32-CPU outcomes in its actual counters.
- [x] Verify the selected 58-scene GPU seed/continuation change: exact old/new images and work, three repeats and effect controls.
- [x] Verify AO chunk-prefix/BaseTable reuse against all 57 accepted GPU scenes, with exact full old/new RGB/work and active-effect controls. Rebinding the three FP32-CPU outcomes is separate from this GPU check.
- [ ] Complete all available ordinary GPU scenes, fixing faults before repeats.
- [ ] Resolve slow cases after smaller diagnostics; do not restart expired full frames blindly.
- [ ] Complete the formula/state and pipeline proof obligations.
- [ ] Complete balanced performance measurements and final independent checks.
- [ ] Achieve and publish the complete acceptance result.

The source queue is `comparison/scene-queue.json`. Detailed dated observations
belong in `comparison/results/`. Candidate maps now are
explicitly user-approved selections, not newly authenticated originals. The
immutable `mb3d/` reference and frozen previous runtimes remain unchanged.

<!-- BEGIN VERIFIED SCENES -->

## Qualified scenes on the current main build

- [x] ABoxPartlyJuliaRoff — QUALIFIED_GPU
- [x] Aexion 10bulbs — QUALIFIED_GPU
- [x] AkuraPare - 463b — QUALIFIED_GPU
- [x] BatJorge - Pong 703 - Swirls — QUALIFIED_GPU; approved selected map 129
- [x] Dainbramage - Hydra — QUALIFIED_GPU
- [x] Fishbowl — QUALIFIED_GPU
- [x] Genetic Menger — QUALIFIED_GPU
- [x] Hal-Tenny-Resistance (slow) — QUALIFIED_GPU
- [x] Insect Menger — QUALIFIED_GPU
- [x] Jost1 - Tweak - New tiny temple — QUALIFIED_GPU
- [x] julia on box — QUALIFIED_GPU
- [x] JuliaSetSample — QUALIFIED_GPU
- [x] MarkJayBee - Curvichrome — QUALIFIED_GPU
- [x] Melting spot bloxx — QUALIFIED_GPU
- [x] Oxnot - Shells — QUALIFIED_FP32_CPU; approved selected map 24
- [x] QuatP4hybridJulia — QUALIFIED_GPU
- [x] Rowratty - Spirals — QUALIFIED_GPU
- [x] Surreal shell — QUALIFIED_GPU
- [x] Theli-At - Black Mandelbulb — QUALIFIED_GPU
- [x] TimeMachine — QUALIFIED_GPU

<!-- END VERIFIED SCENES -->

Selected maps 129 (Swirls) and 24 (Shells) are approved available replacements,
not authenticated originals. Hydra passes its narrow numerical review. Lenord —
Between Dark and Light remains OPEN (490 hit-mask differences; 13.878° median
normal-angle difference), as does Ringlets. Repeatability alone does not resolve
numerical discrepancies.

## Qualified scenes on the retained production identity

- [x] ABoxPartlyJuliaRoff — GPU
- [x] ABoxScale2Start — GPU
- [x] ABoxScale3Start — GPU
- [x] Abominog-Ua - 12.06.2014 — GPU
- [x] Aexion 10bulbs — GPU
- [x] AkuraPare - 463b — GPU
- [x] AureliusCat - Bamboo — GPU
- [x] BatJorge - Pong 703 - Swirls — GPU
- [x] Bill-Snowzell - Tweak - Canyon — GPU
- [x] Blood on the dancefloor — GPU
- [x] BulboxCut — GPU
- [x] Chrystal — GPU
- [x] Dainbramage - Home of the Ancients — GPU
- [x] Dainbramage - Hydra — GPU
- [x] Dodeca Torus mix — GPU
- [x] Ellarien - Embellished — GPU
- [x] Ellarien - Shrooms are coming — GPU
- [x] ExcludeBulbMeng — proven FP32 → native CPU
- [x] Fishbowl — GPU
- [x] Genetic Menger — GPU
- [x] Hal-Tenny-Resistance (slow) — GPU
- [x] HalTenny - Freak Of Nature — proven FP32 → native CPU
- [x] Hyperben2 - Ozosphere tweak — GPU
- [x] InAndOutside box — GPU
- [x] Insect Menger — GPU
- [x] Jost1 - Tweak - New tiny temple — GPU
- [x] JuliaSetSample — GPU
- [x] KentPress - Parameter 3 tweak fogits — GPU
- [x] KochSurf Sample 6 — GPU
- [x] Lenord - ASurf 2 — GPU
- [x] Lenord - The Beanstalk — GPU
- [x] LightBulbMoon - A Fabulous Excuse — GPU
- [x] LightBulbMoon - No, Never — GPU
- [x] Mandelwerk - TgladTetra inner spirals — GPU
- [x] MarkJayBee - Curvichrome — GPU
- [x] Melting spot bloxx — GPU
- [x] MengerTrees — GPU
- [x] Oxnot - Flexing — GPU
- [x] Oxnot - Shells — proven FP32 → native CPU
- [x] QuatP4hybridJulia — GPU
- [x] Rama - Elysium — GPU
- [x] Recycledrelatives - Fractal Fan — GPU
- [x] Recycledrelatives - Test for SurfMod — GPU
- [x] Surreal shell — GPU
- [x] ThePearl dIFS — GPU
- [x] Theli-At - Black Mandelbulb — GPU
- [x] Theli-At - Bone Menger using MH — GPU
- [x] Theli-At - MengerSpheres — GPU
- [x] TimeMachine — GPU
- [x] TreePlanet — GPU
- [x] Virtual tubes world (UndeadAcademy style) — GPU
- [x] Wada basin - Reflect test — GPU
- [x] Waste-and-tragedy - Mandalex — GPU
- [x] dIFS heart shrub otrap — GPU
- [x] julia on box — GPU
- [x] material colors — GPU
- [x] toricaleggs — GPU
- [x] Rowratty - Spirals — GPU
- [x] dIFS heart shrub — GPU
- [x] dIFS shrub — GPU



## Scenes still unqualified on the retained production identity

The two fast state-entry candidates remain outside production totals until promotion and regression checks.

- [ ] 0Encrypted0 - ABoxSOff4d example — explain or fix FP32 image differences
- [ ] 6 AM - Torii temple — CPU-reference provenance
- [ ] AboxShapes-Misty — CPU reference exceeded 300 s
- [ ] ApolloBalloons dIFS — bounded full CPU reference
- [ ] DEcomb1 — bounded full CPU reference
- [ ] Dainbramage - Nothing left but bones — explain or fix FP32 image differences
- [ ] DsyneGrafix - Getting Loopy — explain or fix FP32 image differences
- [ ] Fractales75 - Kiosk — explain or fix FP32 image differences
- [ ] ImpossibleWorld dIFS — slow reflections; diagnose before full repeats
- [ ] Lenord - Between Dark and Light — explain or fix FP32 image differences
- [ ] Mandelwerk - Makin seashell — explain or fix FP32 image differences
- [ ] PaMonk - Ringlets — explain or fix FP32 image differences
- [ ] Sabine62 - Housing problem — signed shading bias and kernel-change review
- [ ] UndeadAcademy tweak - Peacock in blue — explain or fix FP32 image differences
- [ ] cutted sphere over carpet — exact GPU repair of carried dIFS state
- [ ] genQuat helix mix — finite-only candidate promotion and final regressions
- [ ] spineJulia — slow FP32 CPU fallback and full overflow proof
