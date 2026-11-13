import json
import unittest
from plan_native_formula_verification import ROOT, FIRST, plan


class FormulaPlanTests(unittest.TestCase):
    def test_complete_disjoint_pinned_external_scope(self):
        result=plan()
        actual=[(f['id'],f['name']) for g in result['groups'] for f in g['formulas']]
        inventory=json.loads((ROOT/'comparison/contracts/native-gpu-acceptance-v4/inventory.json').read_text())
        expected={(f['id'],f['name']) for f in inventory['formulas'] if f['namespace']=='external'}
        self.assertEqual(len(actual),457)
        self.assertEqual(set(actual),expected)
        self.assertFalse(result['qualification'])
        self.assertTrue(all(g['status']=='OPEN_NO_DEVICE_CREDIT' for g in result['groups']))

    def test_priority_count_does_not_grow_when_partitioning_remaining(self):
        result=plan()
        inventory=json.loads((ROOT/'comparison/contracts/native-gpu-acceptance-v4/inventory.json').read_text())
        priority={int(s['key'].split(':')[1]) for scene in inventory['scenes'] for s in scene['slots']
                  if s['iterations']>0 and s['key'].startswith('external:')}
        self.assertEqual(result['phase1_external_count'],len(priority))
        self.assertLess(len(priority),result['external_count'])
        seen_remaining=False
        for group in result['groups'][1:]:
            for f in group['formulas']:
                if f['id'] not in priority:seen_remaining=True
                else:self.assertFalse(seen_remaining)

    def test_group_size_preserves_first_review_group_and_denominator(self):
        result=plan(group_size=7)
        self.assertEqual(tuple(f['name'] for f in result['groups'][0]['formulas']),FIRST)
        self.assertTrue(all(1<=len(g['formulas'])<=7 for g in result['groups'][1:]))
        for value in (0,33,True):
            with self.assertRaises(ValueError):plan(group_size=value)

if __name__=='__main__':unittest.main()
