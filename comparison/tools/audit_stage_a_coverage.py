"""Inventory ordinary Stage-A evidence and prepare cached-reference replay jobs.

This is an audit, not a parity runner or a completion certificate. Historical PASS,
original reference blockers, unsupported inputs, and unmeasured branches stay
separate. No renderer, compiler, Monte Carlo, or JIT evaluation is invoked.
"""
from __future__ import annotations

import argparse
import ast
from collections import Counter, defaultdict
import hashlib
import json
from pathlib import Path
import struct
import sys
import zlib

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))
from gen_bulk_formula_table import M3F_OPTION_MAP, compute_pvar_offsets
RUN = ROOT / "comparison/runs/external-complete"
RESULT = ROOT / "comparison/results/external-complete-20261013"
EXCLUDED = {"JITTrigExample", "JITBenesiPineTree", "JIT_IQ_Bulb"}
DEFAULT = ROOT / "comparison/results/stage-a-20261018/coverage-ledger-v1.json"


def digest(data):
    return hashlib.sha256(data).hexdigest()


def relative(path):
    return Path(path).resolve().relative_to(ROOT).as_posix()


def local(path):
    p = Path(str(path).replace("\\", "/"))
    return p if p.is_absolute() else ROOT / p


def save(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2, allow_nan=False) + "\n", encoding="utf-8")


def declarations(path):
    """Read option declarations without crossing a line such as standalone .SSE3."""
    result, inside = [], False
    for line in path.read_text(encoding="latin1").splitlines():
        line = line.strip()
        if line.startswith("["):
            inside = line.upper() == "[OPTIONS]"
            continue
        if not inside or not line.startswith(".") or "=" not in line:
            continue
        left, default = line[1:].split("=", 1)
        words = left.split(None, 1)
        if words and words[0].lower() in M3F_OPTION_MAP:
            result.append(dict(type=M3F_OPTION_MAP[words[0].lower()], label=words[1].strip() if len(words) > 1 else "", text=line, default_text=default.strip()))
    return result


