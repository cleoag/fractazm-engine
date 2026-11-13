"""Synthetic validator fixtures: never real GPU qualification evidence."""
import copy,json,sys,tempfile,unittest,uuid
from pathlib import Path
from unittest.mock import patch
from PIL import Image
import gpu_pipeline_proofs as p
import gpu_pipeline_frames as f
import qualify_gpu_parallel_pipeline as q

class Bundle:
    def __init__(self,root):
        self.root=root;self.serial=0;self.clock=10**18
        self.tool=self.raw('tool',b'probe');self.oracle=self.raw('oracle',b'independent oracle')
        self.scene={'id':'s','sha256':'','settings':{'size':[2,2],'ao_kind':'disabled','hard_shadows_enabled':False,
                    'reflection_enabled':False,'dof_enabled':False,'zbuffer_normals':False,'volume_light':0}}
        source=self.raw('scene.m3p',b'unit fixture, not a scene');self.scene['sha256']=source['sha256']
        self.scene['path']=source['path']
        lib=self.raw('native.metallib',b'native fixture');compat=self.raw('compatible.metallib',b'compatible fixture')
        runtime=root/'runtime';runtime.mkdir()
        identity=('external',1,'fixture')
        self.ctx={'binding_id':'b'*64,'formulas':{identity},'scenes':{'s':self.scene},'source':{'unit.cpp':'s'},
           'root':root,'stages':['formula_and_distance'],'training_inputs':set(),'threads':1,
           'renderer':p.binary_identity(root,self.tool),'library':lib,'compatible_library':compat,
           'library_path':root/lib['path'],'runtime':runtime,'runtime_files':{},'inputs':{'s':root/source['path']},
           'benchmarks':{m:p.binary_identity(root,self.tool) for m in ('baseline','candidate')},
           'libraries':{m:root/lib['path'] for m in ('baseline','candidate')},'build_dir':root/'build',
           'tools':{k:p.binary_identity(root,self.tool) for k in (*p.CASE_SETS,'dependency_analysis','cpu_ordinary','formula_gpu')},
           'cache_keys':{'resource_reuse':q.RESOURCE_KEYS,'result_reuse':q.RESULT_KEYS}}
        self.ctx['tools']['oracle']=p.binary_identity(root,self.oracle)
    def raw(self,name,data):
        self.serial+=1;path=self.root/(str(self.serial)+'-'+name);path.write_bytes(data)
        return {'path':path.name,'sha256':p.sha(path)}
    def put(self,name,data):return self.raw(name,json.dumps(data).encode())
    def replace(self,record,data):
        (self.root/record['path']).write_text(json.dumps(data));record['sha256']=p.sha(self.root/record['path'])
    def image(self,name,delta=0):
        self.serial+=1;path=self.root/(str(self.serial)+'-'+name)
        im=Image.new('RGB',(2,2));im.putdata([(1+delta,2,3),(4,5,6),(7,8,9),(10,11,12)]);im.save(path)
        return {'path':path.name,'sha256':p.sha(path)}
    def event(self,argv,outputs,inputs=(),code=0,seconds=10,rid=None):
        exe=Path(argv[0]);rid=rid or uuid.uuid4().hex
        e={'schema':'mb3d.gpu-process.v2','run_id':rid,'pid':self.serial+10,'started_ns':self.clock,
           'finished_ns':self.clock+int(seconds*1e9),'seconds':seconds,'effective_timeout_seconds':300,
           'owned_process_reaped':True,'exit_code':code,'guard':None,'argv':list(map(str,argv)),
           'cwd':str(self.ctx['runtime']),'executable_sha256_before':p.sha(exe),'executable_sha256_after':p.sha(exe),
           'outputs':outputs,'inputs':list(inputs),'log':self.raw('log',b'unit fixture log')}
        self.clock=e['finished_ns']+10**9
        return self.put('event.json',e)
    def input(self):
        return {'identity':list(next(iter(self.ctx['formulas']))),'kind':'point',
                'input_state_bits':{k:'0x3f800000' for k in p.STATE},'parameter_bits':[],'variant':'holdout'}
    def probes(self,kind):
        rows=[]
        for name in sorted(p.CASE_SETS[kind]):
            inp=self.put('input.json',[self.input()]);rid=uuid.uuid4().hex;comparisons=[]
            kinds=('state','logical_work','rgb') if kind in ('result_reuse','independent_heldout') else ('state','logical_work')
            for k in kinds:
                width={'state':188,'logical_work':16,'rgb':3}[k]
                comparisons.append({'kind':k,'records':1,'bytes_per_record':width,
                                    'expected':self.raw('expected',bytes(width)),'actual':self.raw('actual',bytes(width))})
            o={'schema':'mb3d.gpu-probe-observation.v1','case_id':name,'run_id':rid,'binding_sha256':self.ctx['binding_id'],
               'library_marker':[1,4,80] if kind=='metal_compatible' else [2,4,32],
               'library_sha256':self.ctx['compatible_library' if kind=='metal_compatible' else 'library']['sha256'],
               'cpu_pixel_calls':0,'gpu_invocations':1,'faults':0,'expected_faults':0,'comparisons':comparisons,
               'formula_identities':[list(next(iter(self.ctx['formulas'])))],'input_digest':p.digest([self.input()]),
               'stages':self.ctx['stages'],'active_tasks':2,'preserved_task_order':True}
            if kind in ('resource_reuse','result_reuse'):
                keys=dict.fromkeys(self.ctx['cache_keys'][kind],0);changed=dict(keys)
                if name!='hit':changed[p.KEY_CHANGE.get(name,next(iter(changed)))]=1
                o['cache']={'key_fields':list(keys),'hits':1,'misses':2,'evictions':1,'physical_work':1,
                    'uncached_physical_work':2,'peak_entries':1,'entry_limit':2,'peak_bytes':100,'byte_limit':200,
                    'requests':[{'key':keys,'hit':False},{'key':changed,'hit':name=='hit'}],
                    'overlapping_clients':2,'duplicate_compiles':0,'evicted_while_in_use':1,'live_handle_reads_after_eviction':1,
                    'failed_attempts':1,'successful_retries':1}
            obs=self.put('observation.json',o)
            ref=self.event([self.ctx['tools']['oracle']['path'],self.root/inp['path']],
                           [c['expected'] for c in comparisons],[inp])
            event=self.event([self.ctx['tools'][kind]['path'],self.root/inp['path']],
                             [obs,*[c['actual'] for c in comparisons]],[inp],rid=rid)
            rows.append({'id':name,'event':event,'reference_event':ref,'inputs':inp,'observation':obs})
        return {'cases':rows}
    def benchmark(self,mode,seconds):
        samples=[];outputs=[]
        for i in range(4):
            im=self.image('image.png');outputs.append(im)
            stages={n:{'state':'completed' if n in ('load','preparation','primary','color','final_surface','save') else 'not_run','seconds':0}
                    for n in ('load','preparation','primary','color','final_surface','save',*f.FLAGS)}
            execution={'pipeline_archive':[0,1] if i==0 else [1,0],'library_profile':[2,4,32],'validated':True,'engine':'general_formula','primary_records':4,'normal_records':4,
                       'surface_records':8,'color_records':4,'faults':0,'nonfinite':0,'work_exhausted':0}
            execution.update({key:1 for group,key in f.ACTIVITY.values() if group=='execution'})
            for stage,on in f.enabled(self.scene).items():stages[stage]['state']='completed' if on else 'not_run'
            samples.append({'phase':'cold' if i==0 else 'warm','index':0 if i==0 else i-1,'state':'completed','error':'','precision':'native',
              'width':2,'height':2,'image':im['path'],'deadline':{'seconds':seconds,'limit_seconds':300,'completed':True},
              'end_to_end_ms':seconds*1000,'cold_end_to_end_ms':seconds*1000,'stages':stages,
              'backend':{'requested':'metal','effective':'metal','fallback':False,'execution':execution,'reflections':{'faults':0,'rays':1}}})
        r={'schema':'mb3d.backend-render-benchmark.v1','status':'COMPLETE','error':'','backend':'metal',
           'execution':{'precision':'native','deterministic':False,'rng_mode':'scene'},'scene':str(self.ctx['inputs']['s']),
           'resource_root':str(self.ctx['runtime']),'settings':{'width':2,'height':2,'threads':1,'warmups':0,'repeats':3},
           'warmups':[],'cold':samples[0],'samples':samples[1:],'loaded_features':{'ordinary':True,'iOptions':1,'bCalcAmbShadowAutomatic':128},
           'effective_rng':{'primary_first_step_jitter':True,'ao_first_step_jitter':True}}
        report=self.put('report.json',r);outputs.append(report)
        args=[self.ctx['benchmarks'][mode]['path'],'--scene',self.ctx['inputs']['s'],'--output',self.root,
              '--resource-root',self.ctx['runtime'],'--backend','metal','--precision','native','--width','2','--height','2',
              '--threads','1','--warmups','0','--repeats','3','--metal-library',self.ctx['libraries'][mode],'--frame-timeout-seconds','300','--rng-mode','scene']
        event=self.event(args,outputs,seconds=4*seconds+.1)
        return {'report':report,'event':event}
    def timings(self):
        proof=f.frame(self.root,self.frame(),self.scene,self.ctx)
        self.ctx['qualified_scenes']={'s':{'image':proof['image'],'work':proof['work']}}
        a=self.benchmark('baseline',2);b=self.benchmark('candidate',1);c=self.benchmark('candidate',1);d=self.benchmark('baseline',2)
        return {'scenes':[{'id':'s','baseline':[a,d],'candidate':[b,c],'order':['baseline','candidate','candidate','baseline']}]}
    def frame(self,off=None):
        im=self.image('frame.png',1 if off else 0)
        stages={n:{'state':'completed' if n in ('load','preparation','primary','color','final_surface','save') else 'not_run','seconds':0}
                for n in ('load','preparation','primary','color','final_surface','save',*f.FLAGS)}
        for stage,on in f.enabled(self.scene).items():stages[stage]['state']='completed' if on and off!=stage else 'not_run'
        ex={'pipeline_archive':[1,0],'library_profile':[2,4,32],'validated':True,'engine':'general_formula','primary_records':4,'normal_records':4,
            'surface_records':8,'color_records':4,'faults':0,'nonfinite':0,'work_exhausted':0}
        ex.update({key:1 for group,key in f.ACTIVITY.values() if group=='execution'})
        if off and f.ACTIVITY[off][0]=='execution':ex[f.ACTIVITY[off][1]]=0
        prof={'schema':'mb3d.render-stage-profile.v1','status':'complete','render_status':'completed','exit_code':0,'error':'',
              'backend':'metal','backend_requested':'metal','backend_fallback':False,'backend_fallback_code':'','backend_fallback_reason':'',
              'precision':'native','numeric_profile':'native_metal_fp32','width':2,'height':2,'workers':0,'metal_device':'synthetic',
              'input':str(self.ctx['inputs']['s']),'output':str(self.root/im['path']),'metal_library':str(self.ctx['library_path']),
              'stages':stages,'metal_execution':ex,'metal_reflections':{'faults':0,'rays':0 if off=='reflections' else 1}}
        pr=self.put('profile.json',prof)
        argv=[self.ctx['renderer']['path'],self.ctx['inputs']['s'],'--backend','metal','--precision','native','--width','2','--height','2',
              '--threads','1','--metal-library',self.ctx['library_path'],'--output',self.root/im['path'],'--profile-json',self.root/pr['path']]
        if off:argv.append(f.FLAGS[off])
        ev=self.event(argv,[pr,im],[{'path':str(self.ctx['inputs']['s'].relative_to(self.root)),'sha256':self.scene['sha256']},self.ctx['library']])
        return {'profile':pr,'image':im,'event':ev}
    def scene_evidence(self):
        self.scene['settings'].update(ao_kind='DEAO',hard_shadows_enabled=True,reflection_enabled=True,dof_enabled=True,zbuffer_normals=True,volume_light=1)
        return {'frames':[self.frame() for _ in range(3)]},{'scene_sha256':self.scene['sha256'],'runtime_files':{},
                  'controls':[{'stage':k,'off':self.frame(k)} for k in f.FLAGS]}
    def guards(self):
        rows=[]
        for name in sorted(p.CASE_SETS['guard_failures']):
            rid=uuid.uuid4().hex;o=self.put('guard.json',{'case_id':name,'run_id':rid,'detected':True,'error_code':name,'image_path':'not-published.png'})
            ev=self.event([self.ctx['tools']['guard_failures']['path']],[o],code=124,rid=rid)
            rows.append({'id':name,'observation':o,'event':ev})
        return {'cases':rows}
    def dependency(self):
        ident=list(next(iter(self.ctx['formulas'])));rid=uuid.uuid4().hex
        o=self.put('audit.json',{'identity':ident,'run_id':rid,'read_before_write':[], 'writes':['xyz[0]'],'source_hashes':self.ctx['source']})
        e=self.event([self.ctx['tools']['dependency_analysis']['path']],[o],rid=rid)
        return {'formulas':[{'identity':ident,'classification':'independent','reads':['xyz[0]'],'writes':['xyz[0]'],
                 'initialized':['xyz[0]'],'live_in':[],'preserved_dependencies':[],'drivers':[0],
                 'source_files':['unit.cpp'],'event':e,'observation':o}]}
    def ctest(self):
        self.ctx['ordinary_commands']={n:[self.ctx['tools']['formula_gpu']['path']] for n in p.ORDINARY}
        listing=self.put('ctest-list.json',{'tests':[{'name':n,'command':a} for n,a in self.ctx['ordinary_commands'].items()]})
        le=self.event([self.ctx['tools']['cpu_ordinary']['path'],'--show-only=json-v1','-L','ordinary','--test-dir',self.ctx['build_dir']],[listing])
        xml='<testsuite>'+''.join('<testcase name="'+n+'" status="run"/>' for n in sorted(p.ORDINARY))+'</testsuite>'
        ref=self.raw('ordinary.xml',xml.encode())
        ev=self.event([self.ctx['tools']['cpu_ordinary']['path'],'-L','ordinary','--test-dir',self.ctx['build_dir'],'--output-junit',self.root/ref['path']],[ref],inputs=[{'path':str(x.relative_to(self.root)),'sha256':p.sha(x)} for x in p.ordinary_input_files(self.ctx['ordinary_commands'],self.root,self.ctx['build_dir'])])
        return {'event':ev,'junit':ref,'listing':listing,'listing_event':le}
    def trace(self):
        row=self.frame();event=p.read_json(self.root/row['event']['path']);rid=event['run_id']
        o=self.put('trace.json',{'schema':'mb3d.gpu-stage-trace.v1','run_id':rid,'profile_sha256':row['profile']['sha256'],
           'scene_sha256':self.scene['sha256'],'library_sha256':self.ctx['library']['sha256'],
           'cpu_calls':{'formula_and_distance':0},'gpu_calls':{'formula_and_distance':1}})
        event['outputs'].append(o);self.replace(row['event'],event)
        return {'scenes':[dict(row,id='s',trace=o)]}
    def build(self):
        src=self.root/'unit.cpp';src.write_text('synthetic build fixture');self.ctx['source']={'unit.cpp':p.sha(src)}
        frozen=self.root/'frozen';frozen.mkdir();(frozen/'unit.cpp').write_bytes(src.read_bytes())
        self.ctx['baseline_source_root']=frozen;self.ctx['baseline_source']=dict(self.ctx['source'])
        self.ctx['baseline_compiled_source_root']=self.root
        result={};self.ctx['build_products']={};self.ctx['library_air_counts']={}
        for mode in ('baseline','candidate'):
            build=self.root/('baseline' if mode=='baseline' else 'build');build.mkdir()
            self.ctx['baseline_build_dir' if mode=='baseline' else 'build_dir']=build
            records=[];objects=[]
            for i,name in enumerate(['unit.o',*[f'part{j}.air' for j in range(6)],'renderer','native.metallib','benchmark']):
                path=build/name;path.write_bytes(name.encode())
                out={'path':str(path),'sha256':p.sha(path)}
                source={'path':str(src),'sha256':p.sha(src),'classification':'repository'}
                rec={'schema':'mb3d.actual-build-command.v1','exit_code':0,'capture_errors':[],
                     'pid':i+1,'start_time_ns':100*i+1,'end_time_ns':100*i+2,'argv':[sys.executable,'fixture'],
                     'tool':{'path':sys.executable,'sha256':p.sha(sys.executable)},'output':out,
                     'depfile':'synthetic.d','dependencies':[source], 'explicit_inputs_before':list(objects) if i>=7 else []}
                if i<7:objects.append(out)
                records.append(self.put('command.json',rec))
            result[mode]={'commands':records}
            self.ctx['build_products'][mode]=[{'path':str((build/n).relative_to(self.root)),'sha256':p.sha(build/n)} for n in ('renderer','native.metallib','benchmark')]
            self.ctx['library_air_counts'][str((build/'native.metallib').relative_to(self.root))]=6
        return result

