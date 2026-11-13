#!/usr/bin/env python3
"""Restore the saved guarded-v3 Pascal application into a new external directory.

This is source restoration, not a reproduction of a historical executable hash.
It never builds or modifies the immutable reference tree.
"""
from __future__ import annotations

import argparse
import hashlib
import io
import json
from pathlib import Path, PurePosixPath
import subprocess
import tarfile

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "patches/pascal_guarded_v3_source_manifest.json"


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--destination", type=Path, required=True,
                        help="New, nonexistent directory outside this repository")
    args = parser.parse_args()
    destination = args.destination.expanduser().resolve()
    if destination == ROOT or ROOT in destination.parents:
        parser.error("Destination must be outside the immutable reference repository")
    if destination.exists():
        parser.error("Destination already exists; choose a fresh directory")

    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    patch = ROOT / manifest["patch"]
    if sha(patch.read_bytes()) != manifest["patch_sha256"]:
        raise SystemExit("Restoration patch hash mismatch")
    archive = subprocess.check_output(
        ["git", "-C", str(ROOT), "archive", "--format=tar", "HEAD", "mb3d"])
    exported: dict[str, bytes] = {}
    with tarfile.open(fileobj=io.BytesIO(archive)) as tar:
        for member in tar.getmembers():
            if member.isdir():
                continue
            path = PurePosixPath(member.name)
            if not member.isfile() or path.parts[0] != "mb3d" or ".." in path.parts:
                raise SystemExit(f"Unexpected Git archive entry: {member.name}")
            stream = tar.extractfile(member)
            assert stream is not None
            exported[str(PurePosixPath(*path.parts[1:]))] = stream.read()
    expected = manifest["reference_files"]
    if set(exported) != set(expected):
        raise SystemExit("Immutable reference file inventory differs from the saved snapshot")
    for name, data in exported.items():
        if sha(data) != expected[name]:
            raise SystemExit(f"Immutable reference hash mismatch: {name}")

    destination.mkdir(parents=True, exist_ok=False)
    for name, data in exported.items():
        target = destination / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    # git apply works outside a Git checkout; no intermediate Git history or build
    # outputs are required. Check first so a corrupt patch cannot partially apply.
    for check in (True, False):
        command = ["git", "-c", "core.autocrlf=false", "-C", str(destination),
                   "apply", "--whitespace=nowarn"]
        if check:
            command.append("--check")
        subprocess.run(command + [str(patch)], check=True)
    for name, expected_sha in manifest["source_files"].items():
        if sha((destination / name).read_bytes()) != expected_sha:
            raise SystemExit(f"Restored source hash mismatch: {name}")
    receipt = {
        "profile": manifest["profile"],
        "source_manifest_sha256": sha(MANIFEST.read_bytes()),
        "restored_source_files": len(manifest["source_files"]),
        "reference_files": len(exported),
        "destination": str(destination),
        "runtime_status": "Source only; build the i386 math DLL and Pascal application next",
    }
    (destination / "bootstrap-receipt.json").write_text(
        json.dumps(receipt, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(receipt, indent=2))


if __name__ == "__main__":
    main()
