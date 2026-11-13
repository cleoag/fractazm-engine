"""Serial Pascal-vs-C++ parity sweep runner.

Renders every scene in a directory with both the Pascal reference and
the C++ port, diffs the resulting PNGs with threshold-based metrics
(image_diff.py), and writes a per-scene report plus a summary. Runs
one scene at a time (no shared temp state, no parallel renders).

Two modes:
    symmetric  render Pascal + C++, diff, report (default)
    cpp-only   render only C++ (for ablation or smoke with --disable-*)

Ablation flags (--disable-ao, --disable-reflections, --disable-shadows,
--disable-postprocess) are C++-only and therefore require --cpp-only.

Exit codes:
    0   all scenes passed (or in cpp-only, all rendered cleanly)
    1   one or more scenes failed the diff
    2   infrastructure error (missing exe, timeout, crash, stale process)
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
import hashlib
import time
from dataclasses import asdict, dataclass, field
from datetime import datetime, timezone
from pathlib import Path
from typing import Optional

from image_diff import DEFAULT_THRESHOLDS, load_rgb, compute_metrics, evaluate
from compatibility_precision import bind_compatible, precision_evidence

REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_THREADS = 12
DEFAULT_TIMEOUT_MS = 600_000
STALE_PROCESS_NAMES = ("mb3d_render.exe", "Mandelbulb3D_release_fast.exe",
                       "Mandelbulb3D_diag_fast.exe", "Mandelbulb3D.exe")


@dataclass
class SceneResult:
    scene: str
    status: str
    pascal_ms: Optional[int] = None
    cpp_ms: Optional[int] = None
    diff: Optional[dict] = None
    error: Optional[str] = None
    exact_rgb: bool = False
    legacy_profile: Optional[str] = None
    legacy_profile_pass: Optional[bool] = None
    failed_checks: list[str] = field(default_factory=list)
    scene_sha256: str = ""
    cpp_precision: Optional[str] = None
    cpp_precision_profile_sha256: Optional[str] = None


@dataclass
class SweepSummary:
    mode: str
    started: str
    ended: Optional[str] = None
    threads: int = DEFAULT_THREADS
    timeout_ms: int = DEFAULT_TIMEOUT_MS
    scene_count: int = 0
    pass_count: int = 0
    exact_rgb_count: int = 0
    legacy_only_count: int = 0
    provenance: dict = field(default_factory=dict)
    fail_count: int = 0
    error_count: int = 0
    first_failure: Optional[str] = None
    results: list[SceneResult] = field(default_factory=list)


def check_stale_processes() -> list[str]:
    try:
        out = subprocess.check_output(
            ["tasklist", "/FO", "CSV", "/NH"],
            stderr=subprocess.DEVNULL, timeout=10,
        ).decode("cp1252", errors="replace")
    except (subprocess.SubprocessError, FileNotFoundError):
        return []
    stale = []
    for line in out.splitlines():
        parts = [p.strip('"') for p in line.split(",")]
        if parts and parts[0] in STALE_PROCESS_NAMES:
            stale.append(parts[0])
    return stale



def order_scenes(scene_dir: Path) -> list[Path]:
    """Curated order: ext_* first (external formulas), then test_*,
    then everything else. Each group alphabetically."""
    all_scenes = sorted(scene_dir.glob("*.m3p"))
    ext = [p for p in all_scenes if p.stem.startswith("ext_")]
    test = [p for p in all_scenes if p.stem.startswith("test_")]
    other = [p for p in all_scenes if p not in ext and p not in test]
    return ext + test + other


def run_render(exe: Path, scene: Path, out_png: Path,
               threads: int, timeout_ms: int,
               disable_flags: list[str]) -> tuple[int, int, str]:
    """Run a single render. Returns (returncode, elapsed_ms, stderr)."""
    out_png.parent.mkdir(parents=True, exist_ok=True)
    if out_png.exists():
        out_png.unlink()

    # Pascal resolves assets beside its executable; C++ resolves maps from
    # the repository. Both receive absolute scene/output paths.
    scene_abs = scene.resolve()
    out_abs = out_png.resolve()

    exe_name = exe.name.lower()
    is_pascal = "mandelbulb3d" in exe_name
    if is_pascal:
        argv = [str(exe), "--render", str(scene_abs),
                "--output", str(out_abs),
                "--threads", str(threads)]
    else:
        argv = [str(exe), str(scene_abs),
                "--output", str(out_abs),
                "--threads", str(threads),
                "--deterministic"]
        argv.extend(disable_flags)
        try:
            bind_compatible(argv, out_png)
        except ValueError as error:
            return (-3, 0, str(error))

    start = time.monotonic()
    try:
        proc = subprocess.run(argv,
                              capture_output=True, timeout=timeout_ms / 1000.0,
                              cwd=str(exe.parent if is_pascal else REPO_ROOT))
    except subprocess.TimeoutExpired:
        return (-1, int((time.monotonic() - start) * 1000), "TIMEOUT")

    elapsed = int((time.monotonic() - start) * 1000)
    stderr = (proc.stdout + b"\n" + proc.stderr).decode("cp1252", errors="replace")[-2000:]
    if proc.returncode != 0:
        return (proc.returncode, elapsed, stderr)
    if not out_png.exists():
        return (-2, elapsed, "output png missing")
    if not is_pascal:
        try:
            precision_evidence(out_png)
        except (OSError, ValueError) as error:
            return (-3, elapsed, "C++ precision validation failed: " + str(error))
    return (0, elapsed, "")


_PLATFORM_FLOOR_CACHE: Optional[dict] = None


def _load_platform_floor_config() -> dict:
    """Load platform_floor_thresholds.json once per process; tolerate absence.

    Returns a dict with 'profiles' and 'scene_patterns' keys, or an empty
    config if the file is missing or malformed (so the sweep still runs
    with default thresholds).
    """
    global _PLATFORM_FLOOR_CACHE
    if _PLATFORM_FLOOR_CACHE is not None:
        return _PLATFORM_FLOOR_CACHE
    cfg_path = Path(__file__).with_name("platform_floor_thresholds.json")
    if not cfg_path.exists():
        _PLATFORM_FLOOR_CACHE = {"profiles": {}, "scene_patterns": []}
        return _PLATFORM_FLOOR_CACHE
    try:
        data = json.loads(cfg_path.read_text(encoding="utf-8"))
        _PLATFORM_FLOOR_CACHE = {
            "profiles": data.get("profiles", {}),
            "scene_patterns": data.get("scene_patterns", []),
        }
    except (json.JSONDecodeError, OSError):
        _PLATFORM_FLOOR_CACHE = {"profiles": {}, "scene_patterns": []}
    return _PLATFORM_FLOOR_CACHE



def run_diff(pascal_png: Path, cpp_png: Path, report_path: Path,
             scene_name: str = "") -> dict:
    """Default gates are authoritative; legacy allowances are annotation only."""
    a, b = load_rgb(pascal_png), load_rgb(cpp_png)
    metrics = compute_metrics(a, b, int(DEFAULT_THRESHOLDS["diff_ratio_threshold"]))
    checks = evaluate(metrics, DEFAULT_THRESHOLDS)
    payload = {"pascal": str(pascal_png), "cpp": str(cpp_png),
               "size": list(a.shape), "metrics": asdict(metrics),
               "thresholds": dict(DEFAULT_THRESHOLDS), "checks": checks,
               "pass": all(checks.values()), "exact_rgb": metrics.max_abs_delta == 0,
               "legacy_profile": None, "legacy_profile_pass": None}
    cfg = _load_platform_floor_config()
    for entry in cfg["scene_patterns"]:
        if re.match(entry["pattern"], scene_name):
            limits = {k:v for k,v in cfg["profiles"][entry["profile"]].items()
                      if not k.startswith("_")}
            payload["legacy_profile"] = entry["profile"]
            payload["legacy_profile_pass"] = all(evaluate(metrics, limits).values())
            break
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    return payload


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run_sweep(args) -> int:
    scene_dir = args.scene_dir.resolve()
    args.cpp_exe = args.cpp_exe.resolve()
    args.pascal_exe = args.pascal_exe.resolve()
    args.report_dir = args.report_dir.resolve()
    if not scene_dir.is_dir():
        print(f"error: scene dir not found: {scene_dir}", file=sys.stderr)
        return 2

    if not args.cpp_exe.exists():
        print(f"error: C++ exe not found: {args.cpp_exe}", file=sys.stderr)
        return 2
    if args.mode == "symmetric" and not args.pascal_exe.exists():
        print(f"error: Pascal exe not found: {args.pascal_exe}", file=sys.stderr)
        return 2

    disable_flags: list[str] = []
    if args.disable_ao:
        disable_flags.append("--no-ao")
    if args.disable_reflections:
        disable_flags.append("--no-reflections")
    if args.disable_shadows:
        disable_flags.append("--no-shadows")
    if args.disable_postprocess:
        disable_flags.append("--no-postprocess")
    if disable_flags and args.mode != "cpp-only":
        print("error: --disable-* flags require --cpp-only (Pascal ignores them)",
              file=sys.stderr)
        return 2

    stale = check_stale_processes()
    if stale:
        print(f"error: existing render processes; refusing to interfere: {stale}", file=sys.stderr)
        return 2

    ts = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    report_root = args.report_dir / ts
    (report_root / "pascal").mkdir(parents=True, exist_ok=True)
    (report_root / "cpp").mkdir(parents=True, exist_ok=True)
    (report_root / "diffs").mkdir(parents=True, exist_ok=True)

    scenes = order_scenes(scene_dir)
    if args.limit:
        scenes = scenes[:args.limit]
    if not scenes:
        print(f"error: no .m3p scenes in {scene_dir}", file=sys.stderr)
        return 2

    summary = SweepSummary(
        mode=args.mode,
        started=ts,
        threads=args.threads,
        timeout_ms=args.timeout_ms,
        scene_count=len(scenes),
    )

    summary.provenance = {
        "cpp_exe": str(args.cpp_exe), "cpp_sha256": sha256(args.cpp_exe),
        "pascal_exe": str(args.pascal_exe),
        "pascal_sha256": sha256(args.pascal_exe) if args.mode == "symmetric" else None,
        "git_commit": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=REPO_ROOT, text=True).strip(),
        "git_status": subprocess.check_output(["git", "status", "--porcelain"], cwd=REPO_ROOT, text=True).strip(),
        "thresholds": dict(DEFAULT_THRESHOLDS),
        "scene_dir": str(scene_dir), "python": sys.version,
        "cpp_cwd": str(REPO_ROOT), "pascal_cwd": str(args.pascal_exe.parent),
        "cpp_precision_requested": "compatible",
    }

    def save_progress():
        (report_root / "summary.json").write_text(json.dumps(asdict(summary), indent=2), encoding="utf-8")

    print(f"sweep: {len(scenes)} scenes, mode={args.mode}, "
          f"threads={args.threads}, reports={report_root}")

    for i, scene in enumerate(scenes, 1):
        name = scene.stem
        print(f"[{i}/{len(scenes)}] {name} ...", flush=True)
        save_progress()
        result = SceneResult(scene=name, status="pending", scene_sha256=sha256(scene))

        cpp_png = report_root / "cpp" / f"{name}.png"
        pascal_png = report_root / "pascal" / f"{name}.png"

        if args.mode == "symmetric":
            rc, ms, err = run_render(args.pascal_exe, scene, pascal_png,
                                     args.threads, args.timeout_ms, [])
            result.pascal_ms = ms
            if rc != 0:
                result.status = "pascal_error"
                result.error = err[:500]
                summary.results.append(result)
                summary.error_count += 1
                if summary.first_failure is None:
                    summary.first_failure = name
                print(f"  PASCAL ERROR ({ms}ms): {err.splitlines()[0] if err else 'rc=' + str(rc)}")
                if args.stop_on_error:
                    break
                continue

        rc, ms, err = run_render(args.cpp_exe, scene, cpp_png,
                                 args.threads, args.timeout_ms, disable_flags)
        result.cpp_ms = ms
        if rc != 0:
            result.status = "cpp_error"
            result.error = err[:500]
            summary.results.append(result)
            summary.error_count += 1
            if summary.first_failure is None:
                summary.first_failure = name
            print(f"  CPP ERROR ({ms}ms): {err.splitlines()[0] if err else 'rc=' + str(rc)}")
            if args.stop_on_error:
                break
            continue

        if args.mode == "cpp-only":
            result.__dict__.update(precision_evidence(cpp_png))
            result.status = "rendered"
            summary.results.append(result)
            summary.pass_count += 1
            print(f"  rendered ({ms}ms)")
            continue

        diff_report = report_root / "diffs" / f"{name}.json"
        result.__dict__.update(precision_evidence(cpp_png))
        try:
            payload = run_diff(pascal_png, cpp_png, diff_report,
                               scene_name=name)
        except (OSError, ValueError) as exc:
            result.status = "diff_error"
            result.error = str(exc)
            summary.results.append(result)
            summary.error_count += 1
            if summary.first_failure is None:
                summary.first_failure = name
            print(f"  DIFF ERROR: {exc}")
            continue

        result.diff = payload.get("metrics")
        result.exact_rgb = payload["exact_rgb"]
        result.legacy_profile = payload["legacy_profile"]
        result.legacy_profile_pass = payload["legacy_profile_pass"]
        result.failed_checks = [k for k, v in payload["checks"].items() if not v]
        summary.exact_rgb_count += int(result.exact_rgb)
        summary.legacy_only_count += int(not payload["pass"] and bool(result.legacy_profile_pass))
        if payload.get("pass"):
            result.status = "pass"
            summary.pass_count += 1
            m = payload["metrics"]
            print(f"  PASS (pas={result.pascal_ms}ms cpp={result.cpp_ms}ms "
                  f"max={m['max_abs_delta']:.0f} psnr={m['psnr']:.1f} "
                  f"ssim={m['ssim']:.3f})")
        else:
            result.status = "fail"
            summary.fail_count += 1
            if summary.first_failure is None:
                summary.first_failure = name
            failed = [k for k, v in payload.get("checks", {}).items() if not v]
            m = payload.get("metrics", {})
            # image_diff emits "Infinity"/"NaN" strings when all pixels match or
            # when a metric is undefined; coerce to float before formatting.
            def _num(v, default=0.0):
                try:
                    return float(v)
                except (TypeError, ValueError):
                    return default
            print(f"  FAIL  max={_num(m.get('max_abs_delta'), 0):.0f} "
                  f"psnr={_num(m.get('psnr')):.1f} ssim={_num(m.get('ssim')):.3f} "
                  f"[{','.join(failed)}]")
            if args.stop_on_fail:
                summary.results.append(result)
                break

        summary.results.append(result)

    summary.ended = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    summary_path = report_root / "summary.json"
    summary_dict = asdict(summary)
    summary_path.write_text(json.dumps(summary_dict, indent=2, default=str))

    print(f"\ndone: pass={summary.pass_count} fail={summary.fail_count} "
          f"error={summary.error_count}")
    if summary.first_failure:
        print(f"first failure: {summary.first_failure}")
    print(f"summary: {summary_path}")

    if summary.error_count > 0:
        return 2
    if summary.fail_count > 0:
        return 1
    return 0


def main(argv: Optional[list[str]] = None) -> int:
    p = argparse.ArgumentParser(description="Serial Pascal/C++ parity sweep")
    p.add_argument("--mode", choices=["symmetric", "cpp-only"], default="symmetric")
    p.add_argument("--scene-dir", type=Path, default=REPO_ROOT / "comparison" / "scenes")
    p.add_argument("--pascal-exe", type=Path,
                   default=REPO_ROOT / "comparison" / "windows_build" / "Mandelbulb3D_release_fast.exe")
    p.add_argument("--cpp-exe", type=Path,
                   default=REPO_ROOT / "build-vs-release" / "Release" / "mb3d_render.exe")
    p.add_argument("--report-dir", type=Path,
                   default=REPO_ROOT / "comparison" / "runs")
    p.add_argument("--threads", type=int, default=DEFAULT_THREADS)
    p.add_argument("--timeout-ms", type=int, default=DEFAULT_TIMEOUT_MS)
    p.add_argument("--limit", type=int, default=0, help="only first N scenes")
    p.add_argument("--stop-on-fail", action="store_true",
                   help="stop at first failing diff (debug drift target)")
    p.add_argument("--stop-on-error", action="store_true",
                   help="stop at first render error / timeout / crash")
    p.add_argument("--disable-ao", action="store_true")
    p.add_argument("--disable-reflections", action="store_true")
    p.add_argument("--disable-shadows", action="store_true")
    p.add_argument("--disable-postprocess", action="store_true")
    args = p.parse_args(argv)

    return run_sweep(args)


if __name__ == "__main__":
    raise SystemExit(main())
