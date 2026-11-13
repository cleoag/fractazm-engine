"""Real subprocess wiring with synthetic reports; never device qualification."""
import argparse
import json
from pathlib import Path
import unittest
from unittest.mock import patch

import native_formula_observer as observer
import run_m3parameter_gpu as runner
from test_native_formula_observer import REGISTRY, case, report
import test_native_formula_resource_input as fixtures


class ResourceObserver(unittest.TestCase):
    setUp = fixtures.ResourceInput.setUp
    tearDown = fixtures.ResourceInput.tearDown
    resource_report = fixtures.ResourceInput.report

    def capture(self, mutate_asset=False, alter_report=False):
        inp, library, probe, output = [self.root / name for name in
            ('case.json', 'library.bin', 'synthetic-probe.py', 'observation.json')]
        payload = case()
        raw = report(payload, self.root / 'observation.probe/program.bin', library)
        raw.update(self.resource_report())
        if alter_report:
            raw['resources']['pixel_words'][0] = 0
        payload['resources'] = self.payload
        inp.write_text(json.dumps(payload)); library.write_bytes(b'SYNTHETIC LIBRARY')
        probe.write_text('#!/usr/bin/env python3\nimport json,sys\nfrom pathlib import Path\n'
                         'args=dict(zip(sys.argv[1::2],sys.argv[2::2]))\n'
                         f'assert args["--resource-root"] == {str(self.root / "runtime")!r}\n'
                         'assert args["--map-id"] == "17" and args["--map-controls"] == "active"\n'
                         f'Path(args["--output"]).write_text(json.dumps({raw!r}))\n')
        probe.chmod(0o755)
        args = argparse.Namespace(input=str(inp), probe=str(probe), library=str(library),
            output=str(output), run_id='b'*32, slot=0, timeout=30,
            probe_sha256=observer.sha(probe), library_sha256=observer.sha(library))

        def invoke(*a, **kw):
            event = runner.invoke(*a, **kw)
            if mutate_asset:
                (self.root / self.payload['files'][0]['path']).write_bytes(b'changed while child ran')
            return event

        with patch.object(observer, 'ROOT', self.root), \
             patch.object(observer, 'current_registry', return_value=REGISTRY), \
             patch.object(observer, 'dependency_paths', return_value=[]), \
             patch.object(observer, 'invoke', side_effect=invoke), \
             patch.object(runner.shutil, 'disk_usage', return_value=argparse.Namespace(free=32*1024**3)):
            return observer.observe(args)

    def test_child_receives_and_captures_resources(self):
        actual = self.capture()
        self.assertEqual(actual['map_abi'], [1, 32, 3, 1])
        self.assertEqual(actual['resources'], self.resource_report()['resources'])
        event = json.loads((self.root / actual['probe_event']['path']).read_text())
        self.assertIn(self.payload['files'][0], event['inputs'])
        self.assertEqual(event['exit_code'], 0)

    def test_asset_changed_during_child_is_rejected(self):
        with self.assertRaises(ValueError):
            self.capture(mutate_asset=True)
        self.assertFalse((self.root / 'observation.json').exists())

    def test_different_uploaded_pixels_are_rejected(self):
        with self.assertRaises(ValueError):
            self.capture(alter_report=True)
        self.assertFalse((self.root / 'observation.json').exists())


if __name__ == '__main__':
    unittest.main()
