"""Sequential CPU compatible/native timing with exact repeats inside each mode.

Defaults to the nine active-effect fixtures. Explicit external scenes default to
64x48; they are opt-in because some software extended formulas are expensive.
Cross-mode RGB differences are measured, never treated as parity failures.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
import math
from pathlib import Path, PurePosixPath
import re
import shutil
import statistics
import subprocess
import sys

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))
from native_evidence import source_identity
from benchmark_gallery_arm64 import binary_identity, native_host

MODES = ("compatible", "native")
STAGES = {"preparation", "volume", "primary", "normals", "shadows", "ao",
          "color", "reflections", "dof", "load", "final_surface", "save"}
TIMINGS = ("render_ms", "render_and_readback_ms", "end_to_end_ms")
DISK_FLOOR = 12 * 1024**3


def sha(path):
    with Path(path).open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def write_json(path, value):
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(json.dumps(value, indent=2, allow_nan=False) + "\n")
    temporary.replace(path)


def duration(value):
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value < 0:
        raise ValueError("Invalid duration")
    return value


def relative(root, name):
    path = PurePosixPath(name)
    if not name or path.is_absolute() or ".." in path.parts or "\\" in name:
        raise ValueError("Unsafe relative input path")
    result = (root / name).resolve(strict=True)
    if not result.is_relative_to(root.resolve()) or not result.is_file():
        raise ValueError("Input escapes root or is not a file")
    return result


def selection(path, requested):
    manifest = json.loads(path.read_text())
    rows = manifest.get("scenes", [])
    if (manifest.get("schema") != "mb3d.render-benchmark-selection.v1"
            or not rows or manifest.get("expected_count") != len(rows)):
        raise ValueError("Missing selection schema/count")
    by_id = {}
    for row in rows:
        name = row.get("id", "")
        if not re.fullmatch(r"[A-Za-z0-9_-]+", name) or name in by_id:
            raise ValueError("Invalid/duplicate scene id")
        scene = relative(ROOT, row["scene_path"])
        if scene.suffix.lower() != ".m3p" or sha(scene) != row.get("scene_sha256"):
            raise ValueError("Scene hash/type mismatch: " + name)
        if "cpu" not in row.get("backends", []) or not row.get("required_stages"):
            raise ValueError("CPU stage contract missing")
        if set(row["required_stages"]) - STAGES:
            raise ValueError("Unknown required stage")
        by_id[name] = dict(row, source_path=str(scene), external=False)
    chosen = []
    for item in requested or list(by_id):
        if item in by_id:
            row = by_id[item]
        else:
            scene = Path(item).resolve(strict=True)
            if scene.suffix.lower() != ".m3p" or not scene.is_file():
                raise ValueError("Explicit scene must be an ordinary .m3p file")
            row = {"id": scene.stem, "source_path": str(scene), "external": True,
                   "scene_sha256": sha(scene), "required_stages": ["preparation", "primary", "color"]}
        if not re.fullmatch(r"[A-Za-z0-9_-]+", row["id"]) or any(s["id"] == row["id"] for s in chosen):
            raise ValueError("Unsafe or duplicate selected scene id")
        if any(t in row["id"].lower() for t in ("montecarlo", "monte_carlo", "halton", "jit")):
            raise ValueError("Deferred input excluded")
        chosen.append(row)
    destinations = set()
    assets = manifest.get("assets", [])
    for asset in assets:
        source = relative(ROOT, asset["source_path"])
        target = PurePosixPath(asset["runtime_path"])
        if (target.is_absolute() or ".." in target.parts or "\\" in str(target)
                or not str(target).startswith("M3Maps/") or str(target) in destinations
                or sha(source) != asset.get("sha256")):
            raise ValueError("Asset destination/hash mismatch")
        destinations.add(str(target))
    return chosen, assets


def quiet_host():
    """Reject known concurrent VMs/builds; never terminate another process."""
    prlctl = shutil.which("prlctl")
    if prlctl:
        result = subprocess.run([prlctl, "list", "-a"], text=True, capture_output=True, check=True)
        for line in result.stdout.splitlines()[1:]:
            fields = line.split()
            if fields and (len(fields) < 2 or fields[1] != "stopped"):
                raise ValueError("Stop Parallels VMs before precision timing: " + line.strip())
    commands = subprocess.check_output(["ps", "-axo", "comm="], text=True).splitlines()
    blocked = {"cmake", "ninja", "make", "gmake", "clang", "clang++", "cc1", "cc1plus",
               "cl", "MSBuild", "xcodebuild", "mb3d_render", "mb3d_cpu_render_benchmark"}
    active = sorted({Path(c.strip()).name for c in commands} & blocked)
    if active:
        raise ValueError("Concurrent build/renderer activity: " + ", ".join(active))


def check_disk(path):
    free = shutil.disk_usage(path).free
    if free < DISK_FLOOR:
        raise ValueError("Less than 12 GiB free; precision benchmark stopped")
    return free


def observe_disk(report, path, phase, enforce=True):
    free = shutil.disk_usage(path).free
    disk = report["disk_free_bytes"]
    disk["minimum_observed"] = min(disk["minimum_observed"], free)
    disk["observations"].append({"phase": phase, "free": free})
    if phase == "end":
        disk["end"] = free
    if enforce and free < DISK_FLOOR:
        raise ValueError("Less than 12 GiB free; precision benchmark stopped")


def mode_order(cold_index):
    return MODES if cold_index % 2 == 0 else tuple(reversed(MODES))


def admit_profile_reference(references, mode, rgb):
    if mode not in MODES:
        raise ValueError("Unknown requested precision")
    if mode in references and references[mode] != rgb:
        raise ValueError("Cold fresh-process RGB differs within precision profile")
    references.setdefault(mode, rgb)


def pixels(path, size):
    with Image.open(path) as image:
        if image.format != "PNG" or image.size != size:
            raise ValueError("PNG dimensions/type mismatch")
        return image.convert("RGB").tobytes()


def rgb_difference(left, right):
    if len(left) != len(right) or len(left) % 3:
        raise ValueError("RGB lengths differ")
    errors = [abs(a - b) for a, b in zip(left, right)]
    return {"different_channels": sum(e != 0 for e in errors),
            "different_pixels": sum(any(errors[i:i + 3]) for i in range(0, len(errors), 3)),
            "maximum_channel_error": max(errors, default=0),
            "mean_absolute_channel_error": statistics.mean(errors) if errors else 0,
            "exact": left == right, "allowed": True}


def validate_report(report, directory, mode, settings, scene, runtime):
    if mode not in MODES:
        raise ValueError("Unknown requested precision")
    if (report.get("schema") != "mb3d.cpu-render-benchmark.v1" or report.get("backend") != "cpu"
            or report.get("status") != "COMPLETE" or report.get("architecture") != "arm64"):
        raise ValueError("Incomplete or non-native CPU benchmark metadata")
    execution = report.get("execution", {})
    if (execution.get("precision") != mode or execution.get("deterministic") is not True
            or execution.get("profile_timing") is not True or execution.get("work_control") != "none"):
        raise ValueError("Missing/wrong precision or execution metadata")
    if report.get("settings") != settings:
        raise ValueError("Benchmark settings mismatch")
    if (report.get("scene") != str(scene["frozen_path"])
            or report.get("resource_root") != str(runtime)):
        raise ValueError("Scene/resource root metadata mismatch")
    features = report.get("loaded_features", {})
    if (features.get("load_success") is not True or not isinstance(features.get("formulas"), list)
            or len(features["formulas"]) != 6
            or type(features.get("MCoptions")) is not int or features["MCoptions"] & 1):
        raise ValueError("Ordinary loaded-feature metadata missing")
    for slot, formula in enumerate(features["formulas"]):
        if (not isinstance(formula, dict) or formula.get("slot") != slot
                or not isinstance(formula.get("name"), str)
                or type(formula.get("id")) is not int or type(formula.get("iterations")) is not int):
            raise ValueError("Formula-slot metadata missing")
    if any("jit" in f.get("name", "").lower() for f in features["formulas"]):
        raise ValueError("Deferred input excluded")
    cold = report.get("cold")
    warmups, samples = report.get("warmups"), report.get("samples")
    if (not isinstance(cold, dict) or not isinstance(warmups, list) or not isinstance(samples, list)
            or len(warmups) != settings["warmups"] or len(samples) != settings["repeats"]):
        raise ValueError("Incomplete sample inventory")
    inventory = [(cold, "cold", 0), *[(s, "warmup", i) for i, s in enumerate(warmups)],
                 *[(s, "warm", i) for i, s in enumerate(samples)]]
    reference = None
    images = []
    for frame, phase, index in inventory:
        expected_image = "cold.png" if phase == "cold" else f"{phase}-{index:03d}.png"
        if (frame.get("phase") != phase or frame.get("index") != index or frame.get("image") != expected_image
                or frame.get("state") != "completed" or frame.get("exact_rgb_matches_cold") is not True
                or (frame.get("width"), frame.get("height")) != (settings["width"], settings["height"])
                or type(frame.get("threads")) is not int or not 1 <= frame["threads"] <= settings["threads"]):
            raise ValueError("Incomplete or inconsistent sample")
        for name in (*TIMINGS, "readback_ms", "png_encode_write_ms"):
            duration(frame.get(name))
        if phase == "cold":
            duration(frame.get("cold_total_ms")); duration(frame.get("cold_end_to_end_ms"))
        stages = frame.get("stages", {})
        if set(stages) != STAGES:
            raise ValueError("Missing stage inventory")
        for name, stage in stages.items():
            if stage.get("state") not in ("completed", "not_run"):
                raise ValueError("Incomplete stage: " + name)
            duration(stage.get("seconds"))
        for name in [*scene["required_stages"], "final_surface", "save"]:
            if stages[name]["state"] != "completed" or stages[name]["seconds"] <= 0:
                raise ValueError("Required effect stage did not run: " + name)
        data = pixels(directory / expected_image, (settings["width"], settings["height"]))
        if reference is None:
            reference = data
        elif data != reference:
            raise ValueError("RGB repeat differs within precision profile")
        images.append({"path": str((directory / expected_image).name), "png_sha256": sha(directory / expected_image),
                       "rgb_sha256": hashlib.sha256(data).hexdigest()})
    return reference, {"images": images, "loaded_features": features,
                       "cold_stages": cold["stages"], "cold": {k: cold[k] for k in (*TIMINGS, "cold_total_ms")},
                       "warm": {k: [s[k] for s in samples] for k in TIMINGS},
                       "exact_within_process": True}


def summarize_processes(processes):
    cold = {k: [p["cold"][k] for p in processes] for k in (*TIMINGS, "cold_total_ms")}
    warm = {k: [v for p in processes for v in p["warm"][k]] for k in TIMINGS}
    return {"cold_median_ms": {k: statistics.median(v) for k, v in cold.items()},
            "warm_median_ms": {k: statistics.median(v) for k, v in warm.items()},
            "cold_samples_ms": cold, "warm_samples_ms": warm}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cpu", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--selection", type=Path, default=ROOT / "comparison/benchmark_scenes/effects-v1.json")
    parser.add_argument("--scene", action="append", default=[], help="Selection id or explicit .m3p path; repeatable")
    parser.add_argument("--width", type=int)
    parser.add_argument("--height", type=int)
    parser.add_argument("--threads", type=int, default=6)
    parser.add_argument("--cold-repeats", type=int, default=3)
    parser.add_argument("--warmups", type=int, default=1)
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument("--timeout", type=float, default=1800)
    args = parser.parse_args(argv)
    output = args.output.resolve()
    report = None
    try:
        if output.exists() or not output.parent.is_dir():
            raise ValueError("Output must be a new directory with an existing parent")
        if (args.threads < 1 or args.cold_repeats < 3 or args.warmups < 0 or args.repeats < 3
                or not math.isfinite(args.timeout) or args.timeout <= 0):
            raise ValueError("Need threads>=1, cold/warm repeats>=3, warmups>=0, positive timeout")
        scenes, assets = selection(args.selection, args.scene)
        external = any(s["external"] for s in scenes)
        settings = {"width": args.width if args.width is not None else (64 if external else 512),
                    "height": args.height if args.height is not None else (48 if external else 384),
                    "threads": args.threads, "warmups": args.warmups, "repeats": args.repeats}
        if any(not 1 <= settings[k] <= 16384 for k in ("width", "height")):
            raise ValueError("Invalid dimensions")
        host = native_host(); identity = binary_identity(args.cpu.resolve(strict=True))
        quiet_host(); initial_free = check_disk(output.parent)
        source = source_identity()
        output.mkdir(); (output / "inputs").mkdir(); (output / "runtime").mkdir(); (output / "binary").mkdir()
        binary = output / "binary" / args.cpu.name
        shutil.copy2(args.cpu, binary)
        if sha(binary) != identity["sha256"]:
            raise ValueError("Frozen binary mismatch")
        report = {"schema": "mb3d.precision-benchmark.v1", "status": "RUNNING", "host": host,
                  "source_identity": source, "binary": dict(identity, frozen_path=str(binary)),
                  "source_binding": "Observed source snapshot; no claim that observation alone proves binary build inputs",
                  "selection_sha256": sha(args.selection), "settings": settings, "cold_repeats": args.cold_repeats,
                  "disk_free_bytes": {"floor": DISK_FLOOR, "start": initial_free, "minimum_observed": initial_free,
                                      "observations": [{"phase": "start", "free": initial_free}]},
                  "modes": list(MODES), "cross_mode_rgb_differences_allowed": True, "scenes": [],
                  "timing_contract": "Sequential fresh processes; compatible/native order alternates by cold repetition. Each has one cold, warmups, then warm samples; OS caches not flushed; no VM/build activity at launch",
                  "started_utc": datetime.now(timezone.utc).isoformat(), "assets": assets}
        write_json(output / "summary.json", report)
        for asset in assets:
            destination = output / "runtime" / asset["runtime_path"]
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(ROOT / asset["source_path"], destination)
            if sha(destination) != asset["sha256"]:
                raise ValueError("Frozen asset mismatch")
        runtime = (output / "runtime").resolve()
        for scene in scenes:
            scene["frozen_path"] = output / "inputs" / (scene["id"] + ".m3p")
            shutil.copy2(scene["source_path"], scene["frozen_path"])
            if sha(scene["frozen_path"]) != scene["scene_sha256"]:
                raise ValueError("Frozen scene mismatch")
            row = {"id": scene["id"], "scene_sha256": scene["scene_sha256"], "source_path": scene["source_path"],
                   "required_stages": scene["required_stages"], "profiles": {}, "process_order": []}
            report["scenes"].append(row)
            references = {}
            for mode in MODES:
                row["profiles"][mode] = {"processes": []}
                (output / scene["id"] / mode).mkdir(parents=True)
            for index in range(args.cold_repeats):
                for mode in mode_order(index):
                    profile = row["profiles"][mode]
                    parent = output / scene["id"] / mode
                    label = f"{scene['id']}/{mode}/{index}"
                    quiet_host(); observe_disk(report, output, "before:" + label)
                    if sha(binary) != identity["sha256"]:
                        raise ValueError("Benchmark binary changed")
                    destination = parent / f"process-{index:03d}"
                    command = [str(binary), "--scene", str(scene["frozen_path"]), "--output", str(destination),
                               "--resource-root", str(runtime), "--precision", mode]
                    for key, value in settings.items():
                        command += ["--" + key, str(value)]
                    log = parent / f"process-{index:03d}.log"
                    process = {"cold_index": index, "command": command, "status": "RUNNING", "log": str(log.relative_to(output))}
                    row["process_order"].append({"cold_index": index, "precision": mode})
                    profile["processes"].append(process); write_json(output / "summary.json", report)
                    with log.open("w") as stream:
                        completed = subprocess.run(command, cwd=runtime, stdout=stream, stderr=subprocess.STDOUT,
                                                   timeout=args.timeout, check=False)
                    process["exit_code"] = completed.returncode; process["log_sha256"] = sha(log)
                    observe_disk(report, output, "after:" + label)
                    if completed.returncode:
                        raise ValueError("CPU benchmark failed; see " + str(log))
                    raw = json.loads((destination / "report.json").read_text())
                    rgb, detail = validate_report(raw, destination, mode, settings, scene, runtime)
                    admit_profile_reference(references, mode, rgb)
                    process.update(detail, status="PASS", report_sha256=sha(destination / "report.json"))
                    write_json(output / "summary.json", report)
            for profile in row["profiles"].values():
                profile.update(summarize_processes(profile["processes"]), exact_within_profile=True)
            row["cross_profile_rgb"] = rgb_difference(references["compatible"], references["native"])
            row["speed_ratio_compatible_over_native"] = {}
            for phase in ("cold", "warm"):
                key = phase + "_median_ms"
                numerator = row["profiles"]["compatible"][key]["render_ms"]
                denominator = row["profiles"]["native"][key]["render_ms"]
                if denominator <= 0:
                    raise ValueError("Cannot calculate speed ratio from zero native render duration")
                row["speed_ratio_compatible_over_native"][phase] = numerator / denominator
            write_json(output / "summary.json", report)
        report["source_identity_at_end"] = source_identity()
        if report["source_identity_at_end"] != source or sha(args.cpu) != identity["sha256"] or sha(binary) != identity["sha256"]:
            raise ValueError("Source or binary changed during benchmark")
        for scene in scenes:
            if any(sha(p) != scene["scene_sha256"] for p in (scene["source_path"], scene["frozen_path"])):
                raise ValueError("Scene changed during benchmark")
        for asset in assets:
            if any(sha(p) != asset["sha256"] for p in (ROOT / asset["source_path"], runtime / asset["runtime_path"])):
                raise ValueError("Asset changed during benchmark")
        if sha(args.selection) != report["selection_sha256"]:
            raise ValueError("Selection changed during benchmark")
        quiet_host(); observe_disk(report, output, "completed")
        report["status"] = "COMPLETE"
    except (OSError, ValueError, KeyError, TypeError, subprocess.SubprocessError) as error:
        if report is not None:
            report.update(status="FAILED", error=str(error))
        print("Precision benchmark failed: " + str(error), file=sys.stderr)
        return 1
    finally:
        if report is not None:
            observe_disk(report, output, "end", enforce=False)
            report["finished_utc"] = datetime.now(timezone.utc).isoformat()
            write_json(output / "summary.json", report)
    print(output / "summary.json")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
