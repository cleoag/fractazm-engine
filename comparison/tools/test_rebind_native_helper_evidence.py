"""Review-bound helper changes only; synthetic fixtures grant no GPU credit."""
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import rebind_native_helper_evidence as replay
from rebind_native_helper_evidence import helper_changes


class Changes(unittest.TestCase):
    def test_explicit_report_change_and_new_checker(self):
        before = {'comparison/tools/report.py': 'old', 'src/render/a.cpp': 'same'}
        after = dict(before, **{'comparison/tools/report.py': 'new', 'comparison/tools/rebind.py': 'new'})
        changed = helper_changes(before, after, ['comparison/tools/report.py', 'comparison/tools/rebind.py'])
        self.assertIsNone(changed['comparison/tools/rebind.py']['before'])

    def test_unlisted_change_rejected(self):
        with self.assertRaisesRegex(Exception, 'Unreviewed'):
            helper_changes({'comparison/tools/a.py': 'old'}, {'comparison/tools/a.py': 'new'}, [])

    def test_native_change_rejected_even_if_explicit(self):
        for path in ('src/render/a.cpp', 'src/render/a.metal', 'scripts/gen_native_metal_formulas.py'):
            with self.subTest(path=path), self.assertRaisesRegex(Exception, 'Only explicitly'):
                helper_changes({path: 'old'}, {path: 'new'}, [path])

    def test_no_change_is_not_a_rebinding(self):
        with self.assertRaisesRegex(Exception, 'absent'):
            helper_changes({}, {}, [])

    def test_rebind_receipt_keeps_exact_stage_after_later_admission(self):
        with tempfile.TemporaryDirectory() as tmp:
            package = Path(tmp)
            live = package/'evidence.json'
            original = b'{"scenes": ["already-admitted"]}\n'
            live.write_bytes(original)
            sealed = replay.preserve_rebound_manifest(package)
            live.write_text('{"scenes": ["already-admitted", "new-scene"]}\n')
            self.assertEqual(sealed.read_bytes(), original)
            with self.assertRaisesRegex(Exception, 'snapshot already exists'):
                replay.preserve_rebound_manifest(package)

    def test_frozen_package_cannot_contain_rebinding_output(self):
        for source, target in (('/repo/frozen', '/repo/frozen/new'),
                               ('/repo/frozen/old', '/repo/frozen'),
                               ('/repo/frozen', '/repo/frozen')):
            with self.subTest(source=source, target=target), self.assertRaisesRegex(Exception, 'without nesting'):
                replay.distinct_packages(Path(source), Path(target))
        replay.distinct_packages(Path('/repo/old'), Path('/repo/new'))

    def test_traversal_cannot_masquerade_as_helper(self):
        path = 'comparison/tools/../../scripts/emit.py'
        with self.assertRaisesRegex(Exception, 'Only explicitly'):
            helper_changes({path: 'old'}, {path: 'new'}, [path])

    def test_python_helper_used_by_compiler_cannot_change(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp).resolve()
            # A direct repository/build layout must not exclude the repository
            # itself from checking actual compiler inputs.
            build = root/'build'
            build.mkdir(parents=True)
            source = root/'helper.py'
            source.write_text('original compiler input')
            manifest = build.parent/'source-manifest.json'
            manifest.write_text('{}')
            record = build.parent/'record.json'
            record.write_text(json.dumps({'dependencies': [
                {'path': str(source), 'sha256': replay.sha(source)}]}))
            (build.parent/'build-receipt.json').write_text(json.dumps({
                'source_manifest_sha256': replay.sha(manifest), 'records': [{'path': record.name}]}))
            # Product/capture validation has its own tests. Isolate the actual
            # compiler-input guard here, including Python build dependencies.
            with patch.object(replay, 'ROOT', root), patch.object(replay, 'validate_build_products'):
                self.assertEqual(replay.compiled_inputs_unchanged(build), 1)
                source.write_text('changed compiler input')
                with self.assertRaisesRegex(Exception, 'Actual compiler input changed'):
                    replay.compiled_inputs_unchanged(build)


if __name__ == '__main__':
    unittest.main()
