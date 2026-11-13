"""Original inventory input-binding checks; no device/numerical credit."""
import copy
import struct
import unittest
from pathlib import Path
from unittest.mock import patch

import native_formula_context_input as c
import native_formula_observer as o
from gpu_pipeline_proofs import semantic_input


def mixed_case():
    _, row = c.context()
    program = list(struct.unpack('<2048I', (c.ROOT / row['program_artifacts'][0]['path']).read_bytes()))
    state = [0] * 47
    for s, p in [(36,7),(26,16),(43,8),(44,9),(45,10)]: state[s] = program[p]
    state[12:16] = program[24:28]
    state[37:43] = program[50:56]
    return {'identity': c.IDENTITY.copy(), 'kind': 'orbit',
            'input_state_bits': dict(zip(o.STATE, (f'0x{x:08x}' for x in state))),
            'parameter_bits': [f'0x{x:08x}' for x in program], 'context': c.descriptor()}


class MixedContextInput(unittest.TestCase):
    def test_original_program_and_target_slot(self):
        case = mixed_case(); program, state = o.validate_case(case)
        self.assertEqual(c.validate(case, program, state), 1)
        self.assertEqual(program[7], 400)
        self.assertEqual(state[37:40], [30, 0x80000001, 5])
        self.assertEqual([program[58+332*i] for i in range(3)], [1509, 8065, 7039])

    def test_single_slot_label_cannot_cover_mixed_input(self):
        case = mixed_case(); del case['context']
        with self.assertRaisesRegex(ValueError, 'single-slot'): o.validate_case(case)

    def test_mutations_refused_before_child(self):
        cases = []
        for key, value in [('schema','unknown'),('target_slot',0),('target_slot',True),
                           ('program_index',1),('program_index',False),('context_id','0'*64)]:
            v=mixed_case(); v['context'][key]=value; cases.append(v)
        for key, value in [('kind','point'),('identity',['external',1509,'Amazing Surf']),('resources',{})]:
            v=mixed_case(); v[key]=value; cases.append(v)
        for index in [1,3,4,5,7,8,10,13,50,51,52,58,390,398,400,732]:
            v=mixed_case(); v['parameter_bits'][index]=f'0x{int(v["parameter_bits"][index],16)^1:08x}'; cases.append(v)
        for index in [12,13,14,15,26,36,37,38,39,40,41,42,43,44,45]:
            v=mixed_case(); key=o.STATE[index]; v['input_state_bits'][key]=f'0x{int(v["input_state_bits"][key],16)^1:08x}'; cases.append(v)
        with patch.object(o, 'invoke') as child:
            for index, case in enumerate(cases):
                with self.subTest(index=index), self.assertRaises((ValueError, KeyError)):
                    o.validate_case(case)
            child.assert_not_called()

    def test_carry_and_sample_inputs_are_not_silently_reset(self):
        case=mixed_case()
        for lane in ('c[0]','xyz[2]','sse[0]','dfree2','derivative[3]','first'):
            case['input_state_bits'][lane]='0x3e800000'
        program,state=o.validate_case(case)
        self.assertEqual(state[o.STATE.index('sse[0]')],0x3e800000)

    def test_context_dependencies_include_actual_program_and_sources(self):
        case=mixed_case(); _,row=c.context()
        paths=o.dependency_paths(case)
        for ref in row['artifacts']+[row['build_receipt'],row['host']]:
            self.assertIn(c.ROOT/ref['path'], paths)
        self.assertIn(c.ROOT/'mb3d/M3Formulas/_Translate.m3f', paths)
        self.assertIn(Path(c.__file__).resolve(),paths)

    def test_context_survives_common_gate_semantic_projection(self):
        case=mixed_case()
        self.assertEqual(semantic_input(dict(case, case_id='fixture')),case)
        isolated=dict(case); del isolated['context']
        self.assertNotEqual(o.digest(semantic_input(case)),o.digest(semantic_input(isolated)))

    def test_reachability_is_required_separately_from_program_membership(self):
        targets={'iteration':'0x00000021','current_slot':'0x00000002'}
        c.validate_schedule({'expected_slot_visits':[30,1,3,0,0,0],'expected_work':34,
                             'expected_state_bits':targets})
        for visits,work in [([1,0,0,0,0,0],1),([30,1,0,0,0,0],31),
                            ([30,1,3,0,0,0],33),([30,True,3,0,0,0],34),
                            ([30,1,3],34),([30,1,3,1,0,0],35),
                            ([1,1,1,0,0,0],3),([30,2,1,0,0,0],33)]:
            with self.subTest(visits=visits,work=work),self.assertRaises(ValueError):
                c.validate_schedule({'expected_slot_visits':visits,'expected_work':work,
                                     'expected_state_bits':targets})
        for key in targets:
            bad=dict(targets);bad[key]='0x00000000'
            with self.assertRaisesRegex(ValueError,'count schedule'):
                c.validate_schedule({'expected_slot_visits':[30,1,3,0,0,0],'expected_work':34,
                                     'expected_state_bits':bad})


if __name__ == '__main__': unittest.main()
