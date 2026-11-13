"""Bounded compatibility-policy checks; no real renderer or VM is launched."""
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

from compatibility_precision import precision_evidence, profile_path, verify_precision_evidence
from sweep import run_render


class CompatibilityPrecisionTests(unittest.TestCase):
    def setUp(self):
        temporary=tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root=Path(temporary.name)
        self.scene=self.root/'scene.m3p'
        self.scene.write_bytes(b'scene')
        self.image=self.root/'image.png'
        self.exe=self.root/'native-default-renderer'

    def renderer(self, argv, **kwargs):
        # Model a Native-default binary; explicit selection must override it.
        mode=argv[argv.index('--precision')+1] if '--precision' in argv else 'native'
        self.image.write_bytes(b'compatible RGB' if mode=='compatible' else b'native RGB')
        if '--profile-json' in argv:
            Path(argv[argv.index('--profile-json')+1]).write_text(json.dumps({
                'schema':'mb3d.cpu-stage-profile.v1','precision':mode,
                'status':'complete','exit_code':0,'render_status':'completed'}))
        return subprocess.CompletedProcess(argv,0,b'',b'')

    def run_cpp(self, side_effect=None, flags=()):
        with patch('sweep.subprocess.run',side_effect=side_effect or self.renderer) as process:
            result=run_render(self.exe,self.scene,self.image,1,1000,list(flags))
        return result,process

    def test_native_default_is_explicitly_overridden_and_recorded(self):
        result,process=self.run_cpp()
        self.assertEqual(result[0],0)
        self.assertEqual(self.image.read_bytes(),b'compatible RGB')
        self.assertEqual(process.call_count,1)
        self.assertEqual(precision_evidence(self.image)['cpp_precision'],'compatible')

    def test_ignored_selection_cannot_pass_with_native_rgb(self):
        def ignores_flag(argv,**kwargs):
            self.renderer(argv,**kwargs)
            data=json.loads(profile_path(self.image).read_text());data['precision']='native'
            profile_path(self.image).write_text(json.dumps(data))
            return subprocess.CompletedProcess(argv,0,b'',b'')
        result,_=self.run_cpp(ignores_flag)
        self.assertEqual(result[0],-3)
        self.assertIn('precision',result[2])

    def test_old_executable_rejection_is_not_retried_without_policy(self):
        result,process=self.run_cpp(lambda argv,**kw: subprocess.CompletedProcess(argv,1,b'',b'unknown option --precision'))
        self.assertEqual(result[0],1)
        self.assertEqual(process.call_count,1)

    def test_stale_profile_cannot_qualify_a_missing_new_profile(self):
        profile_path(self.image).write_text(json.dumps(dict(precision='compatible')))
        def no_profile(argv,**kwargs):
            self.image.write_bytes(b'unknown RGB')
            return subprocess.CompletedProcess(argv,0,b'',b'')
        result,_=self.run_cpp(no_profile)
        self.assertEqual(result[0],-3)
        self.assertFalse(profile_path(self.image).exists())

    def test_extra_flags_cannot_override_owned_profile(self):
        for flags in (['--precision','native'],['--precision=native'],['--profile-json','other.json']):
            with self.subTest(flags=flags):
                result,process=self.run_cpp(flags=flags)
                self.assertEqual(result[0],-3)
                process.assert_not_called()

    def test_resume_rejects_precision_profile_changes(self):
        self.run_cpp();evidence=precision_evidence(self.image)
        verify_precision_evidence(self.image,evidence)
        profile_path(self.image).write_text(profile_path(self.image).read_text()+'\n')
        with self.assertRaisesRegex(ValueError,'changed'):
            verify_precision_evidence(self.image,evidence)

    def test_pascal_command_does_not_receive_cpp_flags(self):
        def pascal(argv,**kwargs):
            self.assertNotIn('--precision',argv);self.assertNotIn('--profile-json',argv)
            self.image.write_bytes(b'Pascal RGB')
            return subprocess.CompletedProcess(argv,0,b'',b'')
        with patch('sweep.subprocess.run',side_effect=pascal):
            result=run_render(self.root/'Mandelbulb3D.exe',self.scene,self.image,1,1000,[])
        self.assertEqual(result[0],0)


if __name__=='__main__':
    unittest.main()