class Inventory:
    def __init__(self):
        self.inputs = {}
        self.cache = {}
        self.source_cache = {}
        self.batch_manifests = {}
        self.manifest_index = None

    def read(self, path):
        path = local(path)
        key = relative(path)
        if key not in self.cache:
            raw = path.read_bytes()
            self.inputs[key] = digest(raw)
            self.cache[key] = json.loads(raw)
        return self.cache[key]

    def ref(self, path, anchor):
        if path not in self.source_cache:
            raw = (ROOT / path).read_bytes()
            self.inputs[path] = digest(raw)
            self.source_cache[path] = raw.decode("utf-8", errors="replace")
        text = self.source_cache[path]
        offset = text.find(anchor)
        if offset < 0:
            raise ValueError(f"Source anchor absent: {path}: {anchor}")
        return dict(path=path, line=text.count("\n", 0, offset) + 1, anchor=anchor)

    def phases(self):
        path = ROOT / "comparison/tools/prioritize_external_fixes.py"
        raw = path.read_bytes()
        self.inputs[relative(path)] = digest(raw)
        points, images = [], []
        for node in ast.walk(ast.parse(raw)):
            if not isinstance(node, ast.For) or not isinstance(node.iter, (ast.List, ast.Tuple)):
                continue
            try:
                values = ast.literal_eval(node.iter)
            except (ValueError, TypeError):
                continue
            if "scene-points-current-v9" in values:
                points.extend(values)
            if "general-images-v1" in values or "priority-reference-recovery-v4" in values:
                images.extend(v for v in values if "jit" not in v.lower())
        if not points or not images:
            raise ValueError("Cannot find explicit historical overlay ordering")
        return points, images

    def image_reference(self, case_id, row, evidence, seen=None):
        """Follow recorded provenance to an original batch; never synthesize PNGs."""
        seen = set() if seen is None else seen
        path = local(evidence)
        key = relative(path)
        if key in seen:
            return dict(status="unknown", reason="reference provenance cycle")
        seen.add(key)
        report = self.read(path)
        original = next((x for x in report.get("results", []) if x.get("scene") == case_id), None)
        if original is None:
            return dict(status="unknown", reason="case absent from recorded summary", summary=key)
        if "batches" in report:
            if key not in self.batch_manifests:
                candidates = [path.parent / "manifest.json", RUN / "challenges-v4/manifest.json",
                              RUN / "priority-families-v1.json",
                              RESULT / (path.parent.name + ".json")]
                manifest = None
                for candidate in candidates:
                    if candidate.exists():
                        h = self.inputs.setdefault(relative(candidate), digest(candidate.read_bytes()))
                        if h == report.get("manifest_sha256"):
                            manifest = candidate
                            break
                if manifest is None:
                    if self.manifest_index is None:
                        self.manifest_index = {}
                        pool = list(RUN.glob("*manifest*.json")) + list(RUN.glob("*/*manifest*.json"))
                        for candidate in sorted(pool):
                            if any(word in str(candidate).lower() for word in ("jit", "monte", "mc-", "mc_")):
                                continue
                            raw = candidate.read_bytes()
                            self.manifest_index.setdefault(digest(raw), candidate)
                    manifest = self.manifest_index.get(report.get("manifest_sha256"))
                    if manifest:
                        self.inputs[relative(manifest)] = report["manifest_sha256"]
                self.batch_manifests[key] = manifest
            manifest = self.batch_manifests[key]
            png = path.parent / "pascal" / (case_id + ".png")
            if manifest is None:
                return dict(status="unknown", reason="original batch manifest not resolved", summary=key)
            expected = original.get("pascal_image_sha256")
            if not expected or not png.exists():
                return dict(status="blocked", reason="original Pascal PNG unavailable", summary=key)
            actual = digest(png.read_bytes())
            if actual != expected:
                raise ValueError(f"Pascal PNG identity mismatch: {png}")
            return dict(status="cached_original_png_verified", summary=key,
                        manifest=relative(manifest), png=relative(png), png_sha256=actual,
                        pascal_sha256=report.get("pascal_sha256"),
                        scene_sha256=original.get("scene_sha256"),
                        loader_identity="replay_parity validates original formula-load log before use")
        next_path = original.get("reference_summary") or row.get("reference_summary")
        if not next_path:
            return dict(status="unknown", reason="no original batch reference link", summary=key)
        return self.image_reference(case_id, original, next_path, seen)

    def broad(self):
        report = self.read(RUN / "trifox-broad-v1/summary.json")
        observations = defaultdict(lambda: defaultdict(list))
        scenes = []
        for row in report["results"]:
            origin = self.read(row["reference_summary"])
            path = local(origin["provenance"]["scene_dir"]) / (row["scene"] + ".m3p")
            raw = path.read_bytes()
            if digest(raw) != row["scene_sha256"]:
                raise ValueError(f"Ordinary scene identity mismatch: {path}")
            data = raw
            if len(data) != 1976:
                try:
                    data = zlib.decompress(data)
                except zlib.error:
                    pass
            entry = dict(scene=row["scene"], suite=row["suite"], path=relative(path),
                         sha256=row["scene_sha256"], historical_status=row["status"],
                         historical_exact_rgb=row.get("exact_rgb"),
                         reference_summary=relative(local(row["reference_summary"])))
            if len(data) != 1976 or not 20 <= struct.unpack_from("<i", data)[0] <= 44:
                entry["selector_decode"] = "unknown: not audited modern 1976-byte layout"
                scenes.append(entry)
                continue
            calc3d, ao, dof, optic = bool(data[344]), data[149], data[181], data[148] & 3
            # Both loaders unconditionally reset stereo (Pascal FileHandling
            # UpdateFormulaAndParameterValues / C++ m3p_parser equivalent).
            # Stored labels cannot certify the direct-header stereo kernels.
            selectors = dict(calc3d=calc3d, optics=optic, stereo=0,
                             stereo_raw=data[126], hybrid=data[841] & 3)
            if not calc3d:
                selectors["slice_effective"] = data[345] if data[345] in (1, 3) else 2
                selectors["slice_raw"] = data[345]
            # A header bit is only an observation when its outer stage is enabled.
            if calc3d and ao & 1:
                at = (ao >> 1) & 7
                selectors["ao_effective"] = "DE" if ao & 12 == 12 else str(10 if at == 6 else at + 4 if optic == 2 and 2 <= at <= 5 else at)
                selectors["ao_quality"] = (ao >> 4) & 3
            if calc3d and dof & 1:
                selectors["dof_effective"] = f"{'plain' if dof & 8 else 'sorted'}:passMode{(dof >> 1) & 3}"
            entry["header_selectors"] = selectors
            for name, value in selectors.items():
                observations[name][str(value)].append(row["scene"])
            scenes.append(entry)
        return dict(summary="comparison/runs/external-complete/trifox-broad-v1/summary.json",
                    reference="comparison/runs/20261013T132245Z", cases=scenes,
                    counts=dict(Counter(x["historical_status"] for x in scenes)),
                    exact_rgb=sum(x["historical_exact_rgb"] is True for x in scenes),
                    historical_cpp_sha256=report.get("cpp_sha256"),
                    historical_cpp_seconds=sum(x.get("cpp_ms", 0) for x in report["results"]) / 1000,
                    observations={k: {v: dict(count=len(ids), scenes=ids) for v, ids in sorted(values.items())}
                                  for k, values in sorted(observations.items())},
                    limitation="Gated header observations, not dynamic branch coverage; no current-candidate execution.")

    def feature(self, ident, title, pascal, cpp, tests, selectors, gap, suites=()):
        return dict(id=ident, feature=title, status="partial_historical_evidence",
                    pascal=[self.ref(*r) for r in pascal], cpp=[self.ref(*r) for r in cpp],
                    test_sources=[self.ref(*r) for r in tests], supported_selectors=selectors,
                    related_343_suites=list(suites), uncovered=gap,
                    all_semantics_covered=False, candidate_execution="not_run_by_this_audit")

    def features(self):
        F = self.feature
        T = lambda name: ("tests/test_renderer.cpp", "static void " + name + "(")
        return [
            F("scene-loader", "Scene records, migrations, defaults and malformed input",
              [("mb3d/FileHandling.pas", "LoadParameter")], [("src/io/m3p_parser.cpp", "M3PLoadResult LoadM3PFile(")],
              [("tests/test_m3p_parser.cpp", "test_roundtrip_bytes"), ("tests/test_m3p_parser.cpp", "test_truncated")],
              ["M3P/M3I versions and addon schema", "aliases/default counts", "invalid dimensions/truncation"],
              ["All accepted historical file-version migration paths and their cross-product with external-slot defaults are not enumerated.", "M3I cached-buffer version/size/paint-state coupling needs an explicit matrix."]),
            F("formula-identity", "External name, asset, registry and loader identity",
              [("mb3d/CustomFormulas.pas", "FillCustomVBufWithVars")], [("src/io/m3p_parser.cpp", "void ParseExternFormulas("), ("src/core/external_formula_table.cpp", "RegisterExternalFormulaTable")],
              [T("test_external_formula_registry"), T("test_external_legacy_option_defaults")],
              ["457 active assets", "case-insensitive names/aliases", "six formula slots"],
              ["Static source mapping is complete for names only; dynamic loading and all parameter-layout contracts are separate obligations."]),
            F("integer-loader", "Integer conversion and negative parameter-buffer capacity",
              [("mb3d/CustomFormulas.pas", "const MemNeeded:"), ("mb3d/CustomFormulas.pas", "PInteger(p)^ := Round(dOptionValues[i]);")],
              [("src/core/header_trafos.cpp", "void BuildFormulaConstBuffer("), ("src/core/header_trafos.cpp", "case 20:")],
              [T("test_formula_const_buffer_layouts")],
              ["INTEGER and 2INTEGER", "nearest-even ties", "256-byte negative-buffer bound", "option limit16"],
              ["Shared loader fixes in progress require independent original-Pascal half-integer/boundary fixtures and final candidate replay.", "Prove exact-fit, one-byte/one-store overflow, skipped multi-value option consumption, and subsequent option placement."]),
            F("compound-loader", "Compound options, rotations, reciprocal and scale preprocessing",
              [("mb3d/CustomFormulas.pas", "procedure FillCustomVBufWithVars")], [("src/core/header_trafos.cpp", "void BuildFormulaConstBuffer(")],
              [T("test_original_formula_angle_loader"), T("test_formula_const_buffer_layouts")],
              ["option types0..22", "multi-value cursor advance", "zero/tiny reciprocal", "rotation matrices and constants"],
              ["Recorded raw-layout PASS/FAIL must distinguish immutable used bytes, pointers, mutable state and unused bytes.", "No per-option universal boundary/equivalence proof is recorded."]),
            F("hybrid", "Alternating, interpolating, DE-combination and IFS slot routing",
              [("mb3d/CustomFormulas.pas", "(pHCA.bOptions1 and 3) = 1"), ("mb3d/formulas.pas", "procedure doHybridPas(")],
              [("src/core/header_trafos.cpp", "LongBool bIsInterpolHybrid"), ("src/core/formulas.cpp", "void doHybridPas(")],
              [T("test_external_formula_chain_semantics"), T("test_hybrid_option_normalization")],
              ["mode0/1/2/3", "slot0..5", "repeat/zero/negative stop counts", "DE-combination operator/group boundary"],
              ["343 has no interpolating mode1; arbitrary six-slot orders and state-writing transitions are not covered.", "DE-combination operator, smoothing, mixedIFS/analyticDE and second-group termination need explicit coupled cases."], ["scenes_phase3_decomb", "scenes_phase3_ifs"]),
            F("orbit-state", "Julia/C, 4D, derivative, radius, trap and persistent formula state",
              [("mb3d/formulas.pas", "procedure doHybrid4DPas("), ("mb3d/formulas.pas", "function doHybridIFS3D(")],
              [("src/core/formulas.cpp", "void doHybrid4DPas("), ("src/core/formulas.cpp", "double doHybridIFS3D(")],
              [T("test_external_four_d_original_code"), T("test_aboxvaryscale4d_persistent_state_contract"), T("test_ifs_minimum_carry_between_calls")],
              ["3D/4D", "Julia boolean and C/W writeback", "first iteration/counters", "Rout/DE/SmoothItD/OTrap", "conditional lane writes"],
              ["Independent point samples do not prove complete hybrid orbits or state lifetime between DE/normal/shadow calls.", "Per-formula applicable state lanes and conditional write sets require explicit classification."], ["scenes_phase3_julia", "scenes_phase3_ifs"]),
            F("raymarch", "Primary ray stepping, refinement and distance estimation",
              [("mb3d/CalcThread.pas", "procedure TMandCalcThread.Execute")], [("src/core/ray_marcher.cpp", "RMCalculateVgradsFOV")],
              [T("test_iteration_boundary_original_pascal"), T("test_iteration_refinement_keeps_depth_and_color_mode"), T("test_unordered_orbit_reaches_maxit")],
              ["inside/outside", "analytic/numerical DE", "minimum/maximum iterations", "step limiter/refinement", "NaN/unordered branch"],
              ["Step/refinement boundary classes crossed with DEcomb, IFS and cut planes are not fully enumerated."], ["scenes_builtin", "scenes_phase2"]),
            F("camera", "Perspective/planar/panoramic rays and stereo",
              [("mb3d/HeaderTrafos.pas", "bPlanarOptic")], [("src/core/ray_marcher.cpp", "void RMCalculateVgradsFOV(")],
              [T("test_ray_setup_dump_capture")], ["optics0..2", "stereo selector", "FOV/image scale/camera transform"],
              ["343 observes optics0/2 only, stereo0/1/2 only; other accepted modes and degeneracies need source-qualified tests.", "Stereo with panorama, cut planes and reflection camera reconstruction is not a declared interaction matrix."], ["scenes_phase3_panorama", "scenes_phase3_stereo"]),
            F("slice", "2D slice plane and coloring without late 3D passes",
              [("mb3d/HeaderTrafos.pas", "if calc3D or (iSliceCalc = 1)"), ("mb3d/CalcThread2D.pas", "procedure T2DcalcThread.Execute;")],
              [("src/core/header_trafos.cpp", "if (Result.calc3D || Result.iSliceCalc == 1)"), ("src/core/calc_2d.cpp", "void Calc2DSliceRows(")],
              [], ["1 near", "3 far", "otherwise normalize2 middle", "FormulaType/DE coloring"],
              ["343 contains only middle mode2; near/far and normalized0 have no final343 evidence.", "2D x IFS/DEcomb/color-on-iteration and suppression of shadow/AO/reflection/DOF require explicit assertions."], ["scenes_phase3_slice"]),
            F("cutplanes", "Cut-plane clipping, normals and inside rendering",
              [("mb3d/CalcHardShadow.pas", "HSminLengthToCutPlane")], [("src/core/reflection.cpp", "SR_minLengthToCutPlane")],
              [T("test_unordered_ifs_minimum_and_inside_flags")], ["axis masks/orientation", "inside flag", "primary/shadow/reflection clipping"],
              ["All cut-plane sign/mask/boundary combinations and inside/transmission coupling are not classified."], ["scenes_phase3_cutplanes"]),
            F("hard-shadow", "Hard and soft shadows for selected lights",
              [("mb3d/CalcHardShadow.pas", "procedure THardShadowCalcThread.calcHSsoft;")], [("src/core/renderer.cpp", "static inline bool ShouldRunHardShadowPass(")],
              [T("test_shadows_affect_output"), T("test_paint_replay_restores_completed_hard_shadow_state")],
              ["six light maskbits", "soft flag/softness", "completed-mask state"],
              ["All per-light positional/directional and soft/hard combinations are not enumerated; check completed-header bits before paint replay."], ["scenes_phase3_multilight", "scenes_phase2"]),
            F("ao", "AO dispatch, quality, panorama, threshold and DE sampling",
              [("mb3d/AmbHiQ.pas", "ATi :="), ("mb3d/CalcAmbShadowDE.pas", "implementation")],
              [("src/core/ambient_occlusion.cpp", "void CalcAmbientOcclusion("), ("src/core/ambient_occlusion.cpp", "void CalcAmbientOcclusionRows(")],
              [T("test_ao_threshold_replay_clears_background_quartet_lanes")],
              ["effective dispatch0..10 and DE bypass", "quality0..3", "threshold/random/pano", "repeat count and first-step dithering"],
              ["343 effective dispatch only2,3,4,5,6; no0,1,7,8,9,DE. Quality only1.", "Dispatch10 appears shadowed by earlier DE mask for ordinary header mode6; do not count unreachable byte patterns as exercised.", "Pano/threshold/random/quality interactions and repeat-state reset require targeted cases."], ["scenes_phase3_hiqao", "scenes_phase3_panorama"]),
            F("lighting", "Material coloring, gradients, lighting and background",
              [("mb3d/PaintThread.pas", "implementation"), ("mb3d/Paint.pas", "implementation")], [("src/core/colorizer.cpp", "namespace mb3d")],
              [T("test_final_rgb_surface"), T("test_corrected_parity_image_hotspots")],
              ["six lights and light types", "diffuse/specular/interior gradients", "surface color modes/roughness", "background/map gamma"],
              ["Reachable material/color/light selectors have not been exhaustively extracted into a value matrix.", "Orbit-trap/color-on-iteration, lightmap gradients and reflection material reuse share state."], ["scenes_phase3_multilight"]),
            F("maps", "Formula maps and semantic image sampling",
              [("mb3d/HeaderTrafos.pas", "Map")], [("src/core/lightmap.cpp", "namespace mb3d")],
              [T("test_external_formula_map_slot_masks")],
              ["map masks and slots", "real image/synthetic oracle", "projection/channel/filtering/wrapping", "missing resource"],
              ["Synthetic map point inputs alone do not verify production image decoding, projection, mip/filter, missing-map or resource-lifetime behavior.", "Every map formula's used slots/channels and callback ABI need separate contract evidence."]),
            F("fog", "Dynamic fog and iteration-based accumulation",
              [("mb3d/CalcThread.pas", "procedure TMandCalcThread.DoDynFog")], [("src/core/reflection.cpp", "SR_DoDynFog")],
              [T("test_directional_volumetric_map_matches_pascal")], ["distance/iteration fog", "random/nonrandom stepping", "primary/reflected rays"],
              ["Fog modes crossed with volumetric-light selection, inside and refraction are not fully classified."], ["scenes_phase3_fog"]),
            F("volume", "Volumetric-light prepass and primary/reflected accumulation",
              [("mb3d/CalcSR.pas", "procedure TSRCalcThread.DoDynFog")], [("src/core/reflection.cpp", "void CalcVolLightMap(")],
              [T("test_directional_volumetric_map_matches_pascal")], ["selected light index", "map prepass", "substeps/absorption"],
              ["Light type/mask, map extent, zero-step boundaries and primary/reflection reuse require an interaction matrix."], ["scenes_phase3_vollight"]),
            F("reflection", "Reflection, transparency and recursive secondary rays",
              [("mb3d/CalcSR.pas", "procedure TSRCalcThread.CalcRay(")], [("src/core/reflection.cpp", "static void SR_CalcRay(")],
              [T("test_reflection_primary_trace_matches_pascal_hotspot"), T("test_paint_replay_reproduces_reflection_late_pass")],
              ["bounce count", "reflection/transmission", "inside absorption/refraction", "late-pass gates"],
              ["All bounce/inside/transmission/cutplane/volume couplings and termination boundaries are not classified."], ["scenes_phase3_reflections"]),
            F("dof", "Sorted/forward depth-of-field accumulation and pass count",
              [("mb3d/DOF.pas", "procedure doDOFsort("), ("mb3d/DOF.pas", "procedure doDOF(")], [("src/core/dof.cpp", "void CalcDepthOfField(")],
              [], ["sorted/plain", "passMode0..3", "disabled/enabled", "foreground/background blur"],
              ["343 only sorted/passMode0; plain and passMode1/2/3 absent.", "Zero-radius/depth boundaries, clipped image edges and reflection-before-DOF order need explicit coverage."], ["scenes_phase2"]),
            F("packed-output", "Packed depth/normals and surface state",
              [("mb3d/TypeDefinitions.pas", "TsiLight5 =")], [("src/core/ray_marcher.cpp", "namespace mb3d")],
              [T("test_depth_codec_original_assembly"), T("test_silight_populated"), T("test_normalmap_export")],
              ["18-byte surface record", "depth rounding/saturation", "normal encoding", "background sentinel"],
              ["Format contracts have fixtures; per-stage packed-field equality for every feature combination is not proven."]),
            F("exports", "Image/depth/normal export and API error contracts",
              [("mb3d/FileHandling.pas", "implementation")], [("src/core/renderer.cpp", "namespace mb3d"), ("src/core/mb3d_api.cpp", "mb3d_render")],
              [T("test_bmp_writer"), T("test_png_writer"), T("test_zbuffer_export"), T("test_c_api"), T("test_error_invalid_dimensions")],
              ["BMP/PNG", "depth encodings/normalmaps", "dimensions/output paths/API errors"],
              ["All export-format/size/endianness/error combinations are not classified; image parity is not file-byte parity."]),
            F("workers-rng", "Ordinary worker schedule and deterministic random state",
              [("mb3d/CalcThread.pas", "iThreadCount")], [("src/core/thread_pool.cpp", "namespace mb3d")],
              [T("test_multithreaded_consistency"), ("tests/test_pas_lcg.cpp", "int main(")],
              ["fixed logical workers", "ordinary jitter/AO RNG", "single/repeat/trace runs"],
              ["Final candidate fixed-thread plain/repeat/trace matrix is pending; physical-worker independence is not assumed.", "Concurrent different scenes and race checking belong to next-stage ownership preparation."]),
            F("diagnostics", "Stage replay and trace noninterference",
              [("mb3d/DivUtils.pas", "AllAutoProcessings")], [("src/core/renderer.cpp", "PaintFromSiLight")],
              [T("test_parity_trace_capture"), T("test_parity_trace_stage_session_handoff"), T("test_paint_replay_diag_only_traces_reflection_row_window")],
              ["plain/repeat/trace", "primary/AO/paint/reflection windows", "saved packed-state replay"],
              ["Final candidate byte/state noninterference has not been replayed for all representative enabled-stage combinations."]),
        ]

    def build(self):
        queue = self.read(RESULT / "fix-priorities-v37.json")
        names = {r["formula"] for r in queue["queue"]}
        if len(names) != 457 or names & EXCLUDED:
            raise ValueError("Active queue cardinality/exclusion mismatch")
        backend = self.read("comparison/results/backend-readiness-20261018.json")
        mappings = {x["formula"]: x for x in backend["formulas"]}
        meta = {x["name"]: x for x in self.read(RUN / "manifest.json")["formulas"] if x["name"] in names}
        master = self.read(RUN / "challenges-v4/manifest.json")
        cases = {x["id"]: x for x in master["cases"] if all(n in names for n in x["target_formulas"])}
        if len(cases) != 5506:
            raise ValueError("Active case count mismatch")
        point_phases, image_phases = self.phases()
        points, images = {}, {}
        for phase in point_phases:
            path = RUN / phase / "summary.json"
            if not path.exists():
                continue
            for row in self.read(path)["results"]:
                if row["case"] in cases:
                    points[row["case"]] = dict(row, evidence=relative(path))
        coverage = self.read(RESULT / "coverage.json")
        old_formulas = {x["formula"]: x for x in coverage["formulas"] if x["formula"] in names}
        def overlay(row, evidence):
            ident = row.get("scene")
            if ident in cases and row.get("scene_sha256") == cases[ident]["sha256"]:
                images[ident] = dict(row, evidence=evidence)
        for old in old_formulas.values():
            for phase, rows in old["images"].items():
                if "jit" in phase.lower():
                    continue
                for row in rows:
                    overlay(row, relative(RUN / row["evidence"].replace("\\", "/")) if row.get("evidence") else relative(RUN / phase / "summary.json"))
        for phase in image_phases:
            path = RUN / phase / "summary.json"
            if not path.exists():
                continue
            report = self.read(path)
            if phase in ("priority-reference-recovery-v4", "trifox-images-v2") and not report.get("finished"):
                continue
            for row in report["results"]:
                overlay(row, relative(path))
        formula_cases = defaultdict(list)
        for case in cases.values():
            for name in case["target_formulas"]:
                formula_cases[name].append(case)
        jobs = defaultdict(list)
        entries, unresolved = [], []
        for ident, case in sorted(cases.items()):
            p, im = points.get(ident, {}), images.get(ident, {})
            if p.get("scene_sha256") and p["scene_sha256"] != case["sha256"]:
                raise ValueError(f"Point scene identity mismatch: {ident}")
            rejection = None
            for slot in case["slots"]:
                n, opts = slot["name"], slot.get("options", [])
                if not slot.get("iterations"):
                    continue
                if n in ("gnarlyIFS", "gnarly2IFS", "gnarly3IFS", "ducksIFS") and len(opts) > 8 and round(opts[8]) & 1:
                    rejection = "odd-Flip reads caller stack outside local frame; strict input rejection must be verified"
                if n == "TrifoxComplexAngles" and len(opts) > 3 and (round(opts[0]) == 0 or round(opts[3]) == 0):
                    rejection = "original zero integer-power loop does not terminate; strict input rejection must be verified"
            entry = dict(case=ident, target_formulas=case["target_formulas"], scene=case["scene"].replace("\\", "/"), scene_sha256=case["sha256"],
                         point=dict(status=p.get("status", "missing"), evidence=p.get("evidence"), reason=p.get("error")),
                         image=dict(status=im.get("status", "missing"), exact_rgb=im.get("exact_rgb"), evidence=im.get("evidence"), reason=im.get("error")),
                         strict_input_rejection=rejection, final_candidate_status="not_run")
            if im.get("status") == "pass":
                ref = self.image_reference(ident, im, im["evidence"])
                entry["reference"] = ref
                if ref["status"] == "cached_original_png_verified" and ref.get("scene_sha256") != case["sha256"]:
                    raise ValueError(f"Original reference scene mismatch: {ident}")
                if ref["status"] == "cached_original_png_verified" and not rejection:
                    jobs[(ref["summary"], ref["manifest"])].append(ident)
                elif not rejection:
                    unresolved.append(dict(case=ident, reason=ref))
            else:
                entry["reference"] = dict(status="historical_reference_blocked_or_missing", reason=im.get("error"))
            entries.append(entry)
        formulas = []
        for q in sorted(queue["queue"], key=lambda x: x["formula"].casefold()):
            name = q["formula"]
            m, mapping = meta[name], mappings[name]
            point_counts = Counter(points.get(c["id"], {}).get("status", "missing") for c in formula_cases[name])
            if dict(point_counts) != q["point_counts"]:
                raise ValueError(f"Point aggregation differs from active queue: {name}: {point_counts} != {q['point_counts']}")
            opts, stored_index = [], 0
            decls = declarations(ROOT / m["asset"])
            if [d["type"] for d in decls] != m["metadata"]["option_types"]:
                raise ValueError(f"Option declaration types differ from metadata: {name}")
            for index, kind in enumerate(m["metadata"]["option_types"]):
                width = {5: 3, 6: 3, 12: 6, 18: 4, 19: 2}.get(kind, 1)
                values = set()
                for case in formula_cases[name]:
                    for slot in case["slots"]:
                        if slot["name"] == name and slot.get("iterations") and stored_index + width <= len(slot["options"]):
                            values.add(tuple(slot["options"][stored_index:stored_index + width]))
                opts.append(dict(declaration_index=index, stored_value_index=stored_index, stored_value_count=width,
                                 runtime_param_index=len(compute_pvar_offsets(m["metadata"]["option_types"][:index])),
                                 loader_type=kind, declaration=decls[index],
                                 observed_challenge_values=[list(v) for v in sorted(values)], branch_domain="unknown: declaration values are not a proven branch enumeration",
                                 coverage_status="finite_samples_only"))
                stored_index += width
            asset = ROOT / m["asset"]
            asset_hash = digest(asset.read_bytes())
            if asset_hash != m["asset_sha256"]:
                raise ValueError(f"Original formula asset changed: {asset}")
            self.inputs[m["asset"]] = asset_hash
            implementation = mapping["source_mapping"]["implementation"]
            self.inputs[implementation["path"]] = digest((ROOT / implementation["path"]).read_bytes())
            formulas.append(dict(formula=name, downloaded_unique_scenes=q["downloaded_unique_scenes"],
                pascal_asset=dict(path=m["asset"], sha256=asset_hash), cpp_implementation=implementation,
                source_mapping_kind=mapping["source_mapping"]["kind"], registration=mapping["registration"],
                metadata=m["metadata"], options=opts, case_ids=sorted(c["id"] for c in formula_cases[name]),
                historical_point_counts=q["point_counts"], historical_image_counts=q["image_counts"],
                raw_parameter_layout=q["parameter_layout"], raw_layout_is_complete_semantics_proof=False,
                available_historical_profile_names=[p for p, v in old_formulas[name].get("profiles", {}).items() if v and "jit" not in p.lower() and "monte" not in p.lower()],
                current_candidate_status="not_run", all_semantics_covered=False,
                remaining_obligation_ids=["discrete-selector-domains", "parameter-boundaries", "state-lane-applicability", "slot-repeat-interactions", "loader-used-byte-contract"] + (["real-map-contract"] if m["metadata"].get("map_value_mask") else [])))
        broad = self.broad()
        features = self.features()
        point_totals = Counter(x["point"]["status"] for x in entries)
        image_totals = Counter(x["image"]["status"] for x in entries)
        replay_jobs = [dict(reference=k[0], manifest=k[1], cases=v, count=len(v)) for k, v in sorted(jobs.items())]
        return dict(schema_version=1, scope=dict(stage="A", ordinary_formula_groups=457, listed_external_cases=5506,
            excluded_formula_names=sorted(EXCLUDED), deferred=["Monte Carlo", "JIT"], outside_headless_scope=["GUI/editors/animation tooling"]),
            interpretation="Historical evidence inventory and pending obligations; not a complete feature/parameter proof and not final-candidate certification.",
            acceptance_contract=self.ref("docs/PARITY_TRACE.md", "## Completion gates and remaining work"),
            gates={"A1_scope": "open: 457 formula groups mapped, important feature families catalogued, branch/state classification still incomplete",
                   "A2_semantics": "open: explicit gaps and unknown equivalence classes remain",
                   "A3_numerical": "open: historical listed PASS is not new-candidate verification; generic point/caller distinctions retained",
                   "A4_reference_defects": "open: reference blockers and strict rejection contracts are not numerical PASS",
                   "A5_frozen_candidate": "not_run"},
            totals=dict(formulas=len(formulas), external_cases=len(entries), historical_points=dict(point_totals), historical_images=dict(image_totals),
                        historical_exact_external_images=sum(x["image"]["exact_rgb"] is True and x["image"]["status"] == "pass" for x in entries),
                        strict_rejection_cases=sum(bool(x["strict_input_rejection"]) for x in entries),
                        cached_reference_replay_cases=sum(j["count"] for j in replay_jobs), unresolved_reference_mapping=len(unresolved),
                        feature_families=len(features), feature_families_with_complete_semantic_proof=0,
                        historical_generic_groups_both_listed_pass=sum(q["point_counts"].get("pass", 0) == len(formula_cases[q["formula"]]) and q["image_counts"].get("pass", 0) == len(formula_cases[q["formula"]]) for q in queue["queue"])),
            denominators=dict(historical_group_percentage="446/457 ordinary formula groups; finite listed points and images only",
                              historical_point_percentage="5487/5506 original listed active cases; no profile substitution",
                              feature_completion_percentage="unknown: full reachable branch denominator has not been established"),
            ordinary343=broad, features=features, formulas=formulas, external_cases=entries,
            reference_replay_plan=dict(jobs=replay_jobs, unresolved=unresolved,
                historical_selected_cpp_seconds=sum(images[i].get("cpp_ms", 0) for j in replay_jobs for i in j["cases"]) / 1000,
                warning="Timing sums historical per-case C++ time across overlays, excluding process/setup/report cost; not a promised wall-clock duration.",
                preparation="python -X utf8 comparison/tools/audit_stage_a_coverage.py --prepare-replay <NEW_DIR> --cpp-exe <FROZEN_EXE>",
                ordinary343_command=[sys.executable, "-X", "utf8", "comparison/tools/replay_parity.py", "--reference", "comparison/runs/20261013T132245Z", "--cpp-exe", "<FROZEN_EXE>", "--output", "<NEW_FINAL343_DIR>", "--checkpoint-every", "16"],
                point_note="Use original-code fixture-backed ordinary tests first. Generic point replay needs a filtered ordinary manifest; Manowar caller13, oddFlip4, and zero-power2 retain their original statuses. Expanded point profiles are separate evidence and are not replaced by one generic all-cases invocation."),
            input_sha256=dict(sorted(self.inputs.items())))


