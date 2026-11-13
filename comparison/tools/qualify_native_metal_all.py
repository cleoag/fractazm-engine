#!/usr/bin/env python3
"""Fail-closed, passive qualification of the pinned FULL ordinary Metal scope.

This does not render, build, repair inputs, execute persisted commands, or inherit
historical PASS results. Missing evidence is OPEN. Evidence is a retained bundle
with current-source/binary binding; independently reviewed receipts complement,
but never replace, direct PNG/profile/input checks. See --write-schema-example.
"""
from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
import math
from pathlib import Path
import re
import struct
import subprocess
import sys
import zlib

ROOT = Path(__file__).resolve().parents[2]
SCHEMA = "mb3d.native-metal-all.evidence.v1"
RECEIPT_SCHEMA = "mb3d.native-metal-all.receipt.v1"
STAGES = {"load", "preparation", "volume", "primary", "normals", "shadows", "ao", "color", "reflections", "dof", "final_surface", "save"}
GLOBAL_RECEIPTS = ("build_provenance", "independent_heldout", "cpu_ordinary", "metal_compatible",
                   "native_regressions", "guard_failures", "full_render_timings", "no_cpu_pixel_work")
SOURCE_ROOTS = ("CMakeLists.txt", "cmake", "src", "include", "external_formulas", "third_party", "libs",
                "comparison/metal_exact", "comparison/tools", "tests", "scripts")
SOURCE_SUFFIXES = {".c", ".cpp", ".h", ".hpp", ".mm", ".metal", ".inc", ".asm", ".cmake", ".py", ".json", ".txt"}


class InvalidEvidence(ValueError):
    pass


def require(condition, message):
    if not condition:
        raise InvalidEvidence(message)


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False).encode()).hexdigest()


def read_json(path):
    def pairs(items):
        obj = {}
        for key, value in items:
            require(key not in obj, "Duplicate JSON key: " + key)
            obj[key] = value
        return obj
    return json.loads(Path(path).read_text(), object_pairs_hook=pairs,
                      parse_constant=lambda value: (_ for _ in ()).throw(InvalidEvidence("Nonfinite JSON: " + value)))


def contained(base, name):
    require(isinstance(name, str) and bool(name) and not Path(name).is_absolute(), "Expected relative artifact path")
    path = (Path(base) / name).resolve()
    require(path.is_relative_to(Path(base).resolve()), "Artifact escapes its root: " + name)
    return path


def artifact(base, record):
    require(isinstance(record, dict), "Missing artifact identity")
    path = contained(base, record.get("path"))
    require(path.is_file() and sha(path) == record.get("sha256"), "Missing/stale artifact: " + str(path))
    return path


def unique(rows, key, label):
    require(isinstance(rows, list), label + " must be a list")
    result = {}
    for row in rows:
        require(isinstance(row, dict), "Invalid " + label + " record")
        ident = key(row)
        require(ident not in result, "Duplicate " + label + ": " + str(ident))
        result[ident] = row
    return result


def formula_key(row):
    require(row.get("namespace") in ("internal", "external") and type(row.get("id")) is int
            and isinstance(row.get("name"), str), "Malformed formula identity")
    return (row["namespace"], row["id"], row["name"])


def current_registry(root):
    # Only identity extraction: implementation/hash changes are expected during
    # migration and do not invalidate the immutable original inventory.
    from audit_backend_readiness import Audit, CONSTANT, strip_comments
    audit = Audit(root)
    result = set()
    for name, records in audit.registrations().items():
        require(len(records) == 1, "Nonunique current registration: " + name)
        row = records[0]
        candidates = row["candidates"]
        if candidates:
            require(len(candidates) == 1, "Ambiguous current ID: " + name)
            number = candidates[0][1]
        else:
            numbers = [int(m[2]) for m in CONSTANT.finditer(audit.read(row["path"])) if m[1] == row["token"]]
            require(len(numbers) == 1, "Missing current native ID: " + name)
            number = numbers[0]
        result.add(("external", number, name))
    source = strip_comments((root / "src/core/formula_registry.cpp").read_text())
    match = re.search(r'InternFormulaNames\s*=\s*\{\{(.*?)\}\}', source, re.S)
    require(match is not None, "Cannot read current internal registry")
    result.update(("internal", i, name) for i, name in enumerate(re.findall(r'"([^"\\]+)"', match[1])))
    return result


def source_snapshot(root):
    names = subprocess.check_output(["git", "ls-files", "--cached", "--others", "--exclude-standard", "-z", "--", *SOURCE_ROOTS], cwd=root)
    return {name: sha(root / name) for name in sorted(set(names.decode().split("\0")) - {""})
            if (root / name).is_file() and (root / name).suffix in SOURCE_SUFFIXES}


