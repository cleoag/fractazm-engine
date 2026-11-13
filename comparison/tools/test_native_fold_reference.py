"""Independent semantic/ABI checks only; no probe, device or compiled renderer."""
import copy
from fractions import Fraction
import hashlib
import itertools
import struct
import unittest

import native_fold_reference as f
from gpu_pipeline_proofs import STATE
from native_formula_observer import validate_case


class FoldSemantics(unittest.TestCase):
    def test_octahedral_original_store_order(self):
        # Original CODE stores ST0 toY, ST1 toX after the final exchange.
        for values in itertools.permutations((-2, 3, -1)):
            s=f._state()
            s.update({f'xyz[{i}]':f.fp32(v) for i,v in enumerate(values)})
            out=f._point('_FoldingOct',s,[])
            self.assertEqual([out[f'xyz[{i}]'] for i in range(3)], [0x40400000,0x40000000,0x3f800000])
            self.assertEqual(out['xyz[3]'],s['xyz[3]'])
            self.assertEqual(out['j[0]'],s['j[0]'])
        s=f._state();s.update({'xyz[0]':0x80000000,'xyz[1]':0,'xyz[2]':0x80000000})
        out=f._point('_FoldingOct',s,[])
        self.assertEqual([out[f'xyz[{i}]'] for i in range(3)],[0,0,0])

    def test_absolute_fold_threshold_neighbors_and_rounding(self):
        # Independently calculated exact words around the fold atY=-1.
        for y,expected in [(0xbf800000,0xbf800000),(0xbf800001,0xbf7ffffe),(0xbf7fffff,0xbf7fffff)]:
            s=f._state();s['xyz[1]']=y
            self.assertEqual(f._point('_AbsY',s,[0x3f800000])['xyz[1]'],expected)
        s=f._state();s['xyz[1]']=0x3f800000
        self.assertEqual(f._point('_AbsY',s,[0x33800000])['xyz[1]'],0x3f7fffff)
        s['xyz[1]']=0x80000000
        self.assertEqual(f._point('_AbsY',s,[0])['xyz[1]'],0)

    def test_flip_c_is_conditional_and_an_involution(self):
        for n in (0,1,-1,-2147483648,2147483647):
            s=f._state();s['xyz[0]']=0x80000000;s['xyz[1]']=0
            out=f._point('_FlipXYc',s,[n])
            self.assertEqual((out['xyz[0]'],out['xyz[1]']),(0,0x80000000))
            expected=(s['j[1]'],s['j[0]']) if n else (s['j[0]'],s['j[1]'])
            self.assertEqual((out['j[0]'],out['j[1]']),expected)
            twice=f._point('_FlipXYc',out,[n]);s['current_slot']=0
            self.assertEqual(twice,s)
            self.assertEqual(out['c[0]'],s['c[0]'])

    def test_recfold_exact_zero_enters_original_ftst_branch(self):
        s=f._state();s['xyz[0]']=s['xyz[2]']=0
        p=[f.fp32(-1),0,0,0,1,0x3f000000,f.narrow64(f.CONSTANTS[8])]
        skipped=f._point('_recFold',s,p)
        self.assertEqual((skipped['xyz[0]'],skipped['xyz[2]']),(0x80000000,0x80000000))
        p[4]=2;entered=f._point('_recFold',s,p)
        self.assertEqual((entered['xyz[0]'],entered['xyz[2]']),(0x40000000,0x80000000))
        p[3]=f.fp32(4);negative=f._point('_recFold',s,p)
        self.assertEqual((negative['xyz[0]'],negative['xyz[2]']),(0xc0000000,0xc0000000))

    def test_recfold_count_domain_and_first_loop_value(self):
        s=f._state();p=[f.fp32(1),f.fp32(-3),f.fp32(10),f.fp32(1),1,0x3f000000,f.narrow64(f.CONSTANTS[8])]
        # Hand-traced phases: u=3.25,v=-9; X=5.625,Z=2.375.
        for n in (-2147483648,-1,0,1,1001,16777217,2147483647):
            p[4]=n;out=f._point('_recFold',s,p)
            self.assertEqual(f.fraction(out['xyz[0]']),Fraction(45,8))
            self.assertEqual(f.fraction(out['xyz[2]']),Fraction(19,8))
        p[4]=2
        self.assertEqual(f.fraction(f._point('_recFold',s,p)['xyz[0]']),Fraction(29,8))
        for n in (999,1000):
            p[4]=n
            self.assertEqual(f.fraction(f._point('_recFold',s,p)['xyz[0]']),Fraction(3,8))


