"""Adversarial synthetic validator tests; fixtures never count as scene evidence."""
import gzip,json,struct,tempfile,unittest
from pathlib import Path
from test_gpu_pipeline_proofs import Bundle
from qualify_native_metal_all import InvalidEvidence
import fp32_overflow_witness as w
import gpu_pipeline_fp32 as f
import gpu_pipeline_frames as g


def bits(x):return struct.unpack('<I',struct.pack('<f',x))[0]

def radius(x=2e20):return [1,0,1,0,bits(x),0,0,0]+[0]*12

def program():
    p=bytearray(8192);struct.pack_into('<II',p,224+32,8,1);struct.pack_into('<I',p,224+1076,bits(-1));return p


class OracleTests(unittest.TestCase):
    def test_exact_float_max(self):self.assertEqual(w.number(0x7f7fffff),w.MAX)
    def test_radius_overflow(self):self.assertTrue(w.genuine(radius(),b''))
    def test_finite_radius_not_overflow(self):self.assertFalse(w.genuine(radius(1e10),b''))
    def test_four_dimensional_radius(self):
        r=radius(0);r[7]=bits(2e20);self.assertFalse(w.genuine(r,b''));r[17]=1;self.assertTrue(w.genuine(r,b''))
    def test_nonfinite_input_rejected(self):
        for b in (0x7f800000,0x7fc00000,0xff800000):
            r=radius();r[4]=b
            with self.subTest(bits=b),self.assertRaises(InvalidEvidence):w.genuine(r,b'')
    def test_unknown_reserved_empty(self):
        for r in ([99]+[0]*19,[0,1]+[0]*18,radius()[:-1]+[1]):
            with self.subTest(row=r),self.assertRaises(InvalidEvidence):w.genuine(r,b'')
    def test_power8(self):
        r=[2,0,3,8,bits(2**17),0,0,0,0,0,0,0,0x7f800000,0,0,0,bits(-1),0,0,0]
        self.assertTrue(w.genuine(r,program()))
        r[4]=bits(1);self.assertFalse(w.genuine(r,program()))
    def test_power8_z_bound(self):
        r=[2,0,3,8,bits(2**16),0,bits(2**16/5),0,0,0,0,0,0,0,0x7f800000,0,bits(-1),0,0,0]
        self.assertTrue(w.genuine(r,program()))
        r[4]=bits(2**15);r[6]=bits(2**15/5);self.assertFalse(w.genuine(r,program()))
    def test_radius_boundary_without_tolerance(self):
        r=radius();r[4]=0x5f7fffff;self.assertFalse(w.genuine(r,b''))
        r[4]=0x5f800000;self.assertTrue(w.genuine(r,b''))
    def test_formula_parameter_binding(self):
        r=[2,0,3,8,bits(2**17),0,0,0,0,0,0,0,0x7f800000,0,0,0,bits(-1),0,0,0]
        p=program();struct.pack_into('<I',p,224+8,1)
        with self.assertRaisesRegex(InvalidEvidence,'formula/parameter'):w.genuine(r,p)
    def test_finite_output_not_witness(self):
        r=[2,0,3,8,bits(2**17),0,0,0,0,0,0,0,0,0,0,0,bits(-1),0,0,0]
        with self.assertRaisesRegex(InvalidEvidence,'overflow XYZ'):w.genuine(r,program())
    def captures(self,root,proven=2,change=None):
        n=100;geom=bytearray(112*n);obs=bytearray(80*n)
        for i in range(n):struct.pack_into('<I',geom,i*112,6)
        for i in range(proven):struct.pack_into('<I',geom,i*112+72,1);struct.pack_into('<20I',obs,i*80,*radius())
        sections=[bytes(8192),bytes(336),geom,bytes(176*n),bytes(48*n),bytes(64*n),bytes(32*n),bytes(64*n),bytes(32*n),bytes(80*n)]
        paths=[]
        for mode in range(2):
            local=list(sections)
            if mode:
                local[-1]=obs
                if change:change(local)
            path=root/str(mode);paths.append(path)
            with gzip.open(path,'wb') as out:
                out.write(struct.pack('<5I',0x4d425257,1,10,10,10))
                for b in local:out.write(struct.pack('<Q',len(b)));out.write(b)
        return paths
    def test_threshold_strict_and_unique(self):
        with tempfile.TemporaryDirectory() as td:
            root=Path(td);p=self.captures(root,1)
            with self.assertRaisesRegex(InvalidEvidence,'strict >1%'):w.compare(*p)
            p=self.captures(root,2);self.assertEqual(w.compare(*p)['proven_pixels'],2)
    def test_changed_primary_rejected(self):
        with tempfile.TemporaryDirectory() as td:
            p=self.captures(Path(td),change=lambda v:v.__setitem__(3,b'X'+v[3][1:]))
            with self.assertRaisesRegex(InvalidEvidence,'ordinary GPU outputs'):w.compare(*p)
    def test_capture_truncation(self):
        with tempfile.TemporaryDirectory() as td:
            p=self.captures(Path(td));p[1].write_bytes(b'not gzip')
            with self.assertRaises(OSError):w.compare(*p)


