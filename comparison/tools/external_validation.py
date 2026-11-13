"""Reproducible external-formula inventory and image qualification.

An image PASS is deliberately separate from useful geometry and formula
influence. This manifest is an inventory of obligations, not a claim that a
finite selection exhausts continuous parameters or all six-slot hybrids.
"""
from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
import os
import tempfile
import time
from pathlib import Path
import re
import struct
import subprocess
import sys
import zlib

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))
from gen_bulk_formula_table import parse_m3f_options
from formula_coverage import inventory


def digest(data):
    return hashlib.sha256(data).hexdigest()


def decode(path):
    data = path.read_bytes()
    try:
        data = zlib.decompress(data)
    except zlib.error:
        pass
    if len(data) != 1976 or not 20 <= struct.unpack_from("<i", data)[0] <= 44:
        raise ValueError("unsupported layout; requires native loader migration")
    return bytearray(data)


def slots(data):
    result = []
    for i in range(6):
        offset = 848 + i * 188
        count, number, options = struct.unpack_from("<iii", data, offset)
        if count <= 0:
            continue
        result.append({"slot": i, "iterations": count, "number": number,
                       "name": bytes(data[offset + 12:offset + 44]).split(b"\0")[0].decode("latin1"),
                       "option_count": options,
                       "options": list(struct.unpack_from("<16d", data, offset + 60))})
    return result


def saved_option_types(metadata):
    """Pascal VALUE-index schema, including compound angle payloads."""
    widths = {5: 3, 6: 3, 12: 6, 18: 4, 19: 2}
    types = [6 if kind == 18 and part else kind
             for kind in metadata['option_types']
             for part in range(widths.get(kind, 1))]
    if len(types) != len(metadata['option_defaults']) or len(types) > 16:
        raise ValueError('Invalid expanded formula option schema')
    return types


def prepare_external_options(data, slot, metadata=None):
    """Materialize effective defaults BEFORE changing options in a new scene.

    Returning a current full-count slot prevents Ini1CFfromHAddon from silently
    replacing a requested selector with a default. Source bytes on disk remain
    untouched. The two historical migrations follow HeaderTrafos.pas; this
    helper is input preparation, not independent proof of the native loader.
    """
    if slot['number'] < 20:
        raise ValueError('Expected an external formula slot')
    if metadata is None:
        metadata = parse_m3f_options(ROOT / 'mb3d/M3Formulas' / (slot['name'] + '.m3f'))
    types = saved_option_types(metadata)
    values = slot['options'][:]
    loaded = max(0, min(16, slot['option_count']))
    if loaded < len(types):
        name = slot['name'].upper()
        if name == 'CYLINDERIFS' and loaded < 9:
            values[8], values[7] = values[7], 0.
            if loaded < 8:
                values[6] = 0.
            loaded = 9
        elif name == 'SPHEREHEIGHTMAP':
            for i in range(len(types) - 1, 8, -1):
                values[i] = values[i - 1]
            values[8] = 1.
            def not_zero(v):
                return v if abs(v) > 1e-16 else v * .5 + (-.5e-16 if v < 0 else .5e-16)
            values[2] /= not_zero(values[5])
            values[3] /= not_zero(values[6])
            loaded = len(types)
        values[loaded:len(types)] = metadata['option_defaults'][loaded:]
    offset = 848 + slot['slot'] * 188
    struct.pack_into('<i', data, offset + 8, len(types))
    data[offset + 44:offset + 60] = bytes(types).ljust(16, b'\0')
    struct.pack_into('<16d', data, offset + 60, *values)
    return {**slot, 'option_count': len(types), 'options': values}


def finite_json(value):
    # Scene corruption/nonfinite values must not become invalid JSON.
    import math
    if isinstance(value, float) and not math.isfinite(value):
        return str(value)
    if isinstance(value, dict):
        return {k: finite_json(v) for k, v in value.items()}
    if isinstance(value, (list, tuple)):
        return [finite_json(v) for v in value]
    return value


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = json.dumps(finite_json(value), indent=2, ensure_ascii=True, allow_nan=False) + "\n"
    # A crash must not truncate the last committed checkpoint.
    fd, temporary = tempfile.mkstemp(prefix=path.name + '.', suffix='.tmp', dir=path.parent)
    try:
        with os.fdopen(fd, 'w', encoding='utf-8') as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        # Windows readers/virus scanners can briefly deny delete-sharing.
        # Retry the atomic rename; never truncate or delete the old checkpoint.
        for attempt in range(20):
            try:
                os.replace(temporary, path)
                break
            except PermissionError:
                if attempt == 19:
                    raise
                time.sleep(min(.025 * (attempt + 1), .25))
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)


