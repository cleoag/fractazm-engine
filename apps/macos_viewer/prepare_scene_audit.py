#!/usr/bin/env python3
"""Freeze a viewer, the 77 ordinary scenes and their approved resource profiles."""
import argparse
import base64
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "comparison/tools"))
from prepare_native_scene_resources import prepare_profiles


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--app", type=Path, required=True)
    p.add_argument("--output", type=Path, required=True)
    a = p.parse_args()
    output = a.output.resolve()
    queue = json.loads((ROOT / "comparison/scene-queue.json").read_text())["rows"]
    contract = ROOT / "comparison/contracts/native-gpu-acceptance-v4/inventory.json"
    scenes = {s["id"]: s for s in json.loads(contract.read_text())["scenes"]}
    # These exclusions are the pinned scope, not a new attempt to load MC/JIT.
    excluded = {
        "Mengerplus for MC.m3p": "2286fe26754ac472288f82331a72ea9b7d0edc3f08619e83d36550046da5219e",
        "MutaGenD4B12E71C70D4FD00357_canary~.m3p": "fd5470a6dd5f6eb3d8dd7237ade9576973dc09383ec352aa835cbe4ea08c7b4a",
    }
    folder = ROOT / "mb3d/M3Parameter"
    expected = {Path(r["scene"]).name for r in queue} | set(excluded)
    if len(queue) != 77 or {p.name for p in folder.iterdir()} != expected:
        raise ValueError("Scene directory changed; re-inventory before dispatch")
    for name, digest in excluded.items():
        if sha(folder / name) != digest:
            raise ValueError("Excluded input changed; re-inventory before dispatch")
    for row in queue:
        if sha(ROOT / row["scene"]) != scenes[row["id"]]["sha256"]:
            raise ValueError("Ordinary input changed: " + row["scene"])
    output.mkdir(parents=True, exist_ok=False)
    app = output / "final/Mandelbulb Viewer.app"
    shutil.copytree(a.app.resolve(), app)
    exe = app / "Contents/MacOS/MandelbulbViewer"
    resources = prepare_profiles(output / "resources")
    (output / "inputs").mkdir()
    (output / "requests").mkdir()
    rows = []
    for row in queue:
        scene = output / "inputs" / Path(row["scene"]).name
        shutil.copyfile(ROOT / row["scene"], scene)
        profile = resources["scene_resource_profiles"][row["id"]]
        root = output / "resources" / resources["profiles"][profile]["runtime"]
        request = json.loads(subprocess.check_output(
            [str(exe), "--request", str(scene), str(root), "metal", "64", "points"], timeout=10))
        header = base64.b64decode(request["header"], validate=True)
        if len(header) != 840:
            raise ValueError("Unexpected private snapshot ABI")
        # Packed TMandHeader10 disk ABI: bStereoMode #126, bPlanarOptic #148.
        stereo, optic = header[126], header[148]
        path = output / "requests" / (row["id"] + ".json")
        path.write_text(json.dumps(request) + "\n")
        rows.append(dict(row, sha256=scenes[row["id"]]["sha256"], resource_profile=profile,
                         request=str(path), stereo=stereo, optic=optic, ui_supported=request.get("camera_supported",stereo == optic == 0)))
    (output / "inventory.json").write_text(json.dumps({"ordinary": rows,
        "excluded": list(excluded), "excluded_sha256": excluded}, indent=2) + "\n")
    print(json.dumps({"ordinary": len(rows), "excluded": len(excluded),
                      "ui_projection_supported": sum(r["ui_supported"] for r in rows),
                      "exe_sha256": sha(exe)}, indent=2))


if __name__ == "__main__":
    main()
