"""Generate Phase-3 .m3p scene variants that exercise cut-plane clipping.

Phase 2 and all prior Phase 3 tasks rendered with `bCutOption = 0`
(no planar clipping). Phase 3 Task 8 exercises the cut-plane path that
truncates rays at axis-aligned planes through `(dCutX, dCutY, dCutZ)`:

    Pascal:  CalcHardShadow.pas / Calc.pas RMmaxLengthToCutPlane
             iCutOptions bits 0/1/2 enable X/Y/Z planes
             iCutOptions bits 3/4/5 flip which half-space is clipped
             dCOX/dCOY/dCOZ set the plane offset from origin
    C++:     src/core/ambient_occlusion.cpp MinLengthToCutPlane /
             AtCutPlane2, src/core/shadow.cpp HSminLengthToCutPlane

Three variants per formula (10 formulas x 3 = 30 scenes):
    _cutX  bCutOption = 1  (X-plane only)
    _cutY  bCutOption = 2  (Y-plane only)
    _cutZ  bCutOption = 4  (Z-plane only)

Plane offsets are held at (0.3, 0.3, 0.3) - a mid-fractal slice that
keeps roughly half the surface visible while cleanly clipping the other
half so the diff is sensitive to cut-plane math drift. Phase 2 effect
flags (AO, single hard shadow, DOF, reflections) remain enabled so this
exercises cut-plane interaction with every shading path.

Medium 480x360 renders (not panorama).

Offsets:
    bCutOption  uint8   #176
    dCutX       double  #346
    dCutY       double  #354
    dCutZ       double  #362
"""

from __future__ import annotations

import argparse
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

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

# Medium render dims (not panorama).
IMG_W, IMG_H = 480, 360

# Cut-plane offset - mid-fractal slice.
CUT_OFFSET = 0.3

CUT_VARIANTS = [
    ("cutX", 0x01),  # X-plane enabled, default half-space
    ("cutY", 0x02),  # Y-plane enabled
    ("cutZ", 0x04),  # Z-plane enabled
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
    bPlanarOptic: int = 148
    bCutOption: int = 176
    dCutX: int = 346
    dCutY: int = 354
    dCutZ: int = 362


OFF = OffsetMap()

SHADOW_MASK_L1_ONLY = 0x04


def patch_scene(src_path: Path, dst_path: Path, width: int, height: int,
                zoom: float, cut_option: int,
                cut_xyz: tuple[float, float, float],
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

    ambsh = data[OFF.bCalcAmbShadowAutomatic]
    ambsh = (ambsh | 0x01) & ~0x80
    data[OFF.bCalcAmbShadowAutomatic] = ambsh

    data[OFF.AODEdithering] = 0

    existing = data[OFF.bCalculateHardShadow] & 0x03
    data[OFF.bCalculateHardShadow] = existing | (SHADOW_MASK_L1_ONLY & 0xFC)

    data[OFF.bCalcDOFtype] = 1

    data[OFF.bCalcSRautomatic] |= 0x01
    struct.pack_into("<B", data, OFF.SRreflectioncount, 1)

    data[OFF.MCoptions] = 0

    # Perspective (not panorama).
    struct.pack_into("<B", data, OFF.bPlanarOptic, 0)

    # --- Phase-3 T8: cut plane ----------------------------------------------
    struct.pack_into("<B", data, OFF.bCutOption, cut_option & 0xFF)
    struct.pack_into("<d", data, OFF.dCutX, float(cut_xyz[0]))
    struct.pack_into("<d", data, OFF.dCutY, float(cut_xyz[1]))
    struct.pack_into("<d", data, OFF.dCutZ, float(cut_xyz[2]))

    dst_path.write_bytes(data)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--scene-dir", type=Path,
                    default=Path("comparison/scenes"),
                    help="Directory containing reference .m3p files")
    ap.add_argument("--out-dir", type=Path,
                    default=Path("comparison/scenes_phase3_cutplanes"),
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
    cut_xyz = (CUT_OFFSET, CUT_OFFSET, CUT_OFFSET)
    for tag, ref_path in refs:
        zooms = FORMULA_ZOOMS.get(tag)
        if zooms is None:
            print(f"error: no zoom triple for formula '{tag}'",
                  file=sys.stderr)
            return 2
        zoom = zooms[1]
        pan = FORMULA_CAMERA_PAN.get(tag)
        for axis_tag, cut_opt in CUT_VARIANTS:
            dst_name = f"phase3_cut_{tag}_{axis_tag}_medium.m3p"
            dst = args.out_dir / dst_name
            pan_str = f" pan=({pan[0]:+.2f},{pan[1]:+.2f})" if pan else ""
            print(f"  {ref_path.name:32s} -> {dst_name:52s} "
                  f"[{IMG_W}x{IMG_H} zoom={zoom} cut=0x{cut_opt:02x}"
                  f" pos=({cut_xyz[0]},{cut_xyz[1]},{cut_xyz[2]}){pan_str}]")
            if not args.dry_run:
                patch_scene(ref_path, dst, IMG_W, IMG_H, zoom, cut_opt,
                            cut_xyz, pan)
            total += 1

    print(f"\nGenerated {total} scene variants in {args.out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
