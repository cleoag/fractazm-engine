"""Negative tests for the full-scope passive Native Metal qualification gate."""
import copy
import hashlib
import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest import mock
import zlib

import qualify_native_metal_all as q


def write_json(path, value):
    path.write_text(json.dumps(value))
    return {"path": path.name, "sha256": q.sha(path)}


def png(path, rgb=b"\x10\x20\x30\x40\x50\x60"):
    def chunk(tag, data):
        return struct.pack(">I", len(data)) + tag + data + struct.pack(">I", zlib.crc32(tag + data) & 0xffffffff)
    path.write_bytes(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", 2, 1, 8, 2, 0, 0, 0))
                     + chunk(b"IDAT", zlib.compress(b"\0" + rgb)) + chunk(b"IEND", b""))
    return {"path": path.name, "sha256": q.sha(path)}


class QualificationTests(unittest.TestCase):
    def test_vendored_header_is_part_of_compiled_source_binding(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            header = root / "libs/stb/stb_image.h"
            header.parent.mkdir(parents=True)
            header.write_text("/* vendored implementation v1 */\n")
            with mock.patch.object(q.subprocess, "check_output", return_value=b"libs/stb/stb_image.h\0") as listing:
                before = q.source_snapshot(root)
                self.assertIn("libs", listing.call_args.args[0])
                self.assertEqual(before["libs/stb/stb_image.h"], q.sha(header))
                header.write_text("/* changed vendored implementation */\n")
                self.assertNotEqual(before, q.source_snapshot(root))

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.base = Path(self.tmp.name)
        self.scene_path = self.base / "scene.m3p"
        self.scene_path.write_bytes(b"ordinary input")
        self.scene = {"path": "scene.m3p", "sha256": q.sha(self.scene_path), "scope_class": "ORDINARY_HEADER_SCREENED",
                      "_source_path":str(self.scene_path)}
        self.key = ("internal", 0, "Integer Power")
        self.scope = {"formulas": {self.key: {}}, "scenes": {"scene.m3p": self.scene}, "features": {"feature:AO"}}
        for filename in ("mb3d_render", "native.metallib"):
            (self.base / filename).write_bytes(filename.encode())
        self.binding = {"renderer": {"path": str(self.base / "mb3d_render"), "sha256": q.sha(self.base / "mb3d_render")},
                        "library": {"path": str(self.base / "native.metallib"), "sha256": q.sha(self.base / "native.metallib")},
                        "source_files_sha256": {"source.cpp": "current"}, "inventory_bindings": {"scene": "pinned"}}
        self.binding_id = q.digest(self.binding)
        self.stages = {s: "completed" if s in ("load", "preparation", "primary", "ao", "color", "final_surface", "save") else "not_run" for s in q.STAGES}
        self.serial = 0

    def receipt(self, kind, **fields):
        self.serial += 1
        obs = write_json(self.base / f"observations-{self.serial}.json", {"observed": True})
        return write_json(self.base / f"receipt-{self.serial}.json", {
            "schema": q.RECEIPT_SCHEMA, "kind": kind, "binding_sha256": self.binding_id, "status": "PASS",
            "producer": "collector", "independent_reviewer": "reviewer", "checks": [{"id": "review", "status": "PASS"}],
            "artifacts": [obs], **fields})

    def frame(self, backend, index, rgb=b"\x10\x20\x30\x40\x50\x60"):
        name = f"{backend}-{index}"
        image = png(self.base / f"{name}.png", rgb)
        profile = {"status": "complete", "render_status": "completed", "exit_code": 0, "error": "",
                   "schema": "mb3d.render-stage-profile.v1" if backend == "metal" else "mb3d.cpu-stage-profile.v1",
                   "backend": backend, "backend_requested": backend, "backend_fallback": False,
                   "backend_fallback_code": "", "backend_fallback_reason": "", "precision": "native",
                   "numeric_profile": "native_metal_fp32" if backend == "metal" else "native_cpu_fp64",
                   "width": 2, "height": 1, "workers": 0 if backend == "metal" else 1, "metal_device": "Apple GPU",
                   "metal_execution":{"validated":backend=="metal","engine":"exact_capability","library_profile":[2,4,32],"primary_records":2,
                       "normal_records":2,"surface_records":4,"color_records":2,"shadow_records":0,"reflection_records":0,
                       "faults":0,"nonfinite":0,"work_exhausted":0},
                   "metal_library": self.binding["library"]["path"], "metal_reflections": {"enabled": False, "faults": 0},
                   "stages": {s: {"state": state, "seconds": 0.1 if state=="completed" else 0} for s, state in self.stages.items()},
                   "input": str(self.scene_path), "output": str(self.base / image["path"])}
        argv = [self.binding["renderer"]["path"], str(self.scene_path), "--backend", backend, "--precision", "native",
                "--output",profile["output"],"--profile-json",str(self.base/f"{name}.json")]
        if backend == "metal": argv += ["--metal-library", self.binding["library"]["path"]]
        return {"profile": write_json(self.base / f"{name}.json", profile), "image": image,
                "log": write_json(self.base / f"{name}.log", {"exit": 0}), "argv": argv, "exit_code": 0,
                "input_sha256": self.scene["sha256"], "input_path": str(self.scene_path), "output_path": profile["output"],
                "process_id": name}

    def scene_evidence(self):
        frames = [self.frame("metal", n) for n in range(3)]
        features = {"ao": True, "formula_slots": 1, "header_ao_enabled": True,
                    "effect_overrides":{"no_ao":False,"no_shadows":False,"no_dof":False,"no_reflections":False}}
        return {"path": "scene.m3p", "input_sha256": self.scene["sha256"],
                "classification": self.receipt("scene_classification", input_sha256=self.scene["sha256"], scope="ordinary",
                    classification="valid", resource_closure_complete=True, resources=[], expected_stages=self.stages,
                    effective_features=features), "frames": frames, "cpu_reference": self.frame("cpu", 0),
                "cpu_metal_differing_pixels": 0,
                "gpu_records": self.receipt("scene_gpu_records", input_sha256=self.scene["sha256"],
                    profile_sha256s=[r["profile"]["sha256"] for r in frames], library_marker=[2, 4, 32], cpu_pixel_calls=0,
                    faults=0, nonfinite=0, work_exhausted=0, effective_features=features)}

    def bundle(self, **fields):
        return {"schema": q.SCHEMA, "binding": copy.deepcopy(self.binding), "formulas": [], "scenes": [], "features": [], "receipts": [], **fields}

    def evaluate(self, bundle):
        return q.evaluate(self.scope, bundle, self.base, self.binding)

    def change_profile(self, row, **fields):
        path = self.base / row["profile"]["path"]
        value = q.read_json(path); value.update(fields)
        row["profile"] = write_json(path, value)

    def assert_scene_invalid(self, row):
        report = self.evaluate(self.bundle(scenes=[row]))
        self.assertNotEqual(report["status"], "PASS")
        self.assertEqual(report["remaining_by_kind"]["scenes"], 1)
        self.assertTrue(report["errors"])

    def test_round_zero_reports_full_open_denominator(self):
        report = self.evaluate(None)
        self.assertEqual(report["remaining"], 12)
        self.assertEqual(report["status"], "OPEN")

    def test_one_valid_scene_never_qualifies_subset(self):
        report = self.evaluate(self.bundle(scenes=[self.scene_evidence()]))
        self.assertEqual(report["remaining_by_kind"]["scenes"], 0)
        self.assertEqual(report["remaining_by_kind"]["formulas"], 1)
        self.assertGreater(report["remaining"], 0)
        self.assertNotEqual(report["status"], "PASS")

    def test_cpu_fallback_cannot_pass(self):
        row = self.scene_evidence()
        self.change_profile(row["frames"][0], backend="cpu", backend_fallback=True, backend_fallback_code="unsupported_scene")
        self.assert_scene_invalid(row)

    def test_wrong_precision_cannot_pass(self):
        row = self.scene_evidence(); self.change_profile(row["frames"][0], precision="compatible")
        self.assert_scene_invalid(row)

    def test_missing_repeat_cannot_pass(self):
        row = self.scene_evidence(); row["frames"].pop()
        self.assert_scene_invalid(row)

    def test_reused_frame_cannot_pass(self):
        row = self.scene_evidence(); row["frames"][2] = copy.deepcopy(row["frames"][0])
        self.assert_scene_invalid(row)

    def test_changed_rgb_cannot_pass(self):
        row = self.scene_evidence(); row["frames"][2] = self.frame("metal", 2, b"\xff"*6)
        self.assert_scene_invalid(row)

    def test_missing_stage_cannot_pass(self):
        row = self.scene_evidence(); path=self.base/row["frames"][0]["profile"]["path"]
        profile=q.read_json(path);del profile["stages"]["ao"]
        row["frames"][0]["profile"]=write_json(path,profile);self.assert_scene_invalid(row)

    def test_failed_stage_and_nonfinite_time_cannot_pass(self):
        for state, seconds in (("failed",1),("completed",float("nan"))):
            with self.subTest(state=state,seconds=seconds):
                row=self.scene_evidence();path=self.base/row["frames"][0]["profile"]["path"]
                profile=q.read_json(path);profile["stages"]["ao"]={"state":state,"seconds":seconds}
                row["frames"][0]["profile"]=write_json(path,profile);self.assert_scene_invalid(row)

    def test_nonzero_faults_cannot_pass(self):
        row=self.scene_evidence();path=self.base/row["gpu_records"]["path"]
        proof=q.read_json(path);proof["faults"]=1;row["gpu_records"]=write_json(path,proof)
        self.assert_scene_invalid(row)

    def normal_profile(self):
        frame = self.frame("metal", 0)
        profile = q.read_json(self.base / frame["profile"]["path"])
        stages = dict(self.stages)
        stages["normals"] = "completed"
        profile["stages"]["normals"] = {"state": "completed", "seconds": 0.1}
        profile["metal_execution"].update({"engine": "general_formula", "znormal_records": 2,
            "znormal_foreground": 1, "znormal_changed": 1, "znormal_neighbors": 1, "znormal_dispatches": 3,
            "normal_samples": 14, "normal_de_calls": 14, "normal_orbit_calls": 0,
            "normal_evaluations": 2, "primary_rough_pixels": 1})
        return profile, stages

    def test_normal_extensions_require_complete_integer_records(self):
        for key in q.ZNORMAL_EXECUTION_FIELDS + q.NORMAL_EXECUTION_FIELDS:
            for value in (None, -1, 0.0, True):
                with self.subTest(key=key, value=value):
                    profile, stages = self.normal_profile()
                    if value is None: del profile["metal_execution"][key]
                    else: profile["metal_execution"][key] = value
                    with self.assertRaises(q.InvalidEvidence): q.validate_profile(profile, "metal", stages)

    def test_normal_extensions_enforce_activity_bounds(self):
        for key, value in (("znormal_records", 1), ("znormal_dispatches", 2),
            ("znormal_foreground", 3), ("znormal_changed", 2), ("znormal_neighbors", 5),
            ("normal_samples", 11), ("normal_evaluations", 0), ("normal_de_calls", 11),
            ("normal_orbit_calls", 1), ("primary_rough_pixels", 3)):
            with self.subTest(key=key, value=value):
                profile, stages = self.normal_profile()
                profile["metal_execution"][key] = value
                with self.assertRaises(q.InvalidEvidence): q.validate_profile(profile, "metal", stages)
        profile, stages = self.normal_profile()
        profile["metal_execution"].update(normal_samples=323, normal_de_calls=323)
        with self.assertRaises(q.InvalidEvidence): q.validate_profile(profile, "metal", stages)
        profile["metal_execution"].update(normal_samples=13, normal_de_calls=13)
        with self.assertRaises(q.InvalidEvidence): q.validate_profile(profile, "metal", stages)

    def test_z_normal_background_is_valid_but_wrong_engine_is_not(self):
        profile, stages = self.normal_profile()
        q.validate_profile(profile, "metal", stages)
        for k in q.NORMAL_EXECUTION_FIELDS: profile["metal_execution"][k] = 0
        for k in ("znormal_foreground", "znormal_changed", "znormal_neighbors"):
            profile["metal_execution"][k] = 0
        q.validate_profile(profile, "metal", stages)
        profile["metal_execution"]["engine"] = "exact_capability"
        with self.assertRaises(q.InvalidEvidence): q.validate_profile(profile, "metal", stages)

    def test_inactive_z_normal_cannot_claim_work(self):
        for key in q.ZNORMAL_EXECUTION_FIELDS:
            with self.subTest(key=key):
                frame = self.frame("metal", 0)
                profile = q.read_json(self.base / frame["profile"]["path"])
                profile["metal_execution"][key] = 1
                with self.assertRaises(q.InvalidEvidence): q.validate_profile(profile, "metal", self.stages)

    def test_normal_repeat_counters_cannot_change_under_identical_rgb(self):
        row = self.scene_evidence()
        for frame in row["frames"]:
            path = self.base / frame["profile"]["path"]
            profile = q.read_json(path)
            profile["metal_execution"].update({"engine": "general_formula", "normal_samples": 7,
                "normal_de_calls": 7, "normal_orbit_calls": 0, "normal_evaluations": 1, "primary_rough_pixels": 0})
            frame["profile"] = write_json(path, profile)
        proof_path = self.base / row["gpu_records"]["path"]
        proof = q.read_json(proof_path)
        proof["profile_sha256s"] = [r["profile"]["sha256"] for r in row["frames"]]
        row["gpu_records"] = write_json(proof_path, proof)
        q.validate_scene(self.base, row, self.scene, self.binding, self.binding_id)
        frame = row["frames"][2]
        path = self.base / frame["profile"]["path"]
        profile = q.read_json(path)
        profile["metal_execution"]["primary_rough_pixels"] = 1
        frame["profile"] = write_json(path, profile)
        self.assert_scene_invalid(row)

    def dof_profile(self, active=True, passes=2):
        frame=self.frame("metal",0)
        profile=q.read_json(self.base/frame["profile"]["path"])
        stages=dict(self.stages)
        if active:
            stages["dof"]="completed"
            profile["stages"]["dof"]={"state":"completed","seconds":0.1}
            profile["metal_execution"].update({"engine":"general_formula",
                "dof_records":2*passes,"dof_passes":passes,"dof_positive_radius_sources":2*passes,
                "dof_sort_work":4,"dof_candidate_scan_checks":8,"dof_candidate_checks":8,
                "dof_contributing_pairs":4,"dof_changed_pixels":1,"dof_maximum_radius":1,"dof_dispatches":12})
        return profile,stages

    def test_active_dof_requires_all_integer_gpu_records(self):
        for key in q.DOF_EXECUTION_FIELDS:
            for value in (None,-1,True,1.5,float("nan"),float("inf")):
                with self.subTest(key=key,value=value):
                    profile,stages=self.dof_profile()
                    if value is None: del profile["metal_execution"][key]
                    else: profile["metal_execution"][key]=value
                    with self.assertRaises(q.InvalidEvidence):q.validate_profile(profile,"metal",stages)

    def test_active_dof_pass_record_and_activity_bounds(self):
        for key,value in (("dof_passes",0),("dof_passes",5),("dof_records",0),("dof_records",3),
                          ("dof_dispatches",0),("dof_positive_radius_sources",5),
                          ("dof_changed_pixels",3),("dof_maximum_radius",1001)):
            with self.subTest(key=key,value=value):
                profile,stages=self.dof_profile();profile["metal_execution"][key]=value
                with self.assertRaises(q.InvalidEvidence):q.validate_profile(profile,"metal",stages)

    def test_complete_focused_dof_can_leave_image_unchanged(self):
        for passes in range(1,5):
            with self.subTest(passes=passes):
                profile,stages=self.dof_profile(passes=passes)
                profile["metal_execution"].update({"dof_positive_radius_sources":0,"dof_sort_work":0,
                    "dof_candidate_scan_checks":0,"dof_candidate_checks":0,"dof_contributing_pairs":0,
                    "dof_changed_pixels":0,"dof_maximum_radius":0})
                q.validate_profile(profile,"metal",stages)

    def test_inactive_dof_accepts_legacy_absence_or_explicit_zeros(self):
        profile,stages=self.dof_profile(active=False)
        q.validate_profile(profile,"metal",stages)
        profile["metal_execution"].update(dict.fromkeys(q.DOF_EXECUTION_FIELDS,0))
        q.validate_profile(profile,"metal",stages)

    def test_inactive_dof_rejects_claimed_work_or_invalid_zero(self):
        for key in q.DOF_EXECUTION_FIELDS:
            for value in (1,-1,False,0.0,None):
                with self.subTest(key=key,value=value):
                    profile,stages=self.dof_profile(active=False);profile["metal_execution"][key]=value
                    with self.assertRaises(q.InvalidEvidence):q.validate_profile(profile,"metal",stages)

    def active_dof_scene(self):
        self.stages["dof"]="completed"
        row=self.scene_evidence()
        template,_=self.dof_profile()
        for frame in row["frames"]:
            path=self.base/frame["profile"]["path"];profile=q.read_json(path)
            profile["metal_execution"].update(template["metal_execution"])
            frame["profile"]=write_json(path,profile)
        path=self.base/row["gpu_records"]["path"];proof=q.read_json(path)
        proof["profile_sha256s"]=[r["profile"]["sha256"] for r in row["frames"]]
        row["gpu_records"]=write_json(path,proof)
        return row

    def test_dof_activity_is_derived_from_all_actual_repeat_profiles(self):
        row=self.active_dof_scene()
        result=q.validate_scene(self.base,row,self.scene,self.binding,self.binding_id)
        self.assertTrue(result["dof_discriminating_activity_observed"])
        self.assertEqual(result["gpu_engine"],"general_formula")
        path=self.base/row["frames"][2]["profile"]["path"];profile=q.read_json(path)
        profile["metal_execution"]["dof_candidate_checks"]+=1
        row["frames"][2]["profile"]=write_json(path,profile)
        with self.assertRaisesRegex(q.InvalidEvidence,"DOF GPU work repeats"):
            q.validate_scene(self.base,row,self.scene,self.binding,self.binding_id)

    def test_dof_feature_control_rejects_engine_change(self):
        controls=[{"gpu_engine":"general_formula","dof_discriminating_activity_observed":True},
                  {"gpu_engine":"exact_capability","dof_discriminating_activity_observed":False}]
        with self.assertRaisesRegex(q.InvalidEvidence,"changed GPU engine"):
            q.validate_dof_control_pair(controls)

    def test_dof_feature_control_requires_positive_activity(self):
        controls=[{"gpu_engine":"general_formula","dof_discriminating_activity_observed":False} for _ in range(2)]
        with self.assertRaisesRegex(q.InvalidEvidence,"positive blur activity"):
            q.validate_dof_control_pair(controls)
        controls[0]["dof_discriminating_activity_observed"]=True
        q.validate_dof_control_pair(controls)

    def test_stale_source_or_binary_binding_cannot_pass(self):
        for key in ("renderer","source_files_sha256","inventory_bindings"):
            with self.subTest(key=key):
                bundle=self.bundle();bundle["binding"][key]={"stale":True}
                self.assertEqual(self.evaluate(bundle)["status"],"INVALID")

    def test_changed_artifact_hash_cannot_pass(self):
        row=self.scene_evidence();(self.base/row["frames"][0]["image"]["path"]).write_bytes(b"replacement")
        self.assert_scene_invalid(row)

    def test_deferred_scene_cannot_enter_evidence(self):
        self.scope["scenes"]["scene.m3p"]["scope_class"]="DEFERRED_EXCLUDED"
        report=self.evaluate(self.bundle(scenes=[self.scene_evidence()]))
        self.assertEqual(report["status"],"INVALID")
        self.assertIn("Deferred",report["errors"][0])

    def test_unclassified_scene_without_ordinary_receipt_stays_open(self):
        self.scope["scenes"]["scene.m3p"]["scope_class"]="UNCLASSIFIED_DO_NOT_EXECUTE"
        row=self.scene_evidence();del row["classification"]
        self.assert_scene_invalid(row)

    def test_duplicate_scene_or_formula_cannot_pass(self):
        row=self.scene_evidence()
        report=self.evaluate(self.bundle(scenes=[row,row]))
        self.assertEqual(report["status"],"INVALID")
        formula={"namespace":"internal","id":0,"name":"Integer Power"}
        self.assertEqual(self.evaluate(self.bundle(formulas=[formula,formula]))["status"],"INVALID")

    def test_unknown_case_cannot_replace_missing_pinned_case(self):
        row=self.scene_evidence();row["path"]="different.m3p"
        self.assert_scene_invalid(row)

    def test_self_review_cannot_pass(self):
        row=self.scene_evidence();path=self.base/row["classification"]["path"]
        value=q.read_json(path);value["independent_reviewer"]="collector";row["classification"]=write_json(path,value)
        self.assert_scene_invalid(row)

    def test_path_escape_rejected_before_read(self):
        with self.assertRaisesRegex(q.InvalidEvidence,"escapes"):
            q.artifact(self.base,{"path":"../victim","sha256":"0"*64})

    def test_duplicate_json_keys_rejected(self):
        path=self.base/"bad.json";path.write_text('{"status":"PASS","status":"FAIL"}')
        with self.assertRaisesRegex(q.InvalidEvidence,"Duplicate"):
            q.read_json(path)

    def test_png_decode_and_crc(self):
        path=self.base/"image.png";png(path)
        self.assertEqual(q.image_rgb(path),(2,1,b"\x10\x20\x30\x40\x50\x60"))
        data=bytearray(path.read_bytes());data[-1]^=1;path.write_bytes(data)
        with self.assertRaisesRegex(q.InvalidEvidence,"CRC"):
            q.image_rgb(path)

    def pinned_scope(self):
        identities=[{"namespace":"internal","id":i,"name":f"internal-{i}"} for i in range(10)]
        identities += [{"namespace":"external","id":1000+i,"name":f"external-{i}"} for i in range(457)]
        formula=write_json(self.base/"formulas.json",{"identities":identities})
        scenes=write_json(self.base/"scenes.json",{"members":[self.scene],"summary":{"paths":1},"feature_obligations":[]})
        contract=self.base/"contract.json"
        write_json(contract,{"formula_inventory":"formulas.json","scene_inventory":"scenes.json",
            "inventory_bindings":{"formulas.json":formula["sha256"],"scenes.json":scenes["sha256"]},
            "formula_identities":{"total":467,"internal":10,"external":457}})
        return contract,{q.formula_key(r) for r in identities}

    def test_pinned_manifest_hash_mismatch_rejected(self):
        contract,registry=self.pinned_scope()
        q.load_scope(contract,self.base,lambda _:registry)
        (self.base/"scenes.json").write_text("{}")
        with self.assertRaisesRegex(q.InvalidEvidence,"inventory hash mismatch"):
            q.load_scope(contract,self.base,lambda _:registry)

    def test_original_scene_hash_mismatch_rejected(self):
        contract,registry=self.pinned_scope();self.scene_path.write_bytes(b"changed input")
        with self.assertRaisesRegex(q.InvalidEvidence,"Pinned scene changed"):
            q.load_scope(contract,self.base,lambda _:registry)

    def test_registry_id_change_rejected_but_implementation_hash_not_compared(self):
        contract,registry=self.pinned_scope()
        # Only identity is pinned: ordinary source implementation is allowed to evolve.
        scope=q.load_scope(contract,self.base,lambda _:registry)
        self.assertEqual(len(scope["formulas"]),467)
        registry.pop()
        with self.assertRaisesRegex(q.InvalidEvidence,"names/IDs differ"):
            q.load_scope(contract,self.base,lambda _:registry)

    def test_reduced_contract_cannot_pass(self):
        contract,registry=self.pinned_scope();value=q.read_json(contract)
        value["formula_identities"]={"total":1,"internal":1,"external":0};write_json(contract,value)
        with self.assertRaisesRegex(q.InvalidEvidence,"FULL467"):
            q.load_scope(contract,self.base,lambda _:registry)

    def test_independent_formula_state_is_checked_not_pass_boolean(self):
        key=("external",5001,"example")
        cases=[{"case_id":kind,"kind":kind,"identity":list(key),"gpu_invocations":1,"faults":0,
                "input_state_bits":{"x":"0x3f800000"},"parameter_bits":[],"read_lanes":["x"],"written_lanes":["x"],
                "expected_state_bits":{"x":"0x40000000"},"gpu_state_bits":{"x":"0x40000000"}}
               for kind in ("point","orbit")]
        value={"schema":"mb3d.native-metal-all.formula-observations.v1","binding_sha256":self.binding_id,
               "library_marker":[2,4,32],"execution":"metal_gpu","cpu_pixel_calls":0,"cases":cases}
        record=write_json(self.base/"formula-observations.json",value)
        q.formula_observations(self.base,record,key,self.binding_id)
        value["cases"][0]["gpu_state_bits"]["x"]="0x40000001"
        record=write_json(self.base/"formula-observations.json",value)
        with self.assertRaisesRegex(q.InvalidEvidence,"fixture mismatch"):
            q.formula_observations(self.base,record,key,self.binding_id)

    def test_legacy_classified_deferred_never_renders(self):
        self.scope["scenes"]["scene.m3p"]["scope_class"]="UNCLASSIFIED_DO_NOT_EXECUTE"
        row={"path":"scene.m3p","input_sha256":self.scene["sha256"],"classification":self.receipt("scene_classification",
             input_sha256=self.scene["sha256"],scope="deferred",classification="deferred",deferred_kind="JIT",ordinary_loader_inspected=True)}
        report=self.evaluate(self.bundle(scenes=[row]))
        self.assertEqual(report["scene_results"]["scene.m3p"]["classification"],"deferred")
        row["frames"]=[self.frame("metal",0)]
        self.assert_scene_invalid(row)

    def reflection_override(self, point_ao_rays):
        row=self.scene_evidence()
        classification=q.read_json(self.base/row["classification"]["path"])
        classification["expected_stages"]["ao"]="not_run"
        classification["expected_stages"]["reflections"]="completed"
        classification["effective_features"]["effect_overrides"]["no_ao"]=True
        row["classification"]=write_json(self.base/row["classification"]["path"],classification)
        for frame in [*row["frames"],row["cpu_reference"]]:
            path=self.base/frame["profile"]["path"];profile=q.read_json(path)
            profile["stages"]["ao"]={"state":"not_run","seconds":0}
            profile["stages"]["reflections"]={"state":"completed","seconds":0.1}
            profile["metal_reflections"]={"enabled":True,"rays":2,"hits":1,"faults":0,"ao_rays":point_ao_rays}
            profile["metal_execution"]["reflection_records"]=2
            frame["profile"]=write_json(path,profile);frame["argv"].append("--no-ao")
        path=self.base/row["gpu_records"]["path"];proof=q.read_json(path)
        proof["effective_features"]=classification["effective_features"]
        proof["profile_sha256s"]=[f["profile"]["sha256"] for f in row["frames"]]
        row["gpu_records"]=write_json(path,proof)
        return row

    def test_effect_override_cannot_replace_full_baseline_scene(self):
        self.assert_scene_invalid(self.reflection_override(3))

    def test_no_ao_preserves_reflection_point_ao(self):
        row=self.reflection_override(3)
        result=q.validate_scene(self.base,row,self.scene,self.binding,self.binding_id,allow_effect_overrides=True)
        self.assertTrue(result["no_ao_reflection_point_ao_observed"])

    def test_no_ao_cannot_drop_reflection_point_ao(self):
        row=self.reflection_override(0)
        with self.assertRaisesRegex(q.InvalidEvidence,"reflection point AO"):
            q.validate_scene(self.base,row,self.scene,self.binding,self.binding_id,allow_effect_overrides=True)

    def test_execution_validation_marker_counts_and_faults_are_required(self):
        for field,bad in (("validated",False),("library_profile",[1,4,80]),("primary_records",1),
                          ("normal_records",True),("surface_records",2),("color_records",1),
                          ("shadow_records",2),("reflection_records",2),("faults",1),("nonfinite",1),("work_exhausted",1)):
            with self.subTest(field=field):
                row=self.scene_evidence();path=self.base/row["frames"][0]["profile"]["path"]
                profile=q.read_json(path);profile["metal_execution"][field]=bad
                row["frames"][0]["profile"]=write_json(path,profile);self.assert_scene_invalid(row)

    def test_missing_or_unknown_execution_engine_is_rejected(self):
        frame=self.frame("metal",0)
        profile=q.read_json(self.base/frame["profile"]["path"])
        for engine in (None,"", "unknown", "cpu", [], 2):
            with self.subTest(engine=engine):
                profile["metal_execution"]["engine"]=engine
                with self.assertRaisesRegex(q.InvalidEvidence,"execution engine"):
                    q.validate_profile(profile,"metal",self.stages)

    def test_shadow_record_count_is_bound_to_explicit_engine(self):
        frame=self.frame("metal",0)
        profile=q.read_json(self.base/frame["profile"]["path"])
        stages=dict(self.stages);stages["shadows"]="completed"
        profile["stages"]["shadows"]={"state":"completed","seconds":0.1}
        for engine,expected,wrong in (("exact_capability",4,2),("general_formula",2,4)):
            with self.subTest(engine=engine):
                profile["metal_execution"]["engine"]=engine
                profile["metal_execution"]["shadow_records"]=expected
                q.validate_profile(profile,"metal",stages)
                profile["metal_execution"]["shadow_records"]=wrong
                with self.assertRaisesRegex(q.InvalidEvidence,"record count"):
                    q.validate_profile(profile,"metal",stages)
                profile["metal_execution"]["shadow_records"]=0
                with self.assertRaisesRegex(q.InvalidEvidence,"record count"):
                    q.validate_profile(profile,"metal",stages)
        profile["stages"]["shadows"]={"state":"not_run","seconds":0}
        q.validate_profile(profile,"metal",self.stages)

    def additional_inventory(self):
        contract,registry=self.pinned_scope()
        generator=self.base/"generator.py";generator.write_text("# finite ordinary fixture generator\n")
        data=bytearray(1976);struct.pack_into("<I",data,0,44)
        path=self.base/"extra.m3p";path.write_bytes(data)
        member={"id":"extra","path":"extra.m3p","sha256":q.sha(path),"scope_class":"ORDINARY_HEADER_SCREENED"}
        inventory={"schema":"mb3d.native-metal-all.additional-scenes.v1","review_status":"APPROVED_ORDINARY_CASES",
                   "reviewer":"independent reviewer","generator":{"path":"generator.py","sha256":q.sha(generator)},
                   "generation_inputs":[{"path":"scene.m3p","sha256":self.scene["sha256"]}],"members":[member]}
        extra=write_json(self.base/"additional.json",inventory)
        current=q.read_json(contract);current["additional_scene_inventory"]="additional.json"
        current["inventory_bindings"]["additional.json"]=extra["sha256"];write_json(contract,current)
        return contract,registry,inventory

    def update_additional(self,contract,inventory):
        extra=write_json(self.base/"additional.json",inventory)
        current=q.read_json(contract);current["inventory_bindings"]["additional.json"]=extra["sha256"]
        write_json(contract,current)

    def test_bound_extra_adds_obligation_without_replacing_pinned(self):
        contract,registry,_=self.additional_inventory()
        scope=q.load_scope(contract,self.base,lambda _:registry)
        self.assertEqual(len(scope["scenes"]),1);self.assertEqual(len(scope["additional_scenes"]),1)
        self.scope["additional_scenes"]=scope["additional_scenes"]
        self.scene_path=self.base/"extra.m3p";self.scene=scope["additional_scenes"]["extra"]
        row=self.scene_evidence();row.update(id="extra",path="extra.m3p")
        report=self.evaluate(self.bundle(additional_scenes=[row]))
        self.assertEqual(report["remaining_by_kind"]["additional_scenes"],0)
        self.assertEqual(report["remaining_by_kind"]["scenes"],1)
        self.assertNotEqual(report["status"],"PASS")

    def test_extra_without_separately_bound_inventory_is_invalid(self):
        row=self.scene_evidence();row["id"]="unbound"
        self.assertEqual(self.evaluate(self.bundle(additional_scenes=[row]))["status"],"INVALID")

    def test_extra_path_collision_is_rejected(self):
        contract,registry,inventory=self.additional_inventory()
        inventory["members"][0].update(path="scene.m3p",sha256=self.scene["sha256"])
        self.update_additional(contract,inventory)
        with self.assertRaisesRegex(q.InvalidEvidence,"collides"):
            q.load_scope(contract,self.base,lambda _:registry)

    def test_extra_deferred_header_is_rejected(self):
        contract,registry,inventory=self.additional_inventory()
        path=self.base/"extra.m3p";data=bytearray(path.read_bytes());data[124]|=1;path.write_bytes(data)
        inventory["members"][0]["sha256"]=q.sha(path);self.update_additional(contract,inventory)
        with self.assertRaisesRegex(q.InvalidEvidence,"Deferred additional header"):
            q.load_scope(contract,self.base,lambda _:registry)

    def test_extra_stale_generator_is_rejected(self):
        contract,registry,_=self.additional_inventory();(self.base/"generator.py").write_text("# changed\n")
        with self.assertRaisesRegex(q.InvalidEvidence,"stale artifact"):
            q.load_scope(contract,self.base,lambda _:registry)

    def test_extra_duplicate_id_is_rejected(self):
        contract,registry,inventory=self.additional_inventory();inventory["members"].append(copy.deepcopy(inventory["members"][0]))
        self.update_additional(contract,inventory)
        with self.assertRaisesRegex(q.InvalidEvidence,"Duplicate additional"):
            q.load_scope(contract,self.base,lambda _:registry)


if __name__=="__main__":unittest.main()