class FrameTests(unittest.TestCase):
    def setUp(self):self.tmp=tempfile.TemporaryDirectory();self.b=Bundle(Path(self.tmp.name).resolve())
    def tearDown(self):self.tmp.cleanup()
    def make(self):
        b=self.b;r=b.frame();profile=json.loads((b.root/r['profile']['path']).read_text())
        profile.update(schema='mb3d.cpu-stage-profile.v1',backend='cpu',backend_fallback=True,backend_fallback_code='fp32_range',
                       backend_fallback_reason='Synthetic genuine-range fixture',numeric_profile='native_cpu_fp64',workers=1,threads_requested=1,render_total_seconds=1,backend_selection_seconds=1,command_total_seconds=2)
        profile['metal_execution'].update(validated=False,fp32_range_gate_stage='primary',fp32_range_gate_units=4,fp32_range_gate_percent=1,fp32_range_gate_exceeded=2)
        b.replace(r['profile'],profile);e=json.loads((b.root/r['event']['path']).read_text());e['outputs'][0]=r['profile']
        e['inputs']=[{'path':str(b.ctx['inputs']['s'].relative_to(b.root)),'sha256':b.scene['sha256']},b.ctx['library']];b.replace(r['event'],e)
        return r
    def check(self,r):return f.frame(self.b.root,r,self.b.scene,self.b.ctx)
    def mutate_profile(self,r,key,value):
        b=self.b;p=json.loads((b.root/r['profile']['path']).read_text());p[key]=value;b.replace(r['profile'],p)
        e=json.loads((b.root/r['event']['path']).read_text());e['outputs'][0]=r['profile'];b.replace(r['event'],e)
    def test_cpu_frame_is_separate(self):
        r=self.make();self.check(r)
        with self.assertRaises(ValueError):g.frame(self.b.root,r,self.b.scene,self.b.ctx)
    def test_reference_flag_cannot_waive_fallback_deadline(self):
        r=self.make()
        with self.assertRaisesRegex(InvalidEvidence,'Only a direct CPU reference'):
            f.frame(self.b.root,r,self.b.scene,self.b.ctx,reference_only=True)
    def test_completed_slow_reference_is_not_cpu_acceptance(self):
        b=self.b;r=self.make();p=json.loads((b.root/r['profile']['path']).read_text())
        p.update(backend_requested='cpu',backend_fallback=False,backend_fallback_code='',backend_fallback_reason='',
                 render_total_seconds=198,backend_selection_seconds=1,command_total_seconds=199)
        b.replace(r['profile'],p);e=json.loads((b.root/r['event']['path']).read_text());e['outputs'][0]=r['profile']
        e['argv'][e['argv'].index('--backend')+1]='cpu';i=e['argv'].index('--metal-library');del e['argv'][i:i+2]
        e['seconds']=200;e['finished_ns']=e['started_ns']+200*10**9;b.replace(r['event'],e)
        with self.assertRaisesRegex(InvalidEvidence,'Diagnostic deadline'):f.frame(b.root,r,b.scene,b.ctx,fallback=False)
        value=f.frame(b.root,r,b.scene,b.ctx,fallback=False,reference_only=True);self.assertTrue(value['reference_only'])
    def test_wrong_fallback_reason(self):
        r=self.make();self.mutate_profile(r,'backend_fallback_code','unsupported_formula')
        with self.assertRaisesRegex(InvalidEvidence,'Unaccepted fallback'):self.check(r)
    def test_hidden_fault(self):
        r=self.make();p=json.loads((self.b.root/r['profile']['path']).read_text())['metal_execution'];p['faults']=1;self.mutate_profile(r,'metal_execution',p)
        with self.assertRaisesRegex(InvalidEvidence,'hidden'):self.check(r)
    def test_underreported_timing(self):
        r=self.make();self.mutate_profile(r,'render_total_seconds',100)
        with self.assertRaisesRegex(InvalidEvidence,'timing exceeds'):self.check(r)
    def test_wrong_precision(self):
        r=self.make();self.mutate_profile(r,'numeric_profile','compatible_cpu_x87')
        with self.assertRaisesRegex(ValueError,'precision'):self.check(r)
    def test_changed_effect(self):
        r=self.make();self.b.scene['settings']['ao_kind']='DEAO'
        with self.assertRaisesRegex(InvalidEvidence,'effect'):self.check(r)
    def test_parity_override_rejected(self):
        r=self.make();b=self.b;e=json.loads((b.root/r['event']['path']).read_text());e['argv'].append('--deterministic');b.replace(r['event'],e)
        with self.assertRaisesRegex(InvalidEvidence,'override'):self.check(r)
    def test_missing_scene_binding(self):
        r=self.make();b=self.b;e=json.loads((b.root/r['event']['path']).read_text());e['inputs']=[];b.replace(r['event'],e)
        with self.assertRaisesRegex(InvalidEvidence,'Scene not bound'):self.check(r)
    def test_summary_without_raw_proof_rejected(self):
        with self.assertRaises((KeyError,InvalidEvidence)):f.scene_frames(self.b.root,{'status':'PASS'},self.b.scene,self.b.ctx,{})
    def test_cpu_timings_reported_separately(self):
        b=self.b;data=b.timings();cpu={'cpu_render_seconds':[3,3,3],'fallback_selection_seconds':[1,1,1],'command_seconds':[4,4,4]}
        b.ctx['scenes']['cpu']={};b.ctx['qualified_fp32_cpu_scenes']={'cpu':cpu}
        data['cpu_fallback_scenes']=[dict(cpu,id='cpu')];g.full_warm_timings(b.root,data,b.ctx)
        data['cpu_fallback_scenes'][0]['cpu_render_seconds']=[2,2,2]
        with self.assertRaisesRegex(InvalidEvidence,'CPU timing differs'):g.full_warm_timings(b.root,data,b.ctx)
    def test_missing_cpu_timings(self):
        b=self.b;data=b.timings();b.ctx['scenes']['cpu']={};b.ctx['qualified_fp32_cpu_scenes']={'cpu':{}}
        with self.assertRaisesRegex(InvalidEvidence,'separately reported'):g.full_warm_timings(b.root,data,b.ctx)
    def test_cpu_not_gpu_timing_sample(self):
        b=self.b;data=b.timings();b.ctx['qualified_fp32_cpu_scenes']={'s':{}}
        with self.assertRaisesRegex(InvalidEvidence,'CPU mixed'):g.full_warm_timings(b.root,data,b.ctx)


if __name__=='__main__':unittest.main()
