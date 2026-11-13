"""Analyze a sweep summary.json to group failures by drift pattern.

Reads a sweep_reports/<ts>/summary.json and prints:
  - overall counts
  - top-N scenes by each metric (worst and best)
  - drift-pattern clusters: groups scenes whose failing-metric signatures
    match, to surface whether failures share a root cause

Usage:
    python sweep_analyze.py <summary.json> [--top N]
"""
from __future__ import annotations

import argparse
import json
from collections import defaultdict
from pathlib import Path


def signature(result: dict) -> str:
    """Return a string signature of which metrics failed, or 'pass' / 'error'."""
    status = result.get("status", "")
    if status == "pass":
        return "pass"
    if status in ("rendered",):
        return "rendered"
    if status.endswith("_error") or status == "diff_timeout":
        return f"infra:{status}"
    diff = result.get("diff") or {}
    if not diff:
        return "fail:unknown"
    return "fail"


def cluster_failures(results: list[dict], diff_dir: Path) -> dict[tuple, list[str]]:
    clusters: dict[tuple, list[str]] = defaultdict(list)
    for r in results:
        if r.get("status") != "fail":
            continue
        name = r["scene"]
        path = diff_dir / f"{name}.json"
        if not path.exists():
            continue
        payload = json.loads(path.read_text())
        checks = payload.get("checks", {})
        sig = tuple(sorted(k for k, v in checks.items() if not v))
        clusters[sig].append(name)
    return clusters


def top_scenes(results: list[dict], diff_dir: Path, metric: str, n: int,
               worst: bool) -> list[tuple[str, float]]:
    scored: list[tuple[str, float]] = []
    for r in results:
        if r.get("status") != "fail":
            continue
        name = r["scene"]
        path = diff_dir / f"{name}.json"
        if not path.exists():
            continue
        payload = json.loads(path.read_text())
        val = payload.get("metrics", {}).get(metric)
        if val is None:
            continue
        scored.append((name, float(val)))
    scored.sort(key=lambda x: x[1], reverse=worst)
    return scored[:n]


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("summary", type=Path)
    p.add_argument("--top", type=int, default=3)
    args = p.parse_args()

    data = json.loads(args.summary.read_text())
    diff_dir = args.summary.parent / "diffs"
    results = data.get("results", [])

    print(f"sweep: mode={data['mode']} scenes={data['scene_count']}")
    print(f"  pass={data['pass_count']} fail={data['fail_count']} "
          f"error={data['error_count']}")
    print(f"  first_failure={data.get('first_failure')}")

    status_counts: dict[str, int] = defaultdict(int)
    for r in results:
        status_counts[r.get("status", "?")] += 1
    print("\nstatus breakdown:")
    for k, v in sorted(status_counts.items(), key=lambda kv: -kv[1]):
        print(f"  {k:18s} {v}")

    clusters = cluster_failures(results, diff_dir)
    if clusters:
        print("\ndrift-pattern clusters (scenes grouped by failing-metric set):")
        for sig, names in sorted(clusters.items(), key=lambda kv: -len(kv[1])):
            sig_str = ",".join(sig) if sig else "(none)"
            print(f"  [{len(names)}] {sig_str}")
            for n in names[:10]:
                print(f"      {n}")
            if len(names) > 10:
                print(f"      ... +{len(names) - 10} more")

    for metric, worst_label in [("max_abs_delta", "highest"),
                                 ("mean_abs_delta", "highest"),
                                 ("psnr", "lowest"),
                                 ("ssim", "lowest")]:
        worst = worst_label == "highest"
        rows = top_scenes(results, diff_dir, metric, args.top, worst)
        if rows:
            print(f"\n{worst_label} {metric}:")
            for name, val in rows:
                print(f"  {val:>10.4f}  {name}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
