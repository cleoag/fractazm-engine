"""Classify full independent loader records without calling ignored bytes exact.

The exemptions are narrow source contracts, not a mask learned from differences.
Old <7 iMaxIts is explicitly original-undefined; it never becomes parity PASS.
"""
import argparse
from collections import Counter
import hashlib
import json
import math
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[2]
CAMERA_TOLERANCE = 2e-12
SOURCES = {
    'canonical_version': 'src/io/m3p_parser.cpp: Mark as fully migrated; MandId=44',
    'old_uninitialized_imaxits': 'mb3d/FileHandling.pas:1368 Convert4to7paras, 1960 LoadParameter local Para9; only the old fields are initialized before copying 704 bytes',
    'camera_roundoff': 'mb3d/HeaderTrafos.pas: CalcVGradsFromHeader8rots; src/io/m3p_parser.cpp: ConvertHeaderFromOldParas; established Double tolerance 2e-12, not bit equality',
    'post_nul_name': 'mb3d/DivUtils.pas:1254 CustomFtoStr stops at first NUL; PutStringInCustomF writes the prefix and terminator only',
    'inactive_initial_default': 'mb3d/Mand.pas:1693 IniHAddon gives every slot option[0]=8; mb3d/HeaderTrafos.pas:184 Ini1CFfromHAddon invalid branch clears count/fnr/name but retains values; count=0 fnr=-1 has no formula consumer',
}

def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def integer(data, offset): return struct.unpack_from('<i', data, offset)[0]
def double(data, offset): return struct.unpack_from('<d', data, offset)[0]
def ordered_bits(x):
    b = struct.unpack('<Q', struct.pack('<d', x))[0]
    return (~b & ((1 << 64) - 1)) if b >> 63 else b | (1 << 63)

def classify(original, actual, version):
    if len(original) != 1976 or len(actual) != 1976:
        return dict(status='FAIL', reason='record size', unexplained_offsets=[])
    permitted = {}
    def allow(start, size, category):
        for i in range(start, start + size): permitted[i] = category
    if integer(original, 0) == version and integer(actual, 0) == 44:
        allow(0, 4, 'canonical_version')
    if version < 7 and integer(actual, 414) == 0:
        allow(414, 4, 'old_uninitialized_imaxits')
    camera = []
    if version < 17:
        for offset in range(246, 318, 8):
            a, b = double(original, offset), double(actual, offset)
            if math.isfinite(a) and math.isfinite(b) and abs(a-b) <= CAMERA_TOLERANCE:
                allow(offset, 8, 'camera_roundoff')
                if a != b:
                    camera.append(dict(offset=offset, absolute_error=abs(a-b),
                        ulps=abs(ordered_bits(a)-ordered_bits(b))))
    for slot in range(6):
        start = 848 + 188 * slot
        a, b = original[start+12:start+44], actual[start+12:start+44]
        if b'\0' in a and b'\0' in b and a.split(b'\0', 1)[0] == b.split(b'\0', 1)[0]:
            end = a.index(0)
            allow(start+13+end, 31-end, 'post_nul_name')
        if (integer(original, start) == integer(actual, start) == 0
                and integer(original, start+4) == integer(actual, start+4) == -1
                and integer(original, start+8) == integer(actual, start+8) == 0
                and original[start+12] == actual[start+12] == 0
                and double(original, start+60) == 8.0 and double(actual, start+60) == 0.0):
            allow(start+60, 8, 'inactive_initial_default')
    diffs = [i for i, (a, b) in enumerate(zip(original, actual)) if a != b]
    unknown = [i for i in diffs if i not in permitted]
    counts = Counter(permitted[i] for i in diffs if i in permitted)
    return dict(status='FAIL' if unknown else 'QUALIFIED_WITH_LIMITATIONS' if diffs else 'EXACT',
        byte_difference_categories=dict(counts), camera_differences=camera,
        original_undefined_fields=['iMaxIts'] if version < 7 else [],
        unexplained_offsets=unknown, raw_equal=not diffs)

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--replay', type=Path, action='append', required=True)
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args(); rows = []; identities = []
    for path in args.replay:
        report = json.loads(path.read_bytes())
        if not report['finished']: raise ValueError('Incomplete replay')
        identities.append(dict(path=str(path.resolve()), sha256=sha(path), cpp_sha256=report['cpp_sha256'],
            original_pascal_sha256=report['original_pascal_sha256']))
        for row in report['results']:
            scene, ref, cpp = (Path(row[k]) for k in ('scene', 'reference', 'cpp_dump'))
            for file, expected in [(scene, row['scene_sha256']), (ref, row['reference_sha256']), (cpp, row['cpp_dump_sha256'])]:
                if sha(file) != expected: raise ValueError('Changed input: '+str(file))
            version = integer(scene.read_bytes(), 0)
            qualified = classify(ref.read_bytes(), cpp.read_bytes(), version)
            rows.append(dict(id=row['id'], source_mand_id=version, **qualified))
    result = dict(scope=__doc__, inputs=identities, tool_sha256=sha(Path(__file__)),
        source_contracts=SOURCES, camera_absolute_tolerance=CAMERA_TOLERANCE,
        counts=dict(Counter(r['status'] for r in rows)),
        category_case_counts=dict(Counter(c for r in rows for c in r['byte_difference_categories'])),
        original_undefined_case_count=sum(bool(r['original_undefined_fields']) for r in rows),
        strict_bit_parity_claim=False, overall_project_completion_claim=False, results=rows)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2)+'\n')
    print(json.dumps({k: result[k] for k in ['counts', 'category_case_counts', 'original_undefined_case_count']}))
    return int(any(r['status'] == 'FAIL' for r in rows))

if __name__ == '__main__': raise SystemExit(main())
