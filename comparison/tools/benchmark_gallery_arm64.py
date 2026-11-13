"""Sequential native ARM64 scene profiling; never a Pascal parity qualification.

Each invocation starts a new process. Warmups can warm filesystem caches, not a
persistent renderer. PNGs, inputs and evidence are retained in a NEW directory.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
import math
import os
from pathlib import Path, PurePosixPath
import platform
import re
import shutil
import signal
import statistics
import subprocess
import sys
import time

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))
from native_evidence import source_identity

SCHEMA = "mb3d.gallery-cpu-benchmark.v1"
SELECTION_SCHEMA = "mb3d.gallery-benchmark-selection.v1"
RGB_LAYOUT = "row-major top-to-bottom interleaved RGB8"
STAGES = ("load", "preparation", "volume", "primary", "normals", "shadows",
          "ao", "color", "reflections", "dof", "final_surface", "save")
RENDER_STAGES = STAGES[1:-2]


def sha(path):
    with Path(path).open("rb") as stream:
        digest = hashlib.file_digest(stream, "sha256")
    return digest.hexdigest()


def utc_now():
    return datetime.now(timezone.utc).isoformat()


def write_json(path, value):
    temporary = path.with_name(path.name + ".tmp")
    temporary.write_text(json.dumps(value, indent=2, allow_nan=False) + "\n")
    temporary.replace(path)


def relative_file(root, value):
    """Reject absolute paths, traversal and symlinks escaping the source root."""
    if not isinstance(value, str) or "\\" in value:
        raise ValueError("Expected a repository-relative POSIX path")
    path = PurePosixPath(value)
    if path.is_absolute() or ".." in path.parts or not path.parts:
        raise ValueError("Unsafe relative path: " + value)
    resolved = (root / value).resolve()
    if not resolved.is_relative_to(root.resolve()) or not resolved.is_file():
        raise ValueError("Missing or escaping input: " + value)
    return resolved


def load_selection(path, root=ROOT, selected_ids=()):
    value = json.loads(path.read_text())
    if value.get("schema") != SELECTION_SCHEMA:
        raise ValueError("Unsupported scene-selection schema")
    scenes = value.get("scenes")
    if not isinstance(scenes, list) or not scenes or value.get("expected_count") != len(scenes):
        raise ValueError("Scene count must match a nonempty scenes list")
    seen = set()
    for row in scenes:
        scene_id = row.get("id", "")
        if not re.fullmatch(r"[A-Za-z0-9_-]+", scene_id) or scene_id in seen:
            raise ValueError("Invalid or duplicate scene ID: " + scene_id)
        if "jit" in scene_id.lower() or "montecarlo" in scene_id.lower():
            raise ValueError("Deferred scene is excluded: " + scene_id)
        seen.add(scene_id)
        scene = relative_file(root, row.get("scene_path"))
        if scene.suffix.lower() != ".m3p" or sha(scene) != row.get("scene_sha256"):
            raise ValueError("Scene type or SHA-256 mismatch: " + scene_id)
    requested = set(selected_ids)
    if len(requested) != len(selected_ids) or requested - seen:
        raise ValueError("Requested scene IDs must be unique and present in the selection")
    assets = value.get("assets", [])
    if not isinstance(assets, list):
        raise ValueError("assets must be a list")
    destinations = set()
    for asset in assets:
        source = relative_file(root, asset.get("source_path"))
        target = asset.get("runtime_path", "")
        parts = PurePosixPath(target)
        if (not target.startswith("M3Maps/") or ".." in parts.parts or "\\" in target
                or target in destinations or sha(source) != asset.get("sha256")):
            raise ValueError("Invalid asset path/hash")
        destinations.add(target)
    result = dict(value)
    result["scenes"] = [r for r in scenes if not requested or r["id"] in requested]
    result["expected_count"] = len(result["scenes"])
    result["full_selection_count"] = len(scenes)
    return result


def finite_seconds(value, name):
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value < 0:
        raise ValueError("Invalid duration: " + name)
    return float(value)


def validate_profile(path, width, height, workers):
    value = json.loads(path.read_text())
    if (value.get("schema") != "mb3d.cpu-stage-profile.v1" or value.get("schema_version") != 1
            or value.get("status") != "complete" or value.get("exit_code") != 0
            or value.get("render_status") != "completed"):
        raise ValueError("Profile must identify a successful schema_version 1 command")
    if value.get("precision") != "compatible":
        raise ValueError("Exact prior-RGB benchmark requires compatible precision")
    if (value.get("width"), value.get("height"), value.get("workers")) != (width, height, workers):
        raise ValueError("Profile dimensions/worker count do not match the command")
    stages = value.get("stages")
    if not isinstance(stages, dict) or set(stages) != set(STAGES):
        raise ValueError("Profile must list every declared stage exactly once")
    actual = {}
    for name, row in stages.items():
        seconds = finite_seconds(row.get("seconds"), name)
        state = row.get("state")
        if state not in ("completed", "not_run") or (state == "not_run" and seconds != 0):
            raise ValueError("Incomplete or inconsistent successful stage: " + name)
        if state == "completed":
            actual[name] = seconds
    if any(name not in actual for name in ("load", "preparation", "primary", "color", "final_surface", "save")):
        raise ValueError("Successful full render lacks a required stage")
    if value.get("stages_seconds") != actual:
        raise ValueError("Profile stages_seconds disagrees with stage states")
    finite_seconds(value.get("render_total_seconds"), "render_total_seconds")
    # CLI schema records command time separately from JSON serialization.
    command_total = value.get("command_total_seconds", value.get("wall_seconds"))
    finite_seconds(command_total, "command_total_seconds")
    return value


def inspect_png(path, width, height):
    with Image.open(path) as image:
        if image.format != "PNG" or image.size != (width, height):
            raise ValueError("Expected PNG with requested dimensions")
        image.verify()
    with Image.open(path) as image:
        rgb = image.convert("RGB").tobytes()
    return {"width": width, "height": height, "image_sha256": sha(path),
            "rgb_sha256": hashlib.sha256(rgb).hexdigest(), "rgb_hash_layout": RGB_LAYOUT}


def execute(command, cwd, log, timeout, env):
    """Reap exactly this child with wait4: independent CPU time and peak RSS."""
    started = time.perf_counter()
    timed_out = False
    with log.open("xb") as stream:
        process = subprocess.Popen(command, cwd=cwd, stdout=stream, stderr=subprocess.STDOUT,
                                   env=env, start_new_session=True)
        def stop(sig):
            try:
                os.killpg(process.pid, sig)
            except ProcessLookupError:
                pass  # The child may exit after the nonblocking wait.
        try:
            while True:
                child, status, usage = os.wait4(process.pid, os.WNOHANG)
                if child:
                    break
                elapsed = time.perf_counter() - started
                if not timed_out and elapsed >= timeout:
                    timed_out = True
                    stop(signal.SIGTERM)
                elif timed_out and elapsed >= timeout + 5:
                    stop(signal.SIGKILL)
                    _, status, usage = os.wait4(process.pid, 0)
                    break
                time.sleep(0.01)
        except BaseException:
            stop(signal.SIGKILL)
            _, status, _ = os.wait4(process.pid, 0)
            process.returncode = os.waitstatus_to_exitcode(status)
            raise
        process.returncode = os.waitstatus_to_exitcode(status)
    return {"returncode": process.returncode, "timed_out": timed_out,
            "wall_seconds": time.perf_counter() - started,
            "cpu_user_seconds": usage.ru_utime, "cpu_system_seconds": usage.ru_stime,
            "cpu_seconds": usage.ru_utime + usage.ru_stime,
            "peak_rss_bytes": int(usage.ru_maxrss * (1 if sys.platform == "darwin" else 1024)),
            "usage_method": "wait4 rusage for this child; Darwin ru_maxrss is bytes"}


def summarize_runs(runs, repeats):
    measured = [r for r in runs if r["kind"] == "measured"]
    all_valid = bool(runs) and all(r["status"] == "rendered" for r in runs)
    hashes = {r.get("rgb_sha256") for r in runs if r["status"] == "rendered"}
    exact = all_valid and len(hashes) == 1
    result = {"completed_measured": sum(r["status"] == "rendered" for r in measured),
              "expected_measured": repeats,
              "repeat_rgb": ("not_checked_single_sample" if exact and len(runs) == 1 else "exact" if exact else "failed"),
              "sample_kind": "single_observation" if repeats == 1 else "repeated_measurement"}
    if not exact or len(measured) != repeats:
        result["status"] = "failed"
        return result
    result.update(status="complete", rgb_sha256=next(iter(hashes)))
    keys = ("wall_seconds", "cpu_seconds", "cpu_user_seconds", "cpu_system_seconds", "peak_rss_bytes")
    statistic = "observation" if repeats == 1 else "median"
    result[statistic] = {key: statistics.median(r[key] for r in measured) for key in keys}
    result[statistic]["render_total_seconds"] = statistics.median(r["profile"]["render_total_seconds"] for r in measured)
    # Absent stages are NOT zero-valued observations; report not_run explicitly.
    result["stage_statistics"] = {}
    for stage in STAGES:
        samples = [r["profile"]["stages"][stage] for r in measured]
        states = {r["state"] for r in samples}
        if len(states) != 1:
            result["status"] = "failed"
            result["error"] = "Stage activation changed between repeats: " + stage
        state = next(iter(states)) if len(states) == 1 else "inconsistent"
        result["stage_statistics"][stage] = {"state": state, "samples": len(samples)}
        if state == "completed":
            result["stage_statistics"][stage][statistic + "_seconds"] = statistics.median(r["seconds"] for r in samples)
    return result


def aggregate_scenes(rows, repeats):
    statistic = "observation" if repeats == 1 else "median"
    good = [r for r in rows if r.get("summary", {}).get("status") == "complete"]
    totals = {stage: sum(row["summary"]["stage_statistics"][stage].get(statistic + "_seconds", 0)
                         for row in good) for stage in STAGES}
    denominator = sum(totals.values())
    render_denominator = sum(totals[s] for s in RENDER_STAGES)
    return {"included_scenes": len(good), "scene_statistic": statistic,
            "scope": "sum of per-scene " + statistic + "s; fractions over timed non-overlapping stages, not total wall time",
            "stage_seconds": totals,
            "stage_fraction": {k: v / denominator if denominator else 0 for k, v in totals.items()},
            "render_stage_fraction": {k: totals[k] / render_denominator if render_denominator else 0 for k in RENDER_STAGES},
            "wall_seconds": sum(r["summary"][statistic]["wall_seconds"] for r in good),
            "render_total_seconds": sum(r["summary"][statistic]["render_total_seconds"] for r in good)}


def native_host():
    if platform.system() != "Darwin" or platform.machine() != "arm64":
        raise ValueError("Requires native Apple Silicon Python; Rosetta and Windows are excluded")
    translated = subprocess.run(["sysctl", "-in", "sysctl.proc_translated"], capture_output=True, text=True)
    if translated.stdout.strip() == "1":
        raise ValueError("Translated execution is excluded")
    def sysctl(key):
        return subprocess.check_output(["sysctl", "-n", key], text=True).strip()
    return {"os": platform.platform(), "machine": platform.machine(), "translated": False,
            "python": sys.version, "cpu": sysctl("machdep.cpu.brand_string"),
            "logical_cpus": int(sysctl("hw.logicalcpu")), "memory_bytes": int(sysctl("hw.memsize"))}


def binary_identity(path):
    arch = subprocess.check_output(["lipo", "-archs", str(path)], text=True).strip()
    if arch != "arm64":
        raise ValueError("Renderer must contain only arm64: " + arch)
    return {"path": str(path), "sha256": sha(path), "binary_arch": arch}


def load_reference(path, selection, width, height, threads):
    value = json.loads(path.read_text())
    if value.get("schema") != SCHEMA or value.get("status") != "COMPLETE":
        raise ValueError("Reference must be a complete CPU benchmark report, not Pascal/historical gallery evidence")
    config = value.get("settings", {})
    if (config.get("width"), config.get("height"), config.get("threads")) != (width, height, threads):
        raise ValueError("Reference dimensions/threads differ")
    by_id = {r["id"]: r for r in value["results"]}
    for row in selection["scenes"]:
        reference = by_id.get(row["id"], {})
        if (reference.get("scene_sha256") != row["scene_sha256"]
                or reference.get("summary", {}).get("status") != "complete"):
            raise ValueError("Reference missing a matching completed scene: " + row["id"])
    return value, by_id


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path, help="NEW retained deliverable directory; never overwritten")
    parser.add_argument("--selection", type=Path, default=ROOT / "comparison/benchmark_scenes/gallery32.json")
    parser.add_argument("--scene", action="append", default=[], help="Exact selection ID; repeat to choose a subset")
    parser.add_argument("--width", type=int, default=1024)
    parser.add_argument("--height", type=int, default=768)
    parser.add_argument("--threads", type=int, default=6)
    parser.add_argument("--warmups", type=int, default=1)
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument("--timeout", type=float, default=900)
    reference_group = parser.add_mutually_exclusive_group()
    reference_group.add_argument("--baseline-renderer", type=Path, help="Render one unprofiled CPU RGB control per scene")
    reference_group.add_argument("--reference", type=Path, help="Existing complete report with same resolution/threads")
    parser.add_argument("--bootstrap", type=Path, help="Retain current build/test report as provenance; no historical PASS is inherited")
    args = parser.parse_args(argv)
    if min(args.width, args.height, args.threads, args.repeats) < 1 or args.warmups < 0 or not math.isfinite(args.timeout) or args.timeout <= 0:
        parser.error("Dimensions, threads, repeats and timeout must be positive; warmups nonnegative")
    try:
        selection = load_selection(args.selection.resolve(), selected_ids=args.scene)
        host = native_host()
        renderer = args.renderer.resolve(strict=True)
        candidate = binary_identity(renderer)
        baseline = binary_identity(args.baseline_renderer.resolve(strict=True)) if args.baseline_renderer else None
        reference, reference_rows = (load_reference(args.reference, selection, args.width, args.height, args.threads)
                                     if args.reference else (None, {}))
        if args.bootstrap and not args.bootstrap.is_file():
            raise ValueError("Bootstrap report is missing")
        output = args.output.expanduser().resolve()
        output.mkdir(parents=True, exist_ok=False)
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        parser.error(str(error))

    for directory in ("inputs/scenes", "evidence/bin", "evidence/source", "runtime", "images", "logs", "profiles"):
        (output / directory).mkdir(parents=True)
    (output / "KEEP_FINAL_IMAGES.txt").write_text("Retained benchmark images and evidence. Routine build/log cleanup must not delete this directory. Explicit user deletion request required.\n")
    shutil.copy2(renderer, output / "evidence/bin/candidate")
    candidate["retained_path"] = "evidence/bin/candidate"
    if baseline:
        shutil.copy2(args.baseline_renderer, output / "evidence/bin/baseline")
        baseline["retained_path"] = "evidence/bin/baseline"
    for identity, original in ((candidate, renderer), (baseline, args.baseline_renderer)):
        if identity:
            if sha(output / identity["retained_path"]) != identity["sha256"]:
                raise ValueError("Executable changed while freezing it")
            cache = original.resolve().parent / "CMakeCache.txt"
            if cache.exists():
                target = "evidence/" + ("candidate" if identity is candidate else "baseline") + "-CMakeCache.txt"
                shutil.copy2(cache, output / target)
                identity["cmake_cache"] = {"path": target, "sha256": sha(cache)}
    source = source_identity()
    (output / "evidence/source/working-tree.patch").write_bytes(subprocess.check_output(["git", "diff", "HEAD", "--binary"], cwd=ROOT))
    for name, digest in source["changed_source_sha256"].items():
        if digest is not None:
            destination = output / "evidence/source" / name
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(ROOT / name, destination)
    if args.bootstrap:
        shutil.copy2(args.bootstrap, output / "evidence/bootstrap.json")
    if reference:
        shutil.copy2(args.reference, output / "evidence/reference-summary.json")
    shutil.copy2(args.selection, output / "inputs/full-selection.json")
    write_json(output / "inputs/selection.json", selection)
    for row in selection["scenes"]:
        destination = output / "inputs/scenes" / (row["id"] + ".m3p")
        shutil.copy2(ROOT / row["scene_path"], destination)
        if sha(destination) != row["scene_sha256"]:
            raise ValueError("Scene changed while freezing it: " + row["id"])
    for asset in selection.get("assets", []):
        destination = output / "runtime" / asset["runtime_path"]
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(ROOT / asset["source_path"], destination)
        if sha(destination) != asset["sha256"]:
            raise ValueError("Asset changed while freezing it")

    env = {k: v for k, v in os.environ.items() if not k.upper().startswith("MB3D_")}
    report = {"schema": SCHEMA, "status": "RUNNING", "started_utc": utc_now(),
              "scope": "Native C++ timing and exact CPU RGB consistency; no Pascal or GPU numerical qualification",
              "host": host, "candidate": candidate, "baseline": baseline,
              "source_observation": source, "source_observation_note": "Checkout observation, not proof of binary build identity; retained bootstrap may provide that binding",
              "settings": {"width": args.width, "height": args.height, "threads": args.threads,
                           "warmups": args.warmups, "repeats": args.repeats, "timeout_seconds": args.timeout,
                           "concurrent_processes": 1, "deterministic": True, "work_budget": None},
              "precision_requested": "compatible",
              "timing_scope": "Each sample is a NEW process including startup/load/render/save. Warmups only warm OS caches; no persistent-process warm timing. No builds/tests should overlap measurements.",
              "stage_scope": "Primary includes formulas, inline normals and dynamic fog; normals is the optional Z-buffer pass. No formula-level hot-path clocks.",
              "rgb_hash_layout": RGB_LAYOUT, "selected_count": len(selection["scenes"]),
              "full_selection_count": selection["full_selection_count"], "selection_sha256": sha(output / "inputs/selection.json"),
              "harness_sha256": sha(Path(__file__)), "bootstrap_sha256": sha(args.bootstrap) if args.bootstrap else None,
              "reference_sha256": sha(args.reference) if args.reference else None,
              "removed_environment_names": sorted(k for k in os.environ if k.upper().startswith("MB3D_")), "results": []}
    shutil.copy2(__file__, output / "evidence/benchmark_gallery_arm64.py")
    write_json(output / "summary.json", report)

    def run(row, kind, index, identity, profile_enabled):
        stem = f"{row['id']}-{kind}-{index:02}"
        image = output / "images" / (stem + ".png")
        log = output / "logs" / (stem + ".log")
        profile = output / "profiles" / (stem + ".json")
        command = [str(output / identity["retained_path"]), str(output / "inputs/scenes" / (row["id"] + ".m3p")),
                   "--width", str(args.width), "--height", str(args.height), "--threads", str(args.threads),
                   "--deterministic", "--precision", "compatible", "--format", "png", "--output", str(image)]
        if profile_enabled:
            command += ["--profile-json", str(profile)]
        result = {"kind": kind, "index": index, "started_utc": utc_now(), "command": command,
                  "cwd": str(output / "runtime"), "log": str(log.relative_to(output)),
                  "image": str(image.relative_to(output)), "status": "running"}
        if kind == "baseline":
            row["baseline"] = result
        else:
            row["runs"].append(result)
        # Preserve the command and in-flight status even on interruption.
        write_json(output / "summary.json", report)
        print(f"{kind} {index}: {row['id']}", flush=True)
        try:
            result.update(execute(command, output / "runtime", log, args.timeout, env))
        except BaseException as error:
            result.update(status="interrupted" if isinstance(error, KeyboardInterrupt) else "runner_error",
                          error=str(error), finished_utc=utc_now())
            if log.exists():
                result["log_sha256"] = sha(log)
            raise
        result["finished_utc"] = utc_now()
        result["log_sha256"] = sha(log)
        if result["timed_out"]:
            result["status"] = "timeout"
        elif result["returncode"] != 0:
            result["status"] = {1: "argument_error", 2: "load_error", 3: "render_error", 4: "write_error",
                                5: "cancelled", 6: "work_exhausted"}.get(result["returncode"], "process_error")
        else:
            try:
                result.update(inspect_png(image, args.width, args.height))
                if profile_enabled:
                    result["profile"] = validate_profile(profile, args.width, args.height, args.threads)
                result["status"] = "rendered"
            except (OSError, ValueError, KeyError, SyntaxError) as error:
                result.update(status="invalid_output", error=str(error))
        if profile.exists():
            result.update(profile_path=str(profile.relative_to(output)), profile_sha256=sha(profile))
        print(f"  {result['status']} {result['wall_seconds']:.3f}s", flush=True)
        return result

    try:
        for scene in selection["scenes"]:
            row = {**scene, "runs": []}
            report["results"].append(row)
            reference_hash = None
            if baseline:
                row["baseline"] = run(row, "baseline", 0, baseline, False)
                reference_hash = row["baseline"].get("rgb_sha256")
            elif reference:
                reference_hash = reference_rows[row["id"]]["summary"]["rgb_sha256"]
            for index in range(args.warmups + args.repeats):
                kind = "warmup" if index < args.warmups else "measured"
                run(row, kind, index if kind == "warmup" else index - args.warmups, candidate, True)
                write_json(output / "summary.json", report)
                if row["runs"][-1]["status"] != "rendered":
                    break
            row["summary"] = summarize_runs(row["runs"], args.repeats)
            row["summary"]["reference_rgb"] = ("not_requested" if not baseline and not reference else
                "exact" if reference_hash and row["summary"].get("rgb_sha256") == reference_hash else "failed")
            if row["summary"]["reference_rgb"] == "failed":
                row["summary"]["status"] = "failed"
            report["aggregate"] = aggregate_scenes(report["results"], args.repeats)
            write_json(output / "summary.json", report)
        report["status"] = "COMPLETE" if all(r["summary"]["status"] == "complete" for r in report["results"]) else "FAILED"
    except KeyboardInterrupt:
        report["status"] = "INTERRUPTED"
    except Exception as error:
        report.update(status="FAILED", error=str(error))
    report["finished_utc"] = utc_now()
    report["source_observation_at_end"] = source_identity()
    report["checkout_changed_during_run"] = report["source_observation_at_end"] != source
    report["aggregate"] = aggregate_scenes(report["results"], args.repeats)
    report["counts"] = {"complete": sum(r.get("summary", {}).get("status") == "complete" for r in report["results"]),
                        "selected": len(selection["scenes"])}
    write_json(output / "summary.json", report)
    print(json.dumps({"status": report["status"], "counts": report["counts"], "summary": str(output / "summary.json")}))
    return 0 if report["status"] == "COMPLETE" else 1


if __name__ == "__main__":
    raise SystemExit(main())
