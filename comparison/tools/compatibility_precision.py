"""Fail-closed arithmetic selection for new C++ compatibility evidence.

Historical executables without these flags must use their frozen historical
tools and identities; never retry without selection against a new candidate.
"""
import hashlib
import json
from pathlib import Path


def profile_path(image):
    return Path(str(image) + ".precision.json")


def bind_compatible(command, image):
    if any(str(arg).split("=", 1)[0] in ("--precision", "--profile-json") for arg in command):
        raise ValueError("Compatibility runner owns precision/profile selection")
    profile = profile_path(image)
    profile.unlink(missing_ok=True)
    command.extend(["--precision", "compatible", "--profile-json", str(profile.resolve())])


def precision_evidence(image):
    path = profile_path(image)
    data = path.read_bytes()
    profile = json.loads(data)
    if (not isinstance(profile, dict) or profile.get("precision") != "compatible" or
            profile.get("schema") != "mb3d.cpu-stage-profile.v1" or
            profile.get("status") != "complete" or profile.get("exit_code") != 0 or
            profile.get("render_status") != "completed"):
        raise ValueError("C++ compatibility render lacks completed compatible precision evidence")
    return {"cpp_precision": "compatible",
            "cpp_precision_profile_sha256": hashlib.sha256(data).hexdigest()}


def verify_precision_evidence(image, row):
    actual = precision_evidence(image)
    if any(row.get(key) != value for key, value in actual.items()):
        raise ValueError("C++ precision evidence missing or changed")
