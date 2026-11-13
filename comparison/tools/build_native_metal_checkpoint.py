#!/usr/bin/env python3
"""Fresh ordinary/Metal build with compiler, linker and AIR dependency receipts.

No archived runs, personal skills or scene resources are required. This builds
the active targets only; it does not render or grant scene qualification.
"""
import argparse
import json
from pathlib import Path
import shlex
import shutil
import subprocess
import sys

import build_m3parameter_main as build_gate
from qualify_native_metal_all import source_snapshot

ROOT = Path(__file__).resolve().parents[2]
TARGETS = [
    "mb3d_render", "mb3d_native_formula_probe", "mb3d_backend_render_benchmark",
    "mb3d_tests_native_smoothing_readback", "mb3d_tests_native_pipeline_cache",
    "mb3d_tests_native_logical_lanes_gpu", "mb3d_tests_native_command_gpu",
    "mb3d_tests_native_dof_gpu", "mb3d_tests_native_zbuffer_gpu",
    "mb3d_tests_native_ao_seed_gpu", "mb3d_tests_native_ao_certificate_gpu",
    "mb3d_tests_native_seed_scan_gpu", "mb3d_tests_native_ao_chunk_gpu",
]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=ROOT)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--jobs", type=int, default=4)
    args = parser.parse_args()
    source, out = args.source.resolve(), args.output.resolve()
    build_gate.need(args.jobs > 0 and not out.exists(), "Positive jobs and a new output required")
    build_gate.need(shutil.disk_usage(source).free > 13 * 1024**3, "Need disk reserve and build headroom")
    build_gate.ROOT = source
    out.mkdir(parents=True)
    provenance = out / "provenance"
    provenance.mkdir()
    capture = provenance / "capture.py"
    shutil.copyfile(Path(__file__).with_name("native_build_capture.py"), capture)
    manifest = {"source_commit": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=source, text=True).strip(),
                "source_files": source_snapshot(source), "capture_sha256": build_gate.sha(capture),
                "working_tree": subprocess.check_output(["git", "status", "--porcelain"], cwd=source, text=True),
                "tracked_diff_sha256": __import__("hashlib").sha256(subprocess.check_output(["git", "diff", "HEAD"], cwd=source)).hexdigest()}
    build_gate.write(out / "source-manifest.json", manifest)
    shutil.copyfile(source / "comparison/contracts/gpu-parallel-legacy-v2/acceptance-contract.json", out / "contract.json")
    context = provenance / "context.json"
    build_gate.write(context, {"root": str(source), "build": str(out / "build"),
                              "records": str(provenance / "records"), "phase": "clean-native-checkpoint"})
    prefix = shlex.join([sys.executable, str(capture)])
    instrument = provenance / "instrument.cmake"
    instrument.write_text("set_property(GLOBAL PROPERTY RULE_LAUNCH_CUSTOM [==[" + prefix + " --kind custom --]==])\n"
                          "set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK [==[" + prefix + " --kind link --]==])\n")
    launcher = ";".join([sys.executable, str(capture), "--kind", "compile", "--"])
    configure = ["cmake", "-S", str(source), "-B", str(out / "build"), "-G", "Unix Makefiles",
                 "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_OSX_ARCHITECTURES=arm64",
                 "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", "-DMB3D_DEFAULT_PRECISION=compatible",
                 "-DMB3D_ENABLE_PARITY_TRACE=ON", "-DMB3D_ENABLE_METAL_RENDERER=ON",
                 "-DMB3D_BUILD_METAL_EXACT_SCENE=OFF", "-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=" + str(instrument)]
    configure += ["-DCMAKE_" + language + "_COMPILER_LAUNCHER=" + launcher for language in ("C", "CXX", "OBJCXX")]
    environment = build_gate.env()
    environment["MB3D_CLEAN_CAPTURE_CONTEXT"] = str(context)
    for name, command in [
        ("configure", configure),
        ("build", ["cmake", "--build", str(out / "build"), "--parallel", str(args.jobs),
                   "--verbose", "--target", "mb3d_ordinary", *TARGETS]),
    ]:
        event = out / "build-events" / name
        event.mkdir(parents=True)
        print("START", name, flush=True)
        build_gate.run(command, event, environment, 1200)
        print("DONE", name, flush=True)
    build_gate.validate(out, TARGETS)


if __name__ == "__main__":
    main()