def load_additional_scope(contract, directory, root, pinned_scenes):
    """Optional reviewed finite extensions; never a replacement denominator."""
    name=contract.get("additional_scene_inventory")
    if name is None:return {}
    path=contained(directory,name)
    require(path.is_file() and sha(path)==contract.get("inventory_bindings",{}).get(name),"Additional inventory hash mismatch")
    inventory=read_json(path)
    require(inventory.get("schema")=="mb3d.native-metal-all.additional-scenes.v1"
            and inventory.get("review_status")=="APPROVED_ORDINARY_CASES"
            and isinstance(inventory.get("reviewer"),str) and bool(inventory["reviewer"]),"Additional ordinary-case review missing")
    artifact(root,inventory.get("generator"))
    require(isinstance(inventory.get("generation_inputs"),list) and bool(inventory["generation_inputs"]),"Additional generation inputs missing")
    for item in inventory["generation_inputs"]:artifact(root,item)
    members=unique(inventory.get("members"),lambda r:r.get("id"),"additional scene ID")
    require(bool(members),"Empty additional inventory")
    paths=set()
    for ident,row in members.items():
        require(isinstance(ident,str) and re.fullmatch(r"[A-Za-z0-9_-]+",ident),"Unsafe additional scene ID")
        name=row.get("path");require(isinstance(name,str) and name.lower().endswith(".m3p")
                                    and name not in paths and name not in pinned_scenes,"Additional scene path collides with pinned/other input")
        paths.add(name);source=artifact(root,row);data=source.read_bytes()
        require(not re.search(r"(?:monte.?carlo|halton|jit|(?:^|[^a-z])mc(?:[^a-z]|$))",name,re.I),"Deferred additional input forbidden")
        modern=len(data)==1976 and int.from_bytes(data[:4],"little")==44
        if modern:
            require(not(data[124]&1),"Deferred additional header forbidden")
            require(not any(b"jit" in data[860+188*i:892+188*i].split(b"\0",1)[0].lower() for i in range(6)),"Deferred additional formula forbidden")
        require(row.get("scope_class")==("ORDINARY_HEADER_SCREENED" if modern else "UNCLASSIFIED_DO_NOT_EXECUTE"),"Additional scope classification is not independently screened")
        row["_source_path"]=str(source)
    return members


def load_scope(contract_path, root, registry_reader=current_registry):
    contract = read_json(contract_path)
    require(isinstance(contract, dict), "Invalid contract")
    inventories = {}
    for field in ("formula_inventory", "scene_inventory"):
        name = contract.get(field)
        path = contained(contract_path.parent, name)
        require(path.is_file() and sha(path) == contract.get("inventory_bindings", {}).get(name), "Pinned inventory hash mismatch: " + str(name))
        inventories[field] = read_json(path)
    formulas = unique(inventories["formula_inventory"].get("identities"), formula_key, "formula")
    expected = contract.get("formula_identities", {})
    require(expected == {"external":457,"internal":10,"total":467}, "This FULL467 predicate cannot qualify a reduced formula contract")
    counts = Counter(k[0] for k in formulas)
    require(len({k[:2] for k in formulas}) == len(formulas), "Duplicate formula numeric identity")
    require(len(formulas) == expected.get("total") and counts["external"] == expected.get("external")
            and counts["internal"] == expected.get("internal"), "Full formula denominator changed")
    require(set(formulas) == registry_reader(root), "Current formula names/IDs differ from pinned identities")
    scenes = unique(inventories["scene_inventory"].get("members"), lambda r: r.get("path"), "scene")
    require(len(scenes) == inventories["scene_inventory"].get("summary", {}).get("paths"), "Scene inventory is incomplete")
    for name, row in scenes.items():
        require(row.get("scope_class") in ("DEFERRED_EXCLUDED", "ORDINARY_HEADER_SCREENED", "UNCLASSIFIED_DO_NOT_EXECUTE"), "Unknown scene scope")
        require(sha(contained(root, name)) == row.get("sha256"), "Pinned scene changed: " + name)
        row["_source_path"]=str(contained(root,name))
    for asset in inventories["scene_inventory"].get("known_declared_assets", []):
        require(sha(contained(root, asset["source_path"])) == asset["sha256"], "Pinned resource changed")
    features = {"inventory:" + r["id"] for r in inventories["scene_inventory"].get("feature_obligations", [])}
    features |= {"feature:" + x for x in contract.get("scope", {}).get("features", [])}
    features |= {"variant:" + x for x in contract.get("required_variants", [])}
    additional=load_additional_scope(contract,contract_path.parent,root,scenes)
    return {"contract": contract, "contract_sha256": sha(contract_path), "formulas": formulas,
            "scenes": scenes,"additional_scenes":additional, "features": features, "inventory_bindings": contract["inventory_bindings"]}


def image_rgb(path):
    """Decode bounded RGB8/RGBA8 PNGs, checking CRC and every scanline."""
    data = Path(path).read_bytes()
    require(data[:8] == b"\x89PNG\r\n\x1a\n", "Not a PNG")
    offset, compressed, header, ended = 8, bytearray(), None, False
    while offset < len(data):
        require(offset + 12 <= len(data), "Truncated PNG chunk")
        size = struct.unpack_from(">I", data, offset)[0]
        tag = data[offset + 4:offset + 8]; payload = data[offset + 8:offset + 8 + size]
        require(offset + size + 12 <= len(data), "Truncated PNG payload")
        require(zlib.crc32(tag + payload) & 0xffffffff == struct.unpack_from(">I", data, offset + 8 + size)[0], "PNG CRC mismatch")
        if tag == b"IHDR":
            require(header is None and size == 13, "Invalid PNG header")
            header = struct.unpack(">IIBBBBB", payload)
        elif tag == b"IDAT": compressed.extend(payload)
        elif tag == b"IEND": ended = True; offset += size + 12; break
        offset += size + 12
    require(ended and offset == len(data) and header is not None, "Incomplete PNG")
    width, height, depth, color, compression, filtering, interlace = header
    require(0 < width <= 4096 and 0 < height <= 4096 and depth == 8 and color in (2, 6)
            and compression == filtering == interlace == 0, "Unsupported PNG format")
    channels = 3 if color == 2 else 4; stride = width * channels
    decoder = zlib.decompressobj(); raw = decoder.decompress(compressed, (stride + 1) * height + 1)
    require(len(raw) == (stride + 1) * height and decoder.eof and not decoder.unused_data, "Invalid PNG scanline size")
    previous = bytearray(stride); rgb = bytearray()
    for y in range(height):
        start = y * (stride + 1); mode = raw[start]; row = bytearray(raw[start + 1:start + 1 + stride])
        require(mode <= 4, "Invalid PNG filter")
        for x in range(stride):
            a = row[x - channels] if x >= channels else 0; b = previous[x]; c = previous[x - channels] if x >= channels else 0
            if mode == 1: prediction = a
            elif mode == 2: prediction = b
            elif mode == 3: prediction = (a + b) // 2
            elif mode == 4:
                p = a + b - c; distances = (abs(p-a), abs(p-b), abs(p-c)); prediction = (a,b,c)[distances.index(min(distances))]
            else: prediction = 0
            row[x] = (row[x] + prediction) & 255
        for x in range(0, stride, channels): rgb.extend(row[x:x+3])
        previous = row
    return width, height, bytes(rgb)


