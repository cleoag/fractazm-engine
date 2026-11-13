"""Source extraction and bounded diagnostic harness controls; no Pascal build."""
import unittest
import build_original_analytic_caller_oracle as b


class Extraction(unittest.TestCase):
    def test_immutable_analytic_body_is_copied_exactly(self):
        source=(b.ROOT/'mb3d/formulas.pas').read_text()
        start=source.index('function doHybridPasDE(',source.index('implementation'))
        body=source[start:source.index('\nend;',start)+5]
        self.assertEqual(b.source().count(body),1)
        self.assertNotIn('procedure doHybridPas(',b.source())

    def test_actual_register_capture_is_before_formatting_and_original_tail(self):
        s=b.source()
        self.assertIn('call doHybridPasDE\n  movupd [OutLanes], xmm0\n  movupd [OutLanes+16], xmm1\n  fstp qword ptr [Distance]',s)
        self.assertIn('jmp dword ptr [CodePtr]',s)
        self.assertIn('(Limit > 8) or (Count < 1) or (Count > 3)',s)
        self.assertIn('It.DEoption:=2',s)
        self.assertIn('begin Halt(90); end;',s)
        self.assertIn('FillChar(It, SizeOf(It), 0)',s)


if __name__=='__main__':unittest.main()
