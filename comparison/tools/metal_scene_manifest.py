"""Read the shared, pinned exact Metal scene catalog without executing code."""
from __future__ import annotations

from dataclasses import dataclass
import hashlib
from pathlib import Path, PurePosixPath
import re
import math
import struct

ROOT = Path(__file__).resolve().parents[2]
MANIFEST = ROOT / "comparison/metal_exact/scene_manifest.inc"
FORMULAS = {1: "SphereIFS", 2: "boxIFS", 3: "IntegerPower"}
DESCRIPTION_SCHEMA = "mb3d.metal-exact-scene-description.v1"
RUNTIME = {101: (1, "sphereifs_runtime"), 102: (2, "boxifs_runtime"),
           103: (3, "integer_power_runtime")}
SPHERE_PARAMETERS = (0x3ff0000000000000, 0x4000000000000000, 0, 0,
                     0x4008000000000000, 0x3ff0000000000000, 0x3ff0000000000000,
                     0x3ff0000000000000, 0x3ff0000000000000, 0,
                     0x3ff0000000000000, 0x3ff0000000000000, 0xbff0000000000000)
_STRING = r'"[A-Za-z0-9_./-]+"'
_INTEGER = r"(?:0|[1-9][0-9]*)"
_BITS = r"0x[0-9a-f]{16}ULL"
_FIELDS = (_INTEGER, _STRING, _STRING, r'"[0-9a-f]{64}"', _INTEGER, _STRING,
           _INTEGER, _INTEGER, _INTEGER, *([_BITS] * 13))
_ROW = re.compile(r"MB3D_EXACT_SCENE\(\s*" + r"\s*,\s*".join(
    "(" + field + ")" for field in _FIELDS) + r"\s*\)")


@dataclass(frozen=True)
class Scene:
    capability_id: int
    scene_id: str
    path: str
    sha256: str
    formula_kind: int
    formula_name: str
    max_iterations: int
    slot_count: int
    original_shadow_bits: int
    parameter_bits: tuple[int, ...]
    builtin_power: int = 0

    def profile(self, full_shading=False):
        name = self.formula_name.upper()
        return f"EXACT_{name}_FULL_SHADING" if full_shading else f"EXACT_BINARY64_PRIMARY_{name}"

    def report_identity(self):
        return {"scene_sha256": self.sha256, "scene_id": self.scene_id,
                "capability_id": self.capability_id, "formula_kind": self.formula_kind,
                "formula_name": self.formula_name}

    def description(self):
        return {**self.report_identity(), "max_iterations": self.max_iterations,
                "slot_count": self.slot_count, "original_shadow_bits": self.original_shadow_bits,
                "parameter_bits": [f"{value:016x}" for value in self.parameter_bits],
                "builtin_power": self.builtin_power}

    def scope(self, full_shading=False):
        stages = ("GPU primary geometry, packed surface, optional hard shadows, HiQ AO/wavelets "
                  "and production palette/light/fog/RGB; independent native CPU full-render comparison"
                  if full_shading else "GPU primary geometry; diagnostic normal visualization")
        return (f"{self.scene_id} M3P ({self.formula_name}): {stages}; "
                "not original Pascal full-image qualification")


def parse_manifest(text):
    scenes = []
    for line_number, line in enumerate(text.splitlines(), 1):
        line = line.strip()
        if not line or line.startswith("//"):
            continue
        row = _ROW.fullmatch(line)
        if row is None:
            raise ValueError(f"Invalid exact scene manifest row {line_number}")
        fields = row.groups()
        scene = Scene(int(fields[0]), fields[1][1:-1], fields[2][1:-1],
                      fields[3][1:-1], int(fields[4]), fields[5][1:-1],
                      int(fields[6]), int(fields[7]), int(fields[8]),
                      tuple(int(value[2:-3], 16) for value in fields[9:]))
        path = PurePosixPath(scene.path)
        if (scene.capability_id < 1 or not re.fullmatch(r"[a-z][a-z0-9_]*", scene.scene_id)
                or path.is_absolute() or ".." in path.parts
                or str(path) != scene.path or path.suffix != ".m3p"
                or not (path.parts[:2] == ("comparison", "scenes")
                        or path.parts[:3] == ("comparison", "metal_exact", "fixtures"))
                or FORMULAS.get(scene.formula_kind) != scene.formula_name
                or not 1 <= scene.max_iterations <= 60 or not 1 <= scene.slot_count <= 60
                or not 0 <= scene.original_shadow_bits <= 255
                or any((value >> 52) & 0x7ff == 0x7ff for value in scene.parameter_bits)):
            raise ValueError(f"Invalid exact scene contract on row {line_number}")
        if scene.formula_kind == 1 and scene.parameter_bits != SPHERE_PARAMETERS:
            raise ValueError("SphereIFS specialization requires its original literal coefficient bits")
        # The renderer admits positive-zero Twist, Apply=1, OTrap=0 and unit
        # orbit-color scale/offset. It does not silently widen that subset when
        # another catalog row is added. Signed zero remains observable.
        if (scene.formula_kind == 2 and scene.parameter_bits[8:] !=
                (0, 0x3ff0000000000000, 0, 0x3ff0000000000000, 0x3ff0000000000000)):
            raise ValueError("Unsupported boxIFS Twist/Apply/OTrap scene contract")
        scenes.append(scene)
    if not scenes:
        raise ValueError("Empty exact scene manifest")
    for field in ("capability_id", "scene_id", "path", "sha256"):
        if len({getattr(scene, field) for scene in scenes}) != len(scenes):
            raise ValueError("Duplicate exact scene " + field)
    if [scene.capability_id for scene in scenes] != list(range(1, len(scenes) + 1)):
        raise ValueError("Exact scene capability IDs must be consecutive and ordered")
    return tuple(scenes)


