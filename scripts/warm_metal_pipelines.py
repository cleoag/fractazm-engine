#!/usr/bin/env python3
"""Fill the persistent Native Metal pipeline archive for a set of scenes.

A specialized pipeline archive is keyed by the shader library bytes, the GPU
name, the macOS version and the scene's specialization. A new library, another
GPU or an OS update therefore starts cold, and each scene's first render pays
backend compilation (up to about two minutes for large reflection scenes).
Run this after such a change so later renders, benchmarks and demos start warm.

Each scene renders once at a small size. The pipeline set does not depend on
the image size or the thread count, so the archives serve full-size renders.
Images are discarded; this is not a qualification or a benchmark.
"""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import json
import os
from pathlib import Path
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build-native-arm64-fast"


def warm(scene: Path, args: argparse.Namespace, scratch: Path, index: int) -> dict:
    output, profile = scratch / f"{index}.png", scratch / f"{index}.json"
    command = [str(args.binary), str(scene.resolve()), "--backend", "metal", "--precision", "native",
               "--width", str(args.width), "--height", str(args.height), "--threads", str(args.threads),
               "--deterministic", "--output", str(output), "--profile-json", str(profile),
               "--metal-library", str(args.library)]
    start = time.monotonic()
    try:
        done = subprocess.run(command, cwd=args.runtime, capture_output=True, text=True, timeout=args.timeout)
        code = done.returncode
    except subprocess.TimeoutExpired:
        code = None
    row = {"scene": str(scene), "seconds": round(time.monotonic() - start, 2), "exit_code": code,
           "backend": None, "fallback": "", "archive": None}
    if profile.exists():
        data = json.loads(profile.read_text())
        row["backend"] = data.get("backend")
        row["fallback"] = data.get("backend_fallback_code", "")
        row["archive"] = data.get("metal_execution", {}).get("pipeline_archive")
    return row


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("scenes", nargs="*", type=Path,
                        help="M3P files or directories (default: mb3d/M3Parameter)")
    parser.add_argument("--binary", type=Path, default=BUILD / "mb3d_render")
    parser.add_argument("--library", type=Path, default=BUILD / "metal_native_scene.metallib")
    parser.add_argument("--runtime", type=Path, default=Path.cwd(),
                        help="resource root the renders run in (maps, formulas)")
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--width", type=int, default=64)
    parser.add_argument("--height", type=int, default=48)
    parser.add_argument("--threads", type=int, default=6)
    parser.add_argument("--timeout", type=int, default=300, help="seconds per scene")
    args = parser.parse_args()
    args.binary, args.library = args.binary.resolve(), args.library.resolve()
    roots = args.scenes or [ROOT / "mb3d/M3Parameter"]
    scenes = [s for root in roots for s in (sorted(root.glob("*.m3p")) if root.is_dir() else [root])]
    if not scenes:
        parser.error("no scenes")
    print("archive directory:", os.environ.get("MB3D_METAL_PIPELINE_CACHE", "~/Library/Caches/mb3d/metal-pipelines"))
    with tempfile.TemporaryDirectory(prefix="mb3d-warm-") as temporary, ThreadPoolExecutor(args.jobs) as pool:
        rows = list(pool.map(lambda item: warm(item[1], args, Path(temporary), item[0]), enumerate(scenes)))
    compiled = loaded = failed = 0
    for row in rows:
        archive = row["archive"] or [0, 0]
        loaded, compiled = loaded + archive[0], compiled + archive[1]
        failed += row["exit_code"] != 0
        note = f"fallback {row['fallback']}" if row["fallback"] else ""
        print(f"{row['seconds']:8.2f} s  hits {archive[0]:3d}  compiled {archive[1]:3d}  {Path(row['scene']).name} {note}")
    print(f"{len(rows)} scenes: {compiled} pipelines compiled, {loaded} already archived, {failed} failed renders")
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