def receipt(base, record, kind, binding_id):
    value = read_json(artifact(base, record))
    require(value.get("schema") == RECEIPT_SCHEMA and value.get("kind") == kind
            and value.get("status") == "PASS" and value.get("binding_sha256") == binding_id, "Stale/invalid " + kind + " receipt")
    require(isinstance(value.get("producer"), str) and bool(value["producer"])
            and isinstance(value.get("independent_reviewer"), str) and bool(value["independent_reviewer"])
            and value["producer"] != value["independent_reviewer"], "Missing independent review: " + kind)
    checks = unique(value.get("checks"), lambda r:r.get("id"), "receipt check")
    require(bool(checks) and all(isinstance(k,str) and k and r.get("status") == "PASS" for k,r in checks.items()), "Failed/missing receipt checks")
    require(isinstance(value.get("artifacts"), list) and bool(value["artifacts"]), "Receipt has no retained observations")
    for item in value["artifacts"]: artifact(base, item)
    return value


def finite_nonnegative(value):
    return type(value) in (int, float) and math.isfinite(value) and value >= 0


DOF_EXECUTION_FIELDS = (
    "dof_records", "dof_passes", "dof_positive_radius_sources", "dof_sort_work",
    "dof_candidate_scan_checks", "dof_candidate_checks", "dof_contributing_pairs",
    "dof_changed_pixels", "dof_maximum_radius", "dof_dispatches",
)

ZNORMAL_EXECUTION_FIELDS = (
    "znormal_records", "znormal_foreground", "znormal_changed", "znormal_neighbors", "znormal_dispatches",
)
NORMAL_EXECUTION_FIELDS = (
    "normal_samples", "normal_de_calls", "normal_orbit_calls", "normal_evaluations", "primary_rough_pixels",
)


def validate_normal_execution(execution, count, zactive):
    if zactive:
        require(execution.get("engine") == "general_formula", "Z normals used an unsupported engine")
        require(all(type(execution.get(k)) is int and execution[k] >= 0 for k in ZNORMAL_EXECUTION_FIELDS),
                "Missing/invalid Z normal GPU records")
        require(execution["znormal_records"] == count and execution["znormal_dispatches"] == 3
                and execution["znormal_changed"] <= execution["znormal_foreground"] <= count
                and execution["znormal_neighbors"] <= 4 * execution["znormal_foreground"],
                "Z normal activity outside execution bounds")
    else:
        require(all(k not in execution or (type(execution[k]) is int and execution[k] == 0)
                    for k in ZNORMAL_EXECUTION_FIELDS), "Inactive Z normals claimed GPU work")
    # Older source-bound records do not have the advanced-normal extension.
    # Once any field is present, the complete extension must be consistent.
    if any(k in execution for k in NORMAL_EXECUTION_FIELDS):
        require(all(type(execution.get(k)) is int and execution[k] >= 0 for k in NORMAL_EXECUTION_FIELDS),
                "Missing/invalid advanced normal GPU records")
        samples, evaluations = execution["normal_samples"], execution["normal_evaluations"]
        require(samples == execution["normal_de_calls"] + execution["normal_orbit_calls"]
                and 7 * evaluations <= samples <= 161 * evaluations
                and execution["primary_rough_pixels"] <= min(count, evaluations),
                "Advanced normal work counters are inconsistent")
        if execution.get("engine") != "general_formula":
            require(all(execution[k] == 0 for k in NORMAL_EXECUTION_FIELDS),
                    "Specialized engine claimed advanced-normal records")


def validate_dof_execution(execution, count, active):
    if not active:
        # Older inactive profiles predate these fields; they cannot claim work.
        require(all(k not in execution or (type(execution[k]) is int and execution[k] == 0)
                    for k in DOF_EXECUTION_FIELDS), "Inactive DOF claimed GPU work")
        return
    require(all(type(execution.get(k)) is int and execution[k] >= 0 for k in DOF_EXECUTION_FIELDS),
            "Missing/invalid DOF GPU records")
    passes = execution["dof_passes"]
    require(1 <= passes <= 4 and execution["dof_records"] == count * passes,
            "DOF pass/record count mismatch")
    require(execution["dof_dispatches"] > 0
            and execution["dof_positive_radius_sources"] <= count * passes
            and execution["dof_changed_pixels"] <= count
            and execution["dof_maximum_radius"] <= 1000,
            "DOF activity outside execution bounds")
    # A valid focused image may be unchanged. Discriminating feature controls
    # must establish visible activity separately from this stage-completeness check.


def validate_dof_control_pair(results):
    require(len(results) == 2 and results[0].get("gpu_engine") == results[1].get("gpu_engine")
            and results[0].get("gpu_engine") in ("exact_capability", "general_formula"),
            "DOF control pair changed GPU engine")
    require(any(r.get("dof_discriminating_activity_observed") is True for r in results),
            "DOF control lacks actual positive blur activity")


