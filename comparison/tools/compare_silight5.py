#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path


HEADER_STRUCT = struct.Struct("<8sIIIII")
RECORD_STRUCT = struct.Struct("<hhhHHHHHH")
FIELD_NAMES = (
    "normal_x",
    "normal_y",
    "normal_z",
    "rough_zpos_fine",
    "zpos",
    "shadow",
    "amb_shadow",
    "si_gradient",
    "o_trap",
)
FIELD_INDEX = {name: idx for idx, name in enumerate(FIELD_NAMES)}


def read_dump(path: Path) -> tuple[int, int, list[tuple[int, ...]]]:
    data = path.read_bytes()
    if len(data) < HEADER_STRUCT.size:
        raise ValueError(f"{path}: too small")
    magic, version, width, height, record_size, count = HEADER_STRUCT.unpack_from(data, 0)
    if magic != b"SIL5DMP1":
        raise ValueError(f"{path}: bad magic {magic!r}")
    if version != 1:
        raise ValueError(f"{path}: unsupported version {version}")
    if record_size != RECORD_STRUCT.size:
        raise ValueError(f"{path}: unexpected record size {record_size}")
    expected = HEADER_STRUCT.size + count * record_size
    if len(data) != expected:
        raise ValueError(f"{path}: size mismatch ({len(data)} != {expected})")
    payload = data[HEADER_STRUCT.size:]
    records = [
        RECORD_STRUCT.unpack_from(payload, offset)
        for offset in range(0, len(payload), RECORD_STRUCT.size)
    ]
    return width, height, records


def packed_z(rec: tuple[int, ...]) -> int | None:
    rough = rec[3]
    zpos = rec[4]
    if zpos > 32767:
        return None
    return (zpos << 8) | ((rough >> 8) & 0xFF)


def first_field_delta(
    lhs: tuple[int, ...],
    rhs: tuple[int, ...],
    ignored: set[int],
) -> dict[str, int] | None:
    for index, (lval, rval) in enumerate(zip(lhs, rhs)):
        if index in ignored:
            continue
        if lval != rval:
            return {
                "field_index": index,
                "field_name": FIELD_NAMES[index],
                "lhs": lval,
                "rhs": rval,
                "delta": rval - lval,
            }
    return None


