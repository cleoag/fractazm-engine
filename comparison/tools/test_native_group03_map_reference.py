import copy
from fractions import Fraction as F
from pathlib import Path
import struct
import tempfile
import unittest
import zlib
from unittest.mock import patch

import native_group03_map_reference as m


class MapReferenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):cls.cases=m.build_cases()

    def test_corpus_predeclared_and_nonempty(self):
        self.assertEqual(len(self.cases),31)
        self.assertEqual(sum(c['input']['kind']=='orbit' for c in self.cases),12)
        self.assertEqual(len({c['id'] for c in self.cases}),31)
        for case in self.cases:
            self.assertEqual(m.evaluate(case['input']),case['expected_state_bits'])
            self.assertEqual(len(case['expected_state_bits']),47)
            if 'typed_id_16777216' not in case['id']:
                self.assertTrue(any(case['expected_state_bits'][f'xyz[{i}]']!=case['input']['input_state_bits'][f'xyz[{i}]'] for i in range(3)))

    def test_pins_and_primitive_lf_crlf(self):
        manifest=m.manifest();self.assertEqual(len(manifest['semantic_sources']),8)
        original=m.dependency_paths()[0].read_bytes().replace(b'\r\n',b'\n')
        self.assertNotEqual(m.primitive_identity(original),m.primitive_identity(original.replace(b'\n',b'\r\n')))
        with self.assertRaises(ValueError):m.primitive_identity(original+b'# change\n')

    def test_error_bound_is_derived_not_arbitrary(self):
        p=m.spline_error_proof()
        self.assertEqual(p['round_error'],F(1,1<<21))
        self.assertLess(p['sample_error'],F(1,512))
        self.assertGreater(p['sample_error'],p['row_error'])
        self.assertLess((p['sample_error']+p['round_error'])*2+p['round_error'],F(1,32))
        self.assertEqual(m.footprint_proof()['allowed_longitude_error'],F(1,64))

    def test_partition_of_unity_and_nonnegative(self):
        # Rational polynomial, not rounded native implementation.
        for a in range(65):
            x=F(a,64);c3=x*x*x/6;c0=F(1,6)-(1-x)*x/2-c3
            c2=x+c0-2*c3;c1=1-c0-c2-c3
            self.assertEqual(sum((c0,c1,c2,c3)),1)
            self.assertTrue(all(0<=v<=1 for v in (c0,c1,c2,c3)))

    def test_sse2_shuffle_is_rgb_not_bgr(self):
        # Symbolic lane routing from original ColToSVecSSE2. Each lane value
        # tags channel and pixel; exact integers isolate shuffles from math.
        b=[1,2,4,8];g=[16,32,64,128];r=[256,512,1024,2048]
        xmm4=r[2:]+b[:2]             # MOVLHPS then MOVHLPS
        xmm3=r[:2]+b[2:]             # SHUFPS E4
        sums=[a+b for a,b in zip(xmm4,xmm3)]
        shuffled=[sums[1],sums[0],sums[3],sums[2]] # PSHUFD B1
        result=[a+b for a,b in zip(sums,shuffled)]
        self.assertEqual((result[0],sum(g),result[2]),(sum(r),sum(g),sum(b)))

    def test_resource_pixels_and_seams(self):
        resources=m.resources()
        self.assertEqual(resources['map_ids'],[1,2,3])
        self.assertEqual(resources['pixel_count'],3*65*33*3)
        self.assertEqual([r[4] for r in resources['descriptor_words']],[0,6435,12870])
        for file in resources['files']:
            pixels=m.png_pixels(m.ROOT/file['path'])
            self.assertEqual(pixels[:64],pixels[-64:])
            self.assertEqual(len(pixels),2048)
        import native_formula_resource_input as inputs
        inputs.validate(resources,m.ROOT)
        inputs.validate(m.resources(True),m.ROOT)

    def test_png_corruption_and_extra_stream_rejected(self):
        data=(m.ROOT/m.resources()['files'][0]['path']).read_bytes()
        with tempfile.TemporaryDirectory() as temp:
            path=Path(temp)/'x.png'
            damaged=bytearray(data);damaged[-5]^=1;path.write_bytes(damaged)
            with self.assertRaises(ValueError):m.png_pixels(path)
            path.write_bytes(data+b'junk')
            with self.assertRaises(ValueError):m.png_pixels(path)

    def test_typed_large_selector_does_not_round_via_single(self):
        cases=[c for c in self.cases if 'typed_id_' in c['id']]
        self.assertEqual(len(cases),2)
        a,b=[c['input'] for c in cases]
        self.assertEqual(a['parameter_bits'][68],b['parameter_bits'][68])
        self.assertNotEqual(a['parameter_bits'][132:134],b['parameter_bits'][132:134])
        for case,selector in zip(cases,(16777216,16777217)):
            p=list(map(lambda v:int(v,16),case['input']['parameter_bits']))
            self.assertEqual(struct.unpack_from('<i',struct.pack('<64I',*p[260:324]),244)[0],selector)
        self.assertNotEqual(cases[0]['expected_state_bits']['xyz[0]'],cases[1]['expected_state_bits']['xyz[0]'])

    def test_raw_typed_and_widened_metadata_tampering_rejected(self):
        for index in (68,69,132,134,321,324):
            payload=copy.deepcopy(self.cases[0]['input'])
            payload['parameter_bits'][index]=m.word(int(payload['parameter_bits'][index],16)^1)
            with self.subTest(index=index),self.assertRaises(ValueError):m.evaluate(payload)

    def test_nonempty_rgb_effect_and_inverse(self):
        a=self.cases[0];b=self.cases[8]
        deltas=[]
        for case in (a,b):
            deltas.append([m.fraction(int(case['expected_state_bits'][f'xyz[{i}]'],16))-m.fraction(int(case['input']['input_state_bits'][f'xyz[{i}]'],16)) for i in range(3)])
        self.assertEqual(deltas,[[0,1,0],[1,0,1]])

    def test_state_preservation_point(self):
        for case in self.cases:
            if case['input']['kind']=='point':
                for k in m.STATE:
                    if k not in {'xyz[0]','xyz[1]','xyz[2]','current_slot'}:
                        self.assertEqual(case['expected_state_bits'][k],case['input']['input_state_bits'][k])

    def test_orbit_strict_escape_and_repeat(self):
        for case in self.cases:
            if case['input']['kind']=='orbit':
                expected=2 if 'escape' in case['id'] else 4
                self.assertEqual(int(case['expected_state_bits']['iteration'],16),expected)
                self.assertEqual(case['expected_state_bits']['xyz[3]'],m.word(m.ONE))
                for i in range(3):
                    source='original_j' if '_j1_' in case['id'] else 'c'
                    self.assertEqual(case['expected_state_bits'][f'j[{i}]'],case['input']['input_state_bits'][f'{source}[{i}]'])

    def test_radius_refuses_unmodelled_multiple_products(self):
        s=m._state()
        with self.assertRaises(ValueError):m.radius(s)
        for i in range(3):s[f'xyz[{i}]']=0
        s['xyz[1]']=m.fp32((1<<20)+1)
        self.assertEqual(m.radius(s),m.fp32(F((1<<20)+1)**2))

    def test_boundary_inputs_and_small_rounding_cells_fail_closed(self):
        for value in (1,1<<19,0):
            payload=copy.deepcopy(self.cases[0]['input']);payload['input_state_bits']['xyz[0]']=m.word(m.fp32(value))
            with self.assertRaises(ValueError):m.evaluate(payload)

    def test_absent_resource_or_unbound_descriptor_rejected(self):
        for field in ('files','descriptor_words','map_ids'):
            payload=copy.deepcopy(self.cases[0]['input']);payload['resources'][field]=[]
            with self.subTest(field=field),self.assertRaises(ValueError):m.evaluate(payload)
        payload=copy.deepcopy(self.cases[0]['input']);del payload['resources']
        with self.assertRaises(ValueError):m.evaluate(payload)

    def test_arbitrary_nonfinite_state_rejected(self):
        for value in (0x7f800000,0xff800000,0x7fc00000,1):
            payload=copy.deepcopy(self.cases[0]['input']);payload['input_state_bits']['j[0]']=m.word(value)
            with self.subTest(value=value),self.assertRaises(ValueError):m.evaluate(payload)


if __name__=='__main__':unittest.main()
