"""Verify RGB stability and trace-on/off invariance at fixed thread counts."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

from PIL import Image
from compatibility_precision import bind_compatible, precision_evidence

ROOT = Path(__file__).resolve().parents[2]
CASES = [
    ("rotate", "comparison/scenes/ext_06_rotate.m3p", "100,75"),
    ("decomb", "comparison/scenes_phase3_decomb/phase3_decomb_intpow-bulbox_v0_medium.m3p", "24,151"),
    ("hiqao", "comparison/scenes_phase3_hiqao/phase3_hiqao_quaternion_random_medium.m3p", "427,0"),
]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--side", choices=["pascal", "cpp"], required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    exe = args.exe.resolve()
    output = args.output.resolve()
    # A previous image must never stand in for a failed or incomplete new run.
    output.mkdir(parents=True, exist_ok=False)
    report = {"side": args.side, "binary_sha256": hashlib.sha256(exe.read_bytes()).hexdigest(),
              "tool_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
              "finished": False, "comparisons": [], "runs": [],
              "scenes": [{"case": name, "path": scene,
                          "sha256": hashlib.sha256((ROOT / scene).read_bytes()).hexdigest()}
                         for name, scene, _ in CASES]}
    (output / "summary.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    for name, scene, point in CASES:
        for threads in [1, 16]:
            pixels = {}
            for mode in ["plain", "repeat", "diag", "all"]:
                stem = f"{name}-{threads}-{mode}"
                png = output / (stem + ".png")
                command = [str(exe)] + (["--render"] if args.side == "pascal" else [])
                command += [str(ROOT / scene), "--output", str(png), "--threads", str(threads)]
                if args.side == "cpp":
                    command += ["--deterministic"]
                    bind_compatible(command,png)
                if mode in ["diag", "all"]:
                    command += ["--diag-points", point]
                    if args.side == "pascal":
                        command += ["--diag-trace", str(output / (stem + ".log"))]
                if mode == "all":
                    command += ["--parity-trace", str(output / (stem + ".trace")), "--parity-trace-level", "all"]
                    if args.side == "cpp":
                        command += ["--parity-trace-schema"]
                if args.side == "pascal" and name == "rotate" and threads == 1 and mode == "diag":
                    command += ["--ray-setup-dump", str(output / (stem + ".ray"))]
                with (output / (stem + ".stdout")).open("wb") as stdout, (output / (stem + ".stderr")).open("wb") as stderr:
                    process = subprocess.run(command, cwd=exe.parent if args.side == "pascal" else ROOT,
                                             stdout=stdout, stderr=stderr, timeout=240)
                run = {"case": name, "threads": threads, "mode": mode, "command": command,
                       "returncode": process.returncode}
                if process.returncode == 0 and png.is_file():
                    if args.side == "cpp":
                        run.update(precision_evidence(png))
                    with Image.open(png) as image:
                        pixels[mode] = (image.size, image.convert("RGB").tobytes())
                    run["rgb_sha256"] = hashlib.sha256(pixels[mode][1]).hexdigest()
                report["runs"].append(run)
                print(stem, process.returncode, flush=True)
            for mode in ["repeat", "diag", "all"]:
                exact = "plain" in pixels and mode in pixels and pixels["plain"] == pixels[mode]
                report["comparisons"].append({"case": name, "threads": threads, "mode": mode, "exact_rgb": exact})
            (output / "summary.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    if hashlib.sha256(exe.read_bytes()).hexdigest() != report["binary_sha256"]:
        raise ValueError("Executable changed during diagnostic qualification")
    for scene in report["scenes"]:
        if hashlib.sha256((ROOT / scene["path"]).read_bytes()).hexdigest() != scene["sha256"]:
            raise ValueError("Scene changed during diagnostic qualification: " + scene["path"])
    report["finished"] = True
    (output / "summary.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    return int(not all(item["exact_rgb"] for item in report["comparisons"]))


if __name__ == "__main__":
    raise SystemExit(main())
