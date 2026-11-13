"""Run a frozen ordinary external replay plan using isolated renderer processes.

Each original case remains in the denominator. Reference-blocked inputs and
strict rejection obligations are preserved and are never labelled parity PASS.
"""
from __future__ import annotations

import argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor, as_completed
import hashlib
import json
import os
from pathlib import Path
import subprocess
import time
from compatibility_precision import verify_precision_evidence

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def write(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def verify_inputs(plan):
    if sha(plan["cpp_exe"]) != plan["cpp_sha256"]:
        raise ValueError("Frozen C++ executable changed")
    for path, expected in plan["input_sha256"].items():
        if sha(path) != expected:
            raise ValueError("Frozen replay input changed: " + path)


def run_job(job, plan):
    output = Path(job["output"])
    summary = output / "summary.json"
    started = time.monotonic()
    reused, code = output.exists(), None
    if not reused:
        with (output.parent / (output.name + ".log")).open("wb") as stream:
            completed = subprocess.run(job["command"], cwd=ROOT, stdout=stream, stderr=subprocess.STDOUT,
                                       creationflags=subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0)
            code = completed.returncode
    if not summary.exists():
        return dict(index=job["index"], status="execution_error", returncode=code, results=[], error="No summary; do not silently restart over partial output")
    report = json.loads(summary.read_bytes())
    rows = report.get("results", [])
    actual = [r["scene"] for r in rows]
    if not report.get("finished") or len(actual) != len(set(actual)) or set(actual) != set(job["cases"]):
        raise ValueError(f"Job {job['index']} is incomplete or case identity differs; preserve its output")
    if report.get("cpp_sha256") != plan["cpp_sha256"]:
        raise ValueError("Job executable identity differs")
    for row in rows:
        if row.get("status") in ("pass", "fail"):
            image = output / "cpp" / row["suite"] / (row["scene"] + ".png")
            if sha(image) != row.get("cpp_image_sha256"):
                raise ValueError("Candidate output image changed: " + str(image))
            verify_precision_evidence(image, row)
    return dict(index=job["index"], status="finished", returncode=code, reused_verified_finished_job=reused,
                wall_seconds=round(time.monotonic() - started, 3), summary=str(summary), summary_sha256=sha(summary),
                results=[{k: row.get(k) for k in ("scene", "scene_sha256", "status", "exact_rgb", "cpp_ms", "error")} for row in rows])


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--plan", type=Path, required=True)
    ap.add_argument("--workers", type=int, default=1, help="Concurrent isolated replay processes; no shared in-process renderer state")
    args = ap.parse_args()
    if not 1 <= args.workers <= 8:
        ap.error("--workers must be in 1..8")
    raw = args.plan.read_bytes()
    plan = json.loads(raw)
    verify_inputs(plan)
    ids = [c for j in plan["jobs"] for c in j["cases"]]
    if len(ids) != len(set(ids)) or len(ids) != plan["scheduled_cases"] or plan["unresolved"]:
        raise ValueError("Plan case identity/count invalid or original references unresolved")
    started = time.monotonic()
    report = dict(scope="Frozen ordinary external image replay; no full Stage-A completion claim",
                  plan_sha256=hashlib.sha256(raw).hexdigest(), cpp_sha256=plan["cpp_sha256"], workers=args.workers,
                  total_original_cases=plan["total_original_cases"], scheduled_cases=len(ids),
                  excluded_from_numeric_replay=plan["excluded_from_numeric_replay"], jobs=[], finished=False)
    out = args.plan.parent / "combined-summary.json"
    write(out, report)
    with ThreadPoolExecutor(max_workers=args.workers) as executor:
        pending = {executor.submit(run_job, j, plan): j for j in plan["jobs"]}
        for future in as_completed(pending):
            job = pending[future]
            try:
                result = future.result()
            except Exception as error:
                result = dict(index=job["index"], status="verification_error", error=str(error), results=[])
            report["jobs"].append(result)
            write(out, report)
            print("job", job["index"], result["status"], len(result["results"]), flush=True)
    verify_inputs(plan)
    if hashlib.sha256(args.plan.read_bytes()).hexdigest() != report["plan_sha256"]:
        raise ValueError("Replay plan changed during execution")
    rows = [r for j in report["jobs"] for r in j["results"]]
    report.update(finished=True, wall_seconds=round(time.monotonic() - started, 3), counts=dict(Counter(r["status"] for r in rows)),
                  exact_rgb=sum(r["status"] == "pass" and r["exact_rgb"] is True for r in rows),
                  missing_scheduled_cases=sorted(set(ids) - {r["scene"] for r in rows}),
                  original_blockers_remain=True)
    report["numeric_replay_pass"] = len(rows) == len(ids) and all(r["status"] == "pass" for r in rows)
    report["jobs"].sort(key=lambda x: x["index"])
    write(out, report)
    print(json.dumps({k: report[k] for k in ("numeric_replay_pass", "counts", "exact_rgb", "wall_seconds")}), flush=True)
    return 0 if report["numeric_replay_pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
