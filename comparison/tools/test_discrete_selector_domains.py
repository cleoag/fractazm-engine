import unittest

from discrete_selector_domains import classify, fingerprint


class DomainTests(unittest.TestCase):
    def test_boolean_and_signed_special_are_three_classes(self):
        spec = classify('const int mode = int(params[7]); if (mode != 0) a(); if (mode == -1) b();', 7)
        self.assertEqual(spec['representatives'], [-1, 0, 1])
        self.assertEqual(fingerprint(12345, spec), fingerprint(1, spec))
        self.assertNotEqual(fingerprint(-1, spec), fingerprint(1, spec))

    def test_noncontiguous_mask_has_only_four_classes(self):
        spec = classify('int mode = (int)params[3]; if (mode & 1) a(); if (mode & 8) b();', 3)
        self.assertEqual(spec['representatives'], [0, 1, 8, 9])
        self.assertEqual(fingerprint(-1, spec), 9)

    def test_interval_endpoints(self):
        spec = classify('int count = int(params[2]); if (count < 2) a(); if (count > 7) b();', 2)
        self.assertEqual(len(spec['representatives']), 3)
        self.assertEqual(fingerprint(2, spec), fingerprint(7, spec))
        self.assertNotEqual(fingerprint(1, spec), fingerprint(2, spec))
        self.assertNotEqual(fingerprint(8, spec), fingerprint(7, spec))

    def test_unknown_uses_are_never_declared_complete(self):
        for expression in ['helper(mode);', 'mode++;', 'int y=mode;', 'if(mode < limit) a();',
                           'if(mode == 1+offset) a();', 'if(mode == 1-offset) a();',
                           'if(mode == 1.5) a();', 'if(mode & 1+offset) a();',
                           'if(other+mode == 1) a();', 'if(mode & 1<<other) a();',
                           'if(-mode == 1) a();']:
            self.assertIsNone(classify('int mode=int(params[3]); '+expression, 3), expression)

    def test_comment_and_other_parameter_are_not_evidence(self):
        self.assertIsNone(classify('// int mode=int(params[3]); if(mode==1) a();', 3))
        self.assertIsNone(classify('int mode=int(params[4]); if(mode==1) a();', 3))


if __name__ == '__main__':
    unittest.main()
