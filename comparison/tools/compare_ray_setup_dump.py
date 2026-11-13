#!/usr/bin/env python3
from __future__ import annotations

import argparse
import ctypes
import math
import struct
import sys
from pathlib import Path


HEADER_STRUCT = struct.Struct("<8sIIIII")
DEFAULT_ABS_TOL = 2e-11
IGNORED_FIELDS = frozenset()


class RaySetupDumpRecord(ctypes.LittleEndianStructure):
    _fields_ = [
        ("x", ctypes.c_int32),
        ("y", ctypes.c_int32),
        ("thread_id", ctypes.c_int32),
        ("seed_before_first_step", ctypes.c_int32),
        ("camera_optic", ctypes.c_int32),
        ("cut_options", ctypes.c_int32),
        ("bin_steps", ctypes.c_int32),
        ("ray_ready_captured", ctypes.c_int32),
        ("calcde_enter_captured", ctypes.c_int32),
        ("calcde_retry_index", ctypes.c_int32),
        ("inside_rendering", ctypes.c_int32),
        ("calc_inside", ctypes.c_int32),
        ("in_and_out", ctypes.c_int32),
        ("calcde_inside_rendering", ctypes.c_int32),
        ("calcde_calc_inside", ctypes.c_int32),
        ("calc_sit", ctypes.c_int32),
        ("it_result", ctypes.c_int32),
        ("max_it", ctypes.c_int32),
        ("cut_plane_hit", ctypes.c_int32),
        ("vary_destop", ctypes.c_int32),
        ("cafx", ctypes.c_double),
        ("cafy", ctypes.c_double),
        ("fovx_off", ctypes.c_double),
        ("fovx_mul", ctypes.c_double),
        ("fovy", ctypes.c_double),
        ("step_width", ctypes.c_double),
        ("de_stop", ctypes.c_double),
        ("ray_ms_destop", ctypes.c_double),
        ("ms_destop", ctypes.c_double),
        ("ray_mzz", ctypes.c_double),
        ("mzz", ctypes.c_double),
        ("zend", ctypes.c_double),
        ("start_destop", ctypes.c_double),
        ("max_ray_length", ctypes.c_double),
        ("zzpos_mul", ctypes.c_double),
        ("de_mul_vary", ctypes.c_double),
        ("de_mul_const", ctypes.c_double),
        ("smooth_it", ctypes.c_double),
        ("rout", ctypes.c_double),
        ("otrap", ctypes.c_double),
        ("r_stop", ctypes.c_double),
        ("ddescale", ctypes.c_double),
        ("mct_de_offset", ctypes.c_double),
        ("mct_de_offset006", ctypes.c_double),
        ("mvgrads_fov", ctypes.c_double * 3),
        ("view_vec", ctypes.c_double * 3),
        ("march_vec", ctypes.c_double * 3),
        ("ray_ready_start", ctypes.c_double * 3),
        ("calcde_enter_pos", ctypes.c_double * 3),
        ("act_pos", ctypes.c_double * 3),
        ("ystart", ctypes.c_double * 3),
        ("vgrads0", ctypes.c_double * 3),
        ("vgrads1", ctypes.c_double * 3),
    ]


def load_dump(path: Path):
    raw = path.read_bytes()
    if len(raw) < HEADER_STRUCT.size:
        raise SystemExit(f"{path}: file too small")
    magic, version, width, height, record_size, count = HEADER_STRUCT.unpack_from(raw, 0)
    if magic != b"RAYSETP1":
        raise SystemExit(f"{path}: bad magic {magic!r}")
    if version != 1:
        raise SystemExit(f"{path}: unsupported version {version}")
    if record_size != ctypes.sizeof(RaySetupDumpRecord):
        raise SystemExit(
            f"{path}: record size mismatch file={record_size} parser={ctypes.sizeof(RaySetupDumpRecord)}"
        )
    payload = raw[HEADER_STRUCT.size :]
    expected = record_size * count
    if len(payload) != expected:
        raise SystemExit(f"{path}: payload size mismatch expected={expected} actual={len(payload)}")
    return {
        "path": path,
        "raw": raw,
        "width": width,
        "height": height,
        "record_size": record_size,
        "count": count,
        "payload": payload,
    }