def validate_profile(profile, backend, stages):
    require(profile.get("status") == "complete" and profile.get("render_status") == "completed"
            and profile.get("exit_code") == 0 and profile.get("error") == "", "Failed render profile")
    require(profile.get("backend") == backend and profile.get("backend_requested") == backend
            and profile.get("backend_fallback") is False and profile.get("backend_fallback_code") == ""
            and profile.get("backend_fallback_reason") == "", "CPU fallback or backend mismatch")
    require(profile.get("precision") == "native" and profile.get("numeric_profile") == "native_" + ("metal_fp32" if backend == "metal" else "cpu_fp64"), "Wrong arithmetic profile")
    require(profile.get("schema") == ("mb3d.render-stage-profile.v1" if backend == "metal" else "mb3d.cpu-stage-profile.v1"), "Wrong profile schema")
    actual = profile.get("stages", {})
    require(set(actual) == STAGES and set(stages) == STAGES, "Missing/unknown stage inventory")
    for name, stage in actual.items():
        require(stages[name] in ("completed", "not_run") and stage.get("state") == stages[name]
                and finite_nonnegative(stage.get("seconds")), "Wrong/nonfinite stage: " + name)
        require(stage["state"] != "not_run" or stage["seconds"] == 0, "Inactive stage has time: " + name)
    require(all(stages[s] == "completed" for s in ("load", "preparation", "primary", "color", "final_surface", "save")), "Required full-render stage omitted")
    if backend == "metal":
        require(profile.get("workers") == 0 and bool(profile.get("metal_device")), "Not a GPU profile")
        execution=profile.get("metal_execution",{})
        require(execution.get("validated") is True and execution.get("library_profile")==[2,4,32],"Missing validated GPU execution/library marker")
        engine=execution.get("engine")
        require(engine in ("exact_capability","general_formula"),"Missing/unknown GPU execution engine")
        width,height=profile.get("width"),profile.get("height")
        require(type(width) is int and type(height) is int and width>0 and height>0,"Invalid execution dimensions")
        count=width*height
        expected={"primary_records":count,"normal_records":count,"surface_records":2*count,"color_records":count,
                  "shadow_records":count*(2 if engine=="exact_capability" else 1) if stages["shadows"]=="completed" else 0,
                  "reflection_records":count if stages["reflections"]=="completed" else 0,
                  "faults":0,"nonfinite":0,"work_exhausted":0}
        require(all(type(execution.get(k)) is int and execution[k]==v for k,v in expected.items()),"GPU execution record count/fault/nonfinite/work mismatch")
        validate_dof_execution(execution, count, stages["dof"] == "completed")
        validate_normal_execution(execution, count, stages["normals"] == "completed")
        reflection = profile.get("metal_reflections", {})
        require(reflection.get("faults") == 0, "Reflection faults/missing records")
        if stages["reflections"] == "completed":
            require(reflection.get("enabled") is True and reflection.get("rays",0) > 0, "Reflection pass inactive")
    else:
        require(profile.get("metal_execution",{}).get("validated") is False,"CPU profile claimed/missed GPU execution metadata")


def check_frame(base, row, backend, scene, expected_stages, binding):
    profile_path = artifact(base, row.get("profile")); profile = read_json(profile_path)
    validate_profile(profile, backend, expected_stages)
    png = artifact(base, row.get("image")); artifact(base, row.get("log"))
    width, height, rgb = image_rgb(png)
    require((profile.get("width"),profile.get("height")) == (width,height), "PNG/profile dimensions differ")
    require(row.get("input_sha256") == scene["sha256"] and row.get("exit_code") == 0, "Wrong scene/process status")
    argv = row.get("argv"); require(isinstance(argv,list) and all(isinstance(x,str) for x in argv), "Missing exact argv")
    def option(name):
        require(argv.count(name) == 1 and argv.index(name) + 1 < len(argv), "Missing/duplicate argument: " + name)
        return argv[argv.index(name)+1]
    require(Path(argv[0]).resolve() == Path(binding["renderer"]["path"]).resolve(), "Wrong executable in argv")
    require(option("--backend") == backend and option("--precision") == "native", "Wrong argv backend/precision")
    require(Path(profile["input"]).resolve() == Path(row.get("input_path", "")).resolve()
            and sha(Path(profile["input"])) == scene["sha256"], "Profile input is not pinned scene")
    require(Path(profile["input"]).resolve()==Path(scene["_source_path"]).resolve(),"Identical-byte alias cannot substitute scene/resource path identity")
    require(str(profile["input"]) in argv and Path(profile["output"]).resolve() == Path(row.get("output_path", "")).resolve(), "Unbound profile IO")
    require(str(profile["output"]) in argv and Path(option("--profile-json")).resolve() == profile_path,
            "Output/profile files not bound to argv")
    require(sha(Path(profile["output"])) == row["image"]["sha256"], "Published image differs from retained image")
    if backend == "metal":
        require(Path(option("--metal-library")).resolve() == Path(binding["library"]["path"]).resolve()
                and Path(profile.get("metal_library", "")).resolve() == Path(binding["library"]["path"]).resolve(), "Wrong library in frame")
    return width, height, rgb


