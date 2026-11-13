#!/usr/bin/env python3
"""Build the ordinary extall queue and summarize recorded previews, never qualify.

Both commands are read-only with respect to scenes and raw coverage. Outputs
must be new. A missing CPU image is missing evidence, not an exact match.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def queue(root=ROOT):
    scenes = sorted((root / "comparison/scenes").glob("extall_*.m3p"))
    if not scenes:
        raise ValueError("No ordinary extall scenes found")
    return {"schema": "mb3d.extall-preview-queue.v1", "qualification": False,
            "rows": [{"rank": i + 1, "scene": p.relative_to(root).as_posix(),
                      "sha256": hashlib.sha256(p.read_bytes()).hexdigest()}
                     for i, p in enumerate(scenes)]}


def summarize(directory):
    from PIL import Image, ImageChops, ImageStat
    report = json.loads((directory / "coverage.json").read_text())
    rows, families = [], defaultdict(Counter)
    seen = set()
    for record in report["results"]:
        rank, scene = record["rank"], record["scene"]
        if type(rank) is not int or rank < 1 or rank in seen:
            raise ValueError("Invalid/duplicate rank")
        seen.add(rank)
        stem = Path(scene).stem
        if not stem.startswith("extall_"):
            raise ValueError("Not an extall scene: " + scene)
        family, formula = stem[len("extall_"):].split("_", 1)
        row = {**record, "family": family, "formula": formula,
               "comparison_status": "NOT_AVAILABLE"}
        families[family][record["status"]] += 1
        images = directory / "scenes" / f"{rank:02d}"
        if record["status"] == "GPU" and (images / "cpu.png").is_file() and (images / "image.png").is_file():
            with Image.open(images / "image.png") as a, Image.open(images / "cpu.png") as b:
                a, b = a.convert("RGB"), b.convert("RGB")
                if a.size != b.size or list(a.size) != report["size"]:
                    raise ValueError("Image dimensions differ from coverage")
                diff = ImageChops.difference(a, b)
                row.update(comparison_status="OBSERVED", exact_rgb=a.tobytes() == b.tobytes(),
                           strong_pixels=sum(max(pixel) > 16 for pixel in diff.getdata()),
                           cpu_constant=all(lo == hi for lo, hi in b.getextrema()),
                           mean_difference=sum(ImageStat.Stat(diff).sum) / (3 * a.width * a.height))
        rows.append(row)
    statuses = dict(Counter(r["status"] for r in rows))
    if statuses != report["statuses"]:
        raise ValueError("Status totals differ from rows")
    return {"schema": "mb3d.extall-preview-summary.v1", "qualification": False,
            "coverage_sha256": hashlib.sha256((directory / "coverage.json").read_bytes()).hexdigest(),
            "binary_sha256": report["binary_sha256"], "library_sha256": report["library_sha256"],
            "size": report["size"], "statuses": statuses,
            "families": {k: dict(v) for k, v in sorted(families.items())},
            "observed_comparisons": sum(r["comparison_status"] == "OBSERVED" for r in rows),
            "timing_caveat": "Use source run metadata for cache state/concurrency; this is not a balanced benchmark.",
            "rows": rows}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    q = sub.add_parser("queue")
    q.add_argument("--root", type=Path, default=ROOT)
    q.add_argument("--output", type=Path, required=True)
    a = sub.add_parser("analyze")
    a.add_argument("coverage", type=Path)
    a.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    result = queue(args.root) if args.command == "queue" else summarize(args.coverage)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("x") as output:
        output.write(json.dumps(result, indent=2) + "\n")
    print(json.dumps({"scenes": len(result["rows"]), "qualification": False,
                      "output": str(args.output)}))


if __name__ == "__main__":
    main()
