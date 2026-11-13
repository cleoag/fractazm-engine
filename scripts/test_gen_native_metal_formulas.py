import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

SPEC=importlib.util.spec_from_file_location("gen_native_metal_formulas",Path(__file__).with_name("gen_native_metal_formulas.py"))
g=importlib.util.module_from_spec(SPEC);sys.modules[SPEC.name]=g;SPEC.loader.exec_module(g)


class NativeMetalGenerationTests(unittest.TestCase):
    def test_arm_branch_selection_is_preprocessing(self):
        source='''#ifdef _MSC_VER
WRONG_MSVC
#elif defined(MB3D_SOFTWARE_EXT80)
#if defined(__aarch64__)
ARM_SELECTED
#else
WRONG_ARCH
#endif
#else
WRONG_FALLBACK
#endif
'''
        self.assertEqual(g.tokens(g.select_arm(source,"/usr/bin/clang")),["ARM_SELECTED"])

    def test_numeric_lowering_preserves_integer_payloads_and_variable_e(self):
        result=g.lower(g.tokens('double E=2.5; double x=E*E; uint64_t bits=UINT64_C(0xfff0000000000000);'))
        text=" ".join(result)
        self.assertIn('float E = 2.5f',text)
        self.assertIn('E * E',text)
        self.assertNotIn('thread E',text)
        self.assertIn('0xfff0000000000000UL',text)

    def test_canonical_norm_is_scaled_with_only_identifier_squares(self):
        for source in ('sqrt(x*x+y*y+z*z)','std::sqrt((x*x+y*y)+z*z)','sqrt(x*x+(y*y+z*z))'):
            self.assertEqual(g.lower(g.tokens(source)),g.tokens('native_formula_length3(x,y,z)'))

    def test_noncanonical_norm_and_side_effects_are_not_rewritten(self):
        for source in ('sqrt(x*x+y*y+z*w)','sqrt(x*x+y*y+z*z+1)',
                       'sqrt(f()*f()+y*y+z*z)','sqrt(a[0]*a[0]+y*y+z*z)',
                       'sqrt(p.x*p.x+y*y+z*z)','sqrt(x*x+y*y)',
                       'other::sqrt(x*x+y*y+z*z)','p.sqrt(x*x+y*y+z*z)'):
            self.assertNotIn('native_formula_length3',g.lower(g.tokens(source)))

    def test_scaled_norm_large_finite_reference_vectors(self):
        import math,struct
        def f(x):
            try:return struct.unpack('<f',struct.pack('<f',x))[0]
            except OverflowError:return math.copysign(math.inf,x)
        for vector in ((1e20,2e20,3e20),(1e30,0,0),(1e-30,2e-30,3e-30),(0,0,0)):
            v=tuple(map(f,vector));m=max(map(abs,v))
            if m:
                a,b,c=(f(x/m) for x in v)
                result=f(m*f(math.sqrt(f(f(f(a*a)+f(b*b))+f(c*c)))))
            else:result=0.0
            self.assertTrue(math.isfinite(result))
            self.assertAlmostEqual(result/math.hypot(*v) if m else 1.0,1.0,places=6)
        self.assertTrue(math.isinf(f(f(1e20)*f(1e20))))

    def test_closure_captures_direct_and_comma_declarations_only(self):
        source='void earlier(float y,float z){} float f(float p){using E=float;const E x(p),y(p),power(2);auto run=[&](E a){const E y(a),z(a);return y+z+power;};return run(x);}'
        text=" ".join(g.lower_lambdas(g.tokens(source)))
        self.assertIn('decltype ( power ) & power',text)
        self.assertNotIn('decltype ( y )',text)
        self.assertNotIn('decltype ( z )',text)
        self.assertNotIn('[ & ]',text)

    def test_explicit_value_capture_is_value_field(self):
        text=" ".join(g.lower_lambdas(g.tokens('float f(float fold){auto run=[fold](float x){return x+fold;};return run(1);}')))
        self.assertIn('decltype ( fold ) fold',text)
        self.assertNotIn('decltype ( fold ) &',text)

    def test_original_binary64_parameter_words_not_reencoded(self):
        text=" ".join(g.lower_context(g.tokens('native_std::memcpy(&bits,&params[4],sizeof(bits));auto hi=native_std::bit_cast<ulong>(params[12])>>32;')))
        self.assertIn('bits = params . bits ( 4 )',text)
        self.assertIn('params . bits ( 12 ) >> 32',text)

    def test_aboxmod1_scale_is_checked_only_in_the_inner_branch(self):
        from types import SimpleNamespace
        source='const float ScaleOverMinR2=params[1]; const float minR2=params[2]; if(rr<minR2){m=ScaleOverMinR2;}'
        result=g.lower_aboxmod1_inner_scale(g.tokens(source),SimpleNamespace(id=8024))
        self.assertEqual(result,g.tokens('if(native_aboxmod1_inner(rr,params)){m=native_aboxmod1_inner_scale(params);}'))
        self.assertEqual(g.lower_aboxmod1_inner_scale(g.tokens(source),SimpleNamespace(id=8025)),g.tokens(source))
        with self.assertRaisesRegex(ValueError,'source contract changed'):
            g.lower_aboxmod1_inner_scale(g.tokens(source.replace('rr<minR2','rr<=minR2')),SimpleNamespace(id=8024))

    def test_unordered_box_scale_is_checked_at_its_original_branch(self):
        from types import SimpleNamespace
        for id,source,expected in [
            (1509,'!(rr>=E(params[2])) ? E(params[1]) : E(vary)',
             'native_box_inner_unordered(float(rr),params) ? E(native_aboxmod1_inner_scale(params)) : E(vary)'),
            (10324,'!(rr>=params[2]) ? params[1] : params[0]/rr',
             'native_box_inner_unordered(rr,params) ? native_aboxmod1_inner_scale(params) : params[0]/rr')]:
            self.assertEqual(g.lower_aboxmod1_inner_scale(g.tokens(source),SimpleNamespace(id=id)),g.tokens(expected))
            self.assertEqual(g.lower_aboxmod1_inner_scale(g.tokens(source),SimpleNamespace(id=8900)),g.tokens(source))
            with self.assertRaisesRegex(ValueError,'source contract changed'):
                g.lower_aboxmod1_inner_scale(g.tokens(source.replace('>=','>')),SimpleNamespace(id=id))

    def test_work_exhaustion_returns_and_records_context(self):
        text=" ".join(g.lower_context(g.tokens('FormulaWorkBudget work("x");while(true){work.beforeStep();}')))
        self.assertIn('FormulaWorkBudget work ( params ,',text)
        self.assertIn('if ( ! work . beforeStep ( ) ) return NAN ;',text)

    def test_counter_helper_uses_exact_integer_sidechannel(self):
        source='inline int incrementFormulaCounter(thread float* params){const uint bits=uint(int(params[BFIRSTIT_SLOT]))+1u;int count;native_std::memcpy(&count,&bits,sizeof(count));params[BFIRSTIT_SLOT]=float(count);return count;}'
        text=" ".join(g.lower_context(g.tokens(source)))
        self.assertIn('return native_increment_formula_counter ( params )',text)
        self.assertNotIn('int ( params [ BFIRSTIT_SLOT ] )',text)
        self.assertNotIn('memcpy',text)

    def test_counter_boundary_vectors_discriminate_fp32_scratch(self):
        import struct
        vectors=[(16777216,16777217),(2147483647,-2147483648),(-2147483648,-2147483647),(-1,0)]
        for before,expected in vectors:
            word=((before&0xffffffff)+1)&0xffffffff
            result=struct.unpack('<i',struct.pack('<I',word))[0]
            self.assertEqual(result,expected)
        rounded=struct.unpack('<f',struct.pack('<f',16777217))[0]
        self.assertNotEqual(int(rounded),16777217)

    def test_named_counter_reads_and_writes_do_not_use_fp32_scratch(self):
        source='int first=int(params[BFIRSTIT_SLOT]);params[BFIRSTIT_SLOT]+=1.0f;params[BFIRSTIT_SLOT]=first;'
        text=' '.join(g.lower_context(g.tokens(source)))
        self.assertIn('int ( native_formula_counter ( params ) )',text)
        self.assertIn('native_increment_formula_counter ( params ) ;',text)
        self.assertIn('native_formula_counter ( params ) = first',text)
        self.assertNotIn('params [ BFIRSTIT_SLOT ]',text)
        with self.assertRaises(ValueError):g.lower_context(g.tokens('params[BFIRSTIT_SLOT]+=f();'))

    def test_wrapper_preserves_untouched_counter_across_unrelated_formula(self):
        source=(g.ROOT/'src/render/native_metal/formula_wrapper.metal.h').read_text()
        self.assertIn('int first_counter=s.first',source)
        self.assertEqual(source.count('s.first=first_counter;'),3)
        self.assertNotIn('native_integer(a[57]',source)
        # An unrelated slot must retain this exact odd integer; its numeric
        # scratch view rounds to the neighboring even integer in binary32.
        import struct
        expected=16777217
        self.assertNotEqual(struct.unpack('<f',struct.pack('<f',expected))[0],expected)

    def test_typed_seed_read_requires_declared_int_and_read_only_bounded_loop(self):
        entry=g.Entry(10320,'F','_TranslateRnd','Ads/_TranslateRnd.inc','Ads',-1,8,parameter_types=(0,0,0,2,2,2,0,0))
        source='for(int axis=0;axis<3;++axis){uint seed=static_cast<uint>(static_cast<int>(params[3+axis]));}'
        text=' '.join(g.lower_typed_seed_reads(g.tokens(source),entry))
        self.assertIn('native_int32_parameter ( params , 3 + axis )',text)
        for invalid in (source.replace('axis<3','axis<4'),source.replace('uint seed=','params[3+axis]=1;uint seed=')):
            with self.assertRaises(ValueError):g.lower_typed_seed_reads(g.tokens(invalid),entry)
        entry.parameter_types=(0,0,0,0,0,0,0,0)
        with self.assertRaises(ValueError):g.lower_typed_seed_reads(g.tokens(source),entry)
        ordinary=g.Entry(7037,'F','BristorBrot',None,'3D',0,3)
        self.assertEqual(g.lower_typed_seed_reads(g.tokens(source),ordinary),g.tokens(source))

    def test_typed_seed_registry_and_low_bits_are_discriminating(self):
        import struct
        entries={e.id:e for e in g.registry()}
        for formula in (10019,10320):self.assertEqual(entries[formula].parameter_types[3:6],(2,2,2))
        count=1;word=((count-0x3d07)&65535)**2&65535
        seeds=[16777216,16777217,2147483647,-2147483648,-1]
        for seed in seeds:
            bits=struct.unpack('<Q',struct.pack('<d',seed))[0]
            magnitude=bits&0x7fffffffffffffff
            exponent=(bits>>52)&2047;shift=1075-exponent
            significand=(1<<52)|(bits&0xfffffffffffff)
            decoded=(significand>>shift)*(-1 if bits>>63 else 1)
            self.assertEqual(decoded,seed)
        self.assertNotEqual(((seeds[0]&65535)*word)&16383,((seeds[1]&65535)*word)&16383)

    def test_map_callback_lowered_to_explicit_static_gpu_call(self):
        text=" ".join(g.lower_map_bindings(g.tokens('TLMSfunction map;map(&out,&in,4);auto v=cpu_map_sphere_sample(x,y,z,4);'),True))
        self.assertIn('native_formula_map ( params , & out , & in , 4 , true )',text)
        self.assertIn('native_formula_map_sphere ( params , x',text)
        self.assertNotIn('TLMSfunction',text)
        self.assertNotIn('cpu_map',text)

    def test_write_if_changed_preserves_timestamp(self):
        with tempfile.TemporaryDirectory() as name:
            p=Path(name)/"a";g.write_if_changed(p,"abc");before=p.stat().st_mtime_ns
            g.write_if_changed(p,"abc");self.assertEqual(before,p.stat().st_mtime_ns)
            g.write_if_changed(p,"def");self.assertEqual(p.read_text(),"def")

    def test_check_rejects_modified_missing_extra_without_rewriting(self):
        def generate(out,*_):
            out.mkdir(exist_ok=True);(out/"expected.h").write_text("expected")
            return {"status":"unqualified"}
        with tempfile.TemporaryDirectory() as name,patch.object(g,"generate",side_effect=generate):
            target=Path(name);(target/"expected.h").write_text("modified")
            with self.assertRaises(ValueError):g.check_generated(target,"unused")
            self.assertEqual((target/"expected.h").read_text(),"modified")
            (target/"expected.h").write_text("expected")
            self.assertEqual(g.check_generated(target,"unused")["check"],"CURRENT")
            (target/"extra.h").write_text("extra")
            with self.assertRaises(ValueError):g.check_generated(target,"unused")
            (target/"expected.h").unlink()
            with self.assertRaises(ValueError):g.check_generated(target,"unused")

    def test_full_ordinary_registry_has_no_source_emission_credit(self):
        rows=g.registry();self.assertEqual(len(rows),457)
        data=json.loads((g.ROOT/'src/render/native_metal/generated/inventory.json').read_text())
        self.assertEqual(data['device_qualified'],0)
        self.assertEqual(data['emitted_uncompiled']+data['driver_implemented_unqualified']+data['blocked'],457)
        self.assertEqual({r.id for r in rows},{r['id'] for r in data['formulas']})


class DirectCoordinateLoweringTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.entries = {e.id: e for e in g.registry() if e.id in (10001, 9525)}
        cls.selected, cls.lowered, cls.traits = {}, {}, {}
        for id, entry in cls.entries.items():
            selected = g.select_arm((g.FORMULAS / entry.source).read_text(), '/usr/bin/clang')
            values, traits = g.remove_metadata(g.tokens(selected), entry.symbol)
            cls.selected[id] = selected
            cls.lowered[id] = g.lower_context(g.lower(values))
            cls.traits[id] = traits

    def test_original_cpu_adapter_lowered_to_direct_coordinates(self):
        for id, entry in self.entries.items():
            result, direct = g.lower_direct_coordinate_result(self.lowered[id], entry, self.traits[id])
            self.assertTrue(direct)
            text = ' '.join(result)
            self.assertIn('WRITE_J1_SLOT', text)
            self.assertIn('WRITE_J4_SLOT', text)
            if id == 10001:
                self.assertIn('p . x + x_add , p . y + y_add , p . z + z_add', text)
                self.assertNotIn('- cx_out', text)
                self.assertIn('FORMULA_STATE_W_SLOT', text)
            else:
                self.assertNotIn('- FormulaVec3', text)
                self.assertNotIn('- params [ 5 ]', text)
                self.assertEqual(text.count('de = de * Scale ;'), 2)
                self.assertIn('params [ 5 ] *= C_Scale', text)

    def test_changed_or_missing_compensation_stops_lowering(self):
        mutations = [(10001, '-cx_out', '-cy_out'),
                     (10001, 'p.x+x_add-cx_out', 'p.x+x_add'),
                     (9525, 'de=de*Scale-params[5];', 'de=de*Scale-params[4];'),
                     (9525, 'p*Scale-FormulaVec3(params[2],params[3],params[4])', 'p*Scale')]
        for id, old, new in mutations:
            changed = g.replace_tokens(self.lowered[id], old, new)
            with self.subTest(id=id, old=old), self.assertRaisesRegex(ValueError, 'source contract changed'):
                g.lower_direct_coordinate_result(changed, self.entries[id], self.traits[id])
        # Duplicate compensations are equally unsafe: never rewrite a changed
        # source merely because at least one expected fragment still appears.
        changed = self.lowered[9525] + g.tokens('de=de*Scale-params[5];')
        with self.assertRaisesRegex(ValueError, 'source contract changed'):
            g.lower_direct_coordinate_result(changed, self.entries[9525], self.traits[9525])

    def test_changed_descriptor_or_traits_stops_lowering(self):
        from dataclasses import replace
        for id, entry in self.entries.items():
            for changes in ({'param_count': 7}, {'adds_c': 1}, {'de_option': 4},
                            {'difs_style': 1}, {'source': 'different.inc'}):
                with self.subTest(id=id, changes=changes), self.assertRaisesRegex(ValueError, 'descriptor contract changed'):
                    g.lower_direct_coordinate_result(self.lowered[id], replace(entry, **changes), self.traits[id])
            with self.assertRaisesRegex(ValueError, 'descriptor contract changed'):
                g.lower_direct_coordinate_result(self.lowered[id], entry, self.traits[id] ^ 1)

    def test_other_formulas_are_not_reassociated(self):
        from dataclasses import replace
        source = self.lowered[9525]
        result, direct = g.lower_direct_coordinate_result(source, replace(self.entries[9525], id=6003), 0)
        self.assertEqual(result, source)
        self.assertFalse(direct)

    def test_generation_fails_instead_of_silently_disabling_changed_body(self):
        changed = self.selected[9525].replace('de * Scale - params[5]', 'de * Scale + params[5]')
        self.assertNotEqual(changed, self.selected[9525])
        with tempfile.TemporaryDirectory() as temp, \
             patch.object(g, 'registry', return_value=[self.entries[9525]]), \
             patch.object(g, 'TABLES', []), patch.object(g, 'expand_local', return_value=changed), \
             patch.object(g, 'select_arm', return_value=changed):
            with self.assertRaisesRegex(ValueError, 'source contract changed'):
                g.generate(Path(temp), None, 'unused')
            self.assertFalse((Path(temp) / 'inventory.json').exists())

    def test_generated_direct_result_contract_is_closed_and_cpu_descriptors_unchanged(self):
        root = g.ROOT / 'src/render/native_metal/generated'
        data = json.loads((root / 'inventory.json').read_text())
        direct = {r['id']: r for r in data['formulas'] if r.get('result_contract') == 'direct_coordinates'}
        self.assertEqual(set(direct), {10001, 9525})
        for id, row in direct.items():
            entry = self.entries[id]
            self.assertEqual((row['adds_c'], row['param_count'], row['de_option'], row['traits']),
                             (entry.adds_c, entry.param_count, entry.de_option, self.traits[id]))
        source = (root / 'formula_result_contracts.metal.h').read_text()
        helper = source.split('inline bool native_external_returns_coordinates', 1)[1]
        import re
        helper_tokens = g.tokens(source)
        begin = helper_tokens.index('{', helper_tokens.index('native_external_returns_coordinates'))
        direct_body = ' '.join(helper_tokens[begin:g.matching(helper_tokens, begin)+1])
        self.assertEqual(set(map(int, re.findall(r'case (\d+)u :', direct_body))), {10001, 9525})
        self.assertIn('default: return false;', helper)


class ReciprocalSquareFirstTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.entries = {e.id: e for e in g.registry() if e.id in (4812, 5823)}
        cls.selected, cls.lowered, cls.traits = {}, {}, {}
        for ident, entry in cls.entries.items():
            selected = g.select_arm((g.FORMULAS / entry.source).read_text(), '/usr/bin/clang')
            values, traits = g.remove_metadata(g.tokens(selected), entry.symbol)
            cls.selected[ident] = selected
            cls.lowered[ident] = g.lower_context(g.lower(values))
            cls.traits[ident] = traits

    def test_real_bodies_change_only_the_second_product(self):
        old = g.tokens('params[5]*value*value')
        new = g.tokens('(value*value)*params[5]')
        for ident, entry in self.entries.items():
            original = self.lowered[ident]
            offset = next(i for i in range(len(original)) if original[i:i+len(old)] == old)
            result = g.lower_reciprocal_square_first(original, entry, self.traits[ident])
            self.assertEqual(result[:offset], original[:offset])
            self.assertEqual(result[offset:offset+len(new)], new)
            self.assertEqual(result[offset+len(new):], original[offset+len(old):])
            # The alternative entry delegates to this same specialization;
            # no policy-specific variant or contraction pragma is introduced.
            text = ' '.join(result)
            self.assertIn('return execute_formula < '+entry.symbol+' > ( p , params , de )', text)
            self.assertNotIn('pragma', result)

    def test_source_drift_and_duplicate_statement_fail_closed(self):
        old = 'result += params[3] - 1.0f / (abs(params[5]*value*value)+params[2]);'
        for ident, entry in self.entries.items():
            source = self.lowered[ident]
            for changed in (g.replace_tokens(source,'params[5]*value*value','params[4]*value*value'),
                            g.replace_tokens(source,old,''), source+g.tokens(old),
                            g.replace_tokens(source,'abs(params[5]*value*value)','abs(value*value*params[5])')):
                with self.assertRaisesRegex(ValueError, 'source contract changed'):
                    g.lower_reciprocal_square_first(changed, entry, self.traits[ident])

    def test_every_pinned_descriptor_field_is_enforced(self):
        from dataclasses import replace
        mutations = ({'symbol':'OTHER'}, {'name':'other'}, {'source':'other.inc'},
                     {'family':'other'}, {'de_option':0}, {'param_count':5},
                     {'adds_c':0}, {'difs_style':1}, {'parameter_types':(1,0,0,0,0,0)})
        for ident, entry in self.entries.items():
            for changed in mutations:
                with self.subTest(ident=ident, changed=changed), self.assertRaisesRegex(ValueError, 'descriptor contract changed'):
                    g.lower_reciprocal_square_first(self.lowered[ident], replace(entry, **changed), self.traits[ident])
            with self.assertRaisesRegex(ValueError, 'descriptor contract changed'):
                g.lower_reciprocal_square_first(self.lowered[ident], entry, self.traits[ident]^1)

    def test_neighbors_with_same_expression_are_untouched(self):
        from dataclasses import replace
        for ident, entry in self.entries.items():
            source = self.lowered[ident]
            self.assertIs(g.lower_reciprocal_square_first(source, replace(entry,id=7007), 0), source)

    def test_generation_stops_instead_of_leaving_a_stale_body(self):
        for ident, entry in self.entries.items():
            changed = self.selected[ident].replace('params[5] * value * value', 'params[5] * value * other')
            self.assertNotEqual(changed, self.selected[ident])
            with tempfile.TemporaryDirectory() as temp, \
                 patch.object(g, 'registry', return_value=[entry]), \
                 patch.object(g, 'TABLES', []), patch.object(g, 'expand_local', return_value=changed), \
                 patch.object(g, 'select_arm', return_value=changed):
                with self.assertRaisesRegex(ValueError, 'source contract changed'):
                    g.generate(Path(temp), None, 'unused')
                self.assertFalse((Path(temp)/'inventory.json').exists())

    def test_predeclared_final_rounding_control_discriminates_order(self):
        import struct
        def f(value):return struct.unpack('<f', struct.pack('<f', value))[0]
        def bits(value):return struct.unpack('<I', struct.pack('<f', value))[0]
        value, multiplier = f(1.3), f(.7)
        original_product = f(f(value*value)*multiplier)
        changed_product = f(f(multiplier*value)*value)
        original_result = f(2-f(1/f(original_product+.5)))
        changed_result = f(2-f(1/f(changed_product+.5)))
        self.assertEqual(bits(original_result), 0x3fb3f201)
        self.assertEqual(bits(changed_result), 0x3fb3f202)
        self.assertNotEqual(original_result, changed_result)

    def test_generation_emits_corrected_common_bodies(self):
        with tempfile.TemporaryDirectory() as temp, \
             patch.object(g, 'registry', return_value=list(self.entries.values())), \
             patch.object(g, 'TABLES', []):
            g.generate(Path(temp), None, '/usr/bin/clang')
            for ident in self.entries:
                text = (Path(temp)/f'formula_{ident}.metal.h').read_text()
                self.assertIn('abs ( ( value * value ) * params [ 5 ] )', text)
                self.assertNotIn('params [ 5 ] * value * value', text)