def validate_scene(base, row, scene, binding, binding_id, allow_effect_overrides=False):
    require(row.get("input_sha256") == scene["sha256"], "Wrong scene evidence input")
    admission = receipt(base, row.get("classification"), "scene_classification", binding_id)
    require(admission.get("input_sha256") == scene["sha256"], "Classification is not bound to input")
    classification = admission.get("classification")
    if admission.get("scope") == "deferred":
        require(scene["scope_class"] == "UNCLASSIFIED_DO_NOT_EXECUTE" and classification == "deferred"
                and admission.get("deferred_kind") in ("Monte Carlo","JIT") and admission.get("ordinary_loader_inspected") is True,
                "Known ordinary scene cannot be silently excluded")
        require(not row.get("frames") and not row.get("cpu_reference"), "Deferred scene must not be executed")
        return {"classification":"deferred","deferred_kind":admission["deferred_kind"]}
    require(admission.get("scope") == "ordinary", "Unclassified/deferred scene cannot be rendered")
    if classification == "invalid":
        rejection = receipt(base, row.get("rejection"), "bounded_invalid_rejection", binding_id)
        require(rejection.get("input_sha256") == scene["sha256"] and rejection.get("image_published") is False
                and rejection.get("bounded") is True, "Invalid-input rejection not proved")
        require(not row.get("frames"), "Invalid input has accepted render frames")
        return {"classification":"invalid"}
    require(classification == "valid" and admission.get("resource_closure_complete") is True, "Scene/resource classification remains open")
    require(isinstance(admission.get("resources"),list), "Missing resolved resource inventory")
    for resource in admission["resources"]: artifact(base, resource)
    expected_stages = admission.get("expected_stages", {})
    features=admission.get("effective_features",{})
    overrides=features.get("effect_overrides",{})
    require(set(overrides)=={"no_ao","no_shadows","no_dof","no_reflections"}
            and all(type(v) is bool for v in overrides.values()),"Effective effect overrides not recorded")
    require(allow_effect_overrides or not any(overrides.values()),"Baseline scene cannot drop effects through CLI overrides")
    if overrides["no_ao"]:require(expected_stages.get("ao")=="not_run","no_ao must skip image AO stage")
    frames = row.get("frames"); require(isinstance(frames,list) and len(frames)>=3, "Fewer than three Metal repeats")
    for frame in [*frames,row.get("cpu_reference",{})]:
        argv=frame.get("argv",[])
        require(all(("--"+key.replace("_","-" ) in argv)==enabled for key,enabled in overrides.items()),
                "Recorded effect overrides differ from actual argv")
    require(len({r.get("process_id") for r in frames})>=3 and all(isinstance(r.get("process_id"),str) and r["process_id"] for r in frames), "Fresh process repeats missing")
    require(len({r["profile"]["path"] for r in frames}) == len(frames)
            and len({r["image"]["path"] for r in frames}) == len(frames), "Repeated evidence reused")
    images=[check_frame(base,r,"metal",scene,expected_stages,binding) for r in frames]
    require(all(x==images[0] for x in images), "Metal RGB repeats differ")
    executions=[read_json(artifact(base,r["profile"]))["metal_execution"] for r in frames]
    require(len({e["engine"] for e in executions})==1,"Metal repeats changed GPU engine")
    normal_records = [{k: e.get(k) for k in ZNORMAL_EXECUTION_FIELDS + NORMAL_EXECUTION_FIELDS} for e in executions]
    require(all(r == normal_records[0] for r in normal_records), "Normal GPU work repeats differ")
    dof_activity=False
    if expected_stages["dof"]=="completed":
        records=[{k:e[k] for k in DOF_EXECUTION_FIELDS} for e in executions]
        require(all(r==records[0] for r in records),"DOF GPU work repeats differ")
        dof_activity=all(all(e[k]>0 for k in ("dof_positive_radius_sources", "dof_candidate_scan_checks",
            "dof_candidate_checks", "dof_contributing_pairs", "dof_changed_pixels", "dof_maximum_radius")) for e in executions)
    gpu=receipt(base,row.get("gpu_records"),"scene_gpu_records",binding_id)
    require(gpu.get("input_sha256")==scene["sha256"] and gpu.get("profile_sha256s")==[r["profile"]["sha256"] for r in frames], "GPU records not bound to frames")
    require(gpu.get("library_marker")==[2,4,32] and gpu.get("cpu_pixel_calls")==0, "Wrong GPU arithmetic/CPU pixel work")
    require(gpu.get("faults")==0 and gpu.get("nonfinite")==0 and gpu.get("work_exhausted")==0, "Fault/nonfinite/exhausted GPU work")
    require(gpu.get("effective_features")==admission.get("effective_features") and bool(gpu.get("effective_features")), "Dropped/unobserved scene features")
    cpu=check_frame(base,row.get("cpu_reference",{}),"cpu",scene,expected_stages,binding)
    require(cpu[:2]==images[0][:2],"CPU/Metal size differs")
    difference=sum(cpu[2][i:i+3]!=images[0][2][i:i+3] for i in range(0,len(cpu[2]),3))
    require(row.get("cpu_metal_differing_pixels")==difference,"Incorrect CPU/Metal difference count")
    point_ao=False
    if overrides["no_ao"] and expected_stages.get("reflections")=="completed" and features.get("header_ao_enabled") is True:
        activities=[read_json(artifact(base,r["profile"])).get("metal_reflections",{}) for r in frames]
        require(all(a.get("hits",0)==0 or a.get("ao_rays",0)>0 for a in activities),"no_ao incorrectly disabled reflection point AO")
        point_ao=all(a.get("hits",0)>0 and a.get("ao_rays",0)>0 for a in activities)
    return {"classification":"valid","rgb_sha256":hashlib.sha256(images[0][2]).hexdigest(),"cpu_metal_differing_pixels":difference,
            "no_ao_reflection_point_ao_observed":point_ao,"gpu_engine":executions[0]["engine"],
            "dof_discriminating_activity_observed":dof_activity}


