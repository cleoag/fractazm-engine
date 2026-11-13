"""Pascal-vs-C++ image parity diff with threshold-based pass/fail.

Computes six metrics on an RGB pixel pair and evaluates each against a
threshold. A scene is "pass" only if every metric is within its limit.

Metrics:
    max_abs_delta   max per-channel |pascal - cpp|, 0..255 scale
    mean_abs_delta  mean per-channel |pascal - cpp|
    p99_abs_delta   99th percentile of per-channel |pascal - cpp|
    diff_ratio      fraction of pixels where any channel diff exceeds
                    `diff_ratio_threshold` (default 2)
    psnr            peak signal-to-noise ratio (dB), infinite if identical
    ssim            structural similarity, luminance channel, 11x11 Gaussian
                    window, K1=0.01 K2=0.03 (Wang 2004 reference values)

Usage:
    python image_diff.py --pascal A.png --cpp B.png [--report report.json]
                         [--thresholds thresholds.json] [--quiet]

Exit codes:
    0   pass
    1   fail
    2   error (missing files, size mismatch, bad input)
"""
from __future__ import annotations

import argparse
import json
import math
import sys
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Optional

import numpy as np
from PIL import Image

DEFAULT_THRESHOLDS = {
    # Fixed checkpoint tolerances. Report exact RGB separately; these gates
    # do not certify bitwise equality or explain the cause of any difference.
    "max_abs_delta": 255.0,
    "mean_abs_delta": 2.0,
    "p99_abs_delta": 10.0,
    "diff_ratio": 0.6,
    "diff_ratio_threshold": 2,
    "psnr_min": 40.0,
    "ssim_min": 0.98,
}


@dataclass
class Metrics:
    max_abs_delta: float
    mean_abs_delta: float
    p99_abs_delta: float
    diff_ratio: float
    diff_ratio_threshold: int
    psnr: float
    ssim: float


def load_rgb(path: Path) -> np.ndarray:
    if not path.exists():
        raise FileNotFoundError(f"image not found: {path}")
    img = Image.open(path).convert("RGB")
    return np.asarray(img, dtype=np.uint8)


def _gaussian_window(size: int = 11, sigma: float = 1.5) -> np.ndarray:
    coords = np.arange(size, dtype=np.float64) - (size - 1) / 2.0
    g = np.exp(-(coords ** 2) / (2.0 * sigma ** 2))
    g /= g.sum()
    return np.outer(g, g)


def _conv2d_same(img: np.ndarray, kernel: np.ndarray) -> np.ndarray:
    """2D convolution with 'same' padding via FFT.

    Luminance SSIM on 480x360 runs in a few ms — FFT is overkill but keeps
    the code short and numpy-only.
    """
    ih, iw = img.shape
    kh, kw = kernel.shape
    ph, pw = ih + kh - 1, iw + kw - 1
    fshape = (ph, pw)
    fimg = np.fft.rfft2(img, fshape)
    fker = np.fft.rfft2(kernel, fshape)
    conv = np.fft.irfft2(fimg * fker, fshape)
    y0 = (kh - 1) // 2
    x0 = (kw - 1) // 2
    return conv[y0:y0 + ih, x0:x0 + iw]


def compute_ssim(a: np.ndarray, b: np.ndarray) -> float:
    """Luminance-channel SSIM with 11x11 Gaussian window.

    Uses Rec. 709 luma weights. a,b are uint8 HxWx3.
    """
    w = np.array([0.2126, 0.7152, 0.0722], dtype=np.float64)
    ya = a.astype(np.float64) @ w
    yb = b.astype(np.float64) @ w

    kernel = _gaussian_window(11, 1.5)
    mu_a = _conv2d_same(ya, kernel)
    mu_b = _conv2d_same(yb, kernel)
    mu_aa = mu_a * mu_a
    mu_bb = mu_b * mu_b
    mu_ab = mu_a * mu_b
    sigma_aa = _conv2d_same(ya * ya, kernel) - mu_aa
    sigma_bb = _conv2d_same(yb * yb, kernel) - mu_bb
    sigma_ab = _conv2d_same(ya * yb, kernel) - mu_ab

    L = 255.0
    c1 = (0.01 * L) ** 2
    c2 = (0.03 * L) ** 2
    num = (2.0 * mu_ab + c1) * (2.0 * sigma_ab + c2)
    den = (mu_aa + mu_bb + c1) * (sigma_aa + sigma_bb + c2)
    ssim_map = num / den
    return float(ssim_map.mean())