class Group02IntegerSemanticsTests(unittest.TestCase):
    REPLACEMENTS = {
        7010: [('const int Index=int(params[0]);',
                'const int Index=native_int32_parameter(params,0);')],
        7065: [('float sector=round(combined/TWOPI*Order);',
                'float sector=rint(combined/TWOPI*Order);')],
        7076: [('float sector=rint(sector_value);int sector_i=int(sector);',
                'int sector_i=0;float sector=native_fistp_sector(sector_value,sector_i);')],
        7081: [('float sector=rint(sector_value);int sector_i=int(sector);',
                'int sector_i=0;float sector=native_fistp_sector(sector_value,sector_i);'),
               ('const int No_yfold=int(params[5]);',
                'const int No_yfold=native_int32_parameter(params,5);')],
    }

    @classmethod
    def setUpClass(cls):
        cls.entries = {e.id: e for e in g.registry() if e.id in cls.REPLACEMENTS}
        cls.selected, cls.lowered, cls.traits = {}, {}, {}
        for ident, entry in cls.entries.items():
            selected = g.select_arm((g.FORMULAS/entry.source).read_text(), '/usr/bin/clang')
            values, traits = g.remove_metadata(g.tokens(selected), entry.symbol)
            cls.selected[ident] = selected
            cls.lowered[ident] = g.lower_context(g.lower(values))
            cls.traits[ident] = traits

    def test_real_bodies_change_only_audited_integer_operations(self):
        self.assertEqual(set(self.entries), {7010, 7065, 7076, 7081})
        for ident, entry in self.entries.items():
            original = self.lowered[ident]
            original_snapshot = list(original)
            expected = list(original)
            for old, new in self.REPLACEMENTS[ident]:
                pattern = g.tokens(old)
                self.assertEqual(sum(original[i:i+len(pattern)] == pattern for i in range(len(original))), 1)
                expected = g.replace_tokens(expected, old, new)
            result = g.lower_group02_integer_semantics(original, entry, self.traits[ident])
            # Whole-token equality protects coordinate sign, constants,
            # trig operations and the remainder of the source simultaneously.
            self.assertEqual(result, expected)
            self.assertEqual(original, original_snapshot)
            if ident == 7065:
                self.assertNotIn('native_fistp_sector', result)
                self.assertIn('rint', result)  # FRNDINT retains its real range.
            if ident in (7076, 7081):
                old_cast = g.tokens('int(sector)')
                self.assertFalse(any(result[i:i+len(old_cast)] == old_cast for i in range(len(result))))
                self.assertIn('native_fistp_sector', result)

    def test_missing_changed_or_duplicate_sites_fail_closed(self):
        for ident, entry in self.entries.items():
            for old, _ in self.REPLACEMENTS[ident]:
                original = self.lowered[ident]
                changed = old.replace('params[0]', 'params[1]').replace('params[5]', 'params[4]')
                changed = changed.replace('combined/TWOPI*Order', 'combined/TWOPI*(Order+1)')
                changed = changed.replace('rint(sector_value)', 'rint(sector_value+1)')
                self.assertNotEqual(changed, old)
                for variant in (g.replace_tokens(original, old, ''),
                                g.replace_tokens(original, old, changed), original+g.tokens(old)):
                    with self.subTest(ident=ident, site=old), self.assertRaises(ValueError):
                        g.lower_group02_integer_semantics(variant, entry, self.traits[ident])

    def test_every_descriptor_field_and_traits_are_pinned(self):
        from dataclasses import replace
        for ident, entry in self.entries.items():
            mutations = ({'symbol':entry.symbol+'_changed'}, {'name':entry.name+'_changed'},
                         {'source':'changed/'+entry.source}, {'family':'other'},
                         {'de_option':entry.de_option+1}, {'param_count':entry.param_count+1},
                         {'adds_c':entry.adds_c^1}, {'difs_style':entry.difs_style^1},
                         {'parameter_types':(entry.parameter_types[0]^1,)+entry.parameter_types[1:]})
            for changes in mutations:
                with self.subTest(ident=ident, changes=changes), self.assertRaises(ValueError):
                    g.lower_group02_integer_semantics(self.lowered[ident], replace(entry, **changes), self.traits[ident])
            with self.assertRaises(ValueError):
                g.lower_group02_integer_semantics(self.lowered[ident], entry, self.traits[ident]^1)

    def test_unrelated_integer_and_real_casts_remain_untouched(self):
        from dataclasses import replace
        for ident, entry in self.entries.items():
            source = self.lowered[ident]+g.tokens('int index=int(params[40]);float other=round(value);')
            self.assertEqual(g.lower_group02_integer_semantics(source, replace(entry, id=4522), 0), source)

    def test_typed_reads_must_remain_immutable_and_unique(self):
        for ident, index in ((7010, 0), (7081, 5)):
            entry, source = self.entries[ident], self.lowered[ident]
            for extra in (f'params[{index}]=1.0f;', f'params[{index}]+=1.0f;',
                          f'float extra=params[{index}];', f'consume(&params[{index}]);'):
                with self.subTest(ident=ident, extra=extra), self.assertRaises(ValueError):
                    g.lower_group02_integer_semantics(source+g.tokens(extra), entry, self.traits[ident])

    def test_fistp_helper_guards_cast_and_reloads_integer_zero(self):
        # Source-contract protection only. GPU execution separately verifies
        # finite overflow, parity, signed zero and nonfinite propagation.
        import re
        tokens = g.tokens((g.ROOT/'src/render/native_metal/formula_math.metal.h').read_text())
        function = tokens.index('native_fistp_sector')
        end_args = g.matching(tokens, function+1, '(', ')')
        args = g.split_token_args(tokens[function+2:end_args])
        self.assertEqual(args[0][:-1], ['float'])
        self.assertEqual(args[1][:-1], ['thread', 'int', '&'])
        input_name, output_name = args[0][-1], args[1][-1]
        end_body = g.matching(tokens, end_args+1, '{', '}')
        body = tokens[end_args+2:end_body]
        rounded_match = re.search(r'const float (\w+) = rint \( '+re.escape(input_name)+r' \) ;', ' '.join(body))
        self.assertIsNotNone(rounded_match)
        rounded = rounded_match.group(1)
        def position(source):
            pattern = g.tokens(source)
            found = [i for i in range(len(body)) if body[i:i+len(pattern)] == pattern]
            self.assertEqual(len(found), 1, source)
            return found[0]
        guard = position(f'if(!isfinite({rounded})){{{output_name}=INT32_MIN;return {rounded};}}')
        finite_cast = position(f'{output_name}=({rounded}>=-2147483648.0f&&{rounded}<2147483648.0f)?int({rounded}):INT32_MIN;')
        reload = position(f'return float({output_name});')
        self.assertLess(guard, finite_cast)
        self.assertLess(finite_cast, reload)
        cast_pattern = g.tokens(f'int({rounded})')
        self.assertEqual(sum(body[i:i+len(cast_pattern)] == cast_pattern for i in range(len(body))), 1)

    def test_generate_cannot_silently_disable_a_changed_target(self):
        for ident, entry in self.entries.items():
            with tempfile.TemporaryDirectory() as temp, \
                 patch.object(g, 'registry', return_value=[entry]), \
                 patch.object(g, 'TABLES', []), \
                 patch.object(g, 'lower_group02_integer_semantics', side_effect=ValueError('source contract changed')):
                with self.assertRaisesRegex(ValueError, 'source contract changed'):
                    g.generate(Path(temp), None, '/usr/bin/clang')
                self.assertFalse((Path(temp)/'inventory.json').exists())

    def test_temporary_generation_reaches_all_four_checked_lowerings(self):
        with tempfile.TemporaryDirectory() as temp, \
             patch.object(g, 'registry', return_value=list(self.entries.values())), \
             patch.object(g, 'TABLES', []):
            g.generate(Path(temp), None, '/usr/bin/clang')
            for ident in self.entries:
                emitted = g.tokens((Path(temp)/f'formula_{ident}.metal.h').read_text())
                for old, new in self.REPLACEMENTS[ident]:
                    before, after = g.tokens(old), g.tokens(new)
                    self.assertFalse(any(emitted[i:i+len(before)] == before for i in range(len(emitted))))
                    self.assertEqual(sum(emitted[i:i+len(after)] == after for i in range(len(emitted))), 1)


