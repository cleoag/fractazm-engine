"""Generate 10 Phase-3 .m3p scene variants that exercise the 2D slice renderer.

When `bCalc3D = 0` (offset #344) the renderer abandons 3D ray marching and
instead evaluates the fractal formula on a 2D slice through the scene. The
`bSliceCalc` byte at offset #345 picks which axis/mode of slice:

    bSliceCalc = 1  near slice (Zstart plane)
    bSliceCalc = 2  middle slice (Zmid plane)        <- default
    bSliceCalc = 3  far slice (Zend plane, flipped)

Pascal references:
    TypeDefinitions.pas:799-800   bCalc3D / bSliceCalc header bytes
    HeaderTrafos.pas:557,576      Result.calc3D, Result.iSliceCalc
    HeaderTrafos.pas:890-896      slice z-coordinate selection
    Calc.pas:252,294              dispatches T2DcalcThread.Create/Start when 2D
    CalcThread2D.pas              T2DcalcThread.Execute - 2D slice evaluator
    Mand.pas:1755-1756            UI path that sets bSliceCalc=2 + bCalc3D=0

C++ references:
    src/core/types.h:335-336      bCalc3D / bSliceCalc fields
    src/core/header_trafos.cpp:1227,1249   calc3D / iSliceCalc propagation
    src/core/renderer.cpp:783-814 dispatches Calc2DSliceRows when !calc3D
    src/core/calc_2d.cpp          Calc2DSliceRows implementation

One slice config per formula (10 formulas x 1 slice = 10 scenes), with the
mid-plane (`bSliceCalc = 2`) which is the canonical default used by the
render registrator (RegisterM3Pgraphic.pas:98) and the preview renderer
(PreviewRenderer.pas:123). Medium 480x360 renders.

Effect flags (AO, hard shadow, DOF, reflections, fog) from the phase-2/phase-3
base pattern are left set but most are no-ops for 2D slice -- the 2D renderer
skips shadow/AO passes (see renderer.cpp:812 goto color_pass). This matches
Pascal behavior: Calc.pas only spawns T2DcalcThread when bCalc3D=0, skipping
the 3D shading passes.

Expected outcome: slice images look radically different from the 3D renders
(flat cross-section vs lit surface) -- that is normal. We verify that the
Pascal and C++ slice images match each other, not that they resemble the 3D
reference.

Offsets:
    bCalc3D     uint8   #344
    bSliceCalc  uint8   #345
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

# One canonical slice variant: middle plane (matches Pascal's default init
# in RegisterM3Pgraphic/PreviewRenderer). Additional slice modes (1 near,
# 3 far) can be added later if mid-plane parity passes cleanly.
SLICE_VARIANTS = [
    ("mid", 2),
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
    sTransmissionAbsorption: int = 370
    bCalc3D: int = 344
    bSliceCalc: int = 345


OFF = OffsetMap()

SHADOW_MASK_L1_ONLY = 0x04


def patch_scene(src_path: Path, dst_path: Path, width: int, height: int,
                zoom: float, slice_mode: int,
                pan: tuple[float, float] | None = None) -> None:
    data = bytearray(src_path.read_bytes())

    # Geometry.
    struct.pack_into("<i", data, OFF.Width, int(width))
    struct.pack_into("<i", data, OFF.Height, int(height))
    struct.pack_into("<d", data, OFF.dZoom, float(zoom))
    if pan is not None:
        struct.pack_into("<d", data, OFF.dXmid, float(pan[0]))
        struct.pack_into("<d", data, OFF.dYmid, float(pan[1]))

    # --- Phase-2 enable pattern (mostly inert in 2D slice mode) ------------
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

    # Default fog from phase-3 base (keep the scene self-consistent).
    struct.pack_into("<f", data, OFF.sTransmissionAbsorption, 0.2)

    # --- Phase-3 T11: 2D slice -----------------------------------------------
    # bCalc3D = 0 disables 3D ray march; bSliceCalc = slice_mode selects plane.
    data[OFF.bCalc3D] = 0
    data[OFF.bSliceCalc] = slice_mode & 0xFF

    dst_path.write_bytes(data)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--scene-dir", type=Path,
                    default=Path("comparison/scenes"),
                    help="Directory containing reference .m3p files")
    ap.add_argument("--out-dir", type=Path,
                    default=Path("comparison/scenes_phase3_slice"),
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
            print(f"error: no zoom triple for formula '{tag}'",
                  file=sys.stderr)
            return 2
        zoom = zooms[1]
        pan = FORMULA_CAMERA_PAN.get(tag)
        for slice_tag, slice_mode in SLICE_VARIANTS:
            dst_name = f"phase3_slice_{tag}_medium.m3p"
            dst = args.out_dir / dst_name
            pan_str = f" pan=({pan[0]:+.2f},{pan[1]:+.2f})" if pan else ""
            print(f"  {ref_path.name:32s} -> {dst_name:52s} "
                  f"[{IMG_W}x{IMG_H} zoom={zoom} slice={slice_tag}({slice_mode}){pan_str}]")
            if not args.dry_run:
                patch_scene(ref_path, dst, IMG_W, IMG_H, zoom, slice_mode, pan)
            total += 1

    print(f"\nGenerated {total} scene variants in {args.out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