class TypedTransport(unittest.TestCase):
    def test_flip_integer_preserved_beyond_binary32_integer_precision(self):
        p=f._program('_FlipXYc',[16777217],f._state());base=56
        self.assertEqual(p[base+12],0x4b800000)  # rounded16777216 transport float
        self.assertEqual(p[base+76] | p[base+77]<<32,0x4170000010000000)
        self.assertEqual(p[base+204+61],0x01000001)  # exact int32 atPVar-12
        self.assertEqual(f._parameters('_FlipXYc',p),[16777217])

    def test_recfold_integer_offsets_and_original_double_constants(self):
        params=[f.fp32(1),f.fp32(-3),f.fp32(10),f.fp32(1),2147483647,0x3f000000,f.narrow64(f.CONSTANTS[8])]
        p=f._program('_recFold',params,f._state());base=56
        self.assertEqual(p[base+12+4],0x4f000000)
        self.assertEqual(p[base+76+8] | p[base+77+8]<<32,0x41dfffffffc00000)
        self.assertEqual(p[base+204+53],0x7fffffff)
        self.assertEqual(p[base+204+54] | p[base+204+55]<<32,0x3ff0000000000000)
        self.assertEqual(p[base+76+12] | p[base+77+12]<<32,0x3fe6a09e667f3bcd)
        self.assertEqual(p[base+300:base+302],[0x3f000000,0x3f3504f3])
        self.assertEqual(len(struct.pack('<2048I',*p)),8192)

    def test_integer_or_constant_corruption_rejected(self):
        row=next(c for c in f.build_cases() if c['id']=='7004-integer_16777217')
        for index in (56+204+61,56+76):
            altered=copy.deepcopy(row['input']);altered['parameter_bits'][index]='0x00000000'
            with self.assertRaises(ValueError):f.evaluate(altered)
        p=[f.fp32(1),0,0,0,2,0x3f000000,0]
        with self.assertRaisesRegex(ValueError,'constant'):f._program('_recFold',p,f._state())
        with self.assertRaisesRegex(ValueError,'int32'):f._program('_FlipXYc',[2147483648],f._state())


class CorpusAndCaller(unittest.TestCase):
    @classmethod
    def setUpClass(cls): cls.cases=f.build_cases()

    def test_closed_corpus_observer_schema_and_state_preservation(self):
        self.assertEqual(len(self.cases),61)
        self.assertEqual(len({c['id'] for c in self.cases}),61)
        registry={('external',desc[0],name) for name,desc in f.FORMULAS.items()}
        for c in self.cases:
            validate_case(c['input'],registry)
            self.assertEqual(c['expected_state_bits'],f.evaluate(c['input']))
            self.assertEqual(set(c['expected_state_bits']),set(STATE))
            for lane in set(STATE)-set(c['write_lanes']):
                self.assertEqual(c['expected_state_bits'][lane],c['input']['input_state_bits'][lane],(c['id'],lane))
        for name in f.FORMULAS:
            self.assertEqual({c['input']['kind'] for c in self.cases if c['identity'][2]==name},{'point','orbit'})

    def test_pascal_caller_julia_initialization_once_and_strict_escape(self):
        for variant in ('orbit_mandel','orbit_julia','orbit_repeat'):
            c=next(c for c in self.cases if c['id']==f'7004-{variant}')
            out=c['expected_state_bits'];source=c['input']['input_state_bits']
            self.assertEqual(out['iteration'],'0x00000004')
            # Four swaps restoreJ; it was initialized fromC orJu atentry only.
            origin='original_j' if variant!='orbit_mandel' else 'c'
            self.assertEqual(out['j[0]'],source[f'{origin}[0]'])
            self.assertEqual(out['xyz[3]'],'0x3f800000')
        c=next(c for c in self.cases if c['id']=='7004-orbit_escape')
        self.assertEqual(c['expected_state_bits']['iteration'],'0x00000001')

    def test_out_of_domain_program_cannot_be_relabeled(self):
        source=self.cases[0]['input']
        for index in (1,65,56+6):
            altered=copy.deepcopy(source);altered['parameter_bits'][index]='0x00000000'
            with self.assertRaises(ValueError):f.evaluate(altered)
        altered=copy.deepcopy(source);altered['input_state_bits']['xyz[0]']='0x7f800000'
        with self.assertRaises(ValueError):f.evaluate(altered)

    def test_primitive_line_endings_preserve_semantic_pin_but_bind_actual_bytes(self):
        lf=f.dependency_paths()[0].read_bytes().replace(b'\r\n',b'\n')
        crlf=lf.replace(b'\n',b'\r\n')
        self.assertEqual(f.primitive_identity(lf),hashlib.sha256(lf).hexdigest())
        self.assertEqual(f.primitive_identity(crlf),hashlib.sha256(crlf).hexdigest())
        self.assertNotEqual(f.primitive_identity(lf),f.primitive_identity(crlf))
        changed=lf.replace(b'def add(',b'def changed_add(',1)
        self.assertNotEqual(changed,lf)
        for data in (changed,changed.replace(b'\n',b'\r\n')):
            with self.assertRaisesRegex(ValueError,'primitive dependency changed'):
                f.primitive_identity(data)

    def test_pinned_semantic_provenance_and_primitive_dependency(self):
        manifest=f.manifest()
        self.assertEqual(len(manifest['code_audit']),4)
        for path,expected in f.SOURCE_PINS.items():
            data=(f.ROOT/path).read_bytes().replace(b'\r\n',b'\n')
            self.assertEqual(hashlib.sha256(data).hexdigest(),expected)
        self.assertEqual(f.dependency_paths(),[f.Path(f.affine.__file__).resolve()])
        self.assertEqual(manifest['dependencies'][str(f.dependency_paths()[0].relative_to(f.ROOT))],
                         hashlib.sha256(f.dependency_paths()[0].read_bytes()).hexdigest())


if __name__=='__main__':unittest.main()
