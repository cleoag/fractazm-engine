"""Generate built-in hybrid scenes by combining two different built-in
formulas in the Formulas[0..1] slots of a .m3p file.

The generator takes the camera setup + slot[0] config from one built-in
test scene and the slot[0] config from another, writing them into
Formulas[0] and Formulas[1] of a fresh .m3p. This exercises the
doHybridPas alternating orbit driver across every interesting pairing.

Output: comparison/scenes_builtin/builtin_hybrid_<A>_<B>_<size>.m3p for
each configured (A, B) combination × 3 sizes.
"""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

HEADER_SIZE = 840
ADDON_SIZE = 1136
FORMULA_SIZE = 188
ADDON_HEADER_FIELDS = 8

# (tag, slot0_template, slot0_iter_count, slot1_template, slot1_iter_count, zoom)
HYBRID_COMBOS = [
    ("ip2_quat",     "test_01_intpow2.m3p",    3, "test_05_quaternion.m3p", 2, 1.0),
    ("ip2_tric",     "test_01_intpow2.m3p",    3, "test_06_tricorn.m3p",    2, 1.0),
    ("ip2_fold",     "test_01_intpow2.m3p",    3, "test_09_foldintpow.m3p", 2, 1.0),
    ("abox_tric",    "test_07_amazingbox.m3p", 3, "test_06_tricorn.m3p",    2, 0.7),
    ("abox_fold",    "test_07_amazingbox.m3p", 3, "test_09_foldintpow.m3p", 2, 0.7),
    ("tric_rpow",    "test_06_tricorn.m3p",    3, "test_04_realpower.m3p",  2, 1.0),
]

SIZES = [
    ("small",  240, 180),
    ("medium", 480, 360),
    ("large",  640, 480),
]


def read_formula(path: Path, slot: int) -> bytes:
    data = path.read_bytes()
    addon = len(data) - ADDON_SIZE
    base = addon + ADDON_HEADER_FIELDS + slot * FORMULA_SIZE
    return bytes(data[base : base + FORMULA_SIZE])


def build_hybrid(base_path: Path, slot0_src: Path, slot0_iter: int,
                 slot1_src: Path, slot1_iter: int, zoom: float,
                 width: int, height: int, out: Path) -> None:
    # Start from base scene (provides header + camera + light + addon header).
    data = bytearray(base_path.read_bytes())

    # Patch geometry + determinism flags on the main header.
    struct.pack_into("<i", data, 4, int(width))
    struct.pack_into("<i", data, 8, int(height))
    struct.pack_into("<d", data, 84, float(zoom))

    # Force determinism (same flags as gen_builtin_scenes).
    iOptions = struct.unpack_from("<H", data, 16)[0] & ~0x0001
    struct.pack_into("<H", data, 16, iOptions)
    data[188] = 0                              # AODEdithering
    data[149] &= ~0x01                         # bCalcAmbShadowAutomatic
    data[133] &= 0x03                          # hard shadows upper bits
    data[181] = 0                              # DOF type
    data[336] &= ~0x01                         # reflections auto
    data[124] = 0                              # MCoptions

    # Overwrite Formulas[0..5]: slot0 from slot0_src, slot1 from slot1_src,
    # the rest cleared.
    addon = len(data) - ADDON_SIZE
    formulas_base = addon + ADDON_HEADER_FIELDS

    slot0 = bytearray(read_formula(slot0_src, 0))
    struct.pack_into("<i", slot0, 0, int(slot0_iter))   # iItCount
    data[formulas_base : formulas_base + FORMULA_SIZE] = slot0

    slot1 = bytearray(read_formula(slot1_src, 0))
    struct.pack_into("<i", slot1, 0, int(slot1_iter))
    data[formulas_base + FORMULA_SIZE : formulas_base + 2 * FORMULA_SIZE] = slot1

    # Zero remaining slots (iItCount=0 deactivates).
    for i in range(2, 6):
        off = formulas_base + i * FORMULA_SIZE
        data[off : off + FORMULA_SIZE] = bytes(FORMULA_SIZE)

    out.write_bytes(data)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--scene-dir", type=Path, default=Path("comparison/scenes"))
    ap.add_argument("--out-dir", type=Path,
                    default=Path("comparison/scenes_builtin"))
    # Use test_21 as the structural base — it's a built-in-only hybrid that
    # already has the right addon layout with 2 active slots.
    ap.add_argument("--base", type=Path,
                    default=Path("comparison/scenes/test_21_hybrid.m3p"))
    args = ap.parse_args()

    if not args.base.exists():
        print(f"error: base scene not found: {args.base}", file=sys.stderr)
        return 2
    if not args.scene_dir.is_dir():
        print(f"error: scene dir not found: {args.scene_dir}", file=sys.stderr)
        return 2
    args.out_dir.mkdir(parents=True, exist_ok=True)

    total = 0
    for tag, s0_name, s0_iter, s1_name, s1_iter, zoom in HYBRID_COMBOS:
        s0 = args.scene_dir / s0_name
        s1 = args.scene_dir / s1_name
        if not s0.exists() or not s1.exists():
            print(f"  skip {tag}: missing template(s)", file=sys.stderr)
            continue
        for sz_tag, w, h in SIZES:
            out = args.out_dir / f"builtin_hybrid_{tag}_{sz_tag}.m3p"
            print(f"  {tag:14s} {sz_tag:6s} {w}x{h} zoom={zoom}")
            build_hybrid(args.base, s0, s0_iter, s1, s1_iter, zoom, w, h, out)
            total += 1
    print(f"\nGenerated {total} hybrid scenes under {args.out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
