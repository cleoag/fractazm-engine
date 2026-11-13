#!/usr/bin/env python3
"""Native CPU sensitivity of a scene to a binary32-sized camera shift.

Each camera-centre coordinate (header dXmid/dYmid/dZmid, offsets 36/44/52)
moves by 2^-E*max(|v|,1). The shifted Native CPU frame is compared with a
reference frame (normally the unshifted Native CPU frame of the same size and
--threads). If GPU-vs-CPU is about this large, the scene is chaotic at binary32
resolution; if GPU-vs-CPU is far larger while CPU is insensitive, look for a
deterministic defect. A coherent shift underestimates per-operation rounding:
the test can clear a scene, not convict it. See docs/GPU.md (FP32 limits).
"""
import argparse
import json
import struct
import subprocess
from pathlib import Path

from native_gpu_coverage import image_difference

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--scene", type=Path, required=True)
    parser.add_argument("--reference", type=Path, required=True, help="PNG to compare with")
    parser.add_argument("--output", type=Path, required=True, help="new or existing directory")
    parser.add_argument("--runtime", type=Path, required=True, help="working directory holding M3Maps")
    parser.add_argument("--binary", type=Path, default=ROOT / "build-native-arm64-fast/mb3d_render")
    parser.add_argument("--shift", type=int, action="append", help="exponent E (repeatable, default 24)")
    parser.add_argument("--width", type=int, default=160)
    parser.add_argument("--height", type=int, default=120)
    parser.add_argument("--threads", type=int, default=6)
    parser.add_argument("--timeout", type=int, default=300)
    a = parser.parse_args()
    a.output.mkdir(parents=True, exist_ok=True)
    rows = []
    for exponent in a.shift or [24]:
        data = bytearray(a.scene.resolve().read_bytes())
        for offset in (36, 44, 52):
            v = struct.unpack_from("<d", data, offset)[0]
            struct.pack_into("<d", data, offset, v + 2.0 ** -exponent * max(abs(v), 1.0))
        scene = a.output / f"{a.scene.stem}-shift{exponent}.m3p"
        image = a.output / f"{a.scene.stem}-shift{exponent}.png"
        scene.write_bytes(bytes(data))
        subprocess.run([str(a.binary.resolve()), str(scene.resolve()), "--precision", "native",
                        "--width", str(a.width), "--height", str(a.height), "--threads", str(a.threads),
                        "--deterministic", "--backend", "cpu", "--output", str(image.resolve())],
                       cwd=a.runtime, capture_output=True, text=True, timeout=a.timeout, check=True)
        rows.append({"shift": exponent, "difference": image_difference(image, a.reference.resolve())})
        print(f"2^-{exponent}: {rows[-1]['difference']}", flush=True)
    (a.output / "sensitivity.json").write_text(json.dumps({"scene": str(a.scene), "rows": rows}, indent=1) + "\n")


if __name__ == "__main__":
    main()
