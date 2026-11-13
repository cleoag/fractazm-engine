"""Software-only original-semantic regression tests; no device observations."""
import copy
from fractions import Fraction
import hashlib
import struct
import unittest
from unittest.mock import patch
import native_group04_symmetry_reference as model
m = model


class SymmetryReferenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.cases=model.build_cases()
        cls.byid={c['id']:c for c in cls.cases}

    def case(self,ident):return copy.deepcopy(self.byid[ident]['input'])
    def out(self,ident):return model.evaluate(self.case(ident))
    def xyz(self,ident):return tuple(self.out(ident)[f'xyz[{i}]'] for i in range(3))
    def bits(self,*values):return tuple(model.word(model.fp32(v)) for v in values)
    def rebuild(self,payload):
        state={k:int(v,16) for k,v in payload['input_state_bits'].items()}
        program=[int(v,16) for v in payload['parameter_bits']]
        params=model._parameters(payload['identity'][2],program)
        payload['parameter_bits']=[model.word(v) for v in model._program(payload['identity'][2],params,state)]
        return payload

    def test_corpus_shape_uniqueness_and_replay(self):
        self.assertEqual(len(self.cases),63)
        self.assertEqual(len(self.byid),63)
        self.assertEqual(sum(c['input']['kind']=='point' for c in self.cases),48)
        for c in self.cases:
            self.assertEqual(len(c['input']['parameter_bits']),2048)
            self.assertEqual(set(c['expected_state_bits']),set(model.STATE))
            self.assertEqual(model.evaluate(c['input']),c['expected_state_bits'])

    def test_manifest_original_and_dependency_pins(self):
        m=model.manifest()
        self.assertEqual(len(m['code_audit']),3)
        self.assertEqual(len(m['semantic_sources']),8)
        self.assertEqual(len(m['dependencies']),4)
        self.assertIn('mb3d/DivUtils.pas',m['semantic_sources'])
        self.assertEqual(len(model.dependency_paths()),4)
        self.assertEqual(m['model_sha256'],hashlib.sha256(model.Path(model.__file__).read_bytes()).hexdigest())

    def test_Msltoe_abs_swap_and_sign(self):
        expected=self.bits(Fraction(-7,64),Fraction(-3,8),0)
        self.assertEqual(self.xyz('7071-positive_no_swap'),expected)
        self.assertEqual(self.xyz('7071-positive_swap'),expected)
        self.assertEqual(self.xyz('7071-negative_y'),self.bits(Fraction(57,64),Fraction(13,8),0))

    def test_Msltoe_equal_absolute_opposite_and_caller_rout(self):
        self.assertEqual(self.xyz('7071-equal_absolute_opposite'),self.bits(Fraction(-7,128),Fraction(-3,16),Fraction(-5,8)))
        self.assertEqual(self.xyz('7071-equal_absolute_reverse'),self.bits(Fraction(-7,128),Fraction(-3,16),Fraction(5,8)))
        self.assertEqual(self.xyz('7071-caller_rout_sentinel'),self.bits(Fraction(-49,512),Fraction(-21,64),Fraction(5,8)))
        p=self.case('7071-caller_rout_sentinel');p['input_state_bits']['rout']='0x3f000000'
        self.assertNotEqual(model.evaluate(p)['xyz[0]'],self.out('7071-caller_rout_sentinel')['xyz[0]'])

    def test_Msltoe_Jz_not_added(self):
        p=self.case('7071-positive_no_swap');a=model.evaluate(p)
        p['input_state_bits']['j[2]']='0x7f7fffff';b=model.evaluate(p)
        self.assertEqual(a['xyz[2]'],b['xyz[2]'])
        self.assertEqual(a['xyz[2]'],'0x00000000')

    def test_Benesi_strict_comparison_and_sign_byte(self):
        self.assertEqual(self.xyz('7085-above_threshold'),self.bits(-3,-3,2))
        self.assertEqual(self.xyz('7085-negative_Jx'),self.bits(2,-3,2))
        self.assertEqual(self.xyz('7085-negative_zero_Jx'),self.bits(3,-3,2))
        self.assertEqual(self.out('7085-equal_threshold')['xyz[0]'],'0x00000000')
        p=self.case('7085-equal_threshold');p['input_state_bits']['xyz[0]']='0x3f800001'
        self.assertNotEqual(model.evaluate(p)['xyz[0]'],'0x00000000')

    def test_Benesi_irrational_sqrt_independent_midpoint(self):
        # sqrt(2) target is certified by exact rational midpoint inequalities.
        root=0x3fb504f3;low=(model.fraction(root-1)+model.fraction(root))/2
        high=(model.fraction(root)+model.fraction(root+1))/2
        self.assertLess(low*low,2);self.assertGreater(high*high,2)
        self.assertEqual(model.julia.sqrt_bits(model.fp32(2)),root)
        self.assertEqual(self.out('7085-irrational_sqrt')['xyz[0]'],'0xbf800000')
        self.assertEqual(self.out('7085-irrational_sqrt')['xyz[1]'],'0x00000000')

    def test_Kali_polynomial_expected_states_independent_of_model_order(self):
        # Algebraic complex powers on exact unit bases. Degree comes from a
        # separate piecewise interpretation of logical SHR/SUB/JS control.
        def multiply(a,b):return (a[0]*b[0]-a[1]*b[1],a[0]*b[1]+a[1]*b[0])
        for c in self.cases:
            if not c['id'].startswith('10352-power_'):continue
            power=int(c['variant'].split('_',1)[1]);u=power&0xffffffff
            degree=(u if 2<=u<=9 else 2+(u&1) if u<2 else 8+(u&1))
            value=(Fraction(1),Fraction(0))
            for _ in range(degree):value=multiply(value,(Fraction(0),Fraction(1)))
            real,imag=value[0],value[1]+Fraction(1,4)
            result=model.evaluate(c['input'])
            # Both complex lanes identical => independent energy ratio=1/4.
            self.assertEqual(tuple(result[f'xyz[{i}]'] for i in range(3)),self.bits(real,imag,0),c['id'])
            for k,v in [('dfree1',imag),('dfree2',imag),('derivative[1]',real),('derivative[2]',real)]:self.assertEqual(result[k],model.word(model.fp32(v)))
            self.assertEqual(result['first'],'0x00000001')

    def test_typed_Power_float_alias_is_discriminating(self):
        even=self.case('10352-power_16777216');odd=self.case('10352-power_16777217')
        self.assertEqual(even['parameter_bits'][68],odd['parameter_bits'][68])
        self.assertNotEqual(even['parameter_bits'][132:134],odd['parameter_bits'][132:134])
        self.assertNotEqual(self.xyz('10352-power_16777216'),self.xyz('10352-power_16777217'))
        raw=struct.pack('<64I',*[int(x,16) for x in odd['parameter_bits'][260:324]])
        self.assertEqual(struct.unpack_from('<i',raw,244)[0],16777217)

    def test_Kali_carried_first_uses_scratch_not_xyz(self):
        for first in (1,0x1000001,0x7fffffff,0x80000000,0xffffffff):
            ident='10352-carried_first_'+str(first);out=self.out(ident)
            self.assertEqual(tuple(out[f'xyz[{i}]'] for i in range(3)),self.bits(0,Fraction(-3,4),7))
            self.assertEqual(out['first'],model.word(first))
            for k in ('derivative[0]','derivative[3]',*[f'sse[{i}]' for i in range(4)]):self.assertEqual(out[k],self.byid[ident]['input']['input_state_bits'][k])

    def test_Kali_original_semicolon_constant_and_fixed_half(self):
        self.assertEqual(model.parsed_kali_constant(),0)
        p=self.case('10352-power_2')
        self.assertEqual(p['parameter_bits'][69:71],['0x00000000','0x3f000000'])
        state={k:int(v,16) for k,v in p['input_state_bits'].items()}
        with self.assertRaises(model.ScopeError):model._program('KaliLinCombSSE2sm',[2,model.fp32(Fraction(1,10**30)),model.HALF],state)

    def test_integer_sentinels_and_unwritten_lanes(self):
        for c in self.cases:
            if c['input']['kind']!='point':continue
            actual=model.evaluate(c['input'])
            for k in set(model.STATE)-set(c['write_lanes']):self.assertEqual(actual[k],c['input']['input_state_bits'][k],(c['id'],k))
        for ident in (7071,7085,10352):
            out=self.out(f'{ident}-integer_sentinels')
            self.assertEqual(out['iteration'],'0x01000001');self.assertEqual(out['tmp'],'0x80000000');self.assertEqual(out['first'],'0xffffffff')

    def test_orbit_initialization_and_real_early_escape(self):
        for ident in (7071,7085,10352):
            out=self.out(f'{ident}-orbit_escape');p=self.byid[f'{ident}-orbit_escape']['input']
            self.assertEqual(out['iteration'],'0x00000001')
            self.assertGreater(int(p['input_state_bits']['max_iterations'],16),1)
            self.assertEqual(out['xyz[3]'],'0x00000000')
            self.assertEqual(out['first'],'0x00000001' if ident==10352 else '0x00000000')
            self.assertGreater(int(self.out(f'{ident}-orbit_equal_threshold')['iteration'],16),1)

    def test_orbit_Julia_and_counts_discriminate(self):
        for ident in (7071,7085,10352):
            self.assertNotEqual(self.xyz(f'{ident}-orbit_mandel'),self.xyz(f'{ident}-orbit_julia'))
            self.assertEqual(self.out(f'{ident}-orbit_count_cycle')['tmp'],'0x00000001')
            self.assertEqual(self.out(f'{ident}-orbit_count_cycle')['iteration'],'0x00000003')

    def test_singular_inputs_rejected(self):
        for ident in ('7071-positive_no_swap','7085-above_threshold','10352-power_2'):
            p=self.case(ident)
            if ident.startswith('7071'):p['input_state_bits']['rout']='0x00000000'
            else:
                for i in range(3):p['input_state_bits'][f'xyz[{i}]']='0x00000000';p['input_state_bits'][f'j[{i}]']='0x00000000'
            with self.assertRaises(model.ScopeError,msg=ident):model.evaluate(p)

    def test_nonfinite_subnormal_and_overflow_rejected(self):
        for word in ('0x7f800000','0x7fc00000','0x00000001','0x7f7fffff'):
            p=self.case('7085-above_threshold');p['input_state_bits']['xyz[0]']=word
            with self.assertRaises(model.ScopeError,msg=word):model.evaluate(p)

    def test_payload_and_prepared_ABI_tamper_rejected(self):
        for change in ('identity','raw_int','constant','cache','length','extra','state_lane'):
            p=self.case('10352-power_16777217')
            if change=='identity':p['identity'][1]=7071
            elif change=='raw_int':p['parameter_bits'][321]='0x01000000'
            elif change=='constant':p['parameter_bits'][69]='0x3f800000'
            elif change=='cache':p['parameter_bits'][325]='0x3f800000'
            elif change=='length':p['parameter_bits'].pop()
            elif change=='extra':p['metadata']={}
            else:del p['input_state_bits']['sse[0]']
            with self.assertRaises(ValueError,msg=change):model.evaluate(p)

    def test_orbit_domains_fail_closed(self):
        for field,value in [('max_iterations',9),('hybrid_counts[0]',0),('calc_smooth',1),('inside',1),('julia',2)]:
            p=self.case('7085-orbit_mandel');p['input_state_bits'][field]=model.word(value)
            with self.assertRaises(ValueError):model.evaluate(self.rebuild(p))




