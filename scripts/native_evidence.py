"""Source provenance for native checks, including newly added source files."""
import hashlib
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SOURCE_PATHS = ['CMakeLists.txt', 'cmake', 'src', 'include', 'libs', 'external_formulas',
                'tests', 'scripts', 'comparison/tools', 'comparison/metal_probe',
                'comparison/metal_exact']

def source_identity():
    def git(*args):
        return subprocess.check_output(['git', *args], cwd=ROOT)
    changed = git('diff', '--name-only', '-z', 'HEAD', '--', *SOURCE_PATHS)
    added = git('ls-files', '--others', '--exclude-standard', '-z', '--', *SOURCE_PATHS)
    files = {}
    for raw in sorted(set((changed + added).split(b'\0')) - {b''}):
        name = raw.decode(); path = ROOT / name
        files[name] = hashlib.sha256(path.read_bytes()).hexdigest() if path.is_file() else None
    return {'source_commit': git('rev-parse', 'HEAD').decode().strip(),
            'tracked_diff_sha256': hashlib.sha256(git('diff', 'HEAD')).hexdigest(),
            'changed_source_sha256': files}
