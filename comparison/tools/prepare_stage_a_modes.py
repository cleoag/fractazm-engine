"""Generate a small ordinary-mode gap corpus and optional separate-process Pascal references.

No C++ execution, no production edits, and no batch-state assumptions. A ready
reference is not a parity PASS. Header selectors are qualified against their
outer enable gates; pixel influence is reported separately from selector setup.
"""
from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import time

from external_validation import decode, slots, write_json
from image_diff import DEFAULT_THRESHOLDS, load_rgb
from pascal_formula_identity import loaded_formulas, validate_formula_load

ROOT = Path(__file__).resolve().parents[2]
DEFAULT = ROOT / "comparison/results/stage-a-20261018/additional-modes-v1"


def sha(raw):
    return hashlib.sha256(raw).hexdigest()


def rel(path):
    return path.resolve().relative_to(ROOT).as_posix()


def generate(output):
    definitions = [("control", {}, "ordinary control: HiQ2 quality1, sorted DOF pass0", "phase2")]
    for mode in (0, 1, 2, 3):
        definitions.append((f"slice_{mode}", {344: 0, 345: mode}, f"slice effective {mode if mode in (1, 3) else 2}; late3D flags retained", "phase2"))
    definitions.append(("planar", {148: 1}, "planar camera", "phase2"))
    for mode in (3, 4):
        definitions.append((f"stereo_{mode}", {126: mode}, f"stereo mode {mode}", "phase2"))
    for ati in (0, 1):
        definitions.append((f"ao_type{ati}", {149: 1 | (ati << 1) | 16}, f"AO effective dispatch{ati}", "phase2"))
    for ati in (3, 4, 5):
        definitions.append((f"ao_pano{ati}", {148: 2, 149: 1 | (ati << 1) | 16}, f"AO effective dispatch{ati + 4}, panorama", "phase2"))
    for quality in (0, 2, 3):
        definitions.append((f"ao_hiq_quality{quality}", {149: 1 | 4 | (quality << 4)}, f"HiQ2 quality{quality}", "phase2"))
    for quality in range(4):
        definitions.append((f"ao_de_quality{quality}", {149: 1 | 12 | (quality << 4), 188: 0}, f"DE-AO quality{quality}, dithering0", "phase2"))
    definitions.append(("ao_de_first_random", {149: 1 | 12 | 16 | 128, 188: 1}, "DE-AO first-step random plus dithering1", "phase2"))
    for plain in (False, True):
        for mode in range(4):
            if not plain and mode == 0:
                continue
            definitions.append((f"dof_{'plain' if plain else 'sorted'}{mode}", {181: 1 | (mode << 1) | (8 if plain else 0)}, f"enabled DOF {'plain' if plain else 'sorted'}, passMode{mode}", "phase2"))
    for byte in (0, 2, 8):
        definitions.append((f"dof_disabled{byte}", {181: byte}, "DOF disabled by bit0 regardless of stored mode/pass bits", "phase2"))
    for weights in ((25, 75), (75, 25)):
        definitions.append((f"interpolate_{weights[0]}_{weights[1]}", {841: 1}, f"interpolating two formulas weights {weights}", weights))
    base_path = ROOT / "comparison/scenes_phase2/phase2_intpow2_small.m3p"
    hybrid_path = ROOT / "comparison/scenes/test_21_hybrid.m3p"
    results = []
    for tag, changes, purpose, source in definitions:
        path = base_path if source == "phase2" else hybrid_path
        data = decode(path)
        # A small but nontrivial image keeps independent process checks cheap.
        struct.pack_into("<ii", data, 4, 96, 72)
        data[124] = 0  # Ordinary renderer only; deferred path is never requested.
        data[126] = 0
        if source != "phase2":
            # Interpolation stores IEEE754 Single weights in iItCount, not
            # integer percentages. Historical v1 integer cases remain as
            # explicit subnormal-sum/fallback controls in their saved manifest.
            struct.pack_into("<f", data, 848, source[0] / 100.0)
            struct.pack_into("<f", data, 848 + 188, source[1] / 100.0)
        for offset, value in changes.items():
            data[offset] = value
        scene = output / "scenes" / ("stage_a_" + tag + ".m3p")
        scene.parent.mkdir(parents=True, exist_ok=True)
        scene.write_bytes(data)
        active_slots = slots(data)
        if any(s["number"] >= 20 for s in active_slots):
            raise ValueError("Additional modes must use audited builtin formulas only")
        results.append(dict(id=scene.stem, scene=rel(scene), sha256=sha(data), target_formulas=[],
                            kind="ordinary-discrete-mode-gap", source=rel(path), source_sha256=sha(path.read_bytes()),
                            purpose=purpose, changes={str(k): v for k, v in changes.items()}, slots=active_slots,
                            dimensions=[96, 72], evidence_class="header-qualified; dynamic influence measured only after reference rendering"))
    manifest = dict(schema_version=1, scope="ordinary headless modes only; finite gap corpus, no exhaustive combinations claim",
                    source_contract="docs/PARITY_TRACE.md Stage A", cases=results)
    write_json(output / "manifest.json", manifest)
    return manifest


