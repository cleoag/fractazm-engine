"""Generate 6 Phase-3 .m3p scene variants that exercise IFS (Iterated
Function System) mode.

Two trigger paths exist in the Pascal reference (and the C++ port):

1.  ``Formulas[0].iFnr = 8`` -- testIFS, a built-in single-formula
    IFS (``HybridCustomIFStest`` on the C++ side). The formula itself
    implements an IFS evaluator (PlaneIFS with orbit-trap map colouring
    in the active branch) and drives the iteration loop directly.

2.  ``THeaderCustomAddon.bOptions1 = 3`` -- global IFS mode. When this
    flag is set the iteration loop uses ``doHybridIFS3D`` (Pascal:
    formulas.pas:24; C++: src/core/formulas.cpp:2437) which tracks the
    minimum DE/VaryScale ratio across all iterations and can be paired
    with any built-in slot formula.

Pascal references:
    formulas.pas:24              doHybridIFS3D
    formulas.pas:25              doHybridIFS3DnoVecIni
    formulas.pas:132..           testIFSOption* constants
    CustomFormulas.pas:79        InternFNames has '..testIFS..' at fnr 8
    CustomFormulas.pas:131       (pHCA.bOptions1 and 3) = 1
    CustomFormulas.pas:144       (pHCA.bOptions1 and 3) = 0 / 2 / 3 branches

C++ references:
    src/core/formula_registry.cpp:17   "testIFS" at slot 8
    src/core/formula_registry.cpp:57   returns HybridCustomIFStest
    src/core/formulas.cpp:1328..1332   HybridCustomIFStest definition
    src/core/formulas.cpp:2421..2496   doHybridIFS3D implementation

We take three built-in base formulas (IntPow, AmazingBox, Quaternion)
and emit two variants each:

    _ifs8        Formulas[0].iFnr := 8   (testIFS slot formula)
    _globalifs   bOptions1       := 3   (global IFS, slot formula kept)

That gives 3 x 2 = 6 scenes at 480x360. We start from the same Phase-3
reference scenes (test_01_intpow2.m3p, test_21_hybrid.m3p,
test_05_quaternion.m3p) so lighting / AO / DOF / SR toggles match the
rest of Phase 3's medium sweeps.

Offsets (THeaderCustomAddon starts at byte 840 of the .m3p file for
MId >= 20 with no embedded pixel data):

    bOptions1               uint8   addon +1
    Formulas[0].iFnr        int32   addon +8 + 4
"""

from __future__ import annotations

import argparse
import struct
import sys
from dataclasses import dataclass
from pathlib import Path

# Addon starts at file offset 840 (sizeof(TMandHeader10) for MId >= 20,
# scenes in comparison/scenes have no embedded pixel data).
ADDON_OFFSET = 840
FORMULA_STRIDE = 188
FORMULA_BASE = ADDON_OFFSET + 8  # Formulas[0] record start
FORMULA0_IFNR_OFF = FORMULA_BASE + 4  # int32 iFnr within Formulas[0]
BOPTIONS1_OFF = ADDON_OFFSET + 1

# Medium render dims (match phase-3 fog, decomb, cutplanes, etc).
IMG_W, IMG_H = 480, 360

# Three built-in base formulas. Each references a Phase-3 base scene
# with sensible zoom / camera defaults for that fractal type. iFnr values
# are <= 9 (built-in only). testIFS (iFnr=8) intentionally NOT used as a
# base -- we flip into it via the `_ifs8` variant.
BASES: list[tuple[str, str, float, tuple[float, float] | None]] = [
    # (tag,           source_scene,              zoom, pan)
    ("intpow2",       "test_01_intpow2.m3p",      1.2, None),
    ("amazingbox",    "test_21_hybrid.m3p",       1.2, None),
    ("quaternion",    "test_05_quaternion.m3p", 18.0, (-0.35, 0.30)),
]

# Two IFS trigger variants.
#   "_ifs8"     : Formulas[0].iFnr = 8 (testIFS built-in slot)
#   "_globalifs": bOptions1 = 3 (global IFS mode, original slot formula kept)
IFS_VARIANTS: list[tuple[str, str]] = [
    ("ifs8",       "slot"),
    ("globalifs",  "global"),
]