def prepare_replay(ledger, directory, exe, chunk_size=384):
    """Write reviewable jobs only; leave all actual execution to the caller."""
    directory = directory.resolve()
    if directory.exists():
        raise ValueError("Preparation directory must be new; refusing to mix candidate plans")
    if not exe or not exe.is_file():
        raise ValueError("--prepare-replay requires an existing --cpp-exe")
    directory.mkdir(parents=True)
    binary_hash = digest(exe.read_bytes())
    jobs, input_hashes = [], {}
    chunks = [dict(job, cases=job["cases"][start:start + chunk_size]) for job in ledger["reference_replay_plan"]["jobs"] for start in range(0, job["count"], chunk_size)]
    for index, job in enumerate(chunks):
        cases_path = directory / f"cases-{index:02d}.json"
        save(cases_path, job["cases"])
        command = [sys.executable, "-X", "utf8", str(ROOT / "comparison/tools/replay_parity.py"),
                         "--reference", str(ROOT / job["reference"]), "--manifest", str(ROOT / job["manifest"]),
                         "--case-list", str(cases_path), "--cpp-exe", str(exe.resolve()),
                         "--output", str(directory / f"replay-{index:02d}"), "--checkpoint-every", "16"]
        jobs.append(dict(index=index, cases=job["cases"], command=command, output=str(directory / f"replay-{index:02d}")))
        for path in (ROOT / job["reference"], ROOT / job["manifest"], cases_path):
            input_hashes[str(path.resolve())] = digest(path.read_bytes())
    input_hashes[str(ROOT / "comparison/tools/replay_parity.py")] = digest((ROOT / "comparison/tools/replay_parity.py").read_bytes())
    for tool in ("sweep.py", "compatibility_precision.py"):
        path=ROOT/"comparison/tools"/tool
        input_hashes[str(path)]=digest(path.read_bytes())
    # Explicit omissions are obligations, never a changed parity denominator.
    omissions = [r for r in ledger["external_cases"] if r["strict_input_rejection"] or r["image"]["status"] != "pass"]
    save(directory / "plan.json", dict(schema_version=1, cpp_exe=str(exe.resolve()), cpp_sha256=binary_hash, jobs=jobs,
         input_sha256=input_hashes, chunk_size=chunk_size,
         total_original_cases=5506, scheduled_cases=sum(j["count"] for j in ledger["reference_replay_plan"]["jobs"]),
         excluded_from_numeric_replay=omissions, unresolved=ledger["reference_replay_plan"]["unresolved"],
         status="prepared_only_not_executed", remaining="Run and verify strict rejection/caller profiles separately; original blockers remain blockers."))


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--output", type=Path, default=DEFAULT)
    ap.add_argument("--prepare-replay", type=Path)
    ap.add_argument("--cpp-exe", type=Path)
    ap.add_argument("--chunk-size", type=int, default=384)
    args = ap.parse_args()
    if args.chunk_size < 1:
        ap.error("--chunk-size must be positive")
    ledger = Inventory().build()
    save(args.output, ledger)
    if args.prepare_replay:
        prepare_replay(ledger, args.prepare_replay, args.cpp_exe, args.chunk_size)
    print(json.dumps(ledger["totals"], ensure_ascii=False))
    print(args.output)


if __name__ == "__main__":
    main()
