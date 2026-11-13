"""Compare guarded Pascal finite renders with recorded original Pascal images."""
import argparse
import json
from pathlib import Path

import check_formula_hang_guards
import external_validation
from check_formula_hang_guards import ROOT, read_checked, run, sha, write_json


def validate_cases(cases):
    if len(cases) != 65 or len({case["id"] for case in cases}) != 65:
        raise ValueError("Expected 65 distinct recorded finite controls")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--runtime-identity", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--timeout", type=float, default=20.)
    args = parser.parse_args()
    exe, output = args.exe.resolve(), args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    summary = output / "summary.json"
    if summary.exists():
        parser.error("Use a fresh output directory")
    manifest = json.loads(args.manifest.read_text())
    validate_cases(manifest["cases"])
    runtime = json.loads(args.runtime_identity.read_text())
    for name, expected in runtime["files"].items():
        read_checked(exe.parent / name, expected)
    sources = {str(p.resolve()): sha(p) for p in (
        args.manifest, args.runtime_identity, Path(__file__),
        Path(check_formula_hang_guards.__file__), Path(external_validation.__file__))}
    cases = []
    for case in manifest["cases"]:
        original = read_checked(Path(case["pascal_reference_image"]), case["pascal_reference_image_sha256"])
        read_checked(Path(case["pascal_reference_summary"]), case["pascal_reference_summary_sha256"])
        scene = read_checked(ROOT / Path(case["scene"]), case["sha256"])
        cases.append(dict(id=case["id"], scene=str(scene), sha256=case["sha256"],
                          expect="rendered", reference_image=str(original),
                          reference_sha256=case["pascal_reference_image_sha256"]))
    report = dict(profile="guarded Pascal versus recorded original Pascal finite controls",
                  executable=str(exe), binary_sha256=sha(exe), sources=sources,
                  cases=cases, results=[], finished=False, success=False)
    write_json(summary, report)
    for case in cases:
        row = run(exe, "pascal", case, manifest["threads"], output, args.timeout)
        report["results"].append(row)
        write_json(summary, report)
        print(case["id"], "OK" if row["contract_pass"] else "FAIL", flush=True)
    for name, expected in runtime["files"].items():
        read_checked(exe.parent / name, expected)
    for name, expected in sources.items():
        read_checked(Path(name), expected)
    for case in cases:
        read_checked(Path(case["scene"]), case["sha256"])
        read_checked(Path(case["reference_image"]), case["reference_sha256"])
    report.update(finished=True, checks=len(cases),
                  passed=sum(row["contract_pass"] for row in report["results"]))
    report["success"] = report["passed"] == report["checks"]
    write_json(summary, report)
    return 0 if report["success"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