SCENES = parse_manifest(MANIFEST.read_text())
DEFAULT_SCENE = next(scene for scene in SCENES if scene.capability_id == 1)


def scene_for_hash(digest):
    for scene in SCENES:
        if scene.sha256 == digest:
            return scene
    raise ValueError("Unsupported scene; requires an unchanged file from the exact Metal catalog")


def scene_for_file(path):
    return scene_for_hash(hashlib.sha256(Path(path).read_bytes()).hexdigest())


def validate_identity(section, scene):
    if scene not in SCENES:
        validate_runtime_scene(scene)
    for field, expected in scene.report_identity().items():
        if type(section.get(field)) is not type(expected) or section[field] != expected:
            raise ValueError("Scene identity differs from the requested capability: " + field)


def scene_for_report(section):
    scene = scene_for_hash(section.get("scene_sha256"))
    validate_identity(section, scene)
    return scene



def validate_runtime_scene(scene):
    profile = RUNTIME.get(scene.capability_id)
    if (profile != (scene.formula_kind, scene.scene_id)
            or FORMULAS.get(scene.formula_kind) != scene.formula_name
            or not isinstance(scene.sha256, str) or not re.fullmatch(r"[0-9a-f]{64}", scene.sha256)
            or type(scene.max_iterations) is not int or not 1 <= scene.max_iterations <= 256
            or type(scene.slot_count) is not int or not 1 <= scene.slot_count <= 65535
            or type(scene.original_shadow_bits) is not int or not 0 <= scene.original_shadow_bits <= 255
            or len(scene.parameter_bits) != 13
            or any(type(value) is not int or not 0 <= value < 2**64 for value in scene.parameter_bits)):
        raise ValueError("Invalid runtime scene capability")
    params = [struct.unpack("<d", struct.pack("<Q", bits))[0] for bits in scene.parameter_bits]
    if not all(map(math.isfinite, params)):
        raise ValueError("Nonfinite runtime formula parameter")
    if scene.formula_kind == 3:
        fixed = (0x2b2bff2ee48e0530, 0x3ff0000000000000, 0x4018000000000000,
                 0x401c000000000000, 0x4020000000000000, 0x403c000000000000,
                 0x4051800000000000)
        if (type(scene.builtin_power) is not int or not 2 <= scene.builtin_power <= 8 or params[0] == 0
                or scene.parameter_bits[1:8] != fixed or scene.parameter_bits[8:] != (0,)*5):
            raise ValueError("Unsupported internal Mandelbulb power or prepared constants")
        return
    if scene.builtin_power != 0:
        raise ValueError("External IFS cannot carry a built-in power")
    positive = (0, 1, 5, 6, 7) if scene.formula_kind == 1 else (0, 1, 2, 3)
    if any(params[index] <= 0 for index in positive):
        raise ValueError("Runtime geometry requires positive dimensions and scale")
    apply = 8 if scene.formula_kind == 1 else 9
    if (scene.parameter_bits[apply] not in (0, 0x3ff0000000000000)
            or scene.parameter_bits[apply+1] not in (0, 0x3ff0000000000000,
                                                     0x4000000000000000, 0x4008000000000000)):
        raise ValueError("Unsupported runtime Apply or OTrap option")
    if scene.formula_kind == 2 and scene.parameter_bits[8] != 0:
        raise ValueError("Runtime boxIFS requires positive-zero Twist")
    for index in (apply+2, apply+3):
        try:
            rounded = struct.unpack("<f", struct.pack("<f", params[index]))[0]
            canonical = struct.unpack("<Q", struct.pack("<d", rounded))[0]
        except OverflowError:
            raise ValueError("OTrap coefficient exceeds finite Single range") from None
        if canonical != scene.parameter_bits[index]:
            raise ValueError("OTrap coefficient is not a prepared Single")