def compute_metrics(a: np.ndarray, b: np.ndarray, diff_ratio_threshold: int) -> Metrics:
    if a.shape != b.shape:
        raise ValueError(f"shape mismatch: {a.shape} vs {b.shape}")

    diff = np.abs(a.astype(np.int16) - b.astype(np.int16))
    max_delta = float(diff.max())
    mean_delta = float(diff.mean())
    p99 = float(np.percentile(diff, 99.0))
    per_pixel_max = diff.max(axis=-1)
    ratio = float((per_pixel_max > diff_ratio_threshold).mean())

    mse = float((diff.astype(np.float64) ** 2).mean())
    psnr = math.inf if mse == 0.0 else 10.0 * math.log10((255.0 ** 2) / mse)

    ssim = compute_ssim(a, b)

    return Metrics(
        max_abs_delta=max_delta,
        mean_abs_delta=mean_delta,
        p99_abs_delta=p99,
        diff_ratio=ratio,
        diff_ratio_threshold=diff_ratio_threshold,
        psnr=psnr,
        ssim=ssim,
    )


def evaluate(m: Metrics, thresholds: dict) -> dict:
    checks = {
        "max_abs_delta": m.max_abs_delta <= thresholds["max_abs_delta"],
        "mean_abs_delta": m.mean_abs_delta <= thresholds["mean_abs_delta"],
        "p99_abs_delta": m.p99_abs_delta <= thresholds["p99_abs_delta"],
        "diff_ratio": m.diff_ratio <= thresholds["diff_ratio"],
        "psnr": m.psnr >= thresholds["psnr_min"],
        "ssim": m.ssim >= thresholds["ssim_min"],
    }
    return checks


def main(argv: Optional[list[str]] = None) -> int:
    p = argparse.ArgumentParser(description="Pascal-vs-C++ image diff")
    p.add_argument("--pascal", required=True, type=Path)
    p.add_argument("--cpp", required=True, type=Path)
    p.add_argument("--report", type=Path, default=None)
    p.add_argument("--thresholds", type=Path, default=None)
    p.add_argument("--quiet", action="store_true")
    args = p.parse_args(argv)

    try:
        thresholds = dict(DEFAULT_THRESHOLDS)
        if args.thresholds is not None:
            thresholds.update(json.loads(args.thresholds.read_text()))

        a = load_rgb(args.pascal)
        b = load_rgb(args.cpp)
        metrics = compute_metrics(a, b, int(thresholds["diff_ratio_threshold"]))
    except (FileNotFoundError, ValueError, json.JSONDecodeError) as err:
        print(f"error: {err}", file=sys.stderr)
        return 2

    checks = evaluate(metrics, thresholds)
    passed = all(checks.values())

    payload = {
        "pascal": str(args.pascal),
        "cpp": str(args.cpp),
        "size": list(a.shape),
        "metrics": asdict(metrics),
        "thresholds": thresholds,
        "checks": checks,
        "pass": passed,
    }

    if args.report is not None:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(payload, indent=2))

    if not args.quiet:
        status = "PASS" if passed else "FAIL"
        print(f"{status}  "
              f"max={metrics.max_abs_delta:.0f} "
              f"mean={metrics.mean_abs_delta:.3f} "
              f"p99={metrics.p99_abs_delta:.1f} "
              f"ratio={metrics.diff_ratio:.4%} "
              f"psnr={metrics.psnr:.2f}dB "
              f"ssim={metrics.ssim:.4f}")
        if not passed:
            failed = [k for k, v in checks.items() if not v]
            print(f"  failed: {', '.join(failed)}", file=sys.stderr)

    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
