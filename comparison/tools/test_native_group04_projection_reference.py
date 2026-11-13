"""Pre-device original instruction, bounded arithmetic, typed ABI and caller tests."""
import copy
from fractions import Fraction as F
import hashlib
import struct
import unittest
from unittest.mock import patch
import native_group04_projection_reference as m
from gpu_pipeline_proofs import STATE
from native_formula_observer import validate_case


def vector(s):return [m.fraction(s[f'xyz[{i}]']) for i in range(3)]
def rawvector(s):return [s[f'xyz[{i}]'] for i in range(3)]
def tparams(a=0,b=0):return [a,b]+m.NARROW_CONSTANTS['TgladTetra']

class OriginalProjectionSemantics(unittest.TestCase):
    def test_riemann_normalization_uses_radius_not_sqrt_radius_from_prose(self):
        # r=2, scale1 normalizesx to1, strict denominator0 selects fallback.
        trace=[];out=m._point('Riemann2',m.xyz_state((2,0,0)),m.riemann_params(),trace)
        self.assertEqual(vector(out),[-2,0,0]);self.assertFalse(trace[0]['reciprocal'])
        # Nonzero transverse coordinate makes the two normalization rules
        # observably different: corrects=1 permits exactPI cancellation.
        out=m._point('Riemann2',m.xyz_state((0,0,2)),m.riemann_params(ssh=m.neg(m.NARROW_CONSTANTS['Riemann2'][0])))
        self.assertEqual(vector(out),[-4,0,0])

    def test_riemann_strict_threshold_equality_and_both_sides(self):
        for threshold,branch in ((F(1,4),True),(F(1,2),False),(F(3,4),False)):
            trace=[];m._point('Riemann2',m.xyz_state((2,0,0)),m.riemann_params(scale=m.fp32(F(1,2)),smallest=m.fp32(threshold)),trace)
            self.assertEqual(trace,[{'op':'riemann_denominator','reciprocal':branch}])

    def test_riemann_nonzero_projected_coordinate_has_proved_zero_sine(self):
        pi=m.NARROW_CONSTANTS['Riemann2'][0]
        for axis in (1,2):
            s=m.xyz_state([int(i==axis) for i in range(3)])
            params=m.riemann_params(ssh=m.neg(pi) if axis==2 else 0,tsh=m.neg(pi) if axis==1 else 0)
            self.assertEqual(vector(m._point('Riemann2',s,params)),[-1,0,0])
        with self.assertRaisesRegex(m.ScopeError,'sine'):
            m._point('Riemann2',m.xyz_state((0,0,1)),m.riemann_params())

    def test_riemann_fabs_before_fold_is_not_signed_sine_and_shift_is_x_only(self):
        a=m._point('Riemann2',m.xyz_state((1,0,0)),m.riemann_params(fold=m.ONE,xshift=m.fp32(2)))
        b=m._point('Riemann2',m.xyz_state((1,0,0)),m.riemann_params(fold=m.neg(m.ONE),xshift=m.fp32(2)))
        self.assertEqual(a,b)
        self.assertEqual(rawvector(a),[m.add(m.sub(m.ONE,m.div(m.fp32(2),m.fp32(3))),m.fp32(2)),m.div(m.fp32(2),m.fp32(3)),m.div(m.fp32(2),m.fp32(3))])

    def test_riemann_does_not_read_mutable_or_immutable_j(self):
        s=m.xyz_state((1,0,0));a=m._point('Riemann2',s,m.riemann_params())
        for lane in ('j','c','original_j'):
            for i in range(3):s[f'{lane}[{i}]']=m.fp32(15+i)
        b=m._point('Riemann2',s,m.riemann_params());self.assertEqual(rawvector(a),rawvector(b))

    def test_tglad_zero_counts_execute_one_round_and_high_bits_are_masked(self):
        s=m.xyz_state((1,0,0))
        for a,b in ((0,0),(8,9),(16777217,16777216),(-2147483648,-2147483647)):
            trace=[];actual=m._point('TgladTetra',s,tparams(a,b),trace)
            self.assertEqual(actual,m._point('TgladTetra',s,tparams(1,1)))
            self.assertEqual(sum(x['op']=='tglad_outer' for x in trace),1)
            self.assertEqual(sum(x['op']=='tglad_map' for x in trace),1)
        for a,b in ((16777219,1),(1,16777219),(-2147483646,1),(1,-2147483646)):
            self.assertEqual(m._point('TgladTetra',s,tparams(a,b)),m._point('TgladTetra',s,tparams(a&7,b&7)))
            bad=[int(m.fraction(m.fp32(v))) for v in (a,b)]
            self.assertNotEqual(rawvector(m._point('TgladTetra',s,tparams(a,b))),rawvector(m._point('TgladTetra',s,tparams(*bad))))
        self.assertNotEqual(rawvector(m._point('TgladTetra',s,tparams(2,1))),rawvector(m._point('TgladTetra',s,tparams(1,2))))
        trace=[];m._point('TgladTetra',s,tparams(2147483647,1),trace)
        self.assertEqual(sum(x['op']=='tglad_outer' for x in trace),7)

    def test_tglad_radius_guard_includes_equality_and_skips_j(self):
        for x in (100000,100001,-100000):
            s=m.xyz_state((x,0,0));trace=[];out=m._point('TgladTetra',s,tparams(7,7),trace)
            self.assertEqual(rawvector(out),[m.NARROW_CONSTANTS['TgladTetra'][-1]]*3)
            self.assertEqual(trace[-1]['op'],'tglad_radius_guard')
        s=m.xyz_state((99999,0,0));trace=[]
        # Only first iteration's radius guard is tested below boundary;
        # subsequent mapping can have other guards, not assumed absent.
        m._point('TgladTetra',s,tparams(),trace)
        self.assertLess(m.fraction(int(trace[0]['radius2'],16)),F(10**10))

    def test_tglad_S_guard_is_positive_comparison_not_abs_and_skips_j(self):
        positive=m.xyz_state((1,0,-2048));negative=m.xyz_state((-1,0,-2048));trace=[]
        a=m._point('TgladTetra',positive,tparams(),trace);self.assertEqual(trace[-1]['op'],'tglad_S_guard')
        self.assertEqual(rawvector(a),[m.fp32(10**10)]*3)
        trace=[];b=m._point('TgladTetra',negative,tparams(),trace)
        self.assertFalse(any(x['op']=='tglad_S_guard' for x in trace));self.assertNotEqual(rawvector(a),rawvector(b))

    def test_tglad_mutable_j_is_added_after_projection_only(self):
        s=m.xyz_state((1,0,0))
        for i in range(3):s[f'j[{i}]']=0
        a=m._point('TgladTetra',s,tparams())
        for i,v in enumerate((4,-2,8)):s[f'j[{i}]']=m.fp32(v)
        b=m._point('TgladTetra',s,tparams())
        self.assertEqual(rawvector(b),[m.add(v,m.fp32(j)) for v,j in zip(rawvector(a),(4,-2,8))])

    def test_point_preserves_every_other_state_word(self):
        for name,params in [('Riemann2',m.riemann_params()),('TgladTetra',tparams())]:
            s=m.xyz_state((1,0,0));o=m._point(name,s,params)
            for lane in set(STATE)-{'xyz[0]','xyz[1]','xyz[2]','current_slot'}:self.assertEqual(o[lane],s[lane])

    def test_original_3d_caller_resets_w_zero_and_initializes_j(self):
        for name,params in [('Riemann2',m.riemann_params()),('TgladTetra',tparams())]:
            for julia in (0,1):
                s=m.xyz_state((1,0,0),'c');s['julia']=julia;s['max_iterations']=1
                out=m._orbit(name,s,params);self.assertEqual(out['xyz[3]'],0);self.assertEqual(out['first'],0);self.assertEqual(out['iteration'],1)
                self.assertEqual([out[f'j[{i}]'] for i in range(3)],[s[f'{"original_j" if julia else "c"}[{i}]'] for i in range(3)])
                for lane in ('derivative[0]','derivative[1]','derivative[2]','j[3]'):self.assertEqual(out[lane],s[lane])

    def test_riemann_orbit_strict_escape_and_repeat_counter(self):
        s=m.xyz_state((1,0,0),'c');s['max_iterations']=4;s['rstop']=m.ONE;s['hybrid_counts[0]']=3
        out=m._orbit('Riemann2',s,m.riemann_params());self.assertEqual((out['iteration'],out['tmp']),(4,2))
        s['rstop']=m.fp32(F(1,2));self.assertEqual(m._orbit('Riemann2',s,m.riemann_params())['iteration'],1)

