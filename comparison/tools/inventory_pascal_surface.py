"""Inventory immutable Pascal sources and form events, without claiming coverage.

Lexical declarations include interface declarations, implementations, nested
routines and conditional alternatives. They are not deduplicated into callable
functions and are not a branch-coverage denominator. Every file is retained,
including files with no recognizable declarations and binary form resources.
"""
import argparse
from collections import Counter
import csv
import re
from pathlib import Path
from external_validation import ROOT, digest, write_json


def mask_comments_strings(source):
    # Preserve offsets/newlines so reported source locations remain reviewable.
    token = re.compile(r"'([^']|'')*'|//[^\r\n]*|\{.*?\}|\(\*.*?\*\)", re.S)
    return token.sub(lambda m: ''.join('\n' if c=='\n' else ' ' for c in m[0]), source)


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True)
    a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=True)
    files=[];declarations=[];events=[]
    declaration=re.compile(r'\b(procedure|function|constructor|destructor|operator)\s+([A-Za-z_][\w.]*)',re.I)
    for path in sorted((ROOT/'mb3d').rglob('*')):
        if not path.is_file() or path.suffix.lower() not in ('.pas','.pp','.inc','.dpr','.lpr','.dpk','.dfm','.lfm'):continue
        raw=path.read_bytes();source=raw.decode('utf-8-sig',errors='replace')
        relative=path.relative_to(ROOT).as_posix();binary=raw.startswith(b'TPF0') or b'\0' in raw
        item={'path':relative,'sha256':digest(raw),'bytes':len(raw),'status':'unverified','binary':binary,'declarations':0,'events':0}
        if not binary and path.suffix.lower() in ('.pas','.pp','.inc','.dpr','.lpr','.dpk'):
            masked=mask_comments_strings(source)
            for match in declaration.finditer(masked):
                declarations.append({'file':relative,'line':source.count('\n',0,match.start())+1,'kind':match[1].lower(),'name':match[2],'status':'unverified'})
                item['declarations']+=1
            # Inventory compiler alternatives; no assumptions about active build.
            item['conditional_directives']=len(re.findall(r'\{\$\s*(?:IFDEF|IFNDEF|IF|ELSEIF|ELSE)\b',source,re.I))
            item['asm_blocks']=len(re.findall(r'\basm\b',masked,re.I))
        elif not binary:
            for match in re.finditer(r'^\s*(On\w+)\s*=\s*(\w+)',source,re.M):
                events.append({'file':relative,'line':source.count('\n',0,match.start())+1,'event':match[1],'handler':match[2],'status':'unverified'})
                item['events']+=1
        files.append(item)
    # Cross-cutting obligations: explicit even when there is finite evidence.
    obligations=[
        ('External formula execution','partial','460 names registered; finite one-step and image evidence in coverage.json; failures remain'),
        ('Formula expansion and initialization','partial','Point oracle shares C++ expansion; independent Pascal expansion still required'),
        ('Maps and pointer-bearing folding','unverified','Dedicated callback/address-space oracle required'),
        ('Arbitrary formula loading and JIT compiler/editor','unsupported','Three shipped JIT programs have static translations only'),
        ('Hybrid order, repetition and DE combinations','partial','Finite scenes and conditional-counter probes; arbitrary combinations unverified'),
        ('Monte Carlo fixed-sample paths','partial','Native implementation; latest finite image counts and frozen binary identities are in coverage.json under monte_carlo. Image PASS does not certify every transport routine or branch'),
        ('Monte Carlo adaptive CalcN scheduling','partial','CalcN decision kernel independently verified on 15,612 buffers; full scheduler is not ported. Original neighbor-counter reads can exceed the image; see mc-adaptive-kernel.json'),
        ('Monte Carlo packed progressive state/resume','partial','Packed codec verified on 23,795 inputs and all 16,777,216 decode codes; fixed-sample renderer still does not resume Pascal accumulators. See mc-codec.json'),
        ('Monte Carlo GUI noise filter','unsupported','No GUI filter parity evidence'),
        ('Monte Carlo C API entry point','unsupported','Available through native C++ and CLI only'),
        ('Ordinary headless rendering and effects','partial','343-scene finite replay; not every Pascal routine or branch'),
        ('GUI, animation, batch UI and parameter editors','unverified','Form events are inventoried; headless image tests do not verify UI behavior'),
        ('Mesh, voxel, heightmap and depth export','unverified','Source inventory includes exporters; no complete parity evidence'),
        ('File formats, migrations, compression and persistence','partial','Parser regression tests exist; all Pascal formats/legacy versions not certified'),
        ('GPU renderer and platform-specific paths','unverified','Source presence does not imply C++ implementation or validation'),
    ]
    totals={'files':len(files),'extensions':dict(Counter(Path(f['path']).suffix.lower() for f in files)),
            'declarations':len(declarations),'form_events':len(events),'binary_resources':sum(f['binary'] for f in files),
            'conditional_directives':sum(f.get('conditional_directives',0) for f in files),'asm_blocks':sum(f.get('asm_blocks',0) for f in files),'complete':False}
    write_json(a.output/'pascal-surface.json',{'scope':__doc__,'totals':totals,'files':files,'declarations':declarations,'form_events':events,'obligations':[{'surface':s,'status':st,'evidence_or_gap':e} for s,st,e in obligations]})
    with (a.output/'pascal-declarations.csv').open('w',newline='',encoding='utf-8-sig') as out:
        w=csv.DictWriter(out,fieldnames=['file','line','kind','name','status']);w.writeheader();w.writerows(declarations)
    lines=['# Pascal feature inventory','','Status: INCOMPLETE. This is a source inventory, not executed line/branch coverage. All declaration rows remain unverified until explicit trace or test mapping exists.','','Totals: '+str(totals),'','Every Pascal source/include/project/form resource under mb3d is hashed. Commented declarations are excluded. Interface/implementation duplicates and conditional alternatives remain visible; binary forms are retained as unparsed resources. The feature grouping below is a review checklist, not an exhaustive semantic classification.','','| Surface | Status | Evidence / remaining work |','|---|---|---|']
    lines += [f'| {s} | {st} | {e} |' for s,st,e in obligations]
    (a.output/'pascal-surface.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')
    print(totals)


if __name__=='__main__':main()
