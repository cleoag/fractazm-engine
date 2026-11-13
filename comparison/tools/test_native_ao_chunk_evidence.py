"""Negative controls for scoped physical-command normalization, not GPU proof."""
import unittest
from native_ao_chunk_evidence import normalize

class ChunkEvidenceTests(unittest.TestCase):
    def fixture(self,flag=",base-table-reuse=1"):
        return {"execution":{"execution_plan":"general-v1|rng:quota=128,chunks=8|seed-geometry:version=1,chunk-prefix=1"+flag,"ao_certificate_version":1,"ao_algorithm":4,"ao_certificate_levels":3,"ao_certificate_pixels":360000,"ao_commands":164,"ao_dispatches":164,"rng_advances":123},"depth_reads":456}
    def test_only_physical_preparation_changes(self):
        source=self.fixture();result,mapping=normalize(source,[400,300],6)
        self.assertEqual(mapping,{"ao_chunk_preparation_commands":64})
        self.assertEqual(result['execution']['ao_commands'],100)
        self.assertEqual(result['execution']['ao_dispatches'],100)
        self.assertEqual(result['depth_reads'],456)
        self.assertEqual(result['execution']['rng_advances'],123)
        self.assertEqual(source['execution']['ao_commands'],164)
    def test_prior_candidate_without_reuse(self):
        self.assertEqual(normalize(self.fixture(''),[400,300],6),normalize(self.fixture(),[400,300],6))
    def test_reject_similar_chunk_number(self):
        x=self.fixture();x['execution']['execution_plan']=x['execution']['execution_plan'].replace('chunks=8','chunks=80')
        with self.assertRaises(AssertionError):normalize(x,[400,300],6)
    def test_reject_chunk_policy_in_wrong_domain(self):
        x=self.fixture();x['execution']['execution_plan']=x['execution']['execution_plan'].replace('|rng:', '|wavelet:')
        with self.assertRaises(AssertionError):normalize(x,[400,300],6)
    def test_reject_reuse_without_chunk_prefix(self):
        x=self.fixture();x['execution']['execution_plan']=x['execution']['execution_plan'].replace('chunk-prefix=1','chunk-prefix=0')
        with self.assertRaises(AssertionError):normalize(x,[400,300],6)
    def test_reject_duplicate_or_invalid_policy(self):
        for marker in (',chunk-prefix=1',',chunk-prefix=2',',base-table-reuse=0',',base-table-reuse=3'):
            x=self.fixture();x['execution']['execution_plan']+=marker
            with self.assertRaises(AssertionError):normalize(x,[400,300],6)
    def test_reject_inconsistent_extent_and_underflow(self):
        for key,value in (('ao_certificate_pixels',360001),('ao_commands',63),('ao_dispatches',0)):
            x=self.fixture();x['execution'][key]=value
            with self.assertRaises(AssertionError):normalize(x,[400,300],6)

if __name__=='__main__':unittest.main()
