#!/usr/bin/env python3
"""Same-engine background controls and exact repeats on three original scenes.

The legacy build receives identical original backgrounds at the resource root;
the candidate must find them in M3Maps. The separate off controls disable only
BGbmp[0]. Three repeats and active-map counters are required. Previews never
qualify originals. Slow/failed frames stop repeats for that scene.
"""
import argparse
import json
from pathlib import Path
import shutil

from native_gpu_coverage import image_difference, rgb_sha256, sha256
from run_m3parameter_gpu import invoke, work_record

ROOT = Path(__file__).resolve().parents[2]


def logical_work(profile):
    data = work_record(profile["metal_execution"])
    # Cache compilation is not fractal work and changes on the first process.
    data.pop("pipeline_archive", None)
    return data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("baseline", "candidate", "resources", "output"):
        parser.add_argument("--" + name, type=Path, required=True)
    parser.add_argument("--original", action="store_true")
    parser.add_argument("--ranks", type=int, nargs="+", default=[26, 66, 72])
    args = parser.parse_args()
    assert set(args.ranks) <= {26, 66, 72} and not args.output.exists()
    args.output = args.output.resolve()
    args.output.mkdir(parents=True)
    args.resources = args.resources.resolve()
    baseline, candidate = args.baseline.resolve(), args.candidate.resolve()
    runtime = args.resources / "runtime"
    legacy = args.output / "legacy-runtime"
    shutil.copytree(runtime, legacy)
    for name in ("3011.jpg", "3038.jpg", "8.jpg"):
        shutil.copyfile(runtime / "M3Maps" / name, legacy / name)
    rows = json.loads((args.resources / "queue.json").read_text())["rows"]
    identities = {str(p): sha256(p) for b in (baseline, candidate)
                  for p in (b / "mb3d_render", b / "metal_native_scene.metallib")}
    results = []

    def render(row, label, build, cwd, scene, backend="metal"):
        out = args.output / str(row["rank"]) / label
        argv = [build / "mb3d_render", scene, "--backend", backend, "--precision", "native",
                "--threads", "6", "--output", out / "image.png", "--profile-json", out / "profile.json"]
        if backend == "metal":
            argv += ["--metal-library", build / "metal_native_scene.metallib"]
        if not args.original:
            argv += ["--width", "160", "--height", "120"]
        event = invoke(argv, out, cwd, timeout=300, evidence_root=ROOT)
        if event["exit_code"] or event["guard"] or event["frame_warning_emitted"]:
            raise ValueError("Failed/slow frame: " + str(out))
        profile = json.loads((out / "profile.json").read_text())
        if profile["backend"] != backend or profile["backend_fallback"] or profile["status"] != "complete":
            raise ValueError("Incomplete or fallback frame: " + str(out))
        return out, profile

    for row in rows:
        if row["rank"] not in args.ranks:
            continue
        result = {"rank": row["rank"], "scene": row["scene"], "status": "OPEN"}
        results.append(result)
        try:
            scene = ROOT / row["scene"]
            repeats = [render(row, "on-0", candidate, runtime, scene)]
            for repeat in (1, 2):
                repeats.append(render(row, "on-" + str(repeat), candidate, runtime, scene))
            hashes = [rgb_sha256(out / "image.png") for out, _ in repeats]
            exact = len(set(hashes)) == 1 and all(logical_work(p) == logical_work(repeats[0][1]) for _, p in repeats)
            if not exact:
                raise ValueError("RGB/logical work repeats differ")
            old, old_profile = render(row, "legacy-root-map", baseline, legacy, scene)
            off, off_profile = render(row, "off", candidate, runtime,
                                      args.resources / "controls" / f"{row['rank']}-background-off.m3p")
            cpu, cpu_profile = render(row, "native-cpu", candidate, runtime, scene, "cpu")
            difference = image_difference(repeats[0][0] / "image.png", off / "image.png")
            old_exact = rgb_sha256(old / "image.png") == hashes[0] and logical_work(old_profile) == logical_work(repeats[0][1])
            active = bool(repeats[0][1]["metal_execution"]["color_flags"] & 512) and difference["pixels"] > 0
            if not old_exact or not active:
                raise ValueError("Legacy resource replay or active-map control failed")
            result.update(status="PASS_ORIGINAL_RESOURCE_CHECK" if args.original else "PASS_PREVIEW_ONLY",
                          exact_repeats=3, legacy_rgb_work_exact=old_exact, active_background=True,
                          off_difference=difference, rgb_sha256=hashes[0],
                          size=[repeats[0][1]["width"], repeats[0][1]["height"]],
                          gpu_seconds=[p["render_total_seconds"] for _, p in repeats],
                          archive_states=[p["metal_execution"]["pipeline_archive"] for _, p in repeats],
                          cpu_seconds=cpu_profile["render_total_seconds"],
                          cpu_difference=image_difference(repeats[0][0] / "image.png", cpu / "image.png"))
        except (ValueError, OSError, KeyError) as error:
            result["error"] = str(error)
        report = {"original_size": args.original, "full_project_qualification": False,
                  "identities": identities, "results": results}
        (args.output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
        print(json.dumps(result), flush=True)
    assert all(sha256(Path(p)) == value for p, value in identities.items()), "Build changed during run"
    return int(any(r["status"] == "OPEN" for r in results))


if __name__ == "__main__":
    raise SystemExit(main())