class Group03OriginalSemanticsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.entries = {e.id: e for e in g.registry() if e.id in (8481, 10006)}
        cls.lowered, cls.traits = {}, {}
        for ident, entry in cls.entries.items():
            source = g.select_arm((g.FORMULAS/entry.source).read_text(), '/usr/bin/clang')
            values, traits = g.remove_metadata(g.tokens(source), entry.symbol)
            cls.lowered[ident] = g.lower_lambdas(g.lower_map_bindings(g.lower_context(g.lower(values)), False))
            cls.traits[ident] = traits

    def test_only_original_negation_order_and_two_integer_reads_change(self):
        replacements = {
            8481: ('r0=fabs(r0+fold)-fabs(r0-fold)-r0;',
                   'r0=-(r0-(fabs(r0+fold)-fabs(r0-fold)));'),
            10006: ('int(params[0])', 'native_int32_parameter(params,0)'),
        }
        for ident, (old, new) in replacements.items():
            source = self.lowered[ident]
            snapshot = list(source)
            expected = g.replace_tokens(source, old, new)
            actual = g.lower_group03_original_semantics(source, self.entries[ident], self.traits[ident])
            self.assertEqual(actual, expected)
            self.assertEqual(source, snapshot)

    def test_original_zero_negation_discriminates_reversed_subtraction(self):
        import struct
        word = lambda value: struct.unpack('<I', struct.pack('<f', value))[0]
        x, fold = 0.0, 1.0
        difference = abs(x + fold) - abs(x - fold)
        self.assertEqual(word(-(x - difference)), 0x80000000)
        self.assertEqual(word(difference - x), 0)

    def test_original_int32_ids_are_distinct_before_float_narrowing(self):
        import struct
        narrow = lambda value: struct.unpack('<f', struct.pack('<f', value))[0]
        self.assertNotEqual(16777216, 16777217)
        self.assertEqual(narrow(16777216), narrow(16777217))
        self.assertEqual(int(float(16777217)), 16777217)  # exact binary64 payload

    def test_changed_sites_extra_reads_and_writes_fail_closed(self):
        entry, source = self.entries[10006], self.lowered[10006]
        for variant in (source + g.tokens('params[0]=1;'),
                        source + g.tokens('float extra=params[0];'),
                        g.replace_tokens(source, 'int(params[0])', 'int(params[1])'),
                        g.replace_tokens(source, '&input,int(params[0]),false', '&input,int(params[0]),true')):
            with self.assertRaises(ValueError):
                g.lower_group03_original_semantics(variant, entry, self.traits[10006])
        source = self.lowered[8481]
        for variant in (source + g.tokens('r0=fabs(r0+fold)-fabs(r0-fold)-r0;'),
                        g.replace_tokens(source, 'fabs(r0+fold)', 'fabs(r0+fold+1)')):
            with self.assertRaises(ValueError):
                g.lower_group03_original_semantics(variant, self.entries[8481], self.traits[8481])

    def test_every_descriptor_field_is_pinned(self):
        from dataclasses import replace
        for ident, entry in self.entries.items():
            for mutation in ({'symbol': entry.symbol+'_changed'}, {'name': entry.name+'_changed'},
                             {'source': 'changed/'+entry.source}, {'family': 'other'},
                             {'de_option': entry.de_option+1}, {'param_count': entry.param_count+1},
                             {'adds_c': entry.adds_c^1}, {'difs_style': entry.difs_style^1},
                             {'parameter_types': (entry.parameter_types[0]^1,)+entry.parameter_types[1:]}):
                with self.assertRaises(ValueError):
                    g.lower_group03_original_semantics(self.lowered[ident], replace(entry, **mutation), self.traits[ident])
            with self.assertRaises(ValueError):
                g.lower_group03_original_semantics(self.lowered[ident], entry, self.traits[ident]^1)

    def test_unrelated_bodies_are_unchanged(self):
        from dataclasses import replace
        for ident, entry in self.entries.items():
            self.assertEqual(g.lower_group03_original_semantics(self.lowered[ident], replace(entry, id=7280), 0),
                             self.lowered[ident])

    def test_generator_propagates_audited_failure(self):
        from unittest.mock import patch
        for entry in self.entries.values():
            with tempfile.TemporaryDirectory() as temp, \
                 patch.object(g, 'registry', return_value=[entry]), patch.object(g, 'TABLES', []), \
                 patch.object(g, 'lower_group03_original_semantics', side_effect=ValueError('original contract changed')):
                with self.assertRaisesRegex(ValueError, 'original contract changed'):
                    g.generate(Path(temp), None, '/usr/bin/clang')
                self.assertFalse((Path(temp)/'inventory.json').exists())


