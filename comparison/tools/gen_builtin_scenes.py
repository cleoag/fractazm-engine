"""Generate 30 built-in-only .m3p scene variants for Pascal↔C++ parity testing.

For each of 10 Pascal internal formula references, produce 3 variants that
differ in (Width, Height, dZoom). All variants clear all known stochastic
flags in the .m3p header so the Pascal render is fully deterministic even
before the Random() → LCG source patch lands.

The generator clones each reference .m3p byte-for-byte, then patches ONLY
the fields it changes. This minimises risk of breaking the complex 840-byte
header layout.
"""

from __future__ import annotations

import argparse
import shutil
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

# Canonical references: one per (formula, power) signature.
# All existing test_* scenes in comparison/scenes/ use 480x360 @ zoom=1.0.
# Some template cameras produce poor compositions at zoom=1.0 — e.g.
# test_07 ends up *inside* the Amazing Box fractal, test_08 sees bulbox as
# a distant speck. Per-formula zoom triples are tuned so every variant
# actually shows the fractal legibly.
REFERENCES = [
    ("intpow2",    "test_01_intpow2.m3p"),
    ("intpow5",    "test_02_intpow5.m3p"),
    ("intpow8",    "test_03_intpow8.m3p"),
    ("realpower",  "test_04_realpower.m3p"),
    ("quaternion", "test_05_quaternion.m3p"),
    ("tricorn",    "test_06_tricorn.m3p"),
    # test_07_amazingbox renders the camera *inside* the Amazing Box at
    # default zoom; test_21_hybrid mixes IntPow2 + AmazingBox (both built-in,
    # iFnr = 0 + 4) and produces a legible fractal body. We swap the
    # reference to still exercise the AmazingBox iteration path.
    ("amazingbox", "test_21_hybrid.m3p"),
    ("bulbox",     "test_08_bulbox.m3p"),
    ("foldintpow", "test_09_foldintpow.m3p"),
    ("aexionc",    "test_10_aexionc.m3p"),
    # IntPow2 + AmazingBox hybrid (iFnr 0 + 4, 3 + 2 iterations). Exercises
    # the generic doHybridPas alternating orbit driver across two different
    # built-in formulas.
    ("hybrid_ip2_abox", "test_21_hybrid.m3p"),
]

# Per-formula zoom + camera pan triples.
# zooms maps (small, medium, large) → dZoom.
# pan_xy optionally shifts (dXmid, dYmid) from the template's 0,0 to centre
# the fractal when its natural position is off-centre (quaternion).
FORMULA_ZOOMS = {
    "intpow2":    (0.7, 1.2, 2.0),
    "intpow5":    (0.7, 1.2, 2.0),
    "intpow8":    (0.7, 1.2, 2.0),
    "realpower":  (0.7, 1.2, 2.0),
    "quaternion": (10.0, 18.0, 30.0), # dust-cluster fractal; must zoom into a single cluster
    "tricorn":    (0.7, 1.2, 2.0),
    "amazingbox": (0.8, 1.2, 2.0),    # IntPow2+AmazingBox hybrid renders well at ~1.0
    "bulbox":     (30.0, 70.0, 150.0),# bulbox default params yield a tiny shape — push zoom hard
    "foldintpow": (0.7, 1.2, 2.0),
    "aexionc":    (0.7, 1.2, 2.0),
    "hybrid_ip2_abox": (0.5, 0.8, 1.4),  # the hybrid builds a large body
}

# Optional camera recenter (dXmid, dYmid) per formula. Applied to all
# variants — we only use this when the fractal is visibly off-centre in
# the template and a mere zoom change cannot rescue the composition.
FORMULA_CAMERA_PAN = {
    "quaternion": (-0.35, 0.30),  # aim at the bottom-left dust cluster
}

# (variant-tag, width, height). Zoom comes from FORMULA_ZOOMS.
VARIANTS = [
    ("small",  240, 180),
    ("medium", 480, 360),
    ("large",  640, 480),
]


