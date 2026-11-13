"""Adversarial round-zero checks; no rendering or deferred suite discovery."""
import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch
import qualify_gpu_parallel_pipeline as q

class GateTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.contract = self.root/'contract.json'
        self.contract.write_text('{}')
        self.scope = ({}, {('external',1,'fixture'):{}}, {'scene':{}})
    def tearDown(self): self.temp.cleanup()
    def evaluate(self, evidence=None):
        if evidence is not None:
            (self.root/'evidence.json').write_text(json.dumps(evidence))
        with patch.object(q, 'scope', return_value=self.scope):
            return q.evaluate(self.contract,self.root)
    def test_missing_manifest_stays_open(self):
        r = self.evaluate()
        self.assertEqual(r['status'], 'OPEN')
        self.assertEqual(r['remaining'], 14)
    def test_summary_pass_is_not_evidence(self):
        r = self.evaluate({'status':'PASS', 'remaining':0})
        self.assertEqual(r['status'], 'OPEN')
    def test_unsupported_proofs_cannot_be_accepted(self):
        binding={'contract_sha256':q.sha(self.contract),'source':{},
                 'renderer':{},'library':{},'baseline_renderer':{},'baseline_library':{}}
        forged={'schema':q.SCHEMA,'binding':binding,
                'proofs':[{'kind':k,'artifact':{}} for k in q.PROOFS]}
        with patch.object(q,'source_snapshot',return_value={}), \
             patch.object(q,'context',return_value={}), \
             patch.object(q,'artifact',return_value=self.root/'irrelevant.txt'), \
             patch.object(q,'receipt',return_value={'status':'PASS'}):
            r=self.evaluate(forged)
        self.assertEqual(r['status'],'OPEN')
        self.assertEqual(r['remaining_by_kind']['proofs'],len(q.PROOFS))
        self.assertEqual(len(r['errors']),len(q.PROOFS))
    def routed_scene(self,outcome,policy):
        self.scope=({'policies':{'whole_frame_fp32_fallback':policy}},self.scope[1],self.scope[2])
        observations=self.root/'observations.json';observations.write_text('{}')
        binding={'contract_sha256':q.sha(self.contract),'source':{}}
        ev={'schema':q.SCHEMA,'binding':binding,'scenes':[{'id':'scene','qualification':outcome,'effect_and_resource_proof':{}}]}
        with patch.object(q,'source_snapshot',return_value={}), patch.object(q,'context',return_value={}), \
             patch.object(q,'receipt',return_value={'observations':{'path':observations.name,'sha256':q.sha(observations)}}), \
             patch.object(q,'fp32_scene_frames') as cpu,patch.object(q,'scene_frames') as gpu:
            result=self.evaluate(ev)
            return result,cpu.call_count,gpu.call_count
    def test_cpu_outcome_counted_separately_and_full_gate_stays_open(self):
        r,cpu,gpu=self.routed_scene('QUALIFIED_FP32_CPU',{'accepted':True,'other_fallbacks_accepted':False})
        self.assertEqual((cpu,gpu),(1,0));self.assertEqual(r['qualified_fp32_cpu_scenes'],1)
        self.assertEqual(r['qualified_gpu_scenes'],0);self.assertEqual(r['status'],'OPEN')
        self.assertEqual(r['remaining_by_kind']['scenes'],0)
        self.assertEqual(r['remaining_by_kind']['formulas'],1)
        self.assertEqual(r['remaining_by_kind']['proofs'],len(q.PROOFS))
    def test_cpu_outcome_requires_approved_policy(self):
        r,cpu,gpu=self.routed_scene('QUALIFIED_FP32_CPU',{})
        self.assertEqual((cpu,gpu),(0,0));self.assertEqual(r['remaining_by_kind']['scenes'],1)
        self.assertEqual(r['qualified_fp32_cpu_scenes'],0)
    def test_other_fallback_policy_cannot_widen_acceptance(self):
        r,cpu,gpu=self.routed_scene('QUALIFIED_FP32_CPU',{'accepted':True,'other_fallbacks_accepted':True})
        self.assertEqual((cpu,gpu),(0,0));self.assertEqual(r['remaining_by_kind']['scenes'],1)
    def test_unknown_cpu_class_is_not_qualified(self):
        r,cpu,gpu=self.routed_scene('QUALIFIED_CPU',{'accepted':True,'other_fallbacks_accepted':False})
        self.assertEqual((cpu,gpu),(0,0));self.assertEqual(r['remaining_by_kind']['scenes'],1)

    def test_duplicate_json_key_rejected(self):
        p=self.root/'duplicate.json';p.write_text('{"status":"OPEN","status":"PASS"}')
        with self.assertRaises(ValueError):q.read_json(p)
    def test_nan_rejected(self):
        p=self.root/'nan.json';p.write_text('{"seconds":NaN}')
        with self.assertRaises(ValueError):q.read_json(p)
    def test_path_escape_rejected(self):
        with self.assertRaises(ValueError):q.contained(self.root,'../outside')
    def test_hash_mismatch_rejected(self):
        p=self.root/'proof';p.write_text('PASS')
        with self.assertRaises(ValueError):q.artifact(self.root,{'path':'proof','sha256':'0'*64})
    def test_duplicate_identity_rejected(self):
        with self.assertRaises(ValueError):q.unique([{'id':'a'},{'id':'a'}],lambda r:r['id'],'scene')
    def test_stale_binding_rejected(self):
        with patch.object(q,'source_snapshot',return_value={'source':'current'}):
            r=self.evaluate({'schema':q.SCHEMA,'binding':{'contract_sha256':q.sha(self.contract),'source':{'source':'old'}}})
        self.assertEqual(r['status'],'OPEN');self.assertIn('Stale',r['errors'][0])

if __name__ == '__main__': unittest.main()