def study(folder, normalizer=None, normalized_dir=None):
    scenes, errors = [], []
    for path in sorted(folder.rglob("*.m3p")):
        try:
            normalized = path
            if normalizer:
                normalized = normalized_dir / path.relative_to(folder)
                normalized.parent.mkdir(parents=True, exist_ok=True)
                run = subprocess.run([str(normalizer.resolve()), str(path.resolve()), str(normalized.resolve())], capture_output=True, timeout=15)
                if run.returncode: raise ValueError(run.stderr.decode(errors='replace').strip())
            data = decode(normalized)
            scenes.append({"path": str(path.relative_to(ROOT)), "sha256": digest(path.read_bytes()),
                           "normalized": bool(normalizer), "normalized_sha256": digest(data),
                           "version": struct.unpack_from("<i", data)[0],
                           "size": list(struct.unpack_from("<ii", data, 4)),
                           "formula_slots": slots(data), "hybrid_mode": data[841] & 3,
                           "julia": bool(data[190]), "camera": data[148],
                           "mc_depth": data[186], "mc_options": data[124],
                           "reflection_options": data[336], "dof_options": data[181]})
        except (ValueError, OSError, struct.error, subprocess.TimeoutExpired) as exc:
            errors.append({"path": str(path.relative_to(ROOT)), "error": str(exc)})
    return {"scenes": scenes, "errors": errors,
            "formula_usage": dict(Counter(s["name"] for sc in scenes for s in sc["formula_slots"] if s["number"] >= 20)),
            "hybrid_modes": dict(Counter(s["hybrid_mode"] for s in scenes))}


def prepare(output, width, height):
    matrix = inventory(ROOT, ROOT / "comparison/results/20261013T132245Z")
    downloaded = study(ROOT / "mb3d/M3Parameter")
    write_json(output / "downloaded-scenes.json", downloaded)
    scene_dir = output / "scenes"
    scene_dir.mkdir(parents=True, exist_ok=True)
    obligations, cases, cases_by_id = [], [], {}
    for item in matrix["formulas"]:
        if not item["assets"]:
            continue
        asset = ROOT / item["assets"][0]
        metadata = parse_m3f_options(str(asset))
        text = asset.read_text(encoding="latin1")
        declarations = re.findall(r"^\.(\w+)\s+(.+?)\s*=\s*(.+)$", text, re.M)
        usages = [s for s in downloaded["scenes"] if any(f["name"].casefold() == item["name"].casefold() and f["number"] >= 20 for f in s["formula_slots"])]
        obligation = {"name": item["name"], "asset": item["assets"][0],
                      "asset_sha256": digest(asset.read_bytes()), "metadata": metadata,
                      "option_declarations": declarations, "registry": item["registry"],
                      "translations": item["translations"],
                      "downloaded_examples": [s["path"] for s in usages],
                      "previous_measurements": item["measurements"],
                      "cases": [], "unverified": ["parameter boundaries", "discrete option branches", "Julia/C/W state", "hybrid order and repetition", "per-iteration numerical oracle"]}
        if metadata["map_value_mask"]:
            obligation["unverified"].append("real map input and frame selection")
        candidates = [s for s in item["scenes"] if Path(s).name.startswith("extall_")]
        # Each legacy generated input remains visible, even if subsequently
        # found empty, invalid or numerically different.
        for source in candidates:
            data = decode(ROOT / source)
            name = Path(source).stem
            struct.pack_into("<ii", data, 4, width, height)
            target = scene_dir / (name + ".m3p")
            target.write_bytes(data)
            case = {"id": name, "target_formulas": [item["name"]], "source": source,
                    "source_sha256": digest((ROOT / source).read_bytes()),
                    "scene": str(target.relative_to(ROOT)), "sha256": digest(data),
                    "changes": {"width": width, "height": height}, "slots": slots(data),
                    "kind": "existing-template-baseline"}
            if name in cases_by_id:
                cases_by_id[name]["target_formulas"].append(item["name"])
            else:
                cases.append(case)
                cases_by_id[name] = case
            obligation["cases"].append(name)
        obligation["status"] = "scheduled" if obligation["cases"] else "missing-scene" if item["registry"] else "missing-implementation"
        obligations.append(obligation)
    report = {"schema_version": 1, "scope": "Every Pascal asset is accounted for. Scheduled does not mean verified.",
              "summary": {"reference_assets": len(obligations), "scheduled_cases": len(cases),
                          "registered": sum(bool(o["registry"]) for o in obligations),
                          "downloaded_scenes": len(downloaded["scenes"]), "downloaded_errors": len(downloaded["errors"])},
              "formulas": obligations, "cases": cases}
    write_json(output / "manifest.json", report)
    print(json.dumps(report["summary"]))


