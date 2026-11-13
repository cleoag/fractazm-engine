#!/usr/bin/env python3
"""Prepare pinned phase-one resources, or audit originals from explicit roots.

Without --maps, copy the three approved resource profiles from tracked assets.
With --maps, preserve the original-only inventory/audit behavior. Resource
availability is never rendering qualification.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import struct
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
SELECTED_RESOURCES = ROOT / "comparison/resources/phase1-selected"


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def prepare_profiles(output, resource_root=SELECTED_RESOURCES):
    """Copy exact approved inputs without depending on previous raw runs."""
    output = Path(output)
    resource_root = Path(resource_root)
    if output.exists():
        raise ValueError("Use a new output directory")
    selection = json.loads((resource_root / "selection.json").read_text())
    contract_path = ROOT / selection["contract"]
    if sha(contract_path) != selection["contract_sha256"]:
        raise ValueError("Selected resource contract changed")
    contract = json.loads(contract_path.read_text())
    prepared = {}
    directories = {"original": "runtime", "available": "runtime-selected",
                   "substitute1633": "runtime-substitute1633"}
    for profile, directory in directories.items():
        pinned = selection["profiles"][profile]
        inventory_path = ROOT / pinned["inventory"]
        approved = contract["resource_profiles"][profile]
        if (inventory_path.resolve() != (contract_path.parent / approved["inventory"]).resolve()
                or pinned["inventory_sha256"] != approved["sha256"]):
            raise ValueError("Resource profile differs from approved contract: " + profile)
        if sha(inventory_path) != pinned["inventory_sha256"]:
            raise ValueError("Resource inventory changed: " + profile)
        inventory = json.loads(inventory_path.read_text())
        for rel, expected in inventory.items():
            path = Path(rel)
            if path.is_absolute() or ".." in path.parts or path.parts[0] != "M3Maps":
                raise ValueError("Unsafe resource path: " + rel)
            if selection["source_files"].get(rel) != expected or sha(resource_root / rel) != expected:
                raise ValueError("Selected resource bytes changed: " + rel)
        prepared[profile] = {"runtime": directory, "files": inventory,
                             "inventory": pinned["inventory"],
                             "inventory_sha256": pinned["inventory_sha256"]}
    # Validate every source before creating any runtime; never silently accept
    # a missing or changed image as the renderer's legacy inactive-map state.
    for profile in prepared.values():
        for rel, expected in profile["files"].items():
            target = output / profile["runtime"] / rel
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(resource_root / rel, target)
            if sha(target) != expected:
                raise ValueError("Resource copy changed: " + rel)
    assignments = {sid: contract.get("preferred_scene_resource_profiles", {}).get(sid, "available")
                   for sid in contract["scene_resource_profiles"]}
    for sid, profile in assignments.items():
        if profile not in contract["scene_resource_profiles"][sid]:
            raise ValueError("Unapproved scene resource profile: " + sid)
        ids = {Path(p).stem for p in prepared[profile]["files"] if Path(p).suffix != ".txt"}
        if not {str(i) for i in contract["scene_required_maps"][sid]} <= ids:
            raise ValueError("Missing required scene map: " + sid)
    report = {"schema": "mb3d.prepared-scene-resources.v1", "qualification": False,
              "source": str(resource_root), "selection_sha256": sha(resource_root / "selection.json"),
              "profiles": prepared, "scene_resource_profiles": assignments,
              "scenes_with_available_resources": len(assignments),
              "original_identity_claimed_for_selected_maps": False}
    (output / "resources.json").write_text(json.dumps(report, indent=2) + "\n")
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--maps", type=Path, action="append",
                        help="Explicit original-map root; omit to use tracked approved profiles")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if not args.maps:
        report = prepare_profiles(args.output)
        print(json.dumps({k: report[k] for k in ("scenes_with_available_resources", "qualification")}))
        return
    inventory = ROOT / "comparison/results/m3parameter-gpu-priority-20261024/inventory.json"
    data = json.loads(inventory.read_text())
    rows = [r for r in data["scenes"] if r["scope"] == "PHASE1"]
    queue = json.loads((ROOT / "comparison/scene-queue.json").read_text())
    assert len(rows) == 77 and {r["path"] for r in rows} == {r["scene"] for r in queue["rows"]}
    assert all(sha(ROOT / r["path"]) == r["sha256"] for r in rows), "Scene inventory changed"
    assert not args.output.exists(), "Use a new output"
    roots = [p.resolve(strict=True) for p in args.maps]
    assert all(p.is_dir() for p in roots)
    requests = {}

    def request(number, scene, kind):
        if number:
            requests.setdefault(str(number), []).append({"scene": scene, "kind": kind})

    formulas = {f["key"]: f for f in data["formulas"]}
    for row in rows:
        settings, scene = row["settings"], row["path"]
        request(settings["diffuse_map_number"], scene, "diffuse")
        for light in settings["lights"]:
            if light["mode"] == 2:
                request(light["map_number"], scene, "light")
        if settings["background_name"]:
            request(Path(settings["background_name"]).stem, scene, "background")
        for slot in row["configured_nonzero_slots"]:
            for lane in range(16):
                if formulas[slot["key"]]["map_value_mask"] & (1 << lane):
                    value = struct.unpack("<d", int(slot["option_values_binary64_hex"][lane], 16).to_bytes(8, "little"))[0]
                    assert value == int(value), "Noninteger map ID"
                    request(int(value), scene, "formula")
    resources = []
    for key, uses in sorted(requests.items(), key=lambda pair: int(pair[0])):
        matches = sorted(p for root in roots for p in root.iterdir()
                         if p.is_file() and p.stem == key and p.suffix.lower() in (".png", ".jpg", ".jpeg", ".bmp", ".pgm"))
        record = {"id": int(key), "uses": uses,
                  "sources": [{"path": str(p), "sha256": sha(p)} for p in matches]}
        hashes = {x["sha256"] for x in record["sources"]}
        if len(hashes) != 1:
            record["status"] = "MISSING_ORIGINAL" if not hashes else "CONFLICTING_ORIGINALS"
        else:
            source = matches[0]
            with Image.open(source) as image:
                image.load()
                record["size"] = list(image.size)
            target = args.output / "runtime/M3Maps" / source.name
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source, target)
            assert sha(target) == next(iter(hashes))
            record.update(status="AVAILABLE_DECODED", path=str(target.relative_to(args.output)), sha256=sha(target))
        resources.append(record)
    missing = {u["scene"] for r in resources if r["status"] != "AVAILABLE_DECODED" for u in r["uses"]}
    terms = next((r / "m3maps terms of use.txt" for r in roots if (r / "m3maps terms of use.txt").is_file()), None)
    if terms:
        shutil.copyfile(terms, args.output / "runtime/M3Maps" / terms.name)
    report = {"schema": "mb3d.native-scene-resources.v1", "qualification": False,
              "inventory_sha256": sha(inventory), "searched_roots": [str(p) for p in roots],
              "resources": resources, "available": sum(r["status"] == "AVAILABLE_DECODED" for r in resources),
              "requested": len(resources), "scenes_with_available_resources": len(rows) - len(missing),
              "blocked_scenes": sorted(missing), "scene_sha256": {r["path"]: r["sha256"] for r in rows}}
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / "resources.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps({k: report[k] for k in ("available", "requested", "scenes_with_available_resources", "qualification")}))


if __name__ == "__main__":
    main()
