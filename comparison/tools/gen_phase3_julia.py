"""Generate 10 Phase-3 built-in .m3p scene variants that exercise Julia mode.

Phase 2 verified parity with `bIsJulia = 0` (the template default). Phase 3
Task 5 widens coverage: for each of the 10 Pascal internal formula
references, produce one variant that enables Julia mode by setting
`bIsJulia = 1` and writing a small non-zero Julia constant
(dJx, dJy, dJz, dJw) = (0.1, 0.1, 0.1, 0.0).

Everything else matches the Phase 2 / Phase 3 T2 / T3 defaults: AO on,
single hard-shadow light (light 1), DOF on, reflections enable bit 0 on with
bounce count = 1, Monte Carlo off, MCT first-step random off. Only the
medium size (480x360) is used, and the per-formula medium zoom from Phase 2
is reused.

Scenes are written to `comparison/scenes_phase3_julia/` with a
`phase3_julia_` filename prefix so sweep reports stay separate from other
phases.

The generator clones each reference .m3p byte-for-byte, then patches ONLY
the fields it changes. `bIsJulia` is a `uint8_t` at offset 190 in
TMandHeader10; dJx/dJy/dJz/dJw are four consecutive `double`s immediately
after at offsets 191, 199, 207, 215.
"""

from __future__ import annotations

import argparse
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

# Canonical references: mirrors Phase 2 / Phase 3 T2 / T3 selections so
# formula coverage stays aligned. test_21_hybrid stands in for amazingbox.
REFERENCES = [
    ("intpow2",    "test_01_intpow2.m3p"),
    ("intpow5",    "test_02_intpow5.m3p"),
    ("intpow8",    "test_03_intpow8.m3p"),
    ("realpower",  "test_04_realpower.m3p"),
    ("quaternion", "test_05_quaternion.m3p"),
    ("tricorn",    "test_06_tricorn.m3p"),
    ("amazingbox", "test_21_hybrid.m3p"),
    ("bulbox",     "test_08_bulbox.m3p"),
    ("foldintpow", "test_09_foldintpow.m3p"),
    ("aexionc",    "test_10_aexionc.m3p"),
]

# Phase 2 per-formula zoom triples (small, medium, large). We use the
# medium slot (index 1) only.
FORMULA_ZOOMS = {
    "intpow2":    (0.7, 1.2, 2.0),
    "intpow5":    (0.7, 1.2, 2.0),
    "intpow8":    (0.7, 1.2, 2.0),
    "realpower":  (0.7, 1.2, 2.0),
    "quaternion": (10.0, 18.0, 30.0),
    "tricorn":    (0.7, 1.2, 2.0),
    "amazingbox": (0.8, 1.2, 2.0),
    "bulbox":     (30.0, 70.0, 150.0),
    "foldintpow": (0.7, 1.2, 2.0),
    "aexionc":    (0.7, 1.2, 2.0),
}

FORMULA_CAMERA_PAN = {
    "quaternion": (-0.35, 0.30),
}

MEDIUM_W, MEDIUM_H = 480, 360

# Julia constant shared by every variant. Small non-zero components produce
# a visually distinct julia set while avoiding degenerate cases near the
# origin.
JULIA_DJX = 0.1
JULIA_DJY = 0.1
JULIA_DJZ = 0.1
JULIA_DJW = 0.0


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
    SRreflectioncount: int = 337
    bCalculateHardShadow: int = 133
    MCoptions: int = 124
    bIsJulia: int = 190
    dJx: int = 191
    dJy: int = 199
    dJz: int = 207
    dJw: int = 215


OFF = OffsetMap()

# Phase 2 default: only light 1's hard-shadow bit set.
SHADOW_MASK_L1_ONLY = 0x04


def patch_scene(src_path: Path, dst_path: Path, width: int, height: int,
                zoom: float,
                pan: tuple[float, float] | None = None) -> None:
    data = bytearray(src_path.read_bytes())

    # Geometry.
    struct.pack_into("<i", data, OFF.Width, int(width))
    struct.pack_into("<i", data, OFF.Height, int(height))
    struct.pack_into("<d", data, OFF.dZoom, float(zoom))
    if pan is not None:
        struct.pack_into("<d", data, OFF.dXmid, float(pan[0]))
        struct.pack_into("<d", data, OFF.dYmid, float(pan[1]))

    # --- Phase-2 enable pattern ---------------------------------------------
    iOptions = struct.unpack_from("<H", data, OFF.iOptions)[0]
    iOptions &= ~0x0001  # MCT first-step random OFF (mirror gap)
    struct.pack_into("<H", data, OFF.iOptions, iOptions)

    # AO ON, automatic-seed OFF.
    ambsh = data[OFF.bCalcAmbShadowAutomatic]
    ambsh = (ambsh | 0x01) & ~0x80
    data[OFF.bCalcAmbShadowAutomatic] = ambsh

    # AO dithering OFF.
    data[OFF.AODEdithering] = 0

    # Single hard-shadow light (light 1 only); clear bits 2..7 then OR.
    existing = data[OFF.bCalculateHardShadow] & 0x03
    data[OFF.bCalculateHardShadow] = existing | (SHADOW_MASK_L1_ONLY & 0xFC)

    # DOF ON (type 1).
    data[OFF.bCalcDOFtype] = 1

    # Reflections ON (bit 0 of bCalcSRautomatic); bounce count = 1 (Phase 2
    # default).
    data[OFF.bCalcSRautomatic] |= 0x01
    struct.pack_into("<B", data, OFF.SRreflectioncount, 1)

    # Monte Carlo OFF.
    data[OFF.MCoptions] = 0

    # --- Phase-3 T5: Julia mode ---------------------------------------------
    struct.pack_into("<B", data, OFF.bIsJulia, 1)
    struct.pack_into("<d", data, OFF.dJx, JULIA_DJX)
    struct.pack_into("<d", data, OFF.dJy, JULIA_DJY)
    struct.pack_into("<d", data, OFF.dJz, JULIA_DJZ)
    struct.pack_into("<d", data, OFF.dJw, JULIA_DJW)

    dst_path.write_bytes(data)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--scene-dir", type=Path,
                    default=Path("comparison/scenes"),
                    help="Directory containing reference .m3p files")
    ap.add_argument("--out-dir", type=Path,
                    default=Path("comparison/scenes_phase3_julia"),
                    help="Destination directory for generated variants")
    ap.add_argument("--dry-run", action="store_true",
                    help="Just print what would be generated")
    args = ap.parse_args()

    if not args.scene_dir.is_dir():
        print(f"error: scene dir not found: {args.scene_dir}", file=sys.stderr)
        return 2

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
        zoom = zooms[1]  # medium
        pan = FORMULA_CAMERA_PAN.get(tag)
        dst_name = f"phase3_julia_{tag}_medium.m3p"
        dst = args.out_dir / dst_name
        pan_str = f" pan=({pan[0]:+.2f},{pan[1]:+.2f})" if pan else ""
        print(f"  {ref_path.name:32s} -> {dst_name:44s} "
              f"[{MEDIUM_W}x{MEDIUM_H} zoom={zoom} "
              f"julia=({JULIA_DJX},{JULIA_DJY},{JULIA_DJZ},{JULIA_DJW})"
              f"{pan_str}]")
        if not args.dry_run:
            patch_scene(ref_path, dst, MEDIUM_W, MEDIUM_H, zoom, pan)
        total += 1

    print(f"\nGenerated {total} scene variants in {args.out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
