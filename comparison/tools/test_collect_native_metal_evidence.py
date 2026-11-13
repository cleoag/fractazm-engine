"""No GPU: independent negative checks for formula evidence phase assembly."""
import copy
import json
from pathlib import Path
import struct
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

import collect_native_metal_evidence as c
import qualify_native_metal_all as q

class FormulaCollectionTests(unittest.TestCase):
    def test_probe_command_rejects_extra_options_and_shell(self):
        probe=Path('/tmp/probe')
        argv=[str(probe),'--program','/tmp/program.bin','--states','/tmp/states.bin',
              '--mode','step','--slot','0','--repeats','3','--library','/tmp/library',
              '--output','/tmp/output']
        c.screen_probe_command(argv,probe)
        for bad in (argv+['--other'], ['sh']+argv[1:], argv[:5]+['--other']+argv[6:],
                    argv[:10]+['999']+argv[11:], argv[:2]+['relative.bin']+argv[3:]):
            with self.subTest(argv=bad),self.assertRaises(q.InvalidEvidence):
                c.screen_probe_command(bad,probe)

    def fixture(self):
        state=list(range(47));expected=state.copy();expected[0]=0x3f800000
        job={"id":"step-integer-p2","kind":"point","formula_id":0,"name":"Integer Power","variant":"power-2",
             "cases":[{"id":"fit-axis","partition":"fit","input_words":state,"expected_state_words":expected,
                       "allowed_written_lanes":[0,1,2],"expected_fault":0}]}
        output=expected+[0,0,0,0,1,4096,0,0,0,1,0,0]
        report={"schema":"mb3d.native-formula-probe.v1","status":"COMPLETE","profile":"native_gpu_fp32","device":"synthetic-test-GPU",
                "library_profile":[2,4,32],"formula_abi":[1,8192,188,236,32],"mode":0,"slot":0,
                "program_words":[123],"repeats":3,"exact_state_repeats":True,"input_immutable":True,
                "allocation_canaries":True,"invalid_descriptor_controls":7,"fault_count":0,"point_count":1,
                "rows":[{"index":0,"input_words":state,"result_words":output,"fault":0,"work":1}]}
        return job,report

    def test_full_state_exact_positive(self):
        job,report=self.fixture();rows=c.validate_observation(report,job,[123])
        self.assertEqual(len(rows[0]["expected_state_bits"]),47)
        self.assertEqual(rows[0]["expected_state_bits"],rows[0]["gpu_state_bits"])

    def test_false_expected_cannot_pass(self):
        job,report=self.fixture();job["cases"][0]["expected_state_words"][17]^=1
        with self.assertRaisesRegex(q.InvalidEvidence,"full47state mismatch"):c.validate_observation(report,job,[123])

    def test_changed_untouched_state_cannot_pass(self):
        job,report=self.fixture();report["rows"][0]["result_words"][46]=1
        with self.assertRaises(q.InvalidEvidence):c.validate_observation(report,job,[123])

    def test_missing_actual_marker_cannot_pass(self):
        for field,bad in (("library_profile",None),("library_profile",[1,4,80]),("formula_abi",[1,8192,188,235,32])):
            with self.subTest(field=field,bad=bad):
                job,report=self.fixture();report[field]=bad
                with self.assertRaises(q.InvalidEvidence):c.validate_observation(report,job,[123])

    def test_missing_repeat_or_canary_cannot_pass(self):
        for field,bad in (("repeats",2),("exact_state_repeats",False),("allocation_canaries",False),
                          ("invalid_descriptor_controls",5),("invalid_descriptor_controls",6),
                          ("invalid_descriptor_controls",7.0)):
            with self.subTest(field=field):
                job,report=self.fixture();report[field]=bad
                with self.assertRaises(q.InvalidEvidence):c.validate_observation(report,job,[123])

    def test_historical_five_and_six_guards_require_explicit_partial_import(self):
        for count in (5,6):
            with self.subTest(count=count):
                job,report=self.fixture();report["invalid_descriptor_controls"]=count
                with self.assertRaises(q.InvalidEvidence):c.validate_observation(report,job,[123])
                controls=c.observation_controls(report,{"imported":True},
                    {"imported":True,"full_source_execution_binding":False})
                self.assertEqual(len(c.validate_observation(report,job,[123],controls=controls)),1)

    def test_imported_row_cannot_bypass_current_namespace_control(self):
        job,report=self.fixture();report["invalid_descriptor_controls"]=6
        with self.assertRaisesRegex(q.InvalidEvidence,"Mixed imported/current"):
            c.observation_controls(report,{"imported":True},{})
        with self.assertRaisesRegex(q.InvalidEvidence,"Mixed imported/current"):
            c.observation_controls(report,{}, {"imported":True,"full_source_execution_binding":False})
        controls=c.observation_controls(report,{}, {})
        self.assertEqual(controls,7)
        with self.assertRaises(q.InvalidEvidence):c.validate_observation(report,job,[123],controls=controls)

    def test_unknown_guard_inventory_cannot_be_imported(self):
        _,report=self.fixture()
        for count in (4,8,7.0,True,None):
            with self.subTest(count=count):
                report["invalid_descriptor_controls"]=count
                with self.assertRaises(q.InvalidEvidence):
                    c.observation_controls(report,{"imported":True},
                        {"imported":True,"full_source_execution_binding":False})

    def test_wrong_input_program_or_mode_cannot_pass(self):
        for mode in ("input","program","mode"):
            with self.subTest(mode=mode):
                job,report=self.fixture()
                if mode=="input":report["rows"][0]["input_words"]=list(range(1,48))
                elif mode=="program":report["program_words"]=[124]
                else:report["mode"]=1
                with self.assertRaises(q.InvalidEvidence):c.validate_observation(report,job,[123])

    def test_missing_case_cannot_pass(self):
        job,report=self.fixture();report["rows"]=[]
        with self.assertRaises(q.InvalidEvidence):c.validate_observation(report,job,[123])

    def test_fault_zero_work_and_reserved_rejected(self):
        for lane,value in ((47,1),(51,0),(53,1),(54,1),(56,0),(58,1)):
            with self.subTest(lane=lane):
                job,report=self.fixture();report["rows"][0]["result_words"][lane]=value
                with self.assertRaises(q.InvalidEvidence):c.validate_observation(report,job,[123])

    def test_heldout_orbit_never_completes_working_formula(self):
        job,report=self.fixture();point=c.validate_observation(report,job,[123])[0]
        orbit=copy.deepcopy(point);orbit.update(kind="orbit",partition="heldout",case_id="orbit-heldout")
        heldpoint=copy.deepcopy(point);heldpoint.update(partition="heldout",case_id="point-heldout")
        ready=c.readiness([point,orbit,heldpoint])[0]
        self.assertFalse(ready["working_point_orbit_ready"])
        self.assertTrue(ready["heldout_point_orbit_ready"])

    def test_unsafe_ids_rejected(self):
        for value in ("../victim","/absolute","a/b", "", None):
            with self.subTest(value=value):
                with self.assertRaises(q.InvalidEvidence):c.safe_id(value)

    def test_partial_import_cannot_claim_new_source_binding(self):
        bound={"renderer":"r","library":"l","contract_sha256":"c","inventory_bindings":{},"source_files_sha256":{"old":"source"}}
        current={**bound,"source_files_sha256":{"new":"source"}}
        result,source_bound=c.execution_binding({"binding":bound},{"imported":True,"full_source_execution_binding":False},current)
        self.assertEqual(result,current);self.assertFalse(source_bound)
        with self.assertRaises(q.InvalidEvidence):
            c.execution_binding({"binding":bound},{"imported":True,"full_source_execution_binding":True},current)
        with self.assertRaises(q.InvalidEvidence):c.execution_binding({"binding":bound},{},current)
        with self.assertRaises(q.InvalidEvidence):
            c.execution_binding({"binding":bound},{"imported":True,"full_source_execution_binding":False},{**current,"library":"changed"})

    def assemble_synthetic_formula(self, controls):
        """Temporary synthetic records exercise assembly, never real evidence."""
        with tempfile.TemporaryDirectory() as temporary:
            base=Path(temporary).resolve()
            binding={"renderer":{"path":"synthetic-renderer","sha256":"test"},
                     "library":{"path":"synthetic-library","sha256":"test"}}
            def save(name,value):
                path=base/name;path.write_text(json.dumps(value))
                return {"path":name,"sha256":q.sha(path)}
            program=base/"program.bin";program.write_bytes(struct.pack("<I",123))
            states=base/"states.bin";states.write_bytes(bytes(188))
            program_ref={"path":program.name,"sha256":q.sha(program)}
            states_ref={"path":states.name,"sha256":q.sha(states)}
            jobs=[];results=[]
            for kind in ("point","orbit"):
                job,report=self.fixture()
                job.update(id=kind+"-tricorn",kind=kind,formula_id=3,name="Tricorn",variant="ordinary",
                           program=program_ref,states=states_ref)
                if kind=="orbit":
                    job["cases"][0].update(expected_distance_word=0,expected_formula_work=1)
                report.update(mode=int(kind=="orbit"),program=str(program),library="synthetic-library",
                              invalid_descriptor_controls=controls)
                jobs.append(job)
                results.append({"id":job["id"],"exit_code":0,
                                "report":save(kind+".json",report),"log":save(kind+"-log.json",{"synthetic":True})})
            value={"binding":binding,"probe":{"synthetic":True},"jobs":jobs,
                   "contract":"synthetic-contract","build":"synthetic-build", "producer":"synthetic-producer", "remaining_phases":[]}
            plan_path=base/"plan.json";save(plan_path.name,value)
            execution=save("execution.json",{"schema":c.EXECUTION_SCHEMA,"plan_sha256":q.sha(plan_path),
                "binding":binding,"probe":value["probe"],"identity_unchanged":True,"results":results})
            review=save("review.json",{"schema":"mb3d.native-metal-all.formula-review.v1","plan_sha256":q.sha(plan_path),
                "execution_sha256":execution["sha256"],"reviewer":"synthetic-independent-reviewer",
                "approved_identities":[["internal",3,"Tricorn"]]})
            scope={"formulas":{("internal",3,"Tricorn"):{}},"scenes":{},"features":set(),"additional_scenes":{}}
            with patch.object(c,"load_plan",return_value=value),patch.object(c,"current_binding",return_value=binding),\
                 patch.object(q,"load_scope",return_value=scope):
                self.assertEqual(c.assemble(SimpleNamespace(plan=plan_path,review=base/review["path"])),0)
            return q.read_json(base/"assembled/ledger.json"),q.read_json(base/"evidence.json")

    def test_reviewed_current_fit_point_and_orbit_close_only_formula_obligation(self):
        ledger,evidence=self.assemble_synthetic_formula(7)
        self.assertEqual(ledger["qualified_formula_receipts"],1)
        self.assertEqual(ledger["full_predicate"]["accepted"]["formulas"],[["internal",3,"Tricorn"]])
        self.assertEqual(ledger["full_predicate"]["status"],"OPEN")
        self.assertEqual(ledger["full_predicate"]["remaining_by_kind"]["receipts"],8)
        self.assertEqual(evidence["scenes"],[])
        self.assertEqual(evidence["receipts"],[])

    def test_old_guards_cannot_close_formula_from_current_execution(self):
        for controls in (5,6):
            with self.subTest(controls=controls):
                ledger,evidence=self.assemble_synthetic_formula(controls)
                self.assertEqual(ledger["qualified_formula_receipts"],0)
                self.assertEqual(len(ledger["errors"]),2)
                self.assertEqual(ledger["full_predicate"]["remaining_by_kind"]["formulas"],1)
                self.assertEqual(evidence["formulas"],[])

if __name__=="__main__":unittest.main()
