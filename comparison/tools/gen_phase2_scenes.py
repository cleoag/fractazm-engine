"""Generate 30 Phase-2 built-in .m3p scene variants with post-effects ENABLED.

Phase 2 flips the flags that Phase 1 cleared. These scenes exercise the AO,
hard-shadow, DOF, and single-bounce reflection code paths, which depend on
the PasLcg deterministic-random source patched in T6-T12. Monte Carlo stays
off (too expensive for the sweep) and `iOptions` bit 0 (MCT first-step random)
stays off (that path has a mirror gap — see §0b).

For each of 10 Pascal internal formula references, produce 3 variants that
differ in (Width, Height, dZoom). All variants keep the same per-formula zoom
triple tuned in Phase 1 so composition stays legible.

Scenes are written to `comparison/scenes_phase2/` with a `phase2_` filename
prefix so sweep reports stay separate from Phase 1's `builtin_*` scenes.

The generator clones each reference .m3p byte-for-byte, then patches ONLY
the fields it changes. This minimises risk of breaking the complex 840-byte
header layout.
"""

from __future__ import annotations

import argparse
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

# Canonical references: one per (formula, power) signature.
# Mirrors Phase 1 selections so formula coverage stays aligned.
REFERENCES = [
    ("intpow2",    "test_01_intpow2.m3p"),
    ("intpow5",    "test_02_intpow5.m3p"),
    ("intpow8",    "test_03_intpow8.m3p"),
    ("realpower",  "test_04_realpower.m3p"),
    ("quaternion", "test_05_quaternion.m3p"),
    ("tricorn",    "test_06_tricorn.m3p"),
    # test_07_amazingbox renders the camera *inside* the Amazing Box at
    # default zoom; test_21_hybrid mixes IntPow2 + AmazingBox (both built-in,
    # iFnr = 0 + 4) and produces a legible fractal body.
    ("amazingbox", "test_21_hybrid.m3p"),
    ("bulbox",     "test_08_bulbox.m3p"),
    ("foldintpow", "test_09_foldintpow.m3p"),
    ("aexionc",    "test_10_aexionc.m3p"),
]

# Per-formula zoom triples — kept identical to Phase 1 (already tuned for
# good composition in small/medium/large renders).
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

    # --- Phase-2 enable pattern ---------------------------------------------
    # iOptions bit 0 = bMCTFirstStepRandom. Keep OFF — that code path has a
    # mirror gap per §0b of the Phase 2 plan.
    iOptions = struct.unpack_from("<H", data, OFF.iOptions)[0]
    iOptions &= ~0x0001
    struct.pack_into("<H", data, OFF.iOptions, iOptions)

    # AO ON: bit 0 of bCalcAmbShadowAutomatic. Preserve bits 2-3 (AO mode)
    # and force bit 7 (automatic seed) OFF — we seed deterministically via
    # PasLcg, fighting the template's seed would be counterproductive.
    ambsh = data[OFF.bCalcAmbShadowAutomatic]
    ambsh = (ambsh | 0x01) & ~0x80
    data[OFF.bCalcAmbShadowAutomatic] = ambsh

    # AO dithering OFF: dithering bakes a second pseudo-random source.
    data[OFF.AODEdithering] = 0

    # Hard shadows ON for light 1 (bit 2). Leave bits 3-7 (lights 2-6) at
    # template defaults.
    data[OFF.bCalculateHardShadow] |= 0x04

    # DOF ON (type 1). Leave sDOFaperture at template default.
    data[OFF.bCalcDOFtype] = 1

    # Reflections ON (bit 0 of bCalcSRautomatic). Leave SRreflectioncount /
    # SRamount at template defaults.
    data[OFF.bCalcSRautomatic] |= 0x01

    # Monte Carlo stays OFF — too expensive for sweep.
    data[OFF.MCoptions] = 0

    dst_path.write_bytes(data)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--scene-dir", type=Path,
                    default=Path("comparison/scenes"),
                    help="Directory containing reference .m3p files")
    ap.add_argument("--out-dir", type=Path,
                    default=Path("comparison/scenes_phase2"),
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
        pan = FORMULA_CAMERA_PAN.get(tag)
        for (vtag, w, h), z in zip(VARIANTS, zooms):
            dst_name = f"phase2_{tag}_{vtag}.m3p"
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
