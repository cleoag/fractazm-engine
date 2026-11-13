"""The loader qualifier must not hide active fields or invented defaults."""
import struct
import unittest
from qualify_legacy_loader import classify

def record(version=44):
    data=bytearray(1976);struct.pack_into('<i',data,0,version);return data

class LegacyQualificationTests(unittest.TestCase):
    def test_exact(self):
        self.assertEqual(classify(record(),record(),44)['status'],'EXACT')

    def test_named_dormant_value_is_not_exempt(self):
        a=record();struct.pack_into('<i',a,852,20);a[860:864]=b'Box\0'
        struct.pack_into('<d',a,908,8.);b=bytearray(a);struct.pack_into('<d',b,908,0.)
        self.assertEqual(classify(a,b,44)['status'],'FAIL')

    def test_only_empty_invalid_initial_value_is_exempt(self):
        a=record();struct.pack_into('<i',a,852,-1);struct.pack_into('<d',a,908,8.)
        b=bytearray(a);struct.pack_into('<d',b,908,0.)
        self.assertEqual(classify(a,b,44)['status'],'QUALIFIED_WITH_LIMITATIONS')
        struct.pack_into('<d',b,908,1.)
        self.assertEqual(classify(a,b,44)['status'],'FAIL')

    def test_post_nul_only(self):
        a=record();a[860:865]=b'Box\0x';b=bytearray(a);b[864]=0
        self.assertEqual(classify(a,b,44)['status'],'QUALIFIED_WITH_LIMITATIONS')
        b[862]=ord('y')
        self.assertEqual(classify(a,b,44)['status'],'FAIL')

    def test_old_undefined_requires_deterministic_zero_policy(self):
        a=record(4);struct.pack_into('<i',a,414,1234);b=record(44)
        self.assertEqual(classify(a,b,4)['original_undefined_fields'],['iMaxIts'])
        self.assertEqual(classify(a,b,4)['status'],'QUALIFIED_WITH_LIMITATIONS')
        struct.pack_into('<i',b,414,8)
        self.assertEqual(classify(a,b,4)['status'],'FAIL')

    def test_camera_tolerance_not_unbounded_mask(self):
        a=record(15);b=record(44);struct.pack_into('<d',b,246,1e-8)
        self.assertEqual(classify(a,b,15)['status'],'FAIL')

    def test_version_marker_must_be_canonical(self):
        self.assertEqual(classify(record(15),record(43),15)['status'],'FAIL')

if __name__=='__main__':unittest.main()