def scene_from_description(description, expected_sha256, source_path):
    if (not isinstance(description, dict) or description.get("schema") != DESCRIPTION_SCHEMA
            or description.get("native_arm64") is not True or description.get("translated") is not False
            or any(type(description.get(key)) is not int or description[key] != value
                   for key, value in (("gpu_dispatches", 0), ("scene_abi_version", 4),
                                      ("scene_bytes", 336), ("pixel_bytes", 176)))):
        raise ValueError("Native scene description or versioned ABI is invalid")
    section = description.get("scene")
    if not isinstance(section, dict) or section.get("scene_sha256") != expected_sha256:
        raise ValueError("Native scene description is not bound to the selected input hash")
    required = {"scene_sha256", "scene_id", "capability_id", "formula_kind", "formula_name",
                "max_iterations", "slot_count", "original_shadow_bits", "parameter_bits", "builtin_power"}
    if set(section) != required:
        raise ValueError("Native scene description fields are incomplete or unexpected")
    bits = section["parameter_bits"]
    if (not isinstance(bits, list) or len(bits) != 13 or
            any(not isinstance(value, str) or not re.fullmatch(r"[0-9a-f]{16}", value) for value in bits)):
        raise ValueError("Native scene parameter encodings are invalid")
    for field in ("capability_id", "formula_kind", "max_iterations", "slot_count",
                  "original_shadow_bits", "builtin_power"):
        if type(section[field]) is not int:
            raise ValueError("Native scene integer metadata is invalid: " + field)
    candidate = Scene(section["capability_id"], section["scene_id"], str(source_path), expected_sha256,
                      section["formula_kind"], section["formula_name"], section["max_iterations"],
                      section["slot_count"], section["original_shadow_bits"],
                      tuple(int(value, 16) for value in bits), section["builtin_power"])
    if candidate.capability_id in RUNTIME:
        validate_runtime_scene(candidate)
        if any(scene.sha256 == expected_sha256 for scene in SCENES):
            raise ValueError("Known input cannot be relabelled as a runtime capability")
        return candidate
    pinned = scene_for_hash(expected_sha256)
    if candidate.description() != pinned.description():
        raise ValueError("Native prepared pinned scene differs from the immutable catalog")
    return pinned


def validate_description(report, scene):
    if any(type(report.get(key)) is not int or report[key] != expected
           for key, expected in (("scene_abi_version", 4), ("scene_bytes", 336), ("pixel_bytes", 176))):
        raise ValueError("Scene report ABI version or size differs from the prepared contract")
    section = report.get("scene_description")
    expected = scene.description()
    if (not isinstance(section, dict) or set(section) != set(expected)
            or any(type(section[key]) is not type(value) or section[key] != value
                   for key, value in expected.items())):
        raise ValueError("Scene report differs from the native prepared input description")
    pipeline = report.get("pipeline_specialization")
    expected_pipeline = {"formula_kind": scene.formula_kind, "builtin_power": scene.builtin_power,
                         "capability_id": scene.capability_id}
    if (not isinstance(pipeline, dict) or set(pipeline) != set(expected_pipeline)
            or any(type(pipeline[key]) is not int or pipeline[key] != value
                   for key,value in expected_pipeline.items())):
        raise ValueError("GPU pipeline specialization differs from the prepared scene")
    creation = report.get("pipeline_creation_ms")
    if (isinstance(creation, bool) or not isinstance(creation, (float, int))
            or not math.isfinite(creation) or creation <= 0):
        raise ValueError("Missing positive finite GPU pipeline creation timing")


def validate_geometry_limits(comparison, scene):
    expected = {"result_kind_limit": 2 if scene.formula_kind == 3 else 1,
                "final_iteration_limit": scene.max_iterations+(scene.formula_kind == 3)}
    if any(type(comparison.get(key)) is not int or comparison[key] != value
           for key,value in expected.items()):
        raise ValueError("Primary result bounds differ from the prepared formula contract")

def validate_formula_rejections(section, pixels, scene=DEFAULT_SCENE):
    if not isinstance(section, dict):
        raise ValueError("Missing GPU formula rejection controls")
    for field in ("cases", "checks", "failures", "no_minimum_cases",
                  "no_minimum_checks", "no_minimum_failures"):
        if type(section.get(field)) is not int or section[field] < 0:
            raise ValueError("Invalid formula rejection counter: " + field)
    stateful_cases = int(scene.formula_kind in (1, 2))
    if (section["cases"] != 5 or section["checks"] != 5 * pixels
            or section["failures"] > section["checks"]
            or section["no_minimum_cases"] != stateful_cases
            or section["no_minimum_checks"] != stateful_cases * pixels
            or section["no_minimum_failures"] > section["no_minimum_checks"]
            or type(section.get("pass")) is not bool
            or section["pass"] != (section["failures"] + section["no_minimum_failures"] == 0)):
        raise ValueError("GPU formula rejection controls are incomplete or inconsistent")
    return section["failures"] + section["no_minimum_failures"]