def record_at(payload: bytes, index: int) -> RaySetupDumpRecord:
    offset = index * ctypes.sizeof(RaySetupDumpRecord)
    return RaySetupDumpRecord.from_buffer_copy(payload[offset : offset + ctypes.sizeof(RaySetupDumpRecord)])


def flatten_value(value):
    if isinstance(value, ctypes.Array):
        return list(value)
    return value


def floats_equal(left: float, right: float, abs_tol: float) -> bool:
    if math.isnan(left) and math.isnan(right):
        return True
    return math.isclose(left, right, rel_tol=0.0, abs_tol=abs_tol)


def values_equal(left, right, abs_tol: float) -> bool:
    if isinstance(left, list) and isinstance(right, list):
        if len(left) != len(right):
            return False
        return all(values_equal(lv, rv, abs_tol) for lv, rv in zip(left, right))
    if isinstance(left, float) or isinstance(right, float):
        return floats_equal(float(left), float(right), abs_tol)
    return left == right


def diff_fields(left: RaySetupDumpRecord, right: RaySetupDumpRecord, abs_tol: float):
    diffs = []
    for field_name, _field_type in left._fields_:
        if field_name in IGNORED_FIELDS:
            continue
        lv = flatten_value(getattr(left, field_name))
        rv = flatten_value(getattr(right, field_name))
        if not values_equal(lv, rv, abs_tol):
            diffs.append((field_name, lv, rv))
    return diffs


def main() -> int:
    parser = argparse.ArgumentParser(description="Compare part-1 ray setup dumps and report the first mismatch.")
    parser.add_argument("left")
    parser.add_argument("right")
    parser.add_argument("--limit", type=int, default=20, help="max differing fields to print for the first bad pixel")
    parser.add_argument(
        "--abs-tol",
        type=float,
        default=DEFAULT_ABS_TOL,
        help=f"absolute tolerance for float fields (default: {DEFAULT_ABS_TOL})",
    )
    args = parser.parse_args()

    left = load_dump(Path(args.left))
    right = load_dump(Path(args.right))

    header_keys = ("width", "height", "record_size", "count")
    header_mismatch = [(k, left[k], right[k]) for k in header_keys if left[k] != right[k]]
    if header_mismatch:
        for key, lv, rv in header_mismatch:
            print(f"header mismatch: {key} left={lv} right={rv}")
        return 1

    if left["raw"] == right["raw"]:
        print(
            f"match: {left['count']} records ({left['width']}x{left['height']}) record_size={left['record_size']}"
        )
        return 0

    record_size = left["record_size"]
    count = left["count"]
    for index in range(count):
        lo = index * record_size
        hi = lo + record_size
        if left["payload"][lo:hi] == right["payload"][lo:hi]:
            continue
        lrec = record_at(left["payload"], index)
        rrec = record_at(right["payload"], index)
        diffs = diff_fields(lrec, rrec, args.abs_tol)
        if not diffs:
            continue
        print(f"record mismatch: index={index} pixel=({lrec.x},{lrec.y})")
        for field_name, lv, rv in diffs[: args.limit]:
            print(f"  {field_name}: left={lv} right={rv}")
        remaining = len(diffs) - args.limit
        if remaining > 0:
            print(f"  ... {remaining} more differing fields")
        return 1

    message = (
        "match: "
        f"{left['count']} records ({left['width']}x{left['height']}) "
        f"record_size={left['record_size']} abs_tol={args.abs_tol}"
    )
    if IGNORED_FIELDS:
        message += f" ignored={','.join(sorted(IGNORED_FIELDS))}"
    print(message)
    return 0


if __name__ == "__main__":
    sys.exit(main())