class IndependentArithmeticAndABI(unittest.TestCase):
    def test_sqrt_correct_rounding_from_rational_midpoints(self):
        for v,e in ((0,0),(1,1),(4,2),(F(1,4),F(1,2)),(25,5)):
            self.assertEqual(m.sqrt_rn(m.fp32(v)),m.fp32(e))
        self.assertEqual(m.sqrt_rn(m.NEG_ZERO),m.NEG_ZERO)
        self.assertEqual(m.sqrt_rn(m.fp32(2)),0x3fb504f3)
        # The returned cell must contain the exact root, expressed without sqrt.
        for v in (F(3),F(5),F(17,4),F(1,3)):
            a=m.fp32(v);b=m.sqrt_rn(a);lo=(m.fraction(b-1)+m.fraction(b))/2;hi=(m.fraction(b)+m.fraction(b+1))/2
            self.assertLess(lo*lo,m.fraction(a));self.assertGreater(hi*hi,m.fraction(a))

    def test_exact_power_rejects_unproven_transcendental_inputs(self):
        self.assertEqual(m.exact_power(m.fp32(4),m.fp32(F(1,2))),m.fp32(2))
        self.assertEqual(m.exact_power(m.fp32(2),m.fp32(-3)),m.fp32(F(1,8)))
        self.assertEqual(m.exact_power(m.fp32(3),0),m.ONE)
        with self.assertRaises(m.ScopeError):m.exact_power(m.fp32(3),m.ONE)
        with self.assertRaises(m.ScopeError):m.exact_power(m.fp32(2),m.fp32(F(1,2)))
        with self.assertRaises(m.ScopeError):m.zero_sine(0,F(1,1<<50))

    def test_typed_integer_metadata_keeps_bits_lost_by_float(self):
        s=m.xyz_state((1,0,0));params=tparams(16777217,-2147483647);p=m._program('TgladTetra',params,s)
        self.assertEqual(p[68],m.fp32(16777216));self.assertEqual(m._parameters('TgladTetra',p)[:2],params[:2])
        raw=struct.pack('<64I',*p[260:324]);self.assertEqual(struct.unpack_from('<i',raw,244)[0],16777217);self.assertEqual(struct.unpack_from('<i',raw,240)[0],-2147483647)
        for i,v in enumerate(params[:2]):self.assertEqual(m.tail.fraction64(p[132+2*i]|p[133+2*i]<<32),v)

    def test_original_constants_and_packed_single_aliases(self):
        s=m.xyz_state((1,0,0));params=m.riemann_params();p=m._program('Riemann2',params,s)
        self.assertEqual(p[56+4],19);self.assertEqual(p[1],0);self.assertEqual(p[6],0)
        raw=struct.pack('<64I',*p[260:324])
        for i in range(9):self.assertEqual(struct.unpack_from('<I',raw,244-4*i)[0],params[i])
        for name,params in [('Riemann2',params),('TgladTetra',tparams())]:
            p=m._program(name,params,s);ci=0
            for i,(offset,kind) in enumerate(m.LAYOUT[name]):
                if kind=='constant':
                    self.assertEqual(p[132+2*i]|p[133+2*i]<<32,m.CONSTANTS[name][ci]);self.assertEqual(p[356+offset//8],m.NARROW_CONSTANTS[name][ci]);ci+=1

    def test_singular_nonfinite_and_subnormal_domains_reject(self):
        for xyz in ((0,0,0),(0,1,0),(0,0,1)):
            with self.assertRaises(m.ScopeError):m._point('TgladTetra',m.xyz_state(xyz),tparams())
        with self.assertRaises(m.ScopeError):m._point('Riemann2',m.xyz_state((0,0,0)),m.riemann_params())
        with self.assertRaises((m.ScopeError,ValueError)):m.finite(0x7f800000)
        with self.assertRaises(m.ScopeError):m.finite(1)

    def test_predeclared_corpus_is_complete_bound_and_non_sentinel_only(self):
        cases=m.build_cases();self.assertEqual(len(cases),82);self.assertEqual(len({x['id'] for x in cases}),82)
        self.assertEqual(sum(x['input']['kind']=='orbit' for x in cases),10)
        self.assertEqual({x['identity'][1] for x in cases},{10263,10269})
        for row in cases:
            validate_case(row['input']);self.assertEqual(row['expected_state_bits'],m.evaluate(row['input']));self.assertEqual(len(row['expected_state_bits']),47)
        interior=[x for x in cases if x['identity'][1]==10269 and 'interior_' in x['id']]
        self.assertEqual(len(interior),8)
        self.assertTrue(all(x['expected_state_bits']['xyz[0]']!=m.word(m.fp32(10**10)) for x in interior))

    def test_evaluate_rejects_raw_alias_descriptor_and_constant_drift(self):
        row=m.build_cases()[0];original=row['input']
        for lane in (56+4,260+61,356,132):
            payload=copy.deepcopy(original);payload['parameter_bits'][lane]=m.word(int(payload['parameter_bits'][lane],16)^1)
            with self.assertRaises(ValueError):m.evaluate(payload)
        payload=copy.deepcopy(original);payload['input_state_bits']['inside']='0x00000001'
        with self.assertRaises(m.ScopeError):m.evaluate(payload)

    def test_manifest_pins_originals_and_frozen_primitive_closure(self):
        manifest=m.manifest();self.assertEqual(len(manifest['semantic_sources']),7);self.assertEqual(len(manifest['dependencies']),3)
        self.assertEqual(manifest['model_sha256'],hashlib.sha256(m.Path(m.__file__).read_bytes()).hexdigest())
        for name,audit in m.CODE_AUDIT.items():self.assertTrue(audit['instructions']);self.assertGreater(audit['bytes'],300)



class CallerSSEProfileV2(unittest.TestCase):
    def state(self, values, maximum=3, stop=64):
        s=m._state()
        for i,v in enumerate(values):s[f'c[{i}]']=m.fp32(v)
        s['max_iterations']=maximum;s['rstop']=m.fp32(stop)
        return s

    def test_max_only_bailout_only_and_simultaneous_exit(self):
        for maximum,stop,expected_iterations,expected_low in ((1,64,1,1),(3,64,3,1),(4,F(1,2),1,F(1,2)),(1,F(1,2),1,1)):
            state=self.state((1,0,0),maximum,stop)
            out=m._orbit('Riemann2',state,m.riemann_params())
            self.assertEqual(out['iteration'],expected_iterations)
            self.assertEqual([out[f'sse[{i}]'] for i in range(4)],
                             [m.fp32(expected_low),0,m.fp32(1),0])

    def test_first_callback_and_continuation_transport(self):
        original=m._point;calls=[]
        def capture(name,state,params,*args,**kwargs):
            calls.append(dict(state));return original(name,state,params,*args,**kwargs)
        with patch.object(m,'_point',side_effect=capture):
            m._orbit('TgladTetra',self.state((4,0,3),1),tparams())
        self.assertEqual([calls[0][f'sse[{i}]'] for i in range(4)],
                         [m.fp32(16),0,m.fp32(25),0])
        calls.clear()
        with patch.object(m,'_point',side_effect=capture):
            m._orbit('Riemann2',self.state((1,0,0),3),m.riemann_params())
        self.assertEqual(len(calls),3)
        for incoming in calls[1:]:
            self.assertEqual([incoming[f'sse[{i}]'] for i in range(4)],
                             [m.fp32(64),0,m.fp32(1),0])

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
