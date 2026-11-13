"""Generate side-by-side composite: Pascal | C++ | amplified diff map.

Diff map: per-pixel max absolute delta across channels, amplified 4x,
clipped to 255, rendered as grayscale -> magenta heatmap on black.
Useful to eyeball *where* the drift lives.
"""
from __future__ import annotations

import argparse
from pathlib import Path

import numpy as np
from PIL import Image


def heatmap(delta: np.ndarray, gain: float = 4.0) -> np.ndarray:
    """delta: HxW uint8-range.  Returns HxWx3 uint8 magenta heatmap."""
    v = np.clip(delta.astype(np.float32) * gain, 0, 255).astype(np.uint8)
    out = np.zeros((*v.shape, 3), dtype=np.uint8)
    out[..., 0] = v
    out[..., 2] = v
    return out


def composite(pascal: Path, cpp: Path, output: Path, gain: float = 4.0) -> None:
    a = np.asarray(Image.open(pascal).convert("RGB"))
    b = np.asarray(Image.open(cpp).convert("RGB"))
    if a.shape != b.shape:
        raise ValueError(f"shape mismatch: {a.shape} vs {b.shape}")

    diff = np.abs(a.astype(np.int16) - b.astype(np.int16)).max(axis=-1).astype(np.uint8)
    hm = heatmap(diff, gain=gain)

    h, w, _ = a.shape
    canvas = np.zeros((h, w * 3 + 8, 3), dtype=np.uint8)
    canvas[:, :w] = a
    canvas[:, w + 4:2 * w + 4] = b
    canvas[:, 2 * w + 8:] = hm
    Image.fromarray(canvas).save(output)


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--report-dir", type=Path, required=True,
                   help="sweep_reports/<ts> directory")
    p.add_argument("--scenes", nargs="+", required=True)
    p.add_argument("--out-dir", type=Path, default=None)
    p.add_argument("--gain", type=float, default=4.0)
    args = p.parse_args()

    out_dir = args.out_dir or (args.report_dir / "composites")
    out_dir.mkdir(parents=True, exist_ok=True)

    for scene in args.scenes:
        pascal = args.report_dir / "pascal" / f"{scene}.png"
        cpp = args.report_dir / "cpp" / f"{scene}.png"
        out = out_dir / f"{scene}_composite.png"
        composite(pascal, cpp, out, gain=args.gain)
        print(f"{scene} -> {out}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