class Group04OriginalSemanticsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.entries = {e.id: e for e in g.registry() if e.id in (7071, 7085, 10269, 10352)}
        cls.lowered, cls.traits = {}, {}
        for ident, entry in cls.entries.items():
            source = g.select_arm(g.expand_local(g.FORMULAS/entry.source, set(), {}), '/usr/bin/clang')
            values, traits = g.remove_metadata(g.tokens(source), entry.symbol)
            cls.lowered[ident] = g.lower_lambdas(g.lower_context(g.lower(values)))
            cls.traits[ident] = traits

    def test_original_counterexamples_distinguish_narrowed_payload_and_sign(self):
        import math, struct
        narrow = lambda x: struct.unpack('<f', struct.pack('<f', x))[0]
        for value in (16777219, -2147483646):
            self.assertNotEqual(value & 7, int(narrow(value)) & 7)
        for value in (16777217, -2147483647):
            self.assertEqual(value & 1, 1)
            self.assertEqual(int(narrow(value)) & 1, 0)
        self.assertTrue(-0.0 >= 0.0)
        self.assertLess(math.copysign(1.0, -0.0), 0)

    def test_changed_or_duplicate_operations_fail_closed(self):
        sites = {7071: ('x*x+y*y+z*z', 'x*x+y*y'),
                 7085: ('Cx>=0.0f', 'Cx>0.0f'),
                 10269: ('int(params[0])', 'int(params[2])'),
                 10352: ('static_cast<int>(params[0])', 'static_cast<int>(params[1])')}
        for ident, (old, new) in sites.items():
            source, entry = self.lowered[ident], self.entries[ident]
            for variant in (source + source, g.replace_tokens(source, old, new)):
                with self.subTest(id=ident), self.assertRaises(ValueError):
                    g.lower_group04_original_semantics(variant, entry, self.traits[ident])
        for ident in (10269, 10352):
            for extra in ('params[0]=1;', 'float extra=params[0];'):
                with self.subTest(id=ident, extra=extra), self.assertRaises(ValueError):
                    g.lower_group04_original_semantics(self.lowered[ident]+g.tokens(extra),
                                                       self.entries[ident], self.traits[ident])

    def test_every_descriptor_field_is_pinned(self):
        from dataclasses import replace
        for ident, entry in self.entries.items():
            for mutation in ({'symbol': entry.symbol+'_changed'}, {'name': entry.name+'_changed'},
                             {'source': 'changed/'+entry.source}, {'family': 'other'},
                             {'de_option': entry.de_option+1}, {'param_count': entry.param_count+1},
                             {'adds_c': entry.adds_c^1}, {'difs_style': entry.difs_style^1},
                             {'parameter_types': entry.parameter_types+(2,)}):
                with self.subTest(id=ident, mutation=mutation), self.assertRaises(ValueError):
                    g.lower_group04_original_semantics(self.lowered[ident], replace(entry, **mutation), self.traits[ident])
            with self.assertRaises(ValueError):
                g.lower_group04_original_semantics(self.lowered[ident], entry, self.traits[ident]^1)

    def test_generator_propagates_audited_failure(self):
        for entry in self.entries.values():
            with tempfile.TemporaryDirectory() as temp, \
                 patch.object(g, 'registry', return_value=[entry]), patch.object(g, 'TABLES', []), \
                 patch.object(g, 'lower_group04_original_semantics', side_effect=ValueError('original contract changed')):
                with self.assertRaisesRegex(ValueError, 'original contract changed'):
                    g.generate(Path(temp), None, '/usr/bin/clang')
                self.assertFalse((Path(temp)/'inventory.json').exists())

    def test_only_audited_active_identity_gets_caller_rout(self):
        for ident, entry in self.entries.items():
            with tempfile.TemporaryDirectory() as temp, \
                 patch.object(g, 'registry', return_value=[entry]):
                result = g.generate(Path(temp), None, '/usr/bin/clang')
                self.assertEqual(result['blocked'], 0)
                helper = (Path(temp)/'formula_result_contracts.metal.h').read_text().split(
                    'inline bool native_external_reads_caller_rout')[1]
                self.assertEqual('case 7071u:' in helper, ident == 7071)
                if ident != 7071:
                    self.assertNotIn(f'case {ident}u:', helper)
                else:
                    self.assertIn('count==1u && traits==0u && option==0 && flags==1u', helper)

    def test_contraction_scope_is_only_the_original_tglad_function(self):
        for ident, entry in self.entries.items():
            text = g.emit_group04_source_contract(self.lowered[ident], entry)
            self.assertEqual(text.count('#pragma clang fp contract(off)'), int(ident == 10269))
        source, entry = self.lowered[10269], self.entries[10269]
        for variant in (source+source, g.replace_tokens(source, 'thread float & de', 'thread float & changed')):
            with self.assertRaises(ValueError):
                g.emit_group04_source_contract(variant, entry)


if __name__=="__main__":unittest.main()
