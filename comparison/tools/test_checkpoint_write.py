"""Atomic checkpoints survive transient sharing violations and permanent errors."""
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch
import external_validation as ev


class CheckpointWriteTests(unittest.TestCase):
    def test_transient_denial_retries_atomic_replace(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / 'summary.json'
            path.write_text('old')
            replace = ev.os.replace
            calls = []

            def temporarily_locked(source, target):
                calls.append(source)
                self.assertEqual(path.read_text(), 'old')
                if len(calls) < 3:
                    raise PermissionError('sharing violation')
                replace(source, target)

            with patch.object(ev.os, 'replace', side_effect=temporarily_locked), \
                    patch.object(ev.time, 'sleep') as sleep:
                ev.write_json(path, {'finished': True})
                self.assertEqual(sleep.call_count, 2)
            self.assertIn('true', path.read_text())
            self.assertEqual(list(Path(folder).iterdir()), [path])

    def test_permanent_denial_preserves_old_checkpoint(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / 'summary.json'
            path.write_text('old')
            with patch.object(ev.os, 'replace', side_effect=PermissionError('denied')) as replace, \
                    patch.object(ev.time, 'sleep'), self.assertRaises(PermissionError):
                ev.write_json(path, {'finished': True})
            self.assertEqual(replace.call_count, 20)
            self.assertEqual(path.read_text(), 'old')
            self.assertEqual(list(Path(folder).iterdir()), [path])

    def test_other_errors_are_not_retried(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / 'summary.json'
            with patch.object(ev.os, 'replace', side_effect=OSError('disk failure')) as replace, \
                    patch.object(ev.time, 'sleep') as sleep, self.assertRaises(OSError):
                ev.write_json(path, {})
            self.assertEqual(replace.call_count, 1)
            sleep.assert_not_called()
            self.assertEqual(list(Path(folder).iterdir()), [])


if __name__ == '__main__':
    unittest.main()
