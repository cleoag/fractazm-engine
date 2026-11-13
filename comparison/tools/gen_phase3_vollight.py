"""Generate 10 Phase-3 .m3p scene variants that exercise volumetric light.

Phase 2 and most prior Phase 3 tasks render with `bVolLightNr = 0` (no
volumetric light). Phase 3 Task 9 turns on the vol-light path driven by
`CalcVolLightMap` + `VolLightMapPos` + `GetVolLightMapVec`.

    Pascal:  CalcSR.pas / CalcThread.pas / CalcMonteCarlo.pas branches
             guarded by `(Header.bVolLightNr and 7) > 0`
             HeaderTrafos.pas builds `MCTparas.DFogOnIt=65535`,
             `VLmul`, `VLstepmul` from the Lamp intensity of the light
             selected by `bVolLightNr & 7`.
    C++:     src/core/reflection.cpp CalcVolLightMap + SR_VolLightMapPos,
             src/core/header_trafos.cpp VLmul/VLstepmul derivation.

Encoding of `bVolLightNr` (uint8 at offset 343):

    bits 0..2   light number (1..6; 0 = vol-light disabled)
    bits 4..7   step-multiplier ((shr 4) - 2 is plugged into
                VLstepmul = sqrt(1 / (1 + 0.1*that_delta))). Pascal uses
                the implicit default `2 << 4` (0x20) so VLstepmul = 1.0.

Phase 3 Task 9 sets `bVolLightNr = 0x21` (light 1, step-mul default 2),
mirroring what FileHandling.pas does for newly-created scenes. Light 1
is already enabled via the Phase-2 `bCalculateHardShadow |= 0x04`
pattern the base scenes carry, so no additional lamp wiring is needed.

One variant per formula (10 formulas x 1 vol-light config = 10 scenes),
medium size (480x360), with all other Phase 2 effect flags (AO, hard
shadow on light 1, DOF, reflections) kept on to exercise vol-light
interaction with the full shading pipeline.

Offsets:
    bVolLightNr  uint8   #343
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

# bVolLightNr encoding: low 3 bits = light number (1..6),
# bits 4..7 = step multiplier delta (Pascal default: 2 -> VLstepmul = 1.0).
VOLLIGHT_VALUE = 0x01 | (2 << 4)   # light 1, default step-mul -> 0x21


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
    bVolLightNr: int = 343
    # TBpos[6] (Pascal 1-indexed) is the dyn-fog gradient byte. Pascal
    # HeaderTrafos.pas:1317 computes sShadGr := (TBpos[6] - 53) * 0.00002
    # when bVolLight is true. TBpos[6]=53 makes sShadGr=0, which zeroes the
    # vol-light pixel-color contribution entirely and makes the sweep
    # oblivious to vol-light bugs. We force TBpos[6]=100 so sShadGr>0 and
    # vol-light actually affects the rendered output.
    TBpos6: int = 452


OFF = OffsetMap()

SHADOW_MASK_L1_ONLY = 0x04


def patch_scene(src_path: Path, dst_path: Path, width: int, height: int,
                zoom: float, vol_light_value: int,
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

    # Light 1 hard shadow on (also wires light 1 as the vol-light source).
    existing = data[OFF.bCalculateHardShadow] & 0x03
    data[OFF.bCalculateHardShadow] = existing | (SHADOW_MASK_L1_ONLY & 0xFC)

    data[OFF.bCalcDOFtype] = 1

    data[OFF.bCalcSRautomatic] |= 0x01
    struct.pack_into("<B", data, OFF.SRreflectioncount, 1)

    data[OFF.MCoptions] = 0

    # Perspective (not panorama).
    struct.pack_into("<B", data, OFF.bPlanarOptic, 0)

    # Keep cut planes off so vol-light is the only Phase-3 delta vs Phase 2.
    struct.pack_into("<B", data, OFF.bCutOption, 0)

    # --- Phase-3 T9: volumetric light ---------------------------------------
    struct.pack_into("<B", data, OFF.bVolLightNr, vol_light_value & 0xFF)
    # Force non-neutral dyn-fog gradient so vol-light actually colors pixels
    # (see field comment on TBpos6 above).
    struct.pack_into("<B", data, OFF.TBpos6, 100)

    dst_path.write_bytes(data)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--scene-dir", type=Path,
                    default=Path("comparison/scenes"),
                    help="Directory containing reference .m3p files")
    ap.add_argument("--out-dir", type=Path,
                    default=Path("comparison/scenes_phase3_vollight"),
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
        dst_name = f"phase3_vl_{tag}_medium.m3p"
        dst = args.out_dir / dst_name
        pan_str = f" pan=({pan[0]:+.2f},{pan[1]:+.2f})" if pan else ""
        print(f"  {ref_path.name:32s} -> {dst_name:44s} "
              f"[{IMG_W}x{IMG_H} zoom={zoom} "
              f"bVolLightNr=0x{VOLLIGHT_VALUE:02X}{pan_str}]")
        if not args.dry_run:
            patch_scene(ref_path, dst, IMG_W, IMG_H, zoom,
                        VOLLIGHT_VALUE, pan)
        total += 1

    print(f"\nGenerated {total} scene variants in {args.out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
