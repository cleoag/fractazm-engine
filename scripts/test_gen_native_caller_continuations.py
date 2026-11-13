#!/usr/bin/env python3
"""Source drift and exact region extraction gates, not numeric qualification."""
import subprocess
import unittest
from pathlib import Path
from gen_native_metal_formulas import tokens,matching
from gen_native_continuations import generate,shared_frame_header,REGION_ORDER,ROOT,locate
from gen_native_caller_continuations import generate_wrapper

class CallerRegions(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source=subprocess.check_output(['git','show','fc00c78e02b1aba530e611a07f2bf72cc5dbd099:src/render/native_metal/formula_wrapper.metal.h'],text=True,cwd=ROOT)
    def test_source_drift_rejected(self):
        with self.assertRaises(ValueError):generate_wrapper(self.source+'\n')
        with self.assertRaises(ValueError):generate_wrapper((ROOT/'src/render/native_metal/formula_wrapper.metal.h').read_text())
    def test_same_prefix_and_suffix(self):
        v=tokens(self.source);a=locate(v,['native_formula_step_body','(']);a=v.index('{',a);body=v[a+1:matching(v,a)]
        a=locate(body,['if','(','!','native_external_raw','(']);b=body.index(';',a)+1
        emitted=tokens(generate_wrapper(self.source))
        for region in [body[:a],body[b:]]:
            self.assertTrue(any(emitted[i:i+len(region)]==region for i in range(len(emitted)-len(region)+1)))
    def test_observer_keeps_original_call(self):
        output=generate_wrapper(self.source)
        self.assertEqual(output.count('native_external_raw'),1)
        self.assertEqual(output.count('observer.record(w,'),2)
    def test_vendored_frame_types_current(self):
        codes=[(id_,generate(id_,(ROOT/f'src/render/native_metal/generated/formula_{id_}.metal.h').read_text())[0]) for id_ in REGION_ORDER]
        self.assertEqual(shared_frame_header(codes),(ROOT/'src/render/native_metal/continuation_frames.h').read_text())

if __name__=='__main__':unittest.main()
