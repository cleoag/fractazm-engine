"""Check the modified Pascal/C++ hang protections, separately from original parity.

Never run the original executable on nonterminating scenes. Expected failures
are behavioral checks, not new original-reference images or numeric PASS rows.
Finite controls reuse already hashed original Pascal images without rerunning it.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import time

from PIL import Image

from external_validation import ROOT, decode, write_json


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read_checked(path, expected):
    if sha(path) != expected:
        raise ValueError(f"Recorded input changed: {path}")
    return path


def rgb(path):
    with Image.open(path) as image:
        return image.size, image.convert("RGB").tobytes()


def run(exe, engine, case, threads, output, timeout):
    label = f"{engine}_t{threads}_{case['id']}"
    image = output / (label + ".png")
    if image.exists():
        raise ValueError(f"Refusing to overwrite prior output: {image}")
    scene = read_checked(Path(case["scene"]), case["sha256"])
    if engine == "pascal":
        command = [str(exe), "--render", str(scene), "--output", str(image),
                   "--threads", str(threads)]
    else:
        command = [str(exe), str(scene), "--output", str(image),
                   "--threads", str(threads)]
    # A parent process's prior diagnostic/batch/MC request must not alter this
    # ordinary isolated-process check. Assets still resolve beside each binary.
    env = {k: v for k, v in os.environ.items()
           if not k.upper().startswith(("MB3D_HEADLESS_", "MB3D_POINT_", "MB3D_PARITY_"))}
    start = time.monotonic()
    row = dict(case=case["id"], engine=engine, threads=threads, command=command)
    try:
        result = subprocess.run(command, cwd=ROOT, env=env, capture_output=True,
                                text=True, errors="replace", timeout=timeout)
        row["returncode"] = result.returncode
        for stream in ("stdout", "stderr"):
            log = output / (label + "." + stream + ".txt")
            log.write_text(getattr(result, stream), encoding="utf-8")
            row[stream + "_sha256"] = sha(log)
        if case["expect"] == "rejected":
            diagnostic = result.stdout + "\n" + result.stderr
            expected = case["diagnostics"][engine]
            row.update(contract_pass=(result.returncode in case["exit_codes"][engine]
                                      and any(s in diagnostic for s in expected)
                                      and all(s in diagnostic for s in
                                              case.get("required_diagnostics", {}).get(engine, []))
                                      and not image.exists()),
                       expected="explicit_failure_without_image",
                       parity_status=case.get("original_status", "unverified_original_nontermination"),
                       output_exists=image.exists())
        else:
            good = result.returncode == 0 and image.is_file()
            exact = good and rgb(image) == rgb(Path(case["reference_image"]))
            row.update(contract_pass=exact, expected="exact_original_control_rgb",
                       exact_original_rgb=exact)
            if good:
                row["image_sha256"] = sha(image)
    except subprocess.TimeoutExpired as error:
        row.update(contract_pass=False, error="timeout", output_exists=image.exists())
        for stream in ("stdout", "stderr"):
            value = getattr(error, stream) or b""
            log = output / (label + "." + stream + ".txt")
            log.write_bytes(value if isinstance(value, bytes) else value.encode("utf-8"))
            row[stream + "_sha256"] = sha(log)
    row["seconds"] = round(time.monotonic() - start, 4)
    return row


def prepare(output):
    inventory = ROOT / "comparison/results/stage-a-20261018/final-candidate-v20/formula-status.json"
    finite_manifest = ROOT / "comparison/runs/external-complete/conditional-finite-scenes-v1/manifest.json"
    finite_summary = ROOT / "comparison/runs/external-complete/conditional-finite-images-v1/summary.json"
    status = json.loads(inventory.read_text())
    cases = []
    for formula in status["formulas"]:
        for case in formula["cases"]:
            if case["image_status"] == "pass":
                continue
            if formula["formula"] != "TrifoxComplexAngles" and not formula["formula"].startswith("_Cond"):
                continue
            scene = ROOT / Path(case["scene"].replace("\\", "/"))
            read_checked(scene, case["scene_sha256"])
            trifox = formula["formula"] == "TrifoxComplexAngles"
            cases.append(dict(id=case["case"], formula=formula["formula"],
                              scene=str(scene), sha256=case["scene_sha256"],
                              expect="rejected", exit_codes={"cpp": [3], "pascal": [1]},
                              required_diagnostics={"pascal": ["EFormulaExecutionError"]},
                              diagnostics={"cpp": ["FORMULA_INPUT_REJECTED[nonterminating_zero_power]"
                                                    if trifox else "Formula iteration failed:"],
                                           "pascal": ["formula code guard: formula=TrifoxComplexAngles reason=zero_integer_power"
                                                      if trifox else "formula iteration guard:"]}))
    if len(cases) != 24:
        raise ValueError("Expected the 22 original conditional and two zero-power cases")
    # Unconditional long formula: exercise the worker failure path in both apps.
    source = ROOT / "comparison/runs/external-complete/challenges-v4/challenge_Knots2IFS_early.m3p"
    data = decode(source)
    struct.pack_into("<d", data, 848 + 60 + 11 * 8, 2147483647.)
    generated = output / "knots_internal_work_exhausted.m3p"
    generated.write_bytes(data)
    cases.append(dict(id="knots_internal_work_exhausted", formula="Knots2IFS",
                      scene=str(generated), sha256=sha(generated),
                      source=str(source), source_sha256=sha(source),
                      change="stored option 11 = INT_MAX", expect="rejected",
                      original_status="unverified_original_excessive_work",
                      exit_codes={"cpp": [3], "pascal": [1]},
                      required_diagnostics={"pascal": ["EFormulaExecutionError"]},
                      diagnostics={"cpp": ["Formula work budget exhausted"],
                                   "pascal": ["formula code guard: formula=Knots2IFS reason=body_work_budget"]}))
    recorded = json.loads(finite_summary.read_text())
    records = {r["scene"]: r for r in recorded["results"]}
    controls = []
    for case in json.loads(finite_manifest.read_text())["cases"]:
        row = records[case["id"]]
        if row["status"] != "pass" or not row["exact_rgb"] or row["scene_sha256"] != case["sha256"]:
            raise ValueError("Invalid finite original control: " + case["id"])
        scene = read_checked(ROOT / Path(case["scene"].replace("\\", "/")), case["sha256"])
        original = read_checked(finite_summary.parent / "pascal" / (case["id"] + ".png"),
                                row["pascal_image_sha256"])
        controls.append(dict(id=case["id"], scene=str(scene), sha256=case["sha256"],
                             formula=case["target_formulas"][0], expect="rendered",
                             reference_image=str(original), reference_sha256=sha(original)))
    return cases, controls, {str(p): sha(p) for p in (inventory, finite_manifest, finite_summary)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cpp", type=Path, required=True)
    parser.add_argument("--patched-pascal", type=Path, required=True,
                        help="New guarded build ONLY; the original executable is never used here")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--timeout", type=float, default=20.)
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    summary = output / "summary.json"
    if summary.exists():
        parser.error("Use a new output directory")
    executables = {"cpp": args.cpp.resolve(), "pascal": args.patched_pascal.resolve()}
    cases, controls, sources = prepare(output)
    hashes = {k: sha(v) for k, v in executables.items()}
    report = dict(scope=__doc__, executables={k: str(v) for k, v in executables.items()},
                  binary_sha256=hashes, tool_sha256=sha(Path(__file__)),
                  source_sha256=sources, cases=cases, controls=controls, results=[],
                  finished=False, success=False)
    write_json(summary, report)
    # Every known hang and every finite control once. Add one representative of
    # each affected formula with multiple workers, avoiding redundant large sweeps.
    representatives = list({case["formula"]: case for case in reversed(cases)}.values())
    schedule = [(c, 1) for c in cases + controls] + [(c, 4) for c in representatives]
    try:
        for engine, exe in executables.items():
            for case, threads in schedule:
                row = run(exe, engine, case, threads, output, args.timeout)
                report["results"].append(row)
                write_json(summary, report)
                print(engine, threads, case["id"], "OK" if row["contract_pass"] else "FAIL", flush=True)
        if any(sha(executables[k]) != v for k, v in hashes.items()):
            raise ValueError("Executable changed during validation")
        for case in cases + controls:
            read_checked(Path(case["scene"]), case["sha256"])
        for case in controls:
            read_checked(Path(case["reference_image"]), case["reference_sha256"])
        report["success"] = all(r["contract_pass"] for r in report["results"])
        report["finished"] = True
        report["checks"] = len(report["results"])
        report["passed"] = sum(r["contract_pass"] for r in report["results"])
    finally:
        write_json(summary, report)
    return 0 if report["success"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
