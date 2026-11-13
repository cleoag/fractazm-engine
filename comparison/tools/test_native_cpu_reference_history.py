"""A slow archived comparison must not trigger a new full CPU render."""
import json
import hashlib
import tempfile
import unittest
from pathlib import Path
from native_cpu_reference import previous_slow_reference,reference_binding
from qualify_native_metal_all import digest,sha


class SlowReferenceHistory(unittest.TestCase):
    def test_missing_resource_capture_does_not_erase_deadline_history(self):
        with tempfile.TemporaryDirectory() as temporary:
            root=Path(temporary).resolve();package=root/'package'
            path=package/'scene-runs'/'scene'/'reference-cpu'/'event.json'
            path.parent.mkdir(parents=True)
            for seconds,warning,expected in ((10,False,False),(180,False,False),
                                             (181,False,True),(10,True,True)):
                with self.subTest(seconds=seconds,warning=warning):
                    path.write_text(json.dumps({'seconds':seconds,'frame_warning_emitted':warning}))
                    result=previous_slow_reference(root,package,{'id':'scene'})
                    self.assertEqual(bool(result),expected)
                    if result:self.assertFalse(result['qualification_credit'])

    def test_absent_history_is_not_a_slow_frame(self):
        with tempfile.TemporaryDirectory() as temporary:
            root=Path(temporary).resolve();package=root/'package';package.mkdir()
            self.assertIsNone(previous_slow_reference(root,None,{'id':'scene'}))
            self.assertIsNone(previous_slow_reference(root,package,{'id':'scene'}))

    def test_reused_package_retains_bound_slow_history(self):
        with tempfile.TemporaryDirectory() as temporary:
            root=Path(temporary).resolve();package=root/'package';folder=package/'scene-runs'/'scene'
            folder.mkdir(parents=True);event=root/'old-event.json'
            event.write_text(json.dumps({'seconds':227,'inputs':[{'sha256':'scene-bytes'}]}))
            (folder/'case.json').write_text(json.dumps({'id':'scene','previous_slow_cpu':{
                'event':{'path':'old-event.json','sha256':hashlib.sha256(event.read_bytes()).hexdigest()}}}))
            result=previous_slow_reference(root,package,{'id':'scene','sha256':'scene-bytes'})
            self.assertEqual(result['seconds'],227)
            self.assertFalse(result['qualification_credit'])
            with self.assertRaises(ValueError):
                previous_slow_reference(root,package,{'id':'scene','sha256':'different-scene'})
            event.write_text('{}')
            with self.assertRaises(ValueError):
                previous_slow_reference(root,package,{'id':'scene','sha256':'scene-bytes'})


class ReferenceBindingHistory(unittest.TestCase):
    def test_pin_survives_aggregate_manifest_promotion_and_rejects_tampering(self):
        with tempfile.TemporaryDirectory() as temporary:
            root=Path(temporary).resolve();package=root/'package';package.mkdir()
            old={'renderer':{'path':'old','sha256':'old-bytes'}};new={'renderer':{'path':'new','sha256':'new-bytes'}}
            current=package/'evidence.json';current.write_text(json.dumps({'binding':old}))
            self.assertEqual(reference_binding(root,package),old)
            snapshot=root/'frozen.json';snapshot.write_bytes(current.read_bytes())
            pin={'schema':'mb3d.native-cpu-reference-binding.v1','manifest':{'path':'frozen.json','sha256':sha(snapshot)},'binding_sha256':digest(old)}
            (package/'cpu-reference-binding.json').write_text(json.dumps(pin));current.write_text(json.dumps({'binding':new}))
            self.assertEqual(reference_binding(root,package),old)
            pin['binding_sha256']=digest(new);(package/'cpu-reference-binding.json').write_text(json.dumps(pin))
            with self.assertRaisesRegex(ValueError,'digest'):reference_binding(root,package)
            pin['binding_sha256']=digest(old);(package/'cpu-reference-binding.json').write_text(json.dumps(pin));snapshot.write_text('{}')
            with self.assertRaises(ValueError):reference_binding(root,package)

if __name__=='__main__':unittest.main()
