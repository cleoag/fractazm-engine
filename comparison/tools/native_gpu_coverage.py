#!/usr/bin/env python3
"""Count queue scenes that render a bounded preview on Native Metal.

Every queue scene runs once through mb3d_render with --backend metal at the
preview size. A scene counts only when the frame completes on Metal without
fallback and publishes an image; a CPU fallback, a GPU fault, a timeout or a
failed run does not count. The last stdout line is the count.

coverage.json records each scene's status, engine, decoded RGB hash and the
fallback or fault reason, plus a histogram of normalized blockers. A scene
that does not fit binary32 (a fallback reason naming "FP32 range", before
dispatch or by the frame's range-exit policy) is FP32_RANGE. GPU scenes record
their FP32 range-exit pixel counts.

Scenes render with --deterministic unless --random keeps their header's
random first steps (RayMarch jitter, point/DE AO). GPU scenes record the
primary jitter mode and advances and the DE AO planned seed draws.
--compare-cpu also renders each GPU scene on native CPU with the same flags
and records the mean/maximum absolute RGB difference and differing pixels.

With --baseline, scenes that rendered on Metal in the baseline coverage must
still render with identical RGB; the tool exits 1 otherwise. The baseline must
use the same random mode. A preview is a coverage measure only: it never
qualifies an original frame.
"""
from __future__ import annotations

import argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
import time

from PIL import Image, ImageChops, ImageStat

ROOT = Path(__file__).resolve().parents[2]
SCHEMA = "mb3d.native-gpu-coverage.v1"


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def rgb_sha256(path):
    with Image.open(path) as image:
        return hashlib.sha256(image.convert("RGB").tobytes()).hexdigest()


def image_difference(a, b):
    """Mean/maximum absolute RGB difference and differing pixels, or None."""
    with Image.open(a) as x, Image.open(b) as y:
        x, y = x.convert("RGB"), y.convert("RGB")
        if x.size != y.size:
            return None
        d = ImageChops.difference(x, y)
        return {"mean": round(sum(ImageStat.Stat(d).sum) / (3 * x.width * x.height), 4),
                "max": max(high for _, high in d.getextrema()),
                "pixels": sum(1 for p in d.getdata() if p != (0, 0, 0))}


def blocker(text):
    """Reason with scene-specific numbers removed, for the histogram."""
    return re.sub(r"\d+(\.\d+)?", "N", text or "")[:160]


def classify(exit_code, timed_out, image, profile):
    if timed_out:
        return "TIMEOUT", "timeout"
    if profile.get("backend_fallback"):
        reason = f"{profile.get('backend_fallback_code')}: {profile.get('backend_fallback_reason')}"
        return ("FP32_RANGE" if "FP32 range" in reason else "FALLBACK"), reason
    metal = profile.get("backend") == "metal"
    if metal and exit_code == 0 and image:
        return "GPU", ""
    reason = profile.get("error") or f"exit {exit_code}"
    if metal:
        return ("FP32_RANGE" if "Native FP32 range:" in reason else "GPU_FAULT"), reason
    return "FAILED", reason


def render(row, args):
    out = args.output / "scenes" / f"{row['rank']:02d}"
    out.mkdir(parents=True)
    image, profile = out / "image.png", out / "profile.json"
    common = [str(args.binary), str(ROOT / row["scene"]), "--precision", "native",
              "--width", str(args.width), "--height", str(args.height), "--threads", str(args.threads)]
    if not args.random:
        common.append("--deterministic")
    argv = common + ["--backend", "metal", "--output", str(image), "--profile-json", str(profile),
                     "--metal-library", str(args.library)]
    begin = time.monotonic()
    try:
        done = subprocess.run(argv, cwd=args.runtime, capture_output=True, text=True, timeout=args.timeout)
        exit_code, timed_out = done.returncode, False
        (out / "process.txt").write_text(done.stdout[-4000:] + done.stderr[-4000:])
    except subprocess.TimeoutExpired:
        exit_code, timed_out = None, True
    seconds = round(time.monotonic() - begin, 3)
    record = json.loads(profile.read_text()) if profile.is_file() else {}
    status, reason = classify(exit_code, timed_out, image.is_file(), record)
    result = {"rank": row["rank"], "scene": row["scene"], "status": status, "reason": reason, "seconds": seconds,
              "exit_code": exit_code, "engine": record.get("metal_execution", {}).get("engine")}
    if status == "GPU":
        result["rgb_sha256"] = rgb_sha256(image)
        execution = record.get("metal_execution", {})
        for key in ("fp32_range_exit_pixels", "ao_fp32_range_exit_pixels", "primary_random_mode",
                    "primary_random_advances", "ao_seed_planning_draws"):
            result[key] = execution.get(key)
        if args.compare_cpu:
            cpu = out / "cpu.png"
            begin = time.monotonic()
            try:
                done = subprocess.run(common + ["--backend", "cpu", "--output", str(cpu)], cwd=args.runtime,
                                      capture_output=True, text=True, timeout=args.timeout)
                (out / "cpu-process.txt").write_text(done.stdout[-4000:] + done.stderr[-4000:])
            except subprocess.TimeoutExpired:
                pass
            result["cpu_seconds"] = round(time.monotonic() - begin, 3)
            result["cpu_difference"] = image_difference(image, cpu) if cpu.is_file() else None
    return result


