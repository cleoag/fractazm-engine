#!/usr/bin/env python3
"""Selected CPU-only tests for experimental region extraction."""
import unittest
from gen_native_continuations import generate, original_observer, body, matching, locate, emit_text, ROOT

class Regions(unittest.TestCase):
    def source(self,id_):return (ROOT/f'src/render/native_metal/generated/formula_{id_}.metal.h').read_text()
    def test_repeatable_source_binding(self):
        for id_ in (10274,10265,10220,10321,10089):
            a=generate(id_,self.source(id_));self.assertEqual(a,generate(id_,self.source(id_)))
            self.assertEqual(a[1]['formula_id'],id_)
    def test_guard_is_preserved(self):
        out,row=generate(10274,self.source(10274))
        self.assertIn('work . beforeStep',out);self.assertIn('work.steps=s.work_steps',out)
        self.assertEqual(len(row['regions']),3)
    def test_unguarded_loops_are_separate(self):
        out,row=generate(10265,self.source(10265))
        self.assertEqual(len(row['regions']),3)
        self.assertIn('theta -= double_half',out);self.assertIn('external_pine_t1',out)
        self.assertNotIn('FormulaWorkBudget',out)
    def test_pine_finisher_preserves_original_expression_tokens(self):
        source=self.source(10265);out,_=generate(10265,source);v=body(source)
        polygon=locate(v,['if','(','polygon',')']);end=matching(v,polygon+4)
        self.assertIn('float pixel_z) { '+emit_text(v[end+1:])+' }',out)
        self.assertIn('s.result=finish_value(x,y,z,Cx,Cy,Cz,pixel_x,pixel_y,pixel_z)',out)
    def test_original_observer_has_original_loops_without_resume(self):
        for id_ in (10274,10265,10220,10321,10089):
            out=original_observer(id_,self.source(id_))
            self.assertIn('o.record(s,params,de)',out)
            self.assertNotIn('switch',out);self.assertNotIn('remaining=1',out)
            self.assertNotIn('::step(',out)
    def test_changed_shape_rejects(self):
        with self.assertRaises(ValueError):generate(10274,self.source(10274).replace('while ( true )','while ( count < 8 )'))
        with self.assertRaises(ValueError):generate(10265,self.source(10265).replace('i < T1_Count','i <= T1_Count'))
    def test_corpus_loop(self):
        out,row=generate(10220,self.source(10220));self.assertEqual(len(row['regions']),3)
        self.assertIn('x87Abs',out);self.assertIn('++s.i',out)
    def test_changed_helper_or_body_requires_audit(self):
        with self.assertRaises(ValueError):generate(10265,self.source(10265).replace('theta -= double_half','theta -= double_half * 2'))
    def test_gnarly_modified_loop_requires_audit(self):
        with self.assertRaises(ValueError):generate(10089,self.source(10089).replace('i < iter_count','i <= iter_count'))
    def test_unsupported_identity_rejects(self):
        with self.assertRaises(ValueError):generate(999,self.source(10274))

if __name__=='__main__':unittest.main()
