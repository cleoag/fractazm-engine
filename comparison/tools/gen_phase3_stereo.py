"""Generate Phase-3 .m3p scene variants that exercise stereo modes.

Phase 2 and earlier Phase 3 tasks verified parity with `bStereoMode = 0`
(the template default: non-stereo monocular). Phase 3 Task 6 widens
coverage by exercising the two structurally distinct stereo codepaths in
HeaderTrafos:

    mode 1  "left eye"  standard stereo: CalcXoff shifts screen-X by
            Xoff = -0.065/StereoScreenWidth; StereoChange offsets
            midPos along hVGrads by eyedist * (SSD - SMD) / SSD.
            (Modes 3/4 are the mirrored right-eye variants of mode 1
            and share the same code path, so they are redundant here.)

    mode 2  "VR fisheye": CalcCamPos forces VPoff = 0 and the render
            treats dFOVy effectively as 180deg. No camera X offset.
            This is a visually and numerically different path from
            modes 1/3/4, so it deserves its own coverage.

Per-formula medium zoom from Phase 2 is reused; medium size 480x360.
All Phase 2 effect flags (AO, single hard shadow, DOF, reflections)
remain enabled. Stereo params StereoScreenWidth / StereoScreenDistance /
StereoMinDistance already hold the Pascal defaults (1.0, 2.0, 0.5/1.5)
in the test_*.m3p templates; we leave them alone so any future re-gen
picks up whatever the template ships with.

Scenes are written to `comparison/scenes_phase3_stereo/` with a
`phase3_stereo_<formula>_mode<N>_medium.m3p` filename prefix.

Offsets:
    bStereoMode          uint8  #126
    StereoScreenWidth    float  #230
    StereoScreenDistance float  #234
    StereoMinDistance    float  #238
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

MEDIUM_W, MEDIUM_H = 480, 360

# Two representative stereo modes (see module docstring).
STEREO_MODES = [1, 2]

# Sensible Pascal defaults (match FileHandling.pas lines 180-182 for a
# header with no explicit stereo config).
STEREO_SCREEN_WIDTH_DEFAULT = 1.0
STEREO_SCREEN_DISTANCE_DEFAULT = 2.0
STEREO_MIN_DISTANCE_DEFAULT = 0.5


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
    bStereoMode: int = 126
    StereoScreenWidth: int = 230
    StereoScreenDistance: int = 234
    StereoMinDistance: int = 238


OFF = OffsetMap()

SHADOW_MASK_L1_ONLY = 0x04


def patch_scene(src_path: Path, dst_path: Path, width: int, height: int,
                zoom: float, stereo_mode: int,
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

    # --- Phase-3 T6: Stereo mode --------------------------------------------
    struct.pack_into("<B", data, OFF.bStereoMode, int(stereo_mode))

    # Backfill sensible Pascal defaults only if the template left them at 0.
    # That way we do not clobber a meaningfully-configured scene, but we
    # still guarantee the Pascal/C++ eyedist math has finite denominators.
    ssw = struct.unpack_from("<f", data, OFF.StereoScreenWidth)[0]
    if ssw <= 0.0:
        struct.pack_into("<f", data, OFF.StereoScreenWidth,
                         STEREO_SCREEN_WIDTH_DEFAULT)
    ssd = struct.unpack_from("<f", data, OFF.StereoScreenDistance)[0]
    if ssd <= 0.0:
        struct.pack_into("<f", data, OFF.StereoScreenDistance,
                         STEREO_SCREEN_DISTANCE_DEFAULT)
    smd = struct.unpack_from("<f", data, OFF.StereoMinDistance)[0]
    if smd <= 0.0:
        struct.pack_into("<f", data, OFF.StereoMinDistance,
                         STEREO_MIN_DISTANCE_DEFAULT)

    dst_path.write_bytes(data)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--scene-dir", type=Path,
                    default=Path("comparison/scenes"),
                    help="Directory containing reference .m3p files")
    ap.add_argument("--out-dir", type=Path,
                    default=Path("comparison/scenes_phase3_stereo"),
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
    for mode in STEREO_MODES:
        for tag, ref_path in refs:
            zooms = FORMULA_ZOOMS.get(tag)
            if zooms is None:
                print(f"error: no zoom triple for formula '{tag}'",
                      file=sys.stderr)
                return 2
            zoom = zooms[1]
            pan = FORMULA_CAMERA_PAN.get(tag)
            dst_name = f"phase3_stereo_{tag}_mode{mode}_medium.m3p"
            dst = args.out_dir / dst_name
            pan_str = f" pan=({pan[0]:+.2f},{pan[1]:+.2f})" if pan else ""
            print(f"  {ref_path.name:32s} -> {dst_name:48s} "
                  f"[{MEDIUM_W}x{MEDIUM_H} zoom={zoom} stereo={mode}"
                  f"{pan_str}]")
            if not args.dry_run:
                patch_scene(ref_path, dst, MEDIUM_W, MEDIUM_H, zoom,
                            mode, pan)
            total += 1

    print(f"\nGenerated {total} scene variants in {args.out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
