import json
from pathlib import Path
import unittest
from report_mc_function_coverage import verified_report


class MCCoverageTests(unittest.TestCase):
    def setUp(self):
        self.path=Path('reference.pas').resolve()
        self.report=dict(sources={str(self.path):'abc'},rows=4,failures=[0,0],exact=True)

    def verify(self,report):
        return verified_report(json.dumps(report).encode(),self.path,'abc')

    def test_accepts_direct_and_wrapped_evidence(self):
        self.assertEqual(self.verify(self.report),self.report)
        self.assertEqual(self.verify({'evidence':self.report}),self.report)

    def test_changed_reference_is_rejected(self):
        with self.assertRaises(ValueError): self.verify({**self.report,'sources':{str(self.path):'changed'}})

    def test_failing_lanes_override_success_flag(self):
        with self.assertRaises(ValueError): self.verify({**self.report,'failures':[0,1]})

    def test_missing_or_empty_measurements_are_rejected(self):
        for change in [dict(rows=0),dict(rows=None),dict(failures=[]),dict(failures=None)]:
            with self.subTest(change=change), self.assertRaises(ValueError): self.verify({**self.report,**change})

    def test_failed_run_is_rejected(self):
        with self.assertRaises(ValueError): self.verify({**self.report,'exact':False})


if __name__=='__main__': unittest.main()
