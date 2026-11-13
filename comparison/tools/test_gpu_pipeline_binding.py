"""Synthetic incremental-binding fixtures, not renderer qualification evidence."""
import json,tempfile,unittest
from pathlib import Path
from unittest.mock import patch
from test_gpu_pipeline_proofs import Bundle
from qualify_native_metal_all import sha,digest,InvalidEvidence
from gpu_pipeline_binding import core_context,for_scene
import qualify_gpu_parallel_pipeline as q


class BindingTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.root=Path(self.tmp.name).resolve();self.f=Bundle(self.root)
        f=self.f;self.c={'policies':{'whole_frame_fp32_fallback':{'accepted':True,'other_fallbacks_accepted':False}},'compute_stages':[],
             '_resource_profiles':{'original':{'files':{},'candidate_map_ids':[]}},'scene_required_maps':{'s':[]},'scene_resource_profiles':{'s':['original']}}
        self.b={'source':{},'threads':1,'renderer':f.tool,'library':f.ctx['library'],'inputs':{'s':{'path':str(f.ctx['inputs']['s'].relative_to(self.root)),'sha256':f.scene['sha256']}},
             'resource_profiles':{'original':{'runtime':'runtime'}},'scene_resources':{'s':'original'},'default_resource_profile':'original'}
    def tearDown(self):self.tmp.cleanup()
    def ctx(self):return core_context(self.root,self.b,self.c,{('external',1,'fixture'):{}},{'s':self.f.scene},self.root)
    def test_valid_core_does_not_invent_tools(self):
        ctx=self.ctx();self.assertEqual(ctx['tools'],{});self.assertEqual(for_scene(ctx,'s')['runtime_files'],{})
    def test_modified_map_rejected(self):
        (self.root/'runtime/map').write_bytes(b'changed')
        with self.assertRaisesRegex(InvalidEvidence,'pinned selection'):self.ctx()
    def test_unknown_profile_rejected(self):
        self.b['scene_resources']['s']='unapproved'
        with self.assertRaisesRegex(InvalidEvidence,'Unapproved'):self.ctx()
    def test_missing_required_map_rejected(self):
        self.c['scene_required_maps']['s']=[1633]
        with self.assertRaisesRegex(InvalidEvidence,'1633'):for_scene(self.ctx(),'s')
    def test_selected_substitute_is_pinned_and_scene_scoped(self):
        runtime=self.root/'substitute/M3Maps';runtime.mkdir(parents=True)
        image=runtime/'1633.jpg';image.write_bytes(b'synthetic texture')
        self.c['_resource_profiles']['substitute']={'files':{'M3Maps/1633.jpg':sha(image)},'candidate_map_ids':[1633]}
        self.b['resource_profiles']['substitute']={'runtime':'substitute'}
        self.c['scene_required_maps']['s']=[1633]
        self.b['scene_resources']['s']='substitute'
        with self.assertRaisesRegex(InvalidEvidence,'Unapproved'):self.ctx()
        self.c['scene_resource_profiles']['s'].append('substitute')
        self.assertEqual(for_scene(self.ctx(),'s')['selected_map_ids'],[1633])
        self.b['scene_resources']['s']='original'
        with self.assertRaisesRegex(InvalidEvidence,'1633'):for_scene(self.ctx(),'s')
    def test_selection_description_hash_is_checked(self):
        p=self.root/'selection.json';p.write_text('{}')
        self.b['map_selection']={'path':'selection.json','sha256':sha(p)}
        self.ctx();p.write_text('{"changed":true}')
        with self.assertRaises(InvalidEvidence):self.ctx()
    def test_incomplete_scene_mapping_rejected(self):
        self.b['inputs']={}
        with self.assertRaisesRegex(InvalidEvidence,'canonical'):self.ctx()
    def test_profile_alias_rejected(self):
        (self.root/'runtime/alias').symlink_to(self.f.ctx['library_path'])
        with self.assertRaisesRegex(InvalidEvidence,'alias'):self.ctx()
    def evaluate(self,change=lambda e:None):
        contract=self.root/'contract.json';contract.write_text('{}');self.b['contract_sha256']=sha(contract)
        obs=self.f.put('observations.json',{})
        ev={'schema':q.INCREMENTAL_SCHEMA,'artifact_root':'repository','binding':self.b,'scenes':[{'id':'s','qualification':'QUALIFIED_FP32_CPU','effect_and_resource_proof':{}}]}
        change(ev);(self.root/'evidence.json').write_text(json.dumps(ev))
        with patch.object(q,'ROOT',self.root),patch.object(q,'source_snapshot',return_value={}), \
             patch.object(q,'scope',return_value=(self.c,{('external',1,'fixture'):{}},{'s':self.f.scene})), \
             patch.object(q,'receipt',return_value={'observations':obs}),patch.object(q,'fp32_scene_frames'):
            return q.evaluate(contract,self.root)
    def test_incremental_scene_counts_but_binding_stays_open(self):
        r=self.evaluate();self.assertEqual(r['qualified_fp32_cpu_scenes'],1);self.assertEqual(r['qualified_gpu_scenes'],0)
        self.assertEqual(r['remaining_by_kind']['binding'],1);self.assertEqual(r['remaining_by_kind']['proofs'],11)
        self.assertEqual(r['status'],'OPEN');self.assertTrue(r['binding_errors'])
    def test_incremental_ordinary_proof_does_not_clear_full_binding(self):
        f=self.f;f.ctx['build_dir']=self.root;data=f.ctest()
        contract=self.root/'contract.json';contract.write_text('{}');self.b['contract_sha256']=sha(contract)
        self.b.update(tools={'cpu_ordinary':f.tool},ordinary_commands=f.ctx['ordinary_commands'])
        binding=digest(self.b);data.update(schema='mb3d.gpu-pipeline-proof.v1',kind='cpu_ordinary',binding_sha256=binding)
        observation=f.put('ordinary-proof.json',data)
        receipt=f.put('receipt.json',{'schema':'mb3d.native-metal-all.receipt.v1','kind':'cpu_ordinary','status':'PASS',
            'binding_sha256':binding,'producer':'synthetic collector','independent_reviewer':'synthetic validator fixture',
            'checks':[{'id':'raw','status':'PASS'}],'artifacts':[observation],'observations':observation})
        ev={'schema':q.INCREMENTAL_SCHEMA,'artifact_root':'repository','binding':self.b,'scenes':[],
            'proofs':[{'kind':'cpu_ordinary','artifact':receipt}]}
        (self.root/'evidence.json').write_text(json.dumps(ev))
        with patch.object(q,'ROOT',self.root),patch.object(q,'source_snapshot',return_value={}), \
             patch.object(q,'scope',return_value=(self.c,{('external',1,'fixture'):{}},{'s':f.scene})):
            result=q.evaluate(contract,self.root)
        self.assertEqual(result['remaining_by_kind']['binding'],1)
        self.assertEqual(result['remaining_by_kind']['proofs'],10)
        self.assertIn(['proofs','cpu_ordinary'],result['accepted'])
        self.assertEqual(result['status'],'OPEN')
    def test_partial_ordinary_mapping_is_rejected(self):
        self.b['ordinary_commands']={}
        with self.assertRaisesRegex(InvalidEvidence,'ordinary command mapping'):self.ctx()

    def test_unknown_artifact_root_rejected(self):
        r=self.evaluate(lambda e:e.__setitem__('artifact_root','../../outside'));self.assertEqual(r['qualified_fp32_cpu_scenes'],0)
    def test_bad_core_cannot_count_scene(self):
        r=self.evaluate(lambda e:e['binding']['library'].__setitem__('sha256','0'*64));self.assertEqual(r['qualified_fp32_cpu_scenes'],0)
    def test_missing_maps_cannot_count_scene(self):
        self.c['scene_required_maps']['s']=[1633];r=self.evaluate();self.assertEqual(r['qualified_fp32_cpu_scenes'],0)


if __name__=='__main__':unittest.main()
