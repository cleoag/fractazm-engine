"""Generate 30 Phase-3 built-in .m3p scene variants that exercise multi-bounce
specular reflections.

Phase 2 verified parity at `SRreflectioncount = 1` (the template default).
Phase 3 Task 3 widens coverage: for each of the 10 Pascal internal formula
references, produce 3 variants that differ ONLY in `SRreflectioncount`,
stepping up to 2, 3, and 5 bounces.

| Variant suffix | SRreflectioncount |
|----------------|-------------------|
| _r2            | 2                 |
| _r3            | 3                 |
| _r5            | 5                 |

Everything else matches Phase 2 defaults: AO on, single hard-shadow light
(light 1), DOF on, reflections enable bit 0 on, Monte Carlo off, MCT
first-step random off. Only the medium size (480x360) is used, and the
per-formula zoom triple from Phase 2 is reused (middle zoom).

Scenes are written to `comparison/scenes_phase3_reflections/` with a
`phase3_refl_` filename prefix so sweep reports stay separate from other
phases.

The generator clones each reference .m3p byte-for-byte, then patches ONLY
the fields it changes. `SRreflectioncount` is a `uint8_t` at offset 337 in
TMandHeader10, immediately after `bCalcSRautomatic` at offset 336.
"""

from __future__ import annotations

import argparse
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

# Canonical references: mirrors Phase 2 / Phase 3 T2 selections so formula
# coverage stays aligned. test_21_hybrid stands in for amazingbox.
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

# (suffix, SRreflectioncount byte value).
REFL_VARIANTS = [
    ("r2", 2),
    ("r3", 3),
    ("r5", 5),
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
    SRreflectioncount: int = 337
    bCalculateHardShadow: int = 133
    MCoptions: int = 124


OFF = OffsetMap()

# Phase 2 default: only light 1's hard-shadow bit set.
SHADOW_MASK_L1_ONLY = 0x04


def patch_scene(src_path: Path, dst_path: Path, width: int, height: int,
                zoom: float, reflection_count: int,
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

    # Reflections ON (bit 0 of bCalcSRautomatic) and set bounce count.
    data[OFF.bCalcSRautomatic] |= 0x01
    struct.pack_into("<B", data, OFF.SRreflectioncount, int(reflection_count))

    # Monte Carlo OFF.
    data[OFF.MCoptions] = 0

    dst_path.write_bytes(data)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--scene-dir", type=Path,
                    default=Path("comparison/scenes"),
                    help="Directory containing reference .m3p files")
    ap.add_argument("--out-dir", type=Path,
                    default=Path("comparison/scenes_phase3_reflections"),
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
        for suffix, refl_count in REFL_VARIANTS:
            dst_name = f"phase3_refl_{tag}_{suffix}.m3p"
            dst = args.out_dir / dst_name
            pan_str = f" pan=({pan[0]:+.2f},{pan[1]:+.2f})" if pan else ""
            print(f"  {ref_path.name:32s} -> {dst_name:44s} "
                  f"[{MEDIUM_W}x{MEDIUM_H} zoom={zoom} "
                  f"refl={refl_count}{pan_str}]")
            if not args.dry_run:
                patch_scene(ref_path, dst, MEDIUM_W, MEDIUM_H, zoom,
                            refl_count, pan)
            total += 1

    print(f"\nGenerated {total} scene variants in {args.out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