def render_one(exe, case, output, suffix=""):
    scene = ROOT / case["scene"]
    target = output / ("repeats" if suffix else "pascal") / (case["id"] + suffix + ".png")
    target.parent.mkdir(parents=True, exist_ok=True)
    command = [str(exe), "--render", str(scene), "--output", str(target), "--threads", "1"]
    start = time.monotonic()
    try:
        completed = subprocess.run(command, cwd=exe.parent, capture_output=True, timeout=60,
                                   creationflags=subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0)
        log = completed.stdout + b"\n" + completed.stderr
        code = completed.returncode
    except subprocess.TimeoutExpired as error:
        log = (error.stdout or b"") + b"\n" + (error.stderr or b"")
        code = "timeout"
    log_path = output / (case["id"] + suffix + ".log")
    log_path.write_bytes(log)
    row = dict(scene=case["id"], scene_sha256=case["sha256"], status="reference_error", command=command,
               pascal_ms=round((time.monotonic() - start) * 1000), returncode=code,
               pascal_log=log_path.name, pascal_log_sha256=sha(log))
    if code != 0 or not target.exists():
        row["error"] = "Pascal execution failed or produced no PNG"
        return row
    text = log.decode("cp1252", errors="replace")
    row["pascal_formula_load"] = validate_formula_load(text, scene, case["slots"])
    found = loaded_formulas(text, scene)
    for expected in case["slots"]:
        actual = found.get(expected["slot"])
        if not actual or actual["number"] != expected["number"] or actual["iterations"] != expected["iterations"]:
            row["error"] = f"Builtin loader identity mismatch: {expected} versus {actual}"
            return row
    pixels = load_rgb(target)
    row.update(status="reference_ready", pascal_image_sha256=sha(target.read_bytes()),
               image_shape=list(pixels.shape), dynamic_range=int(pixels.max()) - int(pixels.min()),
               nonconstant_rgb=bool((pixels != pixels[0, 0]).any()))
    if list(pixels.shape[:2]) != [72, 96]:
        row.update(status="reference_error", error="Unexpected reference dimensions")
    return row


def render_references(output, manifest, exe):
    exe = exe.resolve()
    executable_hash = sha(exe.read_bytes())
    expected = "179dec31f161bad54aa9351e5a438a9709672e047573907eb3d5bd6b74408c10"
    if executable_hash != expected:
        raise ValueError("Reference binary is not the retained independently qualified original batch-audit binary")
    report = dict(scope="Independent separate-process Pascal references; no C++ parity claim", finished=False, batches=[],
        manifest_sha256=sha((output / "manifest.json").read_bytes()), pascal_sha256=executable_hash, thresholds=DEFAULT_THRESHOLDS,
        provenance=dict(pascal_exe=str(exe), binary_origin_evidence="comparison/results/external-complete-20261013/cyl-torus-trifox-checkpoint-v1.json",
                        generator_sha256=sha(Path(__file__).read_bytes()), threads=1,
                        relevant_pascal_source_sha256={p: sha((ROOT / p).read_bytes()) for p in
                            ["mb3d/HeaderTrafos.pas", "mb3d/DOF.pas", "mb3d/AmbHiQ.pas", "mb3d/CalcThread2D.pas", "mb3d/DivUtils.pas", "mb3d/formulas.pas"]}),
        results=[], repeats=[], qualification="pending", influence=[])
    for case in manifest["cases"]:
        if sha(exe.read_bytes()) != executable_hash or sha((ROOT / case["scene"]).read_bytes()) != case["sha256"]:
            raise ValueError("Reference input changed during execution")
        row = render_one(exe, case, output)
        report["results"].append(row)
        report["counts"] = dict(Counter(r["status"] for r in report["results"]))
        write_json(output / "summary.json", report)
        print(case["id"], row["status"], row.get("error", ""), flush=True)
    # Ordinary randomized AO and a deterministic slice/DOF control use new processes.
    repeat_names = {"stage_a_control", "stage_a_slice_0", "stage_a_ao_pano5", "stage_a_ao_de_first_random", "stage_a_dof_plain3"}
    for case in manifest["cases"]:
        if case["id"] in repeat_names:
            row = render_one(exe, case, output, "-repeat")
            primary = output / "pascal" / (case["id"] + ".png")
            secondary = output / "repeats" / (case["id"] + "-repeat.png")
            row["exact_rgb_repeat"] = row["status"] == "reference_ready" and bool((load_rgb(primary) == load_rgb(secondary)).all())
            report["repeats"].append(row)
    for left, right, expectation in [
        ("slice_0", "slice_2", "equal: middle normalization"),
        ("dof_disabled0", "dof_disabled2", "equal: bit0 disable gate"),
        ("dof_disabled0", "dof_disabled8", "equal: bit0 disable gate"),
        ("control", "dof_plain0", "report effect; equal does not prove unreachable"),
        ("control", "ao_type0", "report effect; equal does not prove unreachable"),
        ("control", "interpolate_25_75", "report effect; equal does not prove unreachable")]:
        a = output / "pascal" / ("stage_a_" + left + ".png")
        b = output / "pascal" / ("stage_a_" + right + ".png")
        if a.exists() and b.exists():
            pa, pb = load_rgb(a), load_rgb(b)
            report["influence"].append(dict(left=left, right=right, expectation=expectation,
                exact_rgb=bool((pa == pb).all()), changed_pixels=int((pa != pb).any(axis=2).sum())))
    report["finished"] = True
    report["qualification"] = "references_ready_with_separate_process_repeat_controls" if all(r["status"] == "reference_ready" for r in report["results"]) and all(r["exact_rgb_repeat"] for r in report["repeats"]) else "incomplete_or_nondeterministic"
    write_json(output / "summary.json", report)
    return report


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--output", type=Path, default=DEFAULT)
    ap.add_argument("--render-reference", action="store_true")
    ap.add_argument("--pascal-exe", type=Path, default=ROOT / "comparison/windows_build/Mandelbulb3D_batch_audit.exe")
    args = ap.parse_args()
    if (args.output / "summary.json").exists():
        raise ValueError("Reference output already exists; choose a fresh directory")
    manifest = generate(args.output)
    print("generated", len(manifest["cases"]), flush=True)
    if args.render_reference:
        report = render_references(args.output, manifest, args.pascal_exe)
        print(report["qualification"], report["counts"], flush=True)


if __name__ == "__main__":
    main()