def formula_observations(base, record, identity, binding_id):
    """Exact FP32 state fixtures selected/reviewed independently of the GPU code.

    Native CPU image differences remain permitted. This fixture contract uses
    independently specified FP32 output bits, not a generic tolerance against
    FP64 or a producer's unexamined PASS boolean.
    """
    data=read_json(artifact(base,record))
    require(data.get("schema")=="mb3d.native-metal-all.formula-observations.v1"
            and data.get("binding_sha256")==binding_id and data.get("library_marker")==[2,4,32]
            and data.get("execution")=="metal_gpu" and data.get("cpu_pixel_calls")==0,
            "Missing current actual GPU formula observations")
    cases=unique(data.get("cases"),lambda r:r.get("case_id"),"formula case")
    require(bool(cases),"No actual formula cases")
    kinds=Counter();variants=set()
    for case in cases.values():
        require(tuple(case.get("identity",[]))==identity and case.get("kind") in ("point","orbit")
                and type(case.get("gpu_invocations")) is int and case["gpu_invocations"]>0
                and case.get("faults")==0,"Wrong/inactive GPU formula case")
        expected=case.get("expected_state_bits");actual=case.get("gpu_state_bits")
        require(isinstance(expected,dict) and bool(expected) and actual==expected
                and all(isinstance(k,str) and re.fullmatch(r"0x[0-9a-fA-F]{8}",v or "") for k,v in expected.items()),
                "Independent FP32 state fixture mismatch")
        require(isinstance(case.get("input_state_bits"),dict) and bool(case["input_state_bits"])
                and isinstance(case.get("parameter_bits"),list),"Formula inputs/parameters missing")
        require(isinstance(case.get("read_lanes"),list) and isinstance(case.get("written_lanes"),list)
                and bool(case["written_lanes"]) and set(case["written_lanes"])<=set(expected),"State write contract unobserved")
        kinds[case["kind"]]+=1;variants.add(case.get("variant"))
    require(kinds["point"]>0 and kinds["orbit"]>0,"Point and orbit observations both required")
    if identity[:2]==("internal",0):require({f"power-{p}" for p in range(2,9)}<=variants,"Integer Power2..8 variant coverage incomplete")
    return cases


