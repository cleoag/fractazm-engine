"""Vendor unchanged independent Pascal loader inputs/records into permanent tests."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import struct

ROOT=Path(__file__).resolve().parents[2]
def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def fnv(data):
    value=14695981039346656037
    for byte in data: value=((value ^ byte)*1099511628211)&0xffffffffffffffff
    return value

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--reference',type=Path,action='append',required=True)
    a=ap.parse_args();out=ROOT/'tests/fixtures/legacy_loader';out.mkdir(parents=True,exist_ok=True)
    rows=[];reports=[]
    for report_path in a.reference:
        report=json.loads(report_path.read_bytes())
        if not report['finished']: raise ValueError('Incomplete original reference')
        reports.append(dict(path=str(report_path),sha256=sha(report_path),pascal_sha256=report['pascal_sha256']))
        for row in report['results']:
            name=row['id'];scene=Path(row['path']);expected=report_path.parent/(name+'.pascal.bin')
            if sha(scene)!=row['sha256'] or sha(expected)!=row['pascal_sha256']:
                raise ValueError('Original reference changed: '+name)
            shutil.copy2(scene,out/(name+'.m3p'));shutil.copy2(expected,out/(name+'.pascal.bin'))
            rows.append(dict(id=name,scene_sha256=sha(scene),expected_sha256=sha(expected),
                source_mand_id=struct.unpack_from('<i',scene.read_bytes())[0],
                scene_fnv1a64=fnv(scene.read_bytes()),expected_fnv1a64=fnv(expected.read_bytes()),
                stored_pixel_bytes=row.get('stored_pixel_bytes',0),addon_offset=row.get('addon_offset',0)))
    (out/'manifest.json').write_text(json.dumps(dict(scope=__doc__,references=reports,results=rows,
        generator_sha256=sha(Path(__file__)),expected_records_generated_by_cpp=False),indent=2)+'\n')
    lines=['// Independent full original Pascal records; see legacy_loader/manifest.json.',
        '#pragma once','#include <cstdint>','namespace legacy_loader_fixture {',
        'struct Case { const char* id; int sourceVersion; uint64_t inputHash, expectedHash, storedBytes, addonOffset; };',
        'inline constexpr Case cases[] = {']
    for row in rows:
        lines.append('    {"%s", %d, 0x%016xULL, 0x%016xULL, %dULL, %dULL},' % (
            row['id'],row['source_mand_id'],row['scene_fnv1a64'],row['expected_fnv1a64'],row['stored_pixel_bytes'],row['addon_offset']))
    lines+=['};','}']
    (ROOT/'tests/fixtures/legacy_loader_cases.h').write_text('\n'.join(lines)+'\n')
    print('Vendored',len(rows),'independent original cases')

if __name__=='__main__': main()
