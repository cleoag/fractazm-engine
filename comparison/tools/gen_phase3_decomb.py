"""Generate 6 Phase-3 .m3p scene variants that exercise Distance Estimate
Combination (DEcomb) mode.

Phase 2 and prior Phase 3 tasks rendered with `bOptions1 = 0`
(alternating hybrid). Phase 3 T13 flips two formulas into DEcomb by
setting `bOptions1 = 2` in the `THeaderCustomAddon`. The per-variant
combination rule is selected by `bOptions3` (the low 3 bits choose
`FormulaType = min(6, max(1, bOptions3 + 1))` in Pascal).

    Pascal: HeaderTrafos.pas:598  bIsDEcomb := (PCFA.bOptions1 and 3) = 2
            HeaderTrafos.pas:599  FormulaType := Min(6, Max(1, bOptions3 + 1))
            Calc.pas:424/440      RestoreF1DEcomb / PrepareF1DEcomb
    C++:    src/io/m3p_parser.cpp:862  if (addon.bOptions1 == 2) ...DEcomb
            src/core/formulas.cpp:2709 OTrapPrepareF1DEcomb / ...Restore...
            src/core/formulas.cpp:2738 CalcOTrapDEfull (DEcomb path)
            src/core/header_trafos.cpp:973 ((PCFA->bOptions1 & 3) == 2)

The addon is a 1136-byte block starting at file offset 840 (after the
840-byte TMandHeader10 disk header, for MId >= 20 with no embedded
pixel data). Relevant byte offsets within the addon:

    +0   bHCAversion  (uint8)
    +1   bOptions1    (uint8)  <- set to 2 for DEcomb
    +2   bOptions2    (uint8)
    +3   bOptions3    (uint8)  <- DEcomb variant bits (0..5)
    +8   Formulas[0]           (188 bytes each)
    +196 Formulas[1]

Each THAformula has its iItCount at offset 0 and iFnr at offset 4
within the 188-byte record.

We build THREE formula pairs (slot-0 IntPow + a different slot-1
formula each time) and TWO bOptions3 variants (0 and 3). That gives
6 scenes at 480x360. We start from `test_21_hybrid.m3p` because it
already has two active formulas (IntPow in slot 0, AmazingBox in
slot 1). Pairs 2 and 3 are constructed by byte-patching slot 1's iFnr.

Offsets:
    bOptions1               uint8   addon +1
    bOptions3               uint8   addon +3
    Formulas[1].iFnr        int32   addon +8 + 188 + 4
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
FORMULA_BASE = ADDON_OFFSET + 8

# Medium render dims (match phase-3 fog, cutplanes, etc).
IMG_W, IMG_H = 480, 360

# Source scene carries IntPow (fnr=0) in slot 0 and AmazingBox (fnr=4)
# in slot 1 with both iItCount > 0, so bOptions1=2 becomes immediately
# valid without further editing.
SRC_SCENE = "test_21_hybrid.m3p"

# Three formula pairs. Pair 0 leaves test_21 alone (IntPow + AmazingBox).
# Pair 1 swaps slot 1 to BulBox (fnr=5). Pair 2 swaps slot 1 to Tricorn
# (fnr=3). Slot 0 always stays as the test_21 IntPow (fnr=0).
FORMULA_PAIRS = [
    # AmazingBox (fnr=4, iDEoption=11) in DEcomb slot 1 paired with IntPow
    # (iDEoption=0) crashes Pascal's CalcOTrapDEfull — analytical-DE vs
    # standard-DE combination is unsupported on the Pascal side. Excluded.
    # ("intpow-amazingbox", 4),
    ("intpow-bulbox",     5),
    ("intpow-tricorn",    3),
]

# Two DEcomb variants. bOptions3 selects the combination rule.
# 0 -> FormulaType=1  (min-of-DE, the canonical "combine" rule)
# 3 -> FormulaType=4  (a non-trivial alternate that still works with
#                      IntPow + Box-family pairs per C++ checks).
DECOMB_VARIANTS = [
    ("v0", 0),
    ("v3", 3),
]

# Phase 2 medium zoom for amazingbox-style (test_21) scenes.
MEDIUM_ZOOM = 1.2


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


def patch_scene(src_path: Path, dst_path: Path,
                slot1_fnr: int, decomb_variant: int) -> None:
    data = bytearray(src_path.read_bytes())

    # Geometry.
    struct.pack_into("<i", data, OFF.Width, int(IMG_W))
    struct.pack_into("<i", data, OFF.Height, int(IMG_H))
    struct.pack_into("<d", data, OFF.dZoom, float(MEDIUM_ZOOM))

    # --- Phase-2 enable pattern (mirrors gen_phase3_fog.py) -----------------
    iOptions = struct.unpack_from("<H", data, OFF.iOptions)[0]
    iOptions &= ~0x0001  # MCT first-step random OFF
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

    # --- Phase-3 T13: DEcomb -------------------------------------------------
    # Enable DEcomb mode (bOptions1 = 2).
    data[ADDON_OFFSET + 1] = 2
    # Pick the DEcomb variant (bOptions3).
    data[ADDON_OFFSET + 3] = int(decomb_variant) & 0xFF
    # Patch slot 1's iFnr only when the pair asks for something different
    # from what test_21 already has (fnr=4). iItCount for slot 1 stays at
    # the test_21 value (2), which keeps both formulas active.
    slot1_fnr_off = FORMULA_BASE + FORMULA_STRIDE + 4
    struct.pack_into("<i", data, slot1_fnr_off, int(slot1_fnr))

    dst_path.write_bytes(data)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--scene-dir", type=Path,
                    default=Path("comparison/scenes"))
    ap.add_argument("--out-dir", type=Path,
                    default=Path("comparison/scenes_phase3_decomb"))
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    src = args.scene_dir / SRC_SCENE
    if not src.exists():
        print(f"error: missing source scene: {src}", file=sys.stderr)
        return 2

    if not args.dry_run:
        args.out_dir.mkdir(parents=True, exist_ok=True)

    total = 0
    for pair_tag, slot1_fnr in FORMULA_PAIRS:
        for variant_tag, decomb_variant in DECOMB_VARIANTS:
            dst_name = f"phase3_decomb_{pair_tag}_{variant_tag}_medium.m3p"
            dst = args.out_dir / dst_name
            print(f"  {SRC_SCENE:32s} -> {dst_name:60s} "
                  f"[{IMG_W}x{IMG_H} zoom={MEDIUM_ZOOM} "
                  f"slot1_fnr={slot1_fnr} bOptions3={decomb_variant}]")
            if not args.dry_run:
                patch_scene(src, dst, slot1_fnr, decomb_variant)
            total += 1

    print(f"\nGenerated {total} scene variants in {args.out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