def evaluate(scope, evidence, base, binding):
    """Missing rows remain OPEN; malformed supplied rows are INVALID and open."""
    remaining={"formulas":len(scope["formulas"]),"scenes":sum(r["scope_class"]!="DEFERRED_EXCLUDED" for r in scope["scenes"].values()),
               "additional_scenes":len(scope.get("additional_scenes",{})),"features":len(scope["features"]),"receipts":len(GLOBAL_RECEIPTS),"binding":1}
    report={"status":"OPEN","remaining_by_kind":remaining,"accepted":{"formulas":[],"scenes":[],"additional_scenes":[],"features":[],"receipts":[]},"scene_results":{},"errors":[],
            "excluded_scenes":[p for p,r in scope["scenes"].items() if r["scope_class"]=="DEFERRED_EXCLUDED"],
            "unclassified_scenes":[p for p,r in scope["scenes"].items() if r["scope_class"]=="UNCLASSIFIED_DO_NOT_EXECUTE"],
            "cpu_render_fallbacks_accepted":0}
    if evidence is None:
        report["note"]="No current evidence bundle. Inventory/admission is not GPU qualification."
    else:
        try:
            require(evidence.get("schema")==SCHEMA and evidence.get("binding")==binding, "Stale/mismatched current source/binary/inventory binding")
            require(binding.get("renderer") and binding.get("library"), "Missing current build artifacts")
            binding_id=digest(binding);remaining["binding"]=0
            formulas=unique(evidence.get("formulas",[]),formula_key,"formula evidence")
            scenes=unique(evidence.get("scenes",[]),lambda r:r.get("path"),"scene evidence")
            additional=unique(evidence.get("additional_scenes",[]),lambda r:r.get("id"),"additional scene evidence")
            features=unique(evidence.get("features",[]),lambda r:r.get("id"),"feature evidence")
            receipts=unique(evidence.get("receipts",[]),lambda r:r.get("kind"),"global receipt")
            require(set(formulas)<=set(scope["formulas"]),"Unknown/deferred formula evidence")
            require(set(scenes)<=set(scope["scenes"]),"Unknown scene evidence cannot replace pinned scene")
            require(all(scope["scenes"][p]["scope_class"]!="DEFERRED_EXCLUDED" for p in scenes),"Deferred scene evidence forbidden")
            require(set(additional)<=set(scope.get("additional_scenes",{})),"Additional evidence lacks pinned reviewed inventory")
            require(set(features)<=scope["features"] and set(receipts)<=set(GLOBAL_RECEIPTS),"Unknown evidence obligation")
            working_inputs={}
            def attempt(kind, ident, operation):
                try: operation();remaining[kind]-=1;report["accepted"][kind].append(ident)
                except (InvalidEvidence,KeyError,TypeError,ValueError,OSError) as error: report["errors"].append(f"{kind}:{ident}: {error}")
            for key,row in formulas.items():
                def verify_formula(row=row,key=key):
                    proof=receipt(base,row.get("proof"),"formula_gpu_point_orbit",binding_id)
                    cases=formula_observations(base,proof.get("observations"),key,binding_id)
                    working_inputs[key]={digest({k:c.get(k) for k in ("kind","input_state_bits","parameter_bits","variant")}) for c in cases.values()}
                    require(tuple(proof.get("identity",[]))==key and proof.get("gpu_point_cases",0)>0 and proof.get("gpu_orbit_cases",0)>0
                            and proof.get("faults")==0 and proof.get("state_contract_verified") is True
                            and proof.get("parameter_contract_verified") is True and proof.get("cpu_pixel_calls")==0,
                            "Formula point/orbit/state/parameter proof incomplete")
                attempt("formulas",list(key),verify_formula)
            for path,row in scenes.items():
                def verify_scene(row=row,path=path):
                    report["scene_results"][path]=validate_scene(base,row,scope["scenes"][path],binding,binding_id)
                attempt("scenes",path,verify_scene)
            for ident,row in additional.items():
                def verify_additional(ident=ident,row=row):
                    scene=scope["additional_scenes"][ident]
                    require(row.get("path")==scene["path"],"Additional scene ID/path mismatch")
                    result=validate_scene(base,row,scene,binding,binding_id)
                    require(result["classification"]=="valid","Additional discriminating input must be a valid ordinary scene")
                    report["scene_results"]["additional:"+ident]=result
                attempt("additional_scenes",ident,verify_additional)
            for key,row in features.items():
                def verify_feature(row=row,key=key):
                    proof=receipt(base,row.get("proof"),"feature_controls",binding_id)
                    require(proof.get("feature")==key and proof.get("actual_gpu_work",0)>0 and proof.get("negative_control_verified") is True, "Missing active feature/control proof")
                    valid={p for p,r in report["scene_results"].items() if r["classification"]=="valid"}
                    require(proof.get("scene_paths") and set(proof["scene_paths"])<=valid,"Feature uses unqualified/invalid scenes")
                    controls=proof.get("control_pair",[])
                    require(isinstance(controls,list) and len(controls)==2 and len(set(controls))==2 and set(controls)<=valid,
                            "Missing independently qualified on/off or parameter control pair")
                    require(report["scene_results"][controls[0]]["rgb_sha256"]!=report["scene_results"][controls[1]]["rgb_sha256"],
                            "Feature control did not change decoded RGB")
                    if key=="feature:DOF":
                        validate_dof_control_pair([report["scene_results"][p] for p in controls])
                    if key=="feature:ordinary effect overrides and render controls":
                        combo=proof.get("no_ao_reflection_case",{})
                        scene=scope["scenes"].get(combo.get("path"),{}) or scope.get("additional_scenes",{}).get(combo.get("id"),{})
                        require(combo.get("path")==scene.get("path"),"Override combination input mismatch")
                        require(scene.get("scope_class")=="ORDINARY_HEADER_SCREENED","Unknown/deferred override combination")
                        checked=validate_scene(base,combo,scene,binding,binding_id,allow_effect_overrides=True)
                        require(checked.get("no_ao_reflection_point_ao_observed") is True,
                                "Effect-off fixtures do not prove no_ao plus reflection point AO semantics")
                attempt("features",key,verify_feature)
            for kind,row in receipts.items():
                def verify_global(kind=kind,row=row):
                    proof=receipt(base,row.get("proof"),kind,binding_id)
                    require(proof.get("scope") == "full_pinned_ordinary", "Subset receipt cannot qualify full scope")
                    if kind=="build_provenance":
                        require(proof.get("compiled_source_files_sha256")==binding["source_files_sha256"]
                                and proof.get("renderer_sha256")==binding["renderer"]["sha256"]
                                and proof.get("library_sha256")==binding["library"]["sha256"],"Build inputs not attributed to current binaries")
                    if kind=="independent_heldout":
                        require({tuple(x) for x in proof.get("formula_identities",[])}==set(scope["formulas"])
                                and set(proof.get("features",[]))==scope["features"],"Held-out coverage incomplete")
                        heldout=unique(proof.get("formula_observations"),formula_key,"held-out formula")
                        require(set(heldout)==set(scope["formulas"]),"Missing raw held-out formula observations")
                        for ident,observation in heldout.items():
                            cases=formula_observations(base,observation.get("observations"),ident,binding_id)
                            selected={digest({k:c.get(k) for k in ("kind","input_state_bits","parameter_bits","variant")}) for c in cases.values()}
                            require(ident in working_inputs and not(selected & working_inputs[ident]),"Held-out cases duplicate working inputs")
                        require(proof.get("independently_selected") is True,"Held-out selection not independently verified")
                    if kind in ("cpu_ordinary","metal_compatible"):
                        require(type(proof.get("tests_run")) is int and proof["tests_run"]>0 and proof.get("failed")==0
                                and proof.get("excluded")==["Monte Carlo","JIT"],"Missing current ordinary/compatible test inventory")
                    if kind=="guard_failures":
                        guards=unique(proof.get("cases"),lambda r:r.get("id"),"guard case")
                        require(set(guards)>={"cancellation","bounds","finite_work","invalid_library","invalid_abi","gpu_fault"}
                                and all(r.get("status")=="PASS" and r.get("image_published") is False for r in guards.values()),
                                "Guard/no-image negative cases incomplete")
                    if kind=="no_cpu_pixel_work":
                        require(proof.get("cpu_formula_callbacks")==0 and proof.get("cpu_render_callbacks")==0
                                and proof.get("dispatch_inventory_verified") is True,"Hidden CPU pixel work not ruled out")
                    if kind=="native_regressions":
                        regressions=unique(proof.get("cases"),lambda r:r.get("id"),"native regression")
                        require(bool(regressions),"Missing frozen Native regressions")
                        for regression in regressions.values():
                            original=image_rgb(artifact(base,regression.get("frozen_image")))
                            current=image_rgb(artifact(base,regression.get("current_image")))
                            if original!=current:
                                review=receipt(base,regression.get("intentional_change"),"intentional_native_change",binding_id)
                                require(review.get("frozen_image_sha256")==regression["frozen_image"]["sha256"]
                                        and review.get("current_image_sha256")==regression["current_image"]["sha256"]
                                        and isinstance(review.get("reason"),str) and bool(review["reason"]),
                                        "Native frozen-image regression lacks bound intentional-change review")
                    if kind=="full_render_timings":
                        require(proof.get("main_renderer_api") is True and proof.get("cold_processes_per_backend",0)>=3
                                and proof.get("warm_complete_renders_per_process",0)>=3 and proof.get("backends")==["cpu","metal"],"Incomplete cold/warm full-render timing")
                        processes=proof.get("processes",[])
                        require(isinstance(processes,list) and bool(processes),"Missing retained timing samples")
                        for backend in ("cpu","metal"):
                            selected=[p for p in processes if p.get("backend")==backend]
                            require(len(selected)>=3 and len({p.get("process_id") for p in selected})==len(selected),"Missing independent cold process samples")
                            for process in selected:
                                require(finite_nonnegative(process.get("cold_full_render_ms")) and process["cold_full_render_ms"]>0
                                        and isinstance(process.get("warm_full_render_ms"),list) and len(process["warm_full_render_ms"])>=3
                                        and all(finite_nonnegative(t) and t>0 for t in process["warm_full_render_ms"]),"Invalid complete-render timing sample")
                                artifact(base,process.get("report"))
                attempt("receipts",kind,verify_global)
        except (InvalidEvidence,KeyError,TypeError,ValueError,OSError) as error:
            report["errors"].append(str(error))
    report["remaining"]=sum(remaining.values())
    if report["remaining"]==0 and not report["errors"]: report["status"]="PASS"
    elif report["errors"]:report["status"]="INVALID"
    return report


