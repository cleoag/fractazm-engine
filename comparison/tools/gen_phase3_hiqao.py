"""Generate 30 Phase-3 .m3p scene variants that exercise HiQ AO variants.

Phase 2 only exercised HiQ AO with ATi=2 (regular TAmbHiQCalc) because the
default test scene template `bCalcAmbShadowAutomatic = 0x15` -> bit 0 (AO on),
bits 1-3 = 010 (ATi=2), bits 4-5 = 01 (quality 1). This task (Phase 3 T12)
covers the three remaining non-pano HiQ variants:

    ATi=3  TAmbHiQCalcT0       (HiQ threshold)
    ATi=4  TAmbHiQCalcR        (HiQ random)
    ATi=5  TAmbHiQCalcRT0      (HiQ random threshold)

Pascal dispatch (AmbHiQ.pas:151):
    ATi := (Header.bCalcAmbShadowAutomatic shr 1) and 7;
    //  2: Calc   3: CalcT0   4: CalcR   5: CalcRT0   6(->10): SSAOvideo

C++ dispatch (src/core/ambient_occlusion.cpp:3088):
    int ATi = ((header->bCalcAmbShadowAutomatic >> 1) & 7);
    case 2  -> CalcAO_HiQ
    case 3  -> CalcAO_HiQ_Threshold
    case 4  -> CalcAO_HiQ_Random
    case 5  -> CalcAO_HiQ_RandomThreshold

Byte layout of bCalcAmbShadowAutomatic (offset 149):
    bit 0       AO enable (keep = 1)
    bits 1-3    ATi mode (0..7) -- we set 3, 4, or 5 here
    bits 4-5    quality (0..3)  -- preserved from template (default 1)
    bit 7       FSR (keep = 0)

Note: bits 2 AND 3 both set (i.e., byte & 12 == 12) triggers the DE-AO
dispatch path and bypasses the ATi switch. None of {ATi=3,4,5} hit that
mask because ATi=3 has bits 1,2 set (not 2,3); ATi=5 has bits 1,3 set.

Phase 2 shading flags (single hard shadow, DOF, reflections) remain ON so
the AO pass flows through the full shading pipeline. Medium 480x360.

Offsets:
    bCalcAmbShadowAutomatic  byte  #149
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

# Three HiQ AO variants to exercise (ATi value, short tag).
AO_MODES = [
    (3, "threshold"),  # TAmbHiQCalcT0
    (4, "random"),     # TAmbHiQCalcR
    (5, "variant3"),   # TAmbHiQCalcRT0 (random threshold)
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


OFF = OffsetMap()

SHADOW_MASK_L1_ONLY = 0x04


def patch_scene(src_path: Path, dst_path: Path, width: int, height: int,
                zoom: float, ao_ati: int,
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

    # HiQ AO dispatch: set ATi (bits 1-3), AO enable (bit 0), clear FSR (bit 7).
    # Preserve quality bits (4-5) from the template.
    ambsh = data[OFF.bCalcAmbShadowAutomatic]
    ambsh &= ~0x0E                   # clear ATi bits 1-3
    ambsh |= (ao_ati << 1) & 0x0E    # set requested ATi
    ambsh |= 0x01                    # AO enable on
    ambsh &= ~0x80                   # FSR off
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

    dst_path.write_bytes(data)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--scene-dir", type=Path,
                    default=Path("comparison/scenes"),
                    help="Directory containing reference .m3p files")
    ap.add_argument("--out-dir", type=Path,
                    default=Path("comparison/scenes_phase3_hiqao"),
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
        for ati, mode_tag in AO_MODES:
            dst_name = f"phase3_hiqao_{tag}_{mode_tag}_medium.m3p"
            dst = args.out_dir / dst_name
            pan_str = f" pan=({pan[0]:+.2f},{pan[1]:+.2f})" if pan else ""
            print(f"  {ref_path.name:32s} -> {dst_name:52s} "
                  f"[{IMG_W}x{IMG_H} zoom={zoom} ATi={ati}{pan_str}]")
            if not args.dry_run:
                patch_scene(ref_path, dst, IMG_W, IMG_H, zoom, ati, pan)
            total += 1

    print(f"\nGenerated {total} scene variants in {args.out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
