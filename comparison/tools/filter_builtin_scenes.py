"""Scan a directory of .m3p scenes and classify each as built-in-only or custom-using.

A scene is "built-in-only" when every active formula slot has iFnr < 20.
(Built-in slots often carry a human-readable CustomFname like
"Integer Power" — that field is only meaningful as an external .m3f
reference when iFnr >= 20, matching the C++ parser's convention.)

Minimal layout — tied to src/core/types.h. Only .m3p files with modern MandId
(>=20) and uncompressed layout are understood; older/compressed files are
reported as 'unknown' and skipped.
"""

from __future__ import annotations

import argparse
import json
import struct
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

HEADER_SIZE = 840
ADDON_SIZE = 1136
FORMULA_SIZE = 188
ADDON_HEADER_FIELDS = 8  # bytes before Formulas[]

BUILTIN_NAMES = [
    "IntegerPower", "RealPower", "Quaternion", "Tricorn", "AmazingBox",
    "Bulbox", "FoldingIntPow", "Test7", "Test8", "AexionC",
]


@dataclass
class FormulaSlot:
    iItCount: int
    iFnr: int
    iOptionCount: int
    customName: str  # decoded (non-empty only for external)

    @property
    def active(self) -> bool:
        return self.iItCount > 0

    @property
    def is_builtin(self) -> bool:
        # Inactive slots are trivially fine.
        if not self.active:
            return True
        # iFnr < 20 → one of the 10 Pascal internal formulas. CustomFname on a
        # built-in slot is the display label ("Integer Power") and does not
        # imply an external .m3f file.
        return self.iFnr < 20


@dataclass
class SceneInfo:
    path: Path
    MandId: int
    Width: int
    Height: int
    Iterations: int
    dZoom: float
    iOptions: int
    AODEdithering: int
    bCalcAmbShadowAutomatic: int
    formulas: list[FormulaSlot]
    error: Optional[str] = None

    @property
    def builtin_only(self) -> bool:
        return self.error is None and all(f.is_builtin for f in self.formulas)

    @property
    def formula_ids(self) -> list[int]:
        return [f.iFnr for f in self.formulas if f.active]


def parse_scene(path: Path) -> SceneInfo:
    try:
        data = path.read_bytes()
    except OSError as e:
        return _err(path, f"read error: {e}")

    if len(data) < HEADER_SIZE:
        return _err(path, f"file too small: {len(data)} < {HEADER_SIZE}")

    MandId = struct.unpack_from("<i", data, 0)[0]
    Width = struct.unpack_from("<i", data, 4)[0]
    Height = struct.unpack_from("<i", data, 8)[0]
    Iterations = struct.unpack_from("<i", data, 12)[0]
    iOptions = struct.unpack_from("<H", data, 16)[0]
    dZoom = struct.unpack_from("<d", data, 84)[0]
    bCalcAmbShadowAutomatic = data[149]
    AODEdithering = data[188]

    # Only handle the minimal uncompressed modern layout:
    #   840-byte header + (optional pixel data) + 1136-byte addon.
    addon_offset = HEADER_SIZE
    if len(data) == HEADER_SIZE + ADDON_SIZE:
        pass
    elif len(data) > HEADER_SIZE + ADDON_SIZE:
        # Pixel data sits between header and addon. Addon is at end of file.
        addon_offset = len(data) - ADDON_SIZE
    else:
        return _err(path, f"unexpected size {len(data)} (header-only or truncated)")

    formulas: list[FormulaSlot] = []
    for i in range(6):
        base = addon_offset + ADDON_HEADER_FIELDS + i * FORMULA_SIZE
        if base + FORMULA_SIZE > len(data):
            return _err(path, f"formula slot {i} out of bounds")
        iItCount, iFnr, iOptionCount = struct.unpack_from("<iii", data, base)
        name_raw = data[base + 12 : base + 12 + 32]
        name = name_raw.split(b"\x00", 1)[0].decode("latin-1", errors="replace")
        formulas.append(FormulaSlot(iItCount, iFnr, iOptionCount, name))

    return SceneInfo(
        path=path,
        MandId=MandId,
        Width=Width, Height=Height, Iterations=Iterations, dZoom=dZoom,
        iOptions=iOptions,
        AODEdithering=AODEdithering,
        bCalcAmbShadowAutomatic=bCalcAmbShadowAutomatic,
        formulas=formulas,
    )


