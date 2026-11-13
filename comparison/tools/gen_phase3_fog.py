"""Generate 20 Phase-3 .m3p scene variants that exercise atmospheric fog.

Phase 2 and prior Phase 3 tasks rendered with a default
`sTransmissionAbsorption = 0.2` (float, offset 370). That coefficient drives
the transmission-absorption decay along reflection rays in the SR path and
feeds the colorizer absorption pow via `sAbsorpCoeff`:

    Pascal:  HeaderTrafos.pas:1246   sAbsorpCoeff := Header.sTransmissionAbsorption
             CalcSR.pas:133          SRCalcThread[x].sAbsorption := Header.sTransmissionAbsorption * Header.dStepWidth
             CalcMonteCarlo.pas:237  MCThread[x-1].sAbsorption  := Header.sTransmissionAbsorption * Header.dStepWidth
             PaintThread.pas:1485    LiLSDAI[4] += ... SVecPow(inSD[1], dTmp * sAbsorpCoeff) ...
    C++:     src/core/header_trafos.cpp  Lvals->sAbsorpCoeff = Header->sTransmissionAbsorption
             src/core/reflection.cpp:1026/1122 st.sAbsorption = header->sTransmissionAbsorption * dStepWidth
             src/core/colorizer.cpp:2300   SVecPow(absorpPow, InSD[1], dTmp * PLVals->sAbsorpCoeff)

Two intensity variants per formula (10 formulas x 2 intensities = 20 scenes):
    _low   sTransmissionAbsorption = 0.01   (very thin medium)
    _high  sTransmissionAbsorption = 0.05   (denser atmospheric haze)

We deliberately pick values both BELOW the 0.2 default so the absorption
term dominates less of the final color (Pascal renders are already tuned at
0.2), giving headroom to observe drift sensitivity. Medium 480x360 renders.
Phase 2 effect flags (AO, single hard shadow, DOF, reflections) remain
enabled so fog is exercised through the full shading pipeline.

Offsets:
    sTransmissionAbsorption  float32  #370
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

# Two fog intensities (see module docstring).
FOG_VARIANTS = [
    ("low",  0.01),
    ("high", 0.05),
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


OFF = OffsetMap()

SHADOW_MASK_L1_ONLY = 0x04


def patch_scene(src_path: Path, dst_path: Path, width: int, height: int,
                zoom: float, fog_value: float,
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

    # --- Phase-3 T10: atmospheric fog ---------------------------------------
    struct.pack_into("<f", data, OFF.sTransmissionAbsorption, float(fog_value))

    dst_path.write_bytes(data)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--scene-dir", type=Path,
                    default=Path("comparison/scenes"),
                    help="Directory containing reference .m3p files")
    ap.add_argument("--out-dir", type=Path,
                    default=Path("comparison/scenes_phase3_fog"),
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
        for intensity_tag, fog_value in FOG_VARIANTS:
            dst_name = f"phase3_fog_{tag}_{intensity_tag}_medium.m3p"
            dst = args.out_dir / dst_name
            pan_str = f" pan=({pan[0]:+.2f},{pan[1]:+.2f})" if pan else ""
            print(f"  {ref_path.name:32s} -> {dst_name:52s} "
                  f"[{IMG_W}x{IMG_H} zoom={zoom} fog={fog_value}{pan_str}]")
            if not args.dry_run:
                patch_scene(ref_path, dst, IMG_W, IMG_H, zoom, fog_value, pan)
            total += 1

    print(f"\nGenerated {total} scene variants in {args.out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