def schema_example():
    return {"schema":SCHEMA,"binding":"Copy current-binding.json exactly; changes require new evidence.",
            "formulas":[{"namespace":"internal","id":0,"name":"Integer Power","proof":{"path":"formula.json","sha256":"..."}}],
            "scenes":[],"additional_scenes":[],"features":[],"receipts":[],
            "receipt_contract":{"schema":RECEIPT_SCHEMA,"kind":"formula_gpu_point_orbit","status":"PASS",
                "binding_sha256":"SHA256 canonical compact sorted binding JSON","producer":"collector identity",
                "independent_reviewer":"different reviewer identity","artifacts":[{"path":"raw-observation.json","sha256":"..."}],
                "checks":[{"id":"independent-state-contract","status":"PASS"}]},
            "notes":["All supplied evidence paths are relative to evidence.json's directory and hash checked.",
                     "Scene rows require classification, >=3 fresh Metal frame processes, gpu_records and CPU reference; read validate_scene for exact fields.",
                     "No subset option changes denominator. No commands in evidence are executed. Existing inventories are never rewritten.",
                     "Optional contract.additional_scene_inventory is separately hash-pinned in inventory_bindings. Schema mb3d.native-metal-all.additional-scenes.v1 has reviewed generator/generation_inputs and members with unique id/path/sha256/scope_class. Every extra adds an obligation; none replaces pinned paths. Feature scene_paths/control_pair may reference additional:<id>.",
                     "A reviewer identity is provenance, not cryptographic independence. Human review remains required; self-declared booleans alone are not observational proof."]}


def main(argv=None):
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--contract",type=Path,required=True);parser.add_argument("--build",type=Path,required=True)
    parser.add_argument("--output",type=Path,required=True);parser.add_argument("--evidence",type=Path)
    parser.add_argument("--write-schema-example",action="store_true")
    args=parser.parse_args(argv)
    try:
        scope=load_scope(args.contract.resolve(),ROOT)
        binding={"contract_sha256":scope["contract_sha256"],"inventory_bindings":scope["inventory_bindings"],
                 "source_files_sha256":source_snapshot(ROOT)}
        for key,name in (("renderer","mb3d_render"),("library","metal_native_scene.metallib")):
            path=(args.build/name).resolve();binding[key]={"path":str(path),"sha256":sha(path)} if path.is_file() else None
        evidence_path=(args.evidence or args.output/"evidence.json").resolve()
        evidence=read_json(evidence_path) if evidence_path.is_file() else None
        report=evaluate(scope,evidence,evidence_path.parent,binding)
        require(source_snapshot(ROOT)==binding["source_files_sha256"],"Source changed during qualification")
        for key in ("renderer","library"):
            if binding[key]:require(sha(binding[key]["path"])==binding[key]["sha256"],"Build artifact changed during qualification")
        report.update(schema="mb3d.native-metal-all.qualification.v1",binding_sha256=digest(binding),
                      evidence_sha256=sha(evidence_path) if evidence_path.is_file() else None,
                      execution="passive verifier: no builds/renders",historical_pass_inherited=False)
        args.output.mkdir(parents=True,exist_ok=True)
        (args.output/"current-binding.json").write_text(json.dumps(binding,indent=2)+"\n")
        (args.output/"qualification.json").write_text(json.dumps(report,indent=2)+"\n")
        if args.write_schema_example:(args.output/"evidence-schema-example.json").write_text(json.dumps(schema_example(),indent=2)+"\n")
        print(f'{report["status"]}; remaining={report["remaining"]}; CPU render fallbacks=0 accepted; '+json.dumps(report["remaining_by_kind"],sort_keys=True))
        print(f'Excluded deferred scenes={len(report["excluded_scenes"])}; originally unclassified={len(report["unclassified_scenes"])}; no rendering performed')
        return 0 if report["status"]=="PASS" else 1
    except (InvalidEvidence,OSError,ValueError,KeyError,TypeError,subprocess.CalledProcessError) as error:
        print("INVALID; remaining=unknown; "+str(error),file=sys.stderr)
        return 2


if __name__=="__main__":sys.exit(main())
