#!/usr/bin/env python3
"""Old/new frames for primary state-contract admission.

For each queue scene this runs frames with the same library, runtime
directory and arguments:

- baseline: the previous mb3d_render;
- candidate-N: the mb3d_render under test, repeated.

RGB is compared through decoded pixels. Profile fields are compared after
dropping wall-clock timings, the per-run output path and the pipeline-archive
hit/miss counts (cache state, not render work).

- Candidate repeats must match each other exactly.
- Baseline and candidate must have identical RGB.
- Their fields may differ only in declared records: the masks of a contract
  that compiler-derived effects now complete and, where the candidate admits
  the independent contract (reason 5), primary and shadow-row scheduling.
  RERUN_MATCH marks a conditional (dIFS) candidate that counted carried reads
  and therefore rendered its frame again in CPU order.

--control-library adds a control run: the baseline binary with a fresh-lane
library, whose persistent kernels reset the formula lane before every pixel
(primary, reflections) and every row (shadows). FP32_KERNEL_DIFFERENCE marks an admitted
candidate whose RGB differs from the baseline while the control reproduces the
baseline exactly: carried lane state cannot explain the difference, which then
comes from compiling the pixel kernels as separate functions. It is recorded
with its pixel count and largest channel difference and is never a match.

A frame that fails with a GPU fault is compared through its exact error
record across all runs. An admitted candidate observes pixel records in index
order, while the persistent baseline observes them per 32-pixel continuation
and lane. FAULT_ORDER_ONLY marks errors that differ only in the reported pixel,
with the candidate's index lower. A baseline timeout still runs the candidate, whose
repeats are compared with each other; a candidate timeout stops the scene.
--width/--height give a bounded preview, which never qualifies an original
frame. --reevaluate rebuilds the report from saved runs without rendering.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
SCHEMA = "mb3d.native-primary-admission.v1"
DIAGNOSTIC_SECONDS, STOP_SECONDS = 180, 300
STATE_PIXELS = 5
CONTRACT_RECORDS = {"metal_execution.primary_formula_reads", "metal_execution.primary_orbit_initialized",
                    "metal_execution.primary_caller_controls", "metal_execution.primary_driver_may_write",
                    "metal_execution.primary_scene_inputs", "metal_execution.primary_contained"}
# Pixel ownership; a conditional (dIFS) contract also marks the frame and frees
# shadow rows (WholeRow) from the persistent task order.
ADMISSION_RECORDS = {"metal_execution.primary_state_reason", "metal_execution.execution_plan",
                     "metal_execution.primary_dispatches", "metal_execution.primary_logical_tasks",
                     "metal_execution.difs_conditional_pixels", "metal_execution.shadow_logical_tasks",
                     "metal_execution.shadow_dispatches"}
# A conditional frame that counted carried dIFS reads is discarded and rendered
# again in CPU order; only the discarded attempt's counts may differ.
# A control run differs from the baseline only in the library it loads.
CONTROL_RECORDS = {"metal_library"}
RERUN_RECORDS = {"metal_execution.difs_rerun_carried_reads", "metal_execution.difs_rerun_carried_pixels"}


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def rgb_sha256(path):
    with Image.open(path) as image:
        return hashlib.sha256(image.convert("RGB").tobytes()).hexdigest()


def flatten(value, prefix=""):
    if isinstance(value, dict):
        out = {}
        for key, item in value.items():
            out.update(flatten(item, f"{prefix}.{key}" if prefix else key))
        return out
    return {prefix: value}


def work_fields(profile):
    """Profile without wall-clock timings or the per-run output path."""
    return {k: v for k, v in flatten(profile).items()
            if "seconds" not in k and k not in ("output", "metal_execution.pipeline_archive")}


def differences(a, b):
    return sorted(k for k in set(a) | set(b) if a.get(k) != b.get(k))


def run(executable, scene, output, args, library=None):
    output.mkdir(parents=True, exist_ok=False)
    image, profile = output / "image.png", output / "profile.json"
    size = ["--width", str(args.width), "--height", str(args.height)] if args.width else []
    argv = [str(executable), str(scene), "--backend", "metal", "--precision", "native", *size,
            "--threads", str(args.threads), "--deterministic", "--output", str(image), "--profile-json", str(profile),
            "--metal-library", str(library or args.library)]
    event = {"argv": argv, "cwd": str(args.runtime), "executable_sha256_before": sha256(executable)}
    begin = time.monotonic()
    try:
        process = subprocess.run(argv, cwd=args.runtime, capture_output=True, text=True, timeout=STOP_SECONDS)
        event.update(exit_code=process.returncode, timed_out=False)
        (output / "process.log").write_text(process.stdout + process.stderr)
    except subprocess.TimeoutExpired:
        event.update(exit_code=None, timed_out=True)
    event["seconds"] = round(time.monotonic() - begin, 3)
    event["over_diagnostic_threshold"] = event["seconds"] > DIAGNOSTIC_SECONDS
    event["executable_sha256_after"] = sha256(executable)
    if image.is_file():
        event.update(image=str(image), image_sha256=sha256(image), rgb_sha256=rgb_sha256(image))
    (output / "event.json").write_text(json.dumps(event, indent=1) + "\n")
    record = json.loads(profile.read_text()) if profile.is_file() else {}
    return event, record


def general_metal(profile):
    return (profile.get("backend") == "metal" and not profile.get("backend_fallback") and
            profile.get("metal_execution", {}).get("engine") == "general_formula")


def usable(event, profile):
    return event.get("exit_code") == 0 and "rgb_sha256" in event and general_metal(profile)


def gpu_fault(event, profile):
    """A GPU execution fault that published no image: exit 3 on the Metal general engine."""
    return event.get("exit_code") == 3 and "rgb_sha256" not in event and general_metal(profile)


FAULT_PIXEL = re.compile(r"^(.* at pixel )(\d+)( .*)$")


def fault_order_only(faults, candidates):
    """Errors equal except for an earlier first pixel in the admitted candidate."""
    if len({faults[label] for label in candidates}) != 1 or "baseline" not in faults:
        return False
    base, cand = FAULT_PIXEL.match(faults["baseline"]), FAULT_PIXEL.match(faults[candidates[0]])
    return bool(base and cand and base.group(1, 3) == cand.group(1, 3) and int(cand.group(2)) < int(base.group(2)))


def rgb_difference(a, b):
    """Differing pixels and the largest channel difference of two decoded images."""
    with Image.open(a) as left, Image.open(b) as right:
        x, y = left.convert("RGB").tobytes(), right.convert("RGB").tobytes()
    if len(x) != len(y):
        return None
    deltas = [abs(p - q) for p, q in zip(x, y)]
    return {"pixels": sum(1 for i in range(0, len(deltas), 3) if any(deltas[i:i + 3])), "max": max(deltas, default=0)}


def scene_result(rank, scene, args):
    out = args.output / f"{rank:02d}"
    runs = {}
    plan = [("baseline", args.baseline, None)]
    if args.control_library:
        plan.append(("control", args.baseline, args.control_library))
    plan += [(f"candidate-{n}", args.candidate, None) for n in range(1, args.repeats + 1)]
    for label, executable, library in plan:
        if label == "control" and runs["baseline"]["event"].get("timed_out"):
            continue
        event, profile = run(executable, scene, out / label, args, library)
        runs[label] = {"event": event, "profile": profile}
        if label in ("baseline", "control") and event.get("timed_out"):
            continue
        if not (usable(event, profile) or gpu_fault(event, profile)):
            break
    return evaluate(rank, scene.relative_to(ROOT).as_posix(), runs)


def load_runs(directory):
    runs = {}
    for label in ["baseline", "control", *sorted(p.name for p in directory.glob("candidate-*"))]:
        if (directory / label / "event.json").is_file():
            profile = directory / label / "profile.json"
            runs[label] = {"event": json.loads((directory / label / "event.json").read_text()),
                           "profile": json.loads(profile.read_text()) if profile.is_file() else {}}
    return runs


def evaluate(rank, scene, runs):
    candidates = [label for label in runs if label.startswith("candidate-")]
    base = {"rank": rank, "scene": scene, "seconds": {label: r["event"]["seconds"] for label, r in runs.items()},
            "over_diagnostic_threshold": [label for label, r in runs.items() if r["event"]["over_diagnostic_threshold"]]}
    if any(runs[label]["event"].get("timed_out") for label in candidates):
        return {**base, "status": "TIMED_OUT", "runs": list(runs)}
    baseline_timed_out = runs.get("baseline", {}).get("event", {}).get("timed_out", False)
    finished = {label: r for label, r in runs.items() if not r["event"].get("timed_out")}
    failed = [label for label, r in finished.items() if not (usable(r["event"], r["profile"]) or
                                                             gpu_fault(r["event"], r["profile"]))]
    if not candidates or failed:
        label = failed[0] if failed else "baseline"
        profile = runs[label]["profile"]
        return {**base, "status": "RUN_FAILED", "failed": label, "exit_code": runs[label]["event"].get("exit_code"),
                "error": profile.get("error") or profile.get("backend_fallback_reason")}
    reason = {label: r["profile"]["metal_execution"].get("primary_state_reason") for label, r in finished.items()}
    admitted = all(reason[label] == STATE_PIXELS for label in candidates)
    faults = {label: r["profile"].get("error") for label, r in finished.items() if gpu_fault(r["event"], r["profile"])}
    if faults:
        complete = len(faults) == len(finished)
        status = ("FAULTS_MATCH" if complete and len(set(faults.values())) == 1 else
                  "FAULT_ORDER_ONLY" if complete and admitted and fault_order_only(faults, candidates) else
                  "DIFFERENT_FAULTS")
        return {**base, "status": status, "admitted": admitted,
                "baseline_timed_out": baseline_timed_out, "reasons": reason, "errors": faults}
    fields = {label: work_fields(r["profile"]) for label, r in finished.items()}
    rgb = {label: r["event"]["rgb_sha256"] for label, r in finished.items()}
    first = candidates[0]
    repeat_diff = sorted({k for label in candidates[1:] for k in differences(fields[first], fields[label])})
    repeats_exact = not repeat_diff and len({rgb[label] for label in candidates}) == 1
    result = {**base, "admitted": admitted, "reasons": reason, "rgb_sha256": rgb, "repeat_differences": repeat_diff,
              "size": [fields[first].get("width"), fields[first].get("height")],
              "primary_seconds": {label: r["profile"].get("stages_seconds", {}).get("primary")
                                  for label, r in finished.items()}}
    if baseline_timed_out:
        return {**result, "status": "BASELINE_TIMED_OUT", "checks": {"candidate_repeats_exact": repeats_exact}}
    diff = differences(fields["baseline"], fields[first])
    rerun = all(fields[label].get("metal_execution.difs_rerun_carried_reads") for label in candidates)
    declared = CONTRACT_RECORDS | (ADMISSION_RECORDS if admitted else set()) | (RERUN_RECORDS if rerun else set())
    checks = {"baseline_rgb_equal": rgb["baseline"] == rgb[first],
              "undeclared_fields": [k for k in diff if k not in declared],
              "candidate_repeats_exact": repeats_exact}
    if "control" in fields:
        checks["control_rgb_equal"] = rgb["control"] == rgb["baseline"]
        checks["control_undeclared_fields"] = [k for k in differences(fields["baseline"], fields["control"])
                                               if k not in CONTROL_RECORDS]
    match = checks["baseline_rgb_equal"] and not checks["undeclared_fields"] and repeats_exact
    kernel_difference = (not match and admitted and repeats_exact and checks.get("control_rgb_equal") and
                         not checks["control_undeclared_fields"])
    status = (("ADMITTED_MATCH" if admitted else "RERUN_MATCH" if rerun else "UNCHANGED_MATCH") if match else
              "FP32_KERNEL_DIFFERENCE" if kernel_difference else "DIFFERENT")
    images = [runs[label]["event"].get("image") for label in ("baseline", first)]
    if not checks["baseline_rgb_equal"] and all(images) and all(Path(i).is_file() for i in images):
        result["rgb_difference"] = rgb_difference(*images)
    return {**result, "status": status, "checks": checks,
            "baseline_vs_candidate": {k: [fields["baseline"].get(k), fields[first].get(k)] for k in diff}}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline", type=Path, required=True, help="previous mb3d_render")
    parser.add_argument("--candidate", type=Path, required=True, help="mb3d_render under test")
    parser.add_argument("--library", type=Path, required=True)
    parser.add_argument("--runtime", type=Path, required=True, help="working directory holding M3Maps")
    parser.add_argument("--control-library", type=Path, help="fresh-lane library for a baseline control run")
    parser.add_argument("--queue", type=Path, default=ROOT / "comparison/scene-queue.json")
    parser.add_argument("--rank", type=int, action="append", default=[])
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument("--threads", type=int, default=6)
    parser.add_argument("--width", type=int, default=0, help="Bounded preview width (default: original size)")
    parser.add_argument("--height", type=int, default=0)
    parser.add_argument("--output", type=Path, required=True, help="New output directory, or the one to reevaluate")
    parser.add_argument("--reevaluate", action="store_true", help="Rebuild admission.json from saved runs")
    args = parser.parse_args()
    if bool(args.width) != bool(args.height):
        raise SystemExit("--width and --height go together")
    if not args.reevaluate:
        args.output.mkdir(parents=True, exist_ok=False)
    for name in ("baseline", "candidate", "library", "runtime", "output"):
        setattr(args, name, getattr(args, name).resolve())
    if args.control_library:
        args.control_library = args.control_library.resolve()
    rows = {row["rank"]: row for row in json.loads(args.queue.read_text())["rows"]}
    results = []
    if args.reevaluate:
        for directory in sorted(p for p in args.output.iterdir() if p.is_dir()):
            rank = int(directory.name)
            results.append(evaluate(rank, rows[rank]["scene"], load_runs(directory)))
    for rank in args.rank if not args.reevaluate else ():
        results.append(scene_result(rank, ROOT / rows[rank]["scene"], args))
        print(json.dumps({k: results[-1][k] for k in ("rank", "status")}), flush=True)
    report = {
        "schema": SCHEMA, "status": "PREVIEW_CONTROL" if args.width else "ORIGINAL_FRAMES",
        "scope": "One device and library; old/new binaries with identical arguments.",
        "baseline_sha256": sha256(args.baseline), "candidate_sha256": sha256(args.candidate),
        "library_sha256": sha256(args.library),
        "control_library_sha256": sha256(args.control_library) if args.control_library else None,
        "tool_sha256": sha256(Path(__file__)), "queue_sha256": sha256(args.queue),
        "threads": args.threads, "repeats": args.repeats, "preview_size": [args.width, args.height] if args.width else None,
        "reevaluated": args.reevaluate, "results": results,
    }
    (args.output / "admission.json").write_text(json.dumps(report, indent=1, sort_keys=True) + "\n")


if __name__ == "__main__":
    main()