def regressions(baseline, current):
    """Scenes that rendered on Metal in the baseline but no longer match."""
    now = {r["rank"]: r for r in current}
    bad = []
    for r in baseline:
        if r["status"] != "GPU":
            continue
        n = now.get(r["rank"], {})
        if n.get("status") != "GPU" or n.get("rgb_sha256") != r["rgb_sha256"]:
            bad.append({"rank": r["rank"], "baseline": r["rgb_sha256"], "status": n.get("status"),
                        "rgb_sha256": n.get("rgb_sha256"), "reason": n.get("reason")})
    return bad


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--binary", type=Path, default=ROOT / "build-native-arm64-fast/mb3d_render")
    parser.add_argument("--library", type=Path, default=ROOT / "build-native-arm64-fast/metal_native_scene.metallib")
    parser.add_argument("--runtime", type=Path, required=True, help="working directory holding M3Maps")
    parser.add_argument("--queue", type=Path, default=ROOT / "comparison/scene-queue.json")
    parser.add_argument("--output", type=Path, required=True, help="Output directory; replaced if present")
    parser.add_argument("--width", type=int, default=160)
    parser.add_argument("--height", type=int, default=120)
    parser.add_argument("--threads", type=int, default=6)
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--timeout", type=int, default=180)
    parser.add_argument("--baseline", type=Path, help="coverage.json whose Metal scenes must keep identical RGB")
    parser.add_argument("--random", action="store_true", help="keep random first steps (omit --deterministic)")
    parser.add_argument("--compare-cpu", action="store_true", help="also render GPU scenes on native CPU")
    args = parser.parse_args(argv)
    baseline = json.loads(args.baseline.read_text()) if args.baseline else None
    if baseline and baseline.get("first_step_random", False) != args.random:
        raise SystemExit("baseline coverage uses the other random mode")
    for name in ("binary", "library", "runtime", "output"):
        setattr(args, name, getattr(args, name).resolve())
    if args.output.exists():
        shutil.rmtree(args.output)
    args.output.mkdir(parents=True)
    rows = json.loads(args.queue.read_text())["rows"]
    binary, library = sha256(args.binary), sha256(args.library)
    with ThreadPoolExecutor(args.jobs) as pool:
        results = sorted(pool.map(lambda row: render(row, args), rows), key=lambda r: r["rank"])
    if sha256(args.binary) != binary or sha256(args.library) != library:
        raise SystemExit("binary or library changed during the coverage run")
    statuses = Counter(r["status"] for r in results)
    report = {"schema": SCHEMA, "binary_sha256": binary, "library_sha256": library, "queue_sha256": sha256(args.queue),
              "size": [args.width, args.height], "threads": args.threads, "scenes": len(results),
              "first_step_random": args.random, "cpu_reference": args.compare_cpu,
              "gpu": statuses["GPU"], "statuses": dict(sorted(statuses.items())),
              "blockers": dict(Counter(blocker(r["reason"]) for r in results if r["status"] != "GPU").most_common()),
              "results": results}
    if baseline:
        report["regressions"] = regressions(baseline["results"], results)
    (args.output / "coverage.json").write_text(json.dumps(report, indent=1, sort_keys=True) + "\n")
    print(json.dumps({"statuses": report["statuses"], "regressions": len(report.get("regressions", []))}),
          file=sys.stderr)
    print(report["gpu"])
    return 1 if report.get("regressions") else 0


if __name__ == "__main__":
    sys.exit(main())