class ProofTests(unittest.TestCase):
    def setUp(self):self.tmp=tempfile.TemporaryDirectory();self.b=Bundle(Path(self.tmp.name).resolve())
    def tearDown(self):self.tmp.cleanup()
    def verify(self,k,d):p.VALIDATORS[k](self.b.root,d,self.b.ctx)
    def mutate_observation(self,data,case,fn):
        row=next(c for c in data['cases'] if c['id']==case);ref=row['observation'];old=copy.deepcopy(ref)
        obs=p.read_json(self.b.root/ref['path']);fn(obs);self.b.replace(ref,obs)
        e=p.read_json(self.b.root/row['event']['path']);e['outputs']=[ref if r==old else r for r in e['outputs']];self.b.replace(row['event'],e)
    def test_positive_probe_adapters(self):
        for kind in ('scheduling','resource_reuse','result_reuse','metal_compatible','independent_heldout'):
            with self.subTest(kind=kind):self.verify(kind,self.b.probes(kind))
    def test_each_probe_requires_all_cases(self):
        for kind in ('scheduling','resource_reuse','result_reuse','metal_compatible','independent_heldout'):
            d=self.b.probes(kind);d['cases'].pop()
            with self.subTest(kind=kind),self.assertRaises(ValueError):self.verify(kind,d)
    def test_typed_buffer_not_one_byte(self):
        d=self.b.probes('scheduling');self.mutate_observation(d,'group1',lambda o:o['comparisons'][0].update(records=2))
        with self.assertRaises(ValueError):self.verify('scheduling',d)
    def test_heldout_digest_recomputed(self):
        d=self.b.probes('independent_heldout');self.mutate_observation(d,'state_poison',lambda o:o.update(input_digest='0'*64))
        with self.assertRaises(ValueError):self.verify('independent_heldout',d)
    def test_training_cannot_be_holdout(self):
        d=self.b.probes('independent_heldout');row=self.b.input();row['variant']='renamed-training';self.b.ctx['training_inputs']={p.digest(p.semantic_input(row))}
        with self.assertRaises(ValueError):self.verify('independent_heldout',d)
    def test_cache_stale_hit_rejected(self):
        d=self.b.probes('resource_reuse');self.mutate_observation(d,'library_replace',lambda o:o['cache']['requests'][1].update(hit=True))
        with self.assertRaises(ValueError):self.verify('resource_reuse',d)
    def test_cache_unbounded_rejected(self):
        d=self.b.probes('result_reuse');self.mutate_observation(d,'hit',lambda o:o['cache'].update(peak_bytes=201))
        with self.assertRaises(ValueError):self.verify('result_reuse',d)
    def test_concurrency_must_be_exercised(self):
        d=self.b.probes('resource_reuse');self.mutate_observation(d,'concurrent',lambda o:o['cache'].update(overlapping_clients=1))
        with self.assertRaises(ValueError):self.verify('resource_reuse',d)
    def test_positive_full_warm_timings(self):f.full_warm_timings(self.b.root,self.b.timings(),self.b.ctx)
    def test_duplicate_process_rejected(self):
        d=self.b.timings();d['scenes'][0]['candidate'][1]=d['scenes'][0]['candidate'][0]
        with self.assertRaises(ValueError):f.full_warm_timings(self.b.root,d,self.b.ctx)
    def test_wrong_actual_order_rejected(self):
        d=self.b.timings();d['scenes'][0]['baseline'].reverse()
        with self.assertRaises(ValueError):f.full_warm_timings(self.b.root,d,self.b.ctx)
    def test_underreported_timing_rejected(self):
        d=self.b.timings();job=d['scenes'][0]['candidate'][0];r=p.read_json(self.b.root/job['report']['path']);old=copy.deepcopy(job['report'])
        r['samples'][0]['end_to_end_ms']=1;self.b.replace(job['report'],r)
        e=p.read_json(self.b.root/job['event']['path']);e['outputs']=[job['report'] if x==old else x for x in e['outputs']];self.b.replace(job['event'],e)
        with self.assertRaises(ValueError):f.full_warm_timings(self.b.root,d,self.b.ctx)
    def test_process_clock_reconciled(self):
        d=self.b.timings();ev=d['scenes'][0]['baseline'][0]['event'];e=p.read_json(self.b.root/ev['path']);e['seconds']=.1;self.b.replace(ev,e)
        with self.assertRaises(ValueError):f.full_warm_timings(self.b.root,d,self.b.ctx)
    def test_extreme_number_is_clean_failure(self):
        with self.assertRaises(ValueError):p.positive(10**1000,'too large')
    def test_every_obligation_has_concrete_adapter(self):
        self.assertEqual(set(q.PROOFS),set(p.VALIDATORS))
    def test_gpu_slow_repeats_rejected(self):
        row=self.b.frame();e=p.read_json(self.b.root/row['event']['path'])
        e['seconds']=190;e['finished_ns']=e['started_ns']+190*10**9;self.b.replace(row['event'],e)
        with self.assertRaisesRegex(ValueError,'Diagnostic deadline'):f.frame(self.b.root,row,self.b.scene,self.b.ctx)
    def test_gpu_missing_captured_inputs_rejected(self):
        row=self.b.frame();e=p.read_json(self.b.root/row['event']['path']);e['inputs']=[];self.b.replace(row['event'],e)
        with self.assertRaisesRegex(ValueError,'bound scene/library'):f.frame(self.b.root,row,self.b.scene,self.b.ctx)
    def test_parity_frame_override_rejected(self):
        row=self.b.frame();event=p.read_json(self.b.root/row['event']['path'])
        event['argv'].append('--deterministic');self.b.replace(row['event'],event)
        with self.assertRaises(ValueError):f.frame(self.b.root,row,self.b.scene,self.b.ctx)
    def test_parity_benchmark_override_rejected(self):
        row=self.b.benchmark('candidate',1);event=p.read_json(self.b.root/row['event']['path'])
        event['argv'][event['argv'].index('--rng-mode')+1]='parity';self.b.replace(row['event'],event)
        with self.assertRaises(ValueError):f.benchmark(self.b.root,row,self.b.scene,self.b.ctx,'candidate')
    def test_benchmark_suppressed_jitter_rejected(self):
        row=self.b.benchmark('candidate',1);old=copy.deepcopy(row['report']);report=p.read_json(self.b.root/old['path'])
        report['effective_rng']['primary_first_step_jitter']=False;self.b.replace(row['report'],report)
        event=p.read_json(self.b.root/row['event']['path']);event['outputs']=[row['report'] if x==old else x for x in event['outputs']]
        self.b.replace(row['event'],event)
        with self.assertRaises(ValueError):f.benchmark(self.b.root,row,self.b.scene,self.b.ctx,'candidate')
    def test_missing_archive_observation_rejected(self):
        with self.assertRaises(KeyError):f.archive_state({})
        self.assertEqual(f.archive_state({'pipeline_archive':[0,0]}),[0,0])
        for state in ([True,0],[-1,2],[1,0,0]):
            with self.subTest(state=state),self.assertRaises(ValueError):f.archive_state({'pipeline_archive':state})
    def test_warm_archive_miss_rejected(self):
        row=self.b.benchmark('candidate',1);old=copy.deepcopy(row['report']);report=p.read_json(self.b.root/old['path'])
        report['samples'][0]['backend']['execution']['pipeline_archive']=[1,1];self.b.replace(row['report'],report)
        event=p.read_json(self.b.root/row['event']['path']);event['outputs']=[row['report'] if x==old else x for x in event['outputs']]
        self.b.replace(row['event'],event)
        with self.assertRaises(ValueError):f.benchmark(self.b.root,row,self.b.scene,self.b.ctx,'candidate')
    def test_positive_scene_controls(self):
        r,proof=self.b.scene_evidence();f.scene_frames(self.b.root,r,self.b.scene,self.b.ctx,proof)
    def test_known_numeric_outlier_needs_bound_review_despite_exact_repeats(self):
        r,proof=self.b.scene_evidence();self.b.scene['path']='PaMonk - Ringlets.m3p'
        with self.assertRaisesRegex(ValueError,'Missing bound numerical review'):
            f.scene_frames(self.b.root,r,self.b.scene,self.b.ctx,proof)
    def test_known_bias_outlier_needs_bound_review(self):
        r,proof=self.b.scene_evidence();self.b.scene['path']='UndeadAcademy tweak - Peacock in blue.m3p'
        with self.assertRaisesRegex(ValueError,'Missing bound numerical review'):
            f.scene_frames(self.b.root,r,self.b.scene,self.b.ctx,proof)
    def test_numeric_scope_does_not_reject_other_scenes_by_same_author(self):
        from native_numeric_review import pending
        self.assertTrue(pending({'path':'Lenord - Between Dark and Light.m3p'}))
        self.assertFalse(pending({'path':'Lenord - ASurf 2.m3p'}))
        self.assertFalse(pending({'path':'Lenord - The Beanstalk.m3p'}))
    def test_reused_frames_rejected(self):
        r,proof=self.b.scene_evidence();r['frames'][1]=r['frames'][0]
        with self.assertRaises(ValueError):f.scene_frames(self.b.root,r,self.b.scene,self.b.ctx,proof)
    def test_missing_active_control_rejected(self):
        r,proof=self.b.scene_evidence();proof['controls'].pop()
        with self.assertRaises(ValueError):f.scene_frames(self.b.root,r,self.b.scene,self.b.ctx,proof)
    def test_resource_closure_changed(self):
        r,proof=self.b.scene_evidence();proof['runtime_files']={'extra.png':'x'}
        with self.assertRaises(ValueError):f.scene_frames(self.b.root,r,self.b.scene,self.b.ctx,proof)
    def test_positive_other_adapters(self):
        for kind,make in [('dependency_analysis',self.b.dependency),('guard_failures',self.b.guards),
                          ('cpu_ordinary',self.b.ctest),('no_cpu_pixel_work',self.b.trace),('build_provenance',self.b.build)]:
            with self.subTest(kind=kind):self.verify(kind,make())
    def test_unknown_state_dependency_rejected(self):
        d=self.b.dependency();d['formulas'][0]['initialized']=[]
        with self.assertRaises(ValueError):self.verify('dependency_analysis',d)
    def test_missing_guard_rejected(self):
        d=self.b.guards();d['cases'].pop()
        with self.assertRaises(ValueError):self.verify('guard_failures',d)
    def test_guard_image_published_rejected(self):
        d=self.b.guards();(self.b.root/'not-published.png').write_bytes(b'bad')
        with self.assertRaises(ValueError):self.verify('guard_failures',d)
    def test_malformed_xml_is_clean_failure(self):
        d=self.b.ctest();old=copy.deepcopy(d['junit']);path=self.b.root/d['junit']['path'];path.write_text('<broken')
        d['junit']['sha256']=p.sha(path);e=p.read_json(self.b.root/d['event']['path']);e['outputs']=[d['junit'] if r==old else r for r in e['outputs']];self.b.replace(d['event'],e)
        with self.assertRaises(ValueError):self.verify('cpu_ordinary',d)
    def test_parallel_compilers_can_share_timestamp(self):
        d=self.b.build();refs=d['candidate']['commands'];a=p.read_json(self.b.root/refs[0]['path']);b=p.read_json(self.b.root/refs[1]['path'])
        b['start_time_ns']=a['start_time_ns'];self.b.replace(refs[1],b);self.verify('build_provenance',d)
    def test_duplicate_compiler_identity_rejected(self):
        d=self.b.build();refs=d['candidate']['commands'];a=p.read_json(self.b.root/refs[0]['path']);b=p.read_json(self.b.root/refs[1]['path'])
        b.update(pid=a['pid'],start_time_ns=a['start_time_ns']);self.b.replace(refs[1],b)
        with self.assertRaisesRegex(ValueError,'Duplicate/empty'):self.verify('build_provenance',d)
    def candidate_frozen_build(self):
        b=self.b;data=b.build();frozen=b.root/'candidate-source';frozen.mkdir()
        (frozen/'unit.cpp').write_bytes((b.root/'unit.cpp').read_bytes());b.ctx['candidate_compiled_source_root']=frozen
        for record in data['candidate']['commands']:
            value=p.read_json(b.root/record['path'])
            for dep in value['dependencies']:
                if dep['path']==str(b.root/'unit.cpp'):dep['path']=str(frozen/'unit.cpp')
            b.replace(record,value)
        return data,frozen
    def test_candidate_frozen_compilation_root(self):
        data,_=self.candidate_frozen_build();self.verify('build_provenance',data)
    def test_changed_current_source_cannot_borrow_frozen_provenance(self):
        data,_=self.candidate_frozen_build();(self.b.root/'unit.cpp').write_text('different current source')
        with self.assertRaisesRegex(ValueError,'Changed compiled source copy'):self.verify('build_provenance',data)
    def test_wrong_compilation_root_cannot_relabel_project_inputs_external(self):
        data,frozen=self.candidate_frozen_build();self.b.ctx['candidate_compiled_source_root']=frozen/'wrong'
        with self.assertRaisesRegex(ValueError,'outside declared'):self.verify('build_provenance',data)
    def test_missing_link_input_producer(self):
        d=self.b.build();d['candidate']['commands'].pop(0)
        with self.assertRaises(ValueError):self.verify('build_provenance',d)
    def test_compatible_seventh_air_uses_role_not_filename(self):
        b=self.b;d=b.build();build=b.ctx['build_dir'];source=b.root/'unit.cpp'
        air=build/'seventh.air';air.write_bytes(b'compatible AIR fixture')
        ar={'path':str(air),'sha256':p.sha(air)}
        command={'schema':'mb3d.actual-build-command.v1','exit_code':0,'capture_errors':[],
          'pid':100,'start_time_ns':2001,'end_time_ns':2002,'argv':[sys.executable,'fixture'],
          'tool':{'path':sys.executable,'sha256':p.sha(sys.executable)},'output':ar,
          'depfile':'fixture.d','dependencies':[{'path':str(source),'sha256':p.sha(source)}],'explicit_inputs_before':[]}
        d['candidate']['commands'].append(b.put('seventh.json',command))
        lib=build/'arbitrary-name.metallib';lib.write_bytes(b'compatible library fixture')
        lr={'path':str(lib.relative_to(b.root)),'sha256':p.sha(lib)}
        command=copy.deepcopy(command);command.update(start_time_ns=2101,end_time_ns=2102,
          output={'path':str(lib),'sha256':p.sha(lib)},explicit_inputs_before=[ar])
        d['candidate']['commands'].append(b.put('compat-link.json',command))
        b.ctx['build_products']['candidate'].append(lr);b.ctx['library_air_counts'][lr['path']]=1
        self.verify('build_provenance',d)
        d['candidate']['commands'].pop(-2)
        with self.assertRaises(ValueError):self.verify('build_provenance',d)
    def test_ordinary_test_inputs_are_bound(self):
        d=self.b.ctest();event=p.read_json(self.b.root/d['event']['path']);event['inputs']=[];self.b.replace(d['event'],event)
        with self.assertRaisesRegex(ValueError,'Unbound ordinary test input'):self.verify('cpu_ordinary',d)

    def test_current_ordinary_suites_cannot_be_omitted(self):
        names={'native_render_plan_tests','native_resource_entry_tests','native_formula_resources_tests',
               'native_material_palette_tests','native_postprocess_plan_tests','background_resources_tests'}
        self.assertTrue(names<=p.ORDINARY)
        for missing in names:
            with self.subTest(missing=missing):
                b=self.b;d=b.ctest();self.verify('cpu_ordinary',d);old=copy.deepcopy(d['junit'])
                path=b.root/d['junit']['path']
                path.write_text('<testsuite>'+''.join('<testcase name="'+n+'" status="run"/>' for n in sorted(p.ORDINARY-{missing}))+'</testsuite>')
                d['junit']['sha256']=p.sha(path);event=p.read_json(b.root/d['event']['path'])
                event['outputs']=[d['junit'] if r==old else r for r in event['outputs']];b.replace(d['event'],event)
                with self.assertRaisesRegex(ValueError,'Wrong ordinary suite set'):self.verify('cpu_ordinary',d)

    def test_changed_ctest_command_rejected(self):
        b=self.b;d=b.ctest();old=copy.deepcopy(d['listing']);listing=p.read_json(b.root/d['listing']['path'])
        listing['tests'][0]['command']=['/unbound/test'];b.replace(d['listing'],listing)
        ev=p.read_json(b.root/d['listing_event']['path']);ev['outputs']=[d['listing'] if r==old else r for r in ev['outputs']]
        b.replace(d['listing_event'],ev)
        with self.assertRaises(ValueError):self.verify('cpu_ordinary',d)
    def test_complete_synthetic_bundle_can_pass_then_reject_missing_proof(self):
        # Only the finite inventory and current checkout binding are mocked.
        # Receipts, observations, events, PNGs, compiler closure and every
        # validator execute normally; this is not real GPU evidence.
        b=self.b;build=b.build();contract=b.put('contract.json',{})
        binding={'contract_sha256':contract['sha256'],'source':b.ctx['source']}
        b.ctx['binding_id']=p.digest(binding)
        def receipt(kind,data,**extra):
            obs=b.put(kind+'-data.json',data)
            return b.put(kind+'-receipt.json',{'schema':'mb3d.native-metal-all.receipt.v1','kind':kind,
                  'status':'PASS','binding_sha256':b.ctx['binding_id'],'producer':'synthetic-producer',
                  'independent_reviewer':'synthetic-reviewer','checks':[{'id':'all','status':'PASS'}],
                  'artifacts':[obs],'observations':obs,**extra})
        ident=list(next(iter(b.ctx['formulas'])));cases=[]
        for kind in ('point','orbit'):
            bits=dict.fromkeys(p.STATE,'0x00000000');repeats=[]
            payload={'identity':ident,'kind':kind,'input_state_bits':bits,'parameter_bits':[]}
            inp=b.put('formula-input.json',payload)
            for _ in range(3):
                rid=uuid.uuid4().hex;o=b.put('formula-repeat.json',{'run_id':rid,'state_bits':bits,'library_sha256':b.ctx['library']['sha256'],
                     'identity':ident,'input_digest':p.digest(payload),'faults':0,'gpu_invocations':1})
                e=b.event([b.ctx['tools']['formula_gpu']['path'],b.root/inp['path']],[o],[inp],rid=rid);repeats.append({'event':e,'observation':o,'inputs':inp})
            cases.append({'case_id':kind,'identity':ident,'kind':kind,'variant':'training','gpu_invocations':1,'faults':0,
                 'input_state_bits':bits,'parameter_bits':[],'expected_state_bits':bits,'gpu_state_bits':bits,
                 'read_lanes':[],'written_lanes':list(p.STATE),'repeats':repeats})
        fp=receipt('formula_gpu_point_orbit',{'schema':'mb3d.native-metal-all.formula-observations.v1',
             'binding_sha256':b.ctx['binding_id'],'library_marker':[2,4,32],'execution':'metal_gpu','cpu_pixel_calls':0,'cases':cases},
             identity=ident,state_contract_verified=True,parameter_contract_verified=True)
        p.formula_repeats(b.root,cases,tuple(ident),b.ctx)
        with self.assertRaises(ValueError):p.formula_repeats(b.root,[cases[0],cases[0]],tuple(ident),b.ctx)
        scene,sp=b.scene_evidence();scene.update(id='s',effect_and_resource_proof=receipt('scene_effects_resources',sp))
        data={'build_provenance':build,'dependency_analysis':b.dependency(),'cpu_ordinary':b.ctest(),
              'guard_failures':b.guards(),'no_cpu_pixel_work':b.trace(),'full_warm_timings':b.timings()}
        for k in ('scheduling','resource_reuse','result_reuse','metal_compatible','independent_heldout'):data[k]=b.probes(k)
        for k,d in data.items():d.update(schema='mb3d.gpu-pipeline-proof.v1',kind=k,binding_sha256=b.ctx['binding_id'])
        manifest={'schema':q.SCHEMA,'binding':binding,'formulas':[{'namespace':ident[0],'id':ident[1],'name':ident[2],'proof':fp}],
                 'scenes':[scene],'proofs':[{'kind':k,'artifact':receipt(k,d)} for k,d in data.items()]}
        path=b.root/'evidence.json';path.write_text(json.dumps(manifest))
        with patch.object(q,'scope',return_value=({},dict.fromkeys(b.ctx['formulas']),{'s':b.scene})), \
             patch.object(q,'source_snapshot',return_value=b.ctx['source']),patch.object(q,'context',return_value=b.ctx):
            result=q.evaluate(b.root/contract['path'],b.root)
            self.assertEqual(result['status'],'PASS',result['errors']);self.assertEqual(result['remaining'],0)
            manifest['proofs'].pop();path.write_text(json.dumps(manifest))
            result=q.evaluate(b.root/contract['path'],b.root)
            self.assertEqual(result['status'],'OPEN');self.assertEqual(result['remaining'],1)

if __name__=='__main__':unittest.main()