def qualify(manifest, sweep, output):
    import numpy as np
    from PIL import Image
    report = json.loads(manifest.read_text(encoding="utf-8"))
    measured = json.loads((sweep / "summary.json").read_text(encoding="utf-8"))
    by_id = {r["scene"]: r for r in measured["results"]}
    for case in report["cases"]:
        result = by_id.get(case["id"])
        case["measurement"] = result
        case["qualification"] = "not-measured"
        if not result:
            continue
        case["qualification"] = result["status"]
        path = sweep / "pascal" / (case["id"] + ".png")
        if path.exists():
            a = np.asarray(Image.open(path).convert("RGB"))
            unique = int(len(np.unique(a.reshape(-1, 3), axis=0)))
            case["image_quality"] = {"unique_colors": unique, "stddev": float(a.std()),
                                     "uniform": unique == 1,
                                     "geometry_verified": False, "formula_influence_verified": False}
            if result["status"] == "pass":
                case["qualification"] = "uninformative-image" if unique == 1 else "image-pass-needs-influence-check"
    cases = {c["id"]: c for c in report["cases"]}
    for formula in report["formulas"]:
        formula["outcomes"] = dict(Counter(cases[c]["qualification"] for c in formula["cases"]))
    report["provenance"] = measured["provenance"]
    report["summary"]["outcomes"] = dict(Counter(c["qualification"] for c in report["cases"]))
    report["summary"]["complete"] = False
    write_json(output, report)
    print(json.dumps(report["summary"]))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    prep = sub.add_parser("prepare")
    prep.add_argument("--output", type=Path, required=True)
    prep.add_argument("--width", type=int, default=96)
    prep.add_argument("--height", type=int, default=72)
    study_parser = sub.add_parser("study")
    study_parser.add_argument("--folder", type=Path, default=ROOT / "mb3d/M3Parameter")
    study_parser.add_argument("--normalizer", type=Path, required=True)
    study_parser.add_argument("--output", type=Path, required=True)
    qual = sub.add_parser("qualify")
    qual.add_argument("--manifest", type=Path, required=True)
    qual.add_argument("--sweep", type=Path, required=True)
    qual.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.command == "prepare":
        if not 8 <= args.width <= 8192 or not 8 <= args.height <= 8192:
            parser.error("dimensions must be in [8,8192]")
        prepare(args.output.resolve(), args.width, args.height)
    elif args.command == "study":
        report = study(args.folder, args.normalizer, args.output / "normalized")
        report["normalizer_sha256"] = digest(args.normalizer.read_bytes())
        write_json(args.output / "study.json", report)
        print(json.dumps({"loaded":len(report["scenes"]),"errors":len(report["errors"]),"formulas":len(report["formula_usage"])}))
    else:
        qualify(args.manifest, args.sweep, args.output)


if __name__ == "__main__":
    main()


def compact_image_evidence(item, provenance_records):
    """Intern repeated build manifests without dropping their contents."""
    item = dict(item)
    if 'reference_provenance' in item:
        provenance = item.pop('reference_provenance')
        key = digest(json.dumps(provenance, sort_keys=True, separators=(',', ':')).encode('utf-8'))
        if key in provenance_records and provenance_records[key] != provenance:
            raise ValueError('Conflicting provenance digest')
        if item.get('reference_provenance_sha256', key) != key:
            raise ValueError('Inline provenance differs from its reference')
        provenance_records[key] = provenance
        item['reference_provenance_sha256'] = key
    key = item.get('reference_provenance_sha256')
    if key is not None and key not in provenance_records:
        raise ValueError('Missing provenance record')
    return item

def merge_provenance_records(source, target):
    """Import content-addressed manifests only after checking their identities."""
    for key, value in source.items():
        actual = digest(json.dumps(value, sort_keys=True, separators=(',', ':')).encode('utf-8'))
        if key != actual:
            raise ValueError('Changed provenance record')
        if key in target and target[key] != value:
            raise ValueError('Conflicting provenance digest')
        target[key] = value


def compact_replay_report(report):
    """Migrate old inline manifests losslessly; reject dangling compact references."""
    records = {}
    merge_provenance_records(report.get('provenance_records', {}), records)
    results = [compact_image_evidence(item, records) for item in report['results']]
    for item in results:
        key = item.get('reference_provenance_sha256')
        if key is not None and key not in records:
            raise ValueError('Missing provenance record')
    report['results'] = results
    report['provenance_records'] = records