def compare_pair(lhs_path: Path, rhs_path: Path, ignored: set[int]) -> dict:
    lw, lh, lrecords = read_dump(lhs_path)
    rw, rh, rrecords = read_dump(rhs_path)
    if (lw, lh) != (rw, rh):
        return {
            "status": "SIZE",
            "scene": lhs_path.name,
            "dims": [lw, lh, rw, rh],
        }
    if len(lrecords) != len(rrecords):
        return {
            "status": "COUNT",
            "scene": lhs_path.name,
            "counts": [len(lrecords), len(rrecords)],
        }

    exact_records = 0
    normal_exact = 0
    z_exact = 0
    shadow_exact = 0
    amb_exact = 0
    sig_exact = 0
    otrap_exact = 0
    bg_mask_match = 0
    first_mismatch = None
    max_z_diff = -1
    max_z_xy = (-1, -1)
    z_valid = 0
    z_abs_sum = 0

    for index, (lhs, rhs) in enumerate(zip(lrecords, rrecords)):
        equal = True
        for field_index, (lval, rval) in enumerate(zip(lhs, rhs)):
            if field_index in ignored:
                continue
            if lval != rval:
                equal = False
                break
        if equal:
            exact_records += 1
        else:
            if first_mismatch is None:
                delta = first_field_delta(lhs, rhs, ignored)
                first_mismatch = {
                    "index": index,
                    "x": index % lw,
                    "y": index // lw,
                    "lhs": list(lhs),
                    "rhs": list(rhs),
                    "field": delta,
                }
        if lhs[0:3] == rhs[0:3]:
            normal_exact += 1
        if lhs[5] == rhs[5]:
            shadow_exact += 1
        if lhs[6] == rhs[6]:
            amb_exact += 1
        if lhs[7] == rhs[7]:
            sig_exact += 1
        if lhs[8] == rhs[8]:
            otrap_exact += 1

        lz = packed_z(lhs)
        rz = packed_z(rhs)
        if (lz is None) == (rz is None):
            bg_mask_match += 1
        if lz is not None and rz is not None:
            z_valid += 1
            dz = abs(lz - rz)
            z_abs_sum += dz
            if dz == 0:
                z_exact += 1
            if dz > max_z_diff:
                max_z_diff = dz
                max_z_xy = (index % lw, index // lw)

    count = len(lrecords)
    return {
        "status": "OK",
        "scene": lhs_path.name,
        "width": lw,
        "height": lh,
        "count": count,
        "whole_exact_pct": 100.0 * exact_records / count if count else 100.0,
        "normal_exact_pct": 100.0 * normal_exact / count if count else 100.0,
        "shadow_exact_pct": 100.0 * shadow_exact / count if count else 100.0,
        "amb_exact_pct": 100.0 * amb_exact / count if count else 100.0,
        "sig_exact_pct": 100.0 * sig_exact / count if count else 100.0,
        "otrap_exact_pct": 100.0 * otrap_exact / count if count else 100.0,
        "bg_match_pct": 100.0 * bg_mask_match / count if count else 100.0,
        "z_exact_pct": 100.0 * z_exact / z_valid if z_valid else 100.0,
        "z_mean_abs": z_abs_sum / z_valid if z_valid else 0.0,
        "max_z_diff": max_z_diff if max_z_diff >= 0 else 0,
        "max_z_x": max_z_xy[0],
        "max_z_y": max_z_xy[1],
        "first_mismatch": first_mismatch,
    }


def collect_inputs(path: Path) -> dict[str, Path]:
    if path.is_file():
        return {path.name: path}
    return {item.name: item for item in sorted(path.glob("*.sil5"))}


def main() -> int:
    parser = argparse.ArgumentParser(description="Compare raw siLight5 dumps")
    parser.add_argument("lhs", help="Path to a .sil5 file or directory")
    parser.add_argument("rhs", help="Path to a .sil5 file or directory")
    parser.add_argument("--scene", default="", help="Restrict directory compare to one scene name")
    parser.add_argument("--json-out", default="", help="Optional JSON report path")
    parser.add_argument(
        "--ignore-fields",
        default="",
        help="Comma-separated field names to ignore "
             f"({', '.join(FIELD_NAMES)})",
    )
    args = parser.parse_args()
    ignored = set()
    if args.ignore_fields:
        for raw_name in args.ignore_fields.split(","):
            name = raw_name.strip()
            if not name:
                continue
            if name not in FIELD_INDEX:
                raise ValueError(f"Unknown field name: {name}")
            ignored.add(FIELD_INDEX[name])

    lhs_root = Path(args.lhs)
    rhs_root = Path(args.rhs)
    if lhs_root.is_file() and rhs_root.is_file():
        rows = [compare_pair(lhs_root, rhs_root, ignored)]
    else:
        lhs_files = collect_inputs(lhs_root)
        rhs_files = collect_inputs(rhs_root)
        names = sorted(set(lhs_files) & set(rhs_files))
        if args.scene:
            scene_name = args.scene
            if not scene_name.endswith(".sil5"):
                scene_name += ".sil5"
            names = [scene_name]

        rows = []
        for name in names:
            if name not in lhs_files or name not in rhs_files:
                rows.append({"scene": name, "status": "MISSING"})
                continue
            rows.append(compare_pair(lhs_files[name], rhs_files[name], ignored))

    for row in rows:
        status = row["status"]
        if status != "OK":
            print(f"{row['scene']}: {status}")
            continue
        mismatch = row["first_mismatch"]
        mismatch_text = "none"
        if mismatch is not None and mismatch["field"] is not None:
            field = mismatch["field"]
            mismatch_text = (
                f"{mismatch['x']},{mismatch['y']} {field['field_name']} "
                f"{field['lhs']}->{field['rhs']}"
            )
        print(
            f"{row['scene']}: whole={row['whole_exact_pct']:.3f}% "
            f"z={row['z_exact_pct']:.3f}% normals={row['normal_exact_pct']:.3f}% "
            f"shadow={row['shadow_exact_pct']:.3f}% amb={row['amb_exact_pct']:.3f}% "
            f"sig={row['sig_exact_pct']:.3f}% otrap={row['otrap_exact_pct']:.3f}% "
            f"bg={row['bg_match_pct']:.3f}% max_z={row['max_z_diff']}@{row['max_z_x']},{row['max_z_y']} "
            f"first={mismatch_text}"
        )

    if args.json_out:
        payload = {
            "schema_version": 1,
            "type": "compare_silight5",
            "lhs": str(lhs_root),
            "rhs": str(rhs_root),
            "ignored_fields": [FIELD_NAMES[index] for index in sorted(ignored)],
            "rows": rows,
        }
        out_path = Path(args.json_out)
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