def _err(path: Path, msg: str) -> SceneInfo:
    return SceneInfo(path=path, MandId=0, Width=0, Height=0, Iterations=0,
                     dZoom=0.0, iOptions=0, AODEdithering=0,
                     bCalcAmbShadowAutomatic=0, formulas=[], error=msg)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("scene_dir", type=Path, help="Directory with .m3p scenes")
    ap.add_argument("--out-json", type=Path, default=None,
                    help="Write full JSON report here")
    ap.add_argument("--list-builtin", action="store_true",
                    help="Print only built-in-only scene names")
    args = ap.parse_args()

    if not args.scene_dir.is_dir():
        print(f"error: not a directory: {args.scene_dir}", file=sys.stderr)
        return 2

    scenes = sorted(args.scene_dir.glob("*.m3p"))
    if not scenes:
        print(f"error: no .m3p files under {args.scene_dir}", file=sys.stderr)
        return 2

    results = [parse_scene(p) for p in scenes]

    builtin = [r for r in results if r.builtin_only]
    custom = [r for r in results if not r.builtin_only and r.error is None]
    errors = [r for r in results if r.error is not None]

    if args.list_builtin:
        for r in builtin:
            print(r.path.name)
        return 0

    print(f"Total:        {len(results)}")
    print(f"Built-in:     {len(builtin)}")
    print(f"Custom-using: {len(custom)}")
    print(f"Errors:       {len(errors)}")
    print()

    # Histogram of built-in formula ids used across built-in-only scenes.
    hist: dict[int, int] = {}
    for r in builtin:
        for f in r.formulas:
            if f.active and f.is_builtin:
                hist[f.iFnr] = hist.get(f.iFnr, 0) + 1
    print("Built-in formula usage (slot-count across built-in-only scenes):")
    for fnr in sorted(hist):
        name = BUILTIN_NAMES[fnr] if 0 <= fnr < len(BUILTIN_NAMES) else "?"
        print(f"  iFnr={fnr:2d} ({name:<18s}): {hist[fnr]}")

    if args.out_json:
        payload = {
            "summary": {"total": len(results), "builtin": len(builtin),
                        "custom": len(custom), "errors": len(errors)},
            "builtin_scenes": [_to_dict(r) for r in builtin],
            "custom_scenes":  [_to_dict(r) for r in custom],
            "errored_scenes": [_to_dict(r) for r in errors],
        }
        args.out_json.parent.mkdir(parents=True, exist_ok=True)
        args.out_json.write_text(json.dumps(payload, indent=2), encoding="utf-8")
        print(f"\nJSON written: {args.out_json}")

    return 0


def _to_dict(r: SceneInfo) -> dict:
    return {
        "path": str(r.path),
        "name": r.path.name,
        "MandId": r.MandId,
        "Width": r.Width, "Height": r.Height,
        "Iterations": r.Iterations, "dZoom": r.dZoom,
        "iOptions": r.iOptions,
        "AODEdithering": r.AODEdithering,
        "bCalcAmbShadowAutomatic": r.bCalcAmbShadowAutomatic,
        "formulas": [
            {"iItCount": f.iItCount, "iFnr": f.iFnr,
             "iOptionCount": f.iOptionCount, "customName": f.customName}
            for f in r.formulas
        ],
        "builtin_only": r.builtin_only,
        "error": r.error,
    }


if __name__ == "__main__":
    raise SystemExit(main())