# (base_tag, variant_tag) combinations that Pascal refuses to render.
# testIFS (iFnr=8, iDEoption=20=dIFS) cannot mix with AmazingBox slot 1
# (iDEoption=11) per CheckFormulaOptions in HeaderTrafos.pas:453-458.
EXCLUDED: set[tuple[str, str]] = {
    ("amazingbox", "ifs8"),
}


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


def patch_scene(src_path: Path, dst_path: Path, zoom: float,
                pan: tuple[float, float] | None, mode: str) -> None:
    data = bytearray(src_path.read_bytes())

    # Geometry.
    struct.pack_into("<i", data, OFF.Width, int(IMG_W))
    struct.pack_into("<i", data, OFF.Height, int(IMG_H))
    struct.pack_into("<d", data, OFF.dZoom, float(zoom))
    if pan is not None:
        struct.pack_into("<d", data, OFF.dXmid, float(pan[0]))
        struct.pack_into("<d", data, OFF.dYmid, float(pan[1]))

    # --- Phase-2 enable pattern (mirrors gen_phase3_fog.py etc) -------------
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
    struct.pack_into("<B", data, OFF.bPlanarOptic, 0)

    # --- Phase-3 T15: IFS trigger -------------------------------------------
    if mode == "slot":
        # testIFS via slot-0 formula override. Keep bOptions1 at whatever
        # the source scene had (typically 0 / alternating hybrid).
        struct.pack_into("<i", data, FORMULA0_IFNR_OFF, 8)
    elif mode == "global":
        # Global IFS mode. Keep Formulas[0].iFnr as-is from the source
        # so the base formula drives the iteration, but toggle bOptions1.
        data[BOPTIONS1_OFF] = 3
    else:
        raise ValueError(f"unknown IFS mode: {mode!r}")

    dst_path.write_bytes(data)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--scene-dir", type=Path,
                    default=Path("comparison/scenes"),
                    help="Directory containing reference .m3p files")
    ap.add_argument("--out-dir", type=Path,
                    default=Path("comparison/scenes_phase3_ifs"),
                    help="Destination directory for generated variants")
    ap.add_argument("--dry-run", action="store_true",
                    help="Just print what would be generated")
    args = ap.parse_args()

    if not args.scene_dir.is_dir():
        print(f"error: scene dir not found: {args.scene_dir}", file=sys.stderr)
        return 2

    # Safety belt: refuse to emit a scene whose slot-0 iFnr >= 20 would
    # sneak in an external formula -- this generator is strictly
    # built-in-only (iFnr in 0..9).
    for _, src_name, _, _ in BASES:
        p = args.scene_dir / src_name
        if not p.exists():
            print(f"error: missing base scene: {p}", file=sys.stderr)
            return 2
        base_bytes = p.read_bytes()
        if len(base_bytes) < FORMULA0_IFNR_OFF + 4:
            print(f"error: base scene too small: {p}", file=sys.stderr)
            return 2
        (src_ifnr,) = struct.unpack_from("<i", base_bytes, FORMULA0_IFNR_OFF)
        if src_ifnr >= 20:
            print(f"error: base scene {src_name} has external slot-0 "
                  f"iFnr={src_ifnr} (must be <20)", file=sys.stderr)
            return 2

    if not args.dry_run:
        args.out_dir.mkdir(parents=True, exist_ok=True)

    total = 0
    skipped = 0
    for tag, src_name, zoom, pan in BASES:
        src = args.scene_dir / src_name
        for variant_tag, mode in IFS_VARIANTS:
            if (tag, variant_tag) in EXCLUDED:
                print(f"  SKIP {tag}_{variant_tag} (excluded — Pascal "
                      f"refuses this combination)")
                skipped += 1
                continue
            dst_name = f"phase3_ifs_{tag}_{variant_tag}_medium.m3p"
            dst = args.out_dir / dst_name
            pan_str = f" pan=({pan[0]:+.2f},{pan[1]:+.2f})" if pan else ""
            print(f"  {src_name:32s} -> {dst_name:60s} "
                  f"[{IMG_W}x{IMG_H} zoom={zoom} mode={mode}{pan_str}]")
            if not args.dry_run:
                patch_scene(src, dst, zoom, pan, mode)
            total += 1

    print(f"\nGenerated {total} scene variants in {args.out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