@dataclass
class OffsetMap:
    Width: int = 4
    Height: int = 8
    iOptions: int = 16
    dXmid: int = 36
    dYmid: int = 44
    dZoom: int = 84
    bCalcAmbShadowAutomatic: int = 149
    AODEdithering: int = 188
    bCalcDOFtype: int = 181
    bCalcSRautomatic: int = 336
    bCalculateHardShadow: int = 133
    MCoptions: int = 124


OFF = OffsetMap()


def patch_scene(src_path: Path, dst_path: Path, width: int, height: int,
                zoom: float, pan: tuple[float, float] | None = None) -> None:
    data = bytearray(src_path.read_bytes())

    # Geometry.
    struct.pack_into("<i", data, OFF.Width, int(width))
    struct.pack_into("<i", data, OFF.Height, int(height))
    struct.pack_into("<d", data, OFF.dZoom, float(zoom))
    if pan is not None:
        struct.pack_into("<d", data, OFF.dXmid, float(pan[0]))
        struct.pack_into("<d", data, OFF.dYmid, float(pan[1]))

    # --- Determinism patches -------------------------------------------------
    # iOptions bit 0 = bMCTFirstStepRandom. Force off.
    iOptions = struct.unpack_from("<H", data, OFF.iOptions)[0]
    iOptions &= ~0x0001
    struct.pack_into("<H", data, OFF.iOptions, iOptions)

    # AO dithering off.
    data[OFF.AODEdithering] = 0

    # Disable AO entirely (bCalcAmbShadowAutomatic bit 0). Phase-1 scenes do
    # not exercise the AO code path; we re-introduce AO later with the
    # Random→LCG patch in place.
    ambsh = data[OFF.bCalcAmbShadowAutomatic]
    data[OFF.bCalcAmbShadowAutomatic] = ambsh & ~0x01

    # Disable hard shadows (upper 6 bits = per-light enable).
    data[OFF.bCalculateHardShadow] &= 0x03

    # Disable DOF (depth of field) — uses Random.
    data[OFF.bCalcDOFtype] = 0

    # Disable reflections — raycasts with potential jitter.
    data[OFF.bCalcSRautomatic] &= ~0x01

    # Disable Monte Carlo path tracing.
    data[OFF.MCoptions] = 0

    dst_path.write_bytes(data)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--scene-dir", type=Path,
                    default=Path("comparison/scenes"),
                    help="Directory containing reference .m3p files")
    ap.add_argument("--out-dir", type=Path,
                    default=Path("comparison/scenes_builtin"),
                    help="Destination directory for generated variants")
    ap.add_argument("--dry-run", action="store_true",
                    help="Just print what would be generated")
    args = ap.parse_args()

    if not args.scene_dir.is_dir():
        print(f"error: scene dir not found: {args.scene_dir}", file=sys.stderr)
        return 2

    # Resolve references.
    refs: list[tuple[str, Path]] = []
    for tag, name in REFERENCES:
        p = args.scene_dir / name
        if not p.exists():
            print(f"error: missing reference scene: {p}", file=sys.stderr)
            return 2
        refs.append((tag, p))

    if not args.dry_run:
        args.out_dir.mkdir(parents=True, exist_ok=True)

    total = 0
    for tag, ref_path in refs:
        zooms = FORMULA_ZOOMS.get(tag)
        if zooms is None:
            print(f"error: no zoom triple for formula '{tag}'", file=sys.stderr)
            return 2
        pan = FORMULA_CAMERA_PAN.get(tag)
        for (vtag, w, h), z in zip(VARIANTS, zooms):
            dst_name = f"builtin_{tag}_{vtag}.m3p"
            dst = args.out_dir / dst_name
            pan_str = f" pan=({pan[0]:+.2f},{pan[1]:+.2f})" if pan else ""
            print(f"  {ref_path.name:32s} -> {dst_name:40s} [{w}x{h} zoom={z}{pan_str}]")
            if not args.dry_run:
                patch_scene(ref_path, dst, w, h, z, pan)
            total += 1

    print(f"\nGenerated {total} scene variants in {args.out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
