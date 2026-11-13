"""Reproduce the bounded ordinary effects benchmark without modifying originals.

Run from any directory with Python 3: generate_effects_v1.py --check
Omit --check to create the new fixtures; existing identical files are accepted.
Use --replace-generated explicitly after deliberately changing this recipe.
No renderer, compiler, Pascal executable or deferred execution path is invoked.
The byte offsets below are TMandHeader10/TLightingParas9 disk-format fields in
src/core/types.h; each original is version 44 with a 1136-byte formula addon.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import struct


ROOT = Path(__file__).resolve().parents[2]
DIRECTORY = Path("comparison/benchmark_scenes/effects-v1")
MANIFEST = Path("comparison/benchmark_scenes/effects-v1.json")
ORIGIN_COMMIT = "4027dabcb4b7f885d55b738115c1d72468f5de1c"
SOURCES = {
    "sphere": ("comparison/scenes/extall_dIFS_SphereIFS.m3p", "0ec9d55cc7007f665f93811a8a686859fb2e37de67b9e0ec2d0b40d15b93eb47"),
    "box": ("comparison/scenes/extall_dIFS_boxIFS.m3p", "f1421193b493ce6e9d46161983a2b51ca3f9db6dce195dfc55ef3197c6230b15"),
    "bulb": ("comparison/scenes/test_03_intpow8.m3p", "8e86179ced397fe76aac613cd78ec7ba935e1b41ecea6d5c536fa9240f8d4794"),
}
ASSET = {
    "id": "map9001",
    "source_path": "comparison/fixtures/M3Maps/9001.bmp",
    "runtime_path": "M3Maps/9001.bmp",
    "sha256": "6e368b0451de795ad157725dd1a35864e01fdd3290761f3e148f4c05d2465c09",
}
FIELDS = {
    "Width": (4, "i"), "Height": (8, "i"), "iOptions": (16, "H"),
    "sTRIndex": (116, "f"), "sTRscattering": (120, "f"),
    "MCoptions": (124, "B"), "bNormalsOnDE": (132, "B"),
    "bCalculateHardShadow": (133, "B"), "bCalcAmbShadowAutomatic": (149, "B"),
    "sDOFZsharp": (164, "f"), "sDOFclipR": (168, "f"),
    "sDOFaperture": (172, "f"), "bCalcDOFtype": (181, "B"),
    "SSAORcount": (187, "B"), "AODEdithering": (188, "B"),
    "byCalcNsOnZBufAuto": (331, "B"), "SRamount": (332, "f"),
    "bCalcSRautomatic": (336, "B"), "SRreflectioncount": (337, "B"),
    "bVolLightNr": (343, "B"), "bCalc3D": (344, "B"),
    "sTransmissionAbsorption": (370, "f"), "sDOFZsharp2": (410, "f"),
    "Light.bColorMap": (435, "B"),
    "Light.TBpos[3]": (452, "i"),
    # The original lighting format reuses these bytes for diffuse-map settings.
    "Light.Lights[1].AdditionalByteEx.map_number_high": (548, "B"),
    "Light.Lights[1].FreeByte.map_projection": (556, "B"),
    "Light.Lights[2].AdditionalByteEx.map_scale": (580, "B"),
}
BASE_STAGES = ["preparation", "primary", "shadows", "ao", "color"]
COMMON = {
    "Width": 512, "Height": 384, "iOptions": 0, "MCoptions": 0,
    "bCalc3D": 1, "bNormalsOnDE": 1, "byCalcNsOnZBufAuto": 0,
    "bCalculateHardShadow": 7, "bCalcAmbShadowAutomatic": 21,
    "SSAORcount": 1, "AODEdithering": 0, "bCalcDOFtype": 0,
    "bCalcSRautomatic": 0, "bVolLightNr": 32,
}


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def patch(original: bytes, settings: dict) -> tuple[bytes, list[dict]]:
    data = bytearray(original)
    operations = []
    for name, value in settings.items():
        offset, kind = FIELDS[name]
        fmt = "<" + kind
        length = struct.calcsize(fmt)
        before = bytes(data[offset:offset + length])
        after = struct.pack(fmt, value)
        if before == after:
            continue
        data[offset:offset + length] = after
        operations.append({"field": name, "offset": offset, "struct_format": fmt,
                           "before": struct.unpack(fmt, before)[0],
                           "after": struct.unpack(fmt, after)[0],
                           "before_hex": before.hex(), "after_hex": after.hex()})
    assert data[840:] == original[840:], "Formula addon must remain unchanged"
    return bytes(data), operations


def generate() -> dict[Path, bytes]:
    originals = {}
    for key, (path, expected) in SOURCES.items():
        raw = (ROOT / path).read_bytes()
        if sha(raw) != expected or len(raw) != 1976 or struct.unpack_from("<i", raw)[0] != 44:
            raise ValueError("Pinned original changed: " + path)
        originals[key] = raw
    if sha((ROOT / ASSET["source_path"]).read_bytes()) != ASSET["sha256"]:
        raise ValueError("Pinned map asset changed")
    outputs = {}
    scenes = []

    def fixture(name, source, settings):
        payload, operations = patch(originals[source], settings)
        path = DIRECTORY / (name + ".m3p")
        outputs[path] = payload
        return {"scene_path": path.as_posix(), "scene_sha256": sha(payload),
                "source_scene": {"path": SOURCES[source][0], "sha256": SOURCES[source][1]},
                "patch_operations": operations}

    def add(name, title, source, effect, extra=None, *, metal=False, stages=(), control_settings=None,
            control_not_run=(), assets=(), unsupported=None):
        settings = {**COMMON, **(extra or {})}
        required = BASE_STAGES + list(stages)
        row = {"id": name, "name": title, "category": "ordinary-effects",
               **fixture(name, source, settings),
               "backends": ["cpu", "metal"] if metal else ["cpu"],
               "required_stages": required, "active_effects": effect,
               "assets": list(assets), "require_nonconstant_rgb": True}
        if metal:
            row["metal_profile"] = "exact-full-shading"
            controls = []
            for suffix, field, value, stage in (
                    ("ao-off", "bCalcAmbShadowAutomatic", 20, "ao"),
                    ("shadows-off", "bCalculateHardShadow", 6, "shadows")):
                controls.append({"id": name + "-" + suffix,
                                 **fixture(name + "-" + suffix, source, {**settings, field: value}),
                                 "backends": ["cpu"], "disabled_effects": [stage],
                                 "required_stages": [s for s in required if s != stage],
                                 "required_not_run_stages": [stage], "expect_rgb_difference": True})
            row["effect_controls"] = controls
        else:
            row["unsupported_backends"] = {"metal": unsupported}
            row["effect_control"] = {
                "id": name + "-off",
                **fixture(name + "-off", source, {**settings, **control_settings}),
                "backends": ["cpu"], "disabled_effects": effect,
                "required_stages": [s for s in required if s not in control_not_run],
                "required_not_run_stages": list(control_not_run),
                "expect_rgb_difference": True}
        scenes.append(row)

    add("sphereifs-ao-shadows", "SphereIFS: AO and hard shadows", "sphere",
        ["ao", "shadows"], metal=True)
    add("boxifs-ao-shadows", "boxIFS: AO and hard shadows", "box",
        ["ao", "shadows"], metal=True)
    add("integerpower8-ao-shadows", "Integer Power 8: AO and hard shadows", "bulb",
        ["ao", "shadows"], metal=True)
    add("integerpower8-reflections", "Integer Power 8: two reflection bounces", "bulb",
        ["reflections"], {"bCalcSRautomatic": 1, "SRreflectioncount": 2, "SRamount": 0.8},
        stages=["reflections"], control_settings={"bCalcSRautomatic": 0},
        control_not_run=["reflections"],
        unsupported="The exact Metal scene backend rejects the ordinary reflection pass.")
    add("sphereifs-transparency", "SphereIFS: refraction and transmission", "sphere",
        ["transparency"], {"bCalcSRautomatic": 3, "SRreflectioncount": 3, "SRamount": 0.65,
                             "sTRIndex": 1.3, "sTRscattering": 0.0, "sTransmissionAbsorption": 0.2},
        stages=["reflections"], control_settings={"bCalcSRautomatic": 1},
        unsupported="The exact Metal scene backend rejects reflection/transmission; the control retains reflections and only disables transmission.")
    add("integerpower8-volume", "Integer Power 8: volumetric light", "bulb",
        ["volume"], {"bVolLightNr": 33, "Light.TBpos[3]": 100},
        stages=["volume"], control_settings={"bVolLightNr": 32}, control_not_run=["volume"],
        unsupported="The exact Metal scene backend rejects volumetric-light map preparation and sampling.")
    add("integerpower8-dof", "Integer Power 8: sorted depth of field", "bulb",
        ["dof"], {"bCalcDOFtype": 1, "sDOFZsharp": 0.0, "sDOFZsharp2": 0.0,
                  "sDOFaperture": 0.3, "sDOFclipR": 12.0},
        stages=["dof"], control_settings={"bCalcDOFtype": 0}, control_not_run=["dof"],
        unsupported="The exact Metal scene backend rejects ordered depth-of-field postprocessing.")
    add("integerpower8-zbuffer-normals", "Integer Power 8: Z-buffer normal reconstruction", "bulb",
        ["normals"], {"byCalcNsOnZBufAuto": 1}, stages=["normals"],
        control_settings={"byCalcNsOnZBufAuto": 0}, control_not_run=["normals"],
        unsupported="The exact Metal scene backend rejects the separate Z-buffer normals pass.")
    # Same production diffuse-map encoding used by prepare_render_maps.py.
    add("sphereifs-diffuse-map", "SphereIFS: real diffuse bitmap", "sphere",
        ["diffuse_map"], {"Light.bColorMap": 9001 & 255,
                          "Light.Lights[1].AdditionalByteEx.map_number_high": 9001 >> 8,
                          "Light.Lights[1].FreeByte.map_projection": 0,
                          "Light.Lights[2].AdditionalByteEx.map_scale": 30},
        control_settings={"Light.bColorMap": 0, "Light.Lights[1].AdditionalByteEx.map_number_high": 0},
        assets=["map9001"],
        unsupported="The exact Metal color backend rejects loaded diffuse maps; color is the consuming CPU stage, not a separate map stage.")

    manifest = {
        "schema": "mb3d.render-benchmark-selection.v1", "name": "effects-v1",
        "description": "Nine finite ordinary workloads and twelve effect-off controls. These are new benchmark inputs, not frozen references or historical parity results.",
        "origin_source_commit": ORIGIN_COMMIT,
        "generator": {"path": Path(__file__).relative_to(ROOT).as_posix(),
                      "sha256": sha(Path(__file__).read_bytes())},
        "expected_count": 9, "expected_control_count": 12,
        "default_dimensions": {"width": 512, "height": 384},
        "smoke_dimensions": {"width": 64, "height": 48},
        "excluded": ["Monte Carlo", "JIT"],
        "assets": [ASSET],
        "acceptance": {"require_all_selected_scenes": True, "require_nonconstant_rgb": True,
                       "require_effect_control_rgb_difference": True,
                       "require_declared_stages_completed": True,
                       "repeat_rgb": "exact", "cpu_metal_rgb": "exact_for_common_scenes",
                       "note": "Flags alone do not prove an active effect. Each control must render successfully and change RGB; unsupported Metal effects must be reported explicitly."},
        "scenes": scenes,
    }
    assert len(scenes) == manifest["expected_count"]
    assert sum(len(row.get("effect_controls", [])) + int("effect_control" in row) for row in scenes) == 12
    outputs[MANIFEST] = (json.dumps(manifest, indent=2) + "\n").encode()
    return outputs


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--check", action="store_true", help="Verify exact reproducibility without writing")
    mode.add_argument("--replace-generated", action="store_true", help="Replace only files named by this generator")
    args = parser.parse_args()
    outputs = generate()
    for relative, expected in outputs.items():
        path = ROOT / relative
        if path.exists() and path.read_bytes() == expected:
            continue
        if args.check:
            raise SystemExit("Generated fixture differs or is absent: " + str(relative))
        if path.exists() and not args.replace_generated:
            raise SystemExit("Refusing to replace differing generated file: " + str(relative))
    if not args.check:
        for relative, payload in outputs.items():
            path = ROOT / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(payload)
    print(json.dumps({"status": "MATCH" if args.check else "GENERATED", "scenes": 9,
                      "controls": 12, "files": len(outputs), "manifest": str(MANIFEST)}))


if __name__ == "__main__":
    main()