class CallerSSEProfileV2(unittest.TestCase):
    def state(self, values, maximum=3, stop=64):
        s=m._state()
        for i,v in enumerate(values):s[f'c[{i}]']=m.fp32(v)
        s['max_iterations']=maximum;s['rstop']=m.fp32(stop)
        return s

    def test_max_only_bailout_only_and_simultaneous_exit(self):
        for maximum,stop,expected_iterations,expected_low in ((1,64,1,2),(3,64,3,2),(4,Fraction(1,2),1,Fraction(1,2)),(1,Fraction(1,2),1,2)):
            state=self.state((1,1,0),maximum,stop)
            out=m._orbit('Benesi3Pow2',state,[])
            self.assertEqual(out['iteration'],expected_iterations)
            self.assertEqual([out[f'sse[{i}]'] for i in range(4)],
                             [m.fp32(expected_low),0,m.fp32(2),0])

    def test_first_callback_and_continuation_transport(self):
        original=m._point;calls=[]
        def capture(name,state,params,*args,**kwargs):
            calls.append(dict(state));return original(name,state,params,*args,**kwargs)
        with patch.object(m,'_point',side_effect=capture):
            m._orbit('Benesi3Pow2',self.state((1,0,1),1),[])
        self.assertEqual([calls[0][f'sse[{i}]'] for i in range(4)],
                         [m.fp32(1),0,m.fp32(2),0])
        calls.clear()
        with patch.object(m,'_point',side_effect=capture):
            m._orbit('Benesi3Pow2',self.state((1,1,0),3),[])
        self.assertEqual(len(calls),3)
        for incoming in calls[1:]:
            self.assertEqual([incoming[f'sse[{i}]'] for i in range(4)],
                             [m.fp32(64),0,m.fp32(2),0])

    def test_point_bodies_preserve_all_four_SSE_words(self):
        sentinels=[0x80000000,0x3f800001,0xbf123456,0x7f000000]
        for c in m.build_cases():
            if c['input']['kind']!='point':continue
            payload=copy.deepcopy(c['input'])
            for i,bits in enumerate(sentinels):payload['input_state_bits'][f'sse[{i}]']=m.word(bits)
            out=m.evaluate(payload)
            self.assertEqual([out[f'sse[{i}]'] for i in range(4)],list(map(m.word,sentinels)))
            for lane in set(m.STATE)-{f'sse[{i}]' for i in range(4)}:
                self.assertEqual(out[lane],c['expected_state_bits'][lane],(c['id'],lane))

    def test_manifest_pins_new_profile_and_rejects_tamper(self):
        manifest=m.manifest()
        self.assertTrue(manifest['schema'].endswith('.v2'))
        self.assertEqual(manifest['caller_profile']['sha256'],
                         '52a02f1b539b022d9975baf0ac027e3463d5441aa755c4bee143506aa1e1ed6e')
        with patch.object(m,'CALLER_PROFILE',{**m.CALLER_PROFILE,'sha256':'0'*64}):
            with self.assertRaisesRegex(ValueError,'caller proof'):m.manifest()


if __name__=='__main__':unittest.main()
