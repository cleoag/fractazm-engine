"""Verify smooth iterations against executable assembly extracted from Pascal.

Inputs cover high-word branch boundaries, adjacent radii (cancellation), extreme
iteration counts and nonfinite values. This is finite native x87 coverage.
"""
import argparse
import json
import math
import re
import struct
import subprocess
from pathlib import Path
from external_validation import ROOT, digest, write_json


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--pascal-work-root', type=Path, required=True)
    ap.add_argument('--cpp', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--header', type=Path)
    ap.add_argument('--fpc', default='fpc')
    a = ap.parse_args()
    work, out = a.pascal_work_root.resolve(), a.output.resolve()
    immutable = (ROOT/'mb3d').resolve()
    if work == immutable or immutable in work.parents:
        raise ValueError('Immutable reference tree')
    out.mkdir(parents=True, exist_ok=False)
    paths = [immutable/'formulas.pas', immutable/'TypeDefinitions.pas']
    source, definitions = [p.read_text(encoding='latin1') for p in paths]
    start = source.rindex('procedure CalcSmoothIterations(')
    start = source.index('\nasm', start)+4
    block = re.sub(r'//[^\n]*', '', source[start:source.index('\nend;', start)])
    block = re.sub(r'add\s+eax,\s*\$34', '', block)
    mappings = {
        'TIteration3D.Rout+4-$34': '4',
        'TIteration3D.Rout-$34': '0',
        'TIteration3Dext.Rold-56+4-$34': '12',
        'TIteration3Dext.Rold-56-$34': '8',
        'TIteration3D.ItResultI-$34': '24',
        'TIteration3D.SmoothItD-$34': '28',
        'TIteration3D.LNRStop-$34': '16',
        'edx*4+TIteration3D.fHln-$34': '20',
    }
    def address(match):
        value = re.sub(r'\s+', '', match[1])
        if not value.startswith('eax+') or value[4:] not in mappings:
            raise ValueError('Unknown reference address: '+value)
        return '[eax+'+mappings[value[4:]]+']'
    block = re.sub(r'\[([^\]]+)\]', address, block)
    constants = []
    for name in ('s05', 'd1em100'):
        match = re.search(r'\b'+name+r'\s*:\s*(Single|Double)\s*=\s*([^;]+);', definitions, re.I)
        if not match:
            raise ValueError('Missing reference constant: '+name)
        constants.append(name+':'+match[1]+'='+match[2]+';')
    generated = work/'smooth_iterations_probe_generated.pas'
    pas = '''program smooth_iterations_probe_generated;
{$mode delphi}
uses Math;
const '''+'\n'.join(constants)+'''
type TValues=packed record radius,oldRadius:QWord; logStop,power:LongWord;
iterations:LongInt; result:Single; end;
procedure Smooth(p:Pointer); assembler;
asm
'''+block+'''
end;
var data:TValues;
begin
Set8087CW($137f);
while not EOF(Input) do begin
ReadLn(data.radius,data.oldRadius,data.logStop,data.power,data.iterations);
Smooth(@data);
WriteLn(PLongWord(@data.result)^);
end; end.
'''
    if generated.exists() and not generated.read_text().startswith('program smooth_iterations_probe_generated;'):
        raise ValueError('Unrelated source')
    generated.write_text(pas)
    (out/'reference.pas').write_text(pas)
    cmd = [a.fpc,'-Mdelphi','-O2','-CfSSE2','-al','-FE'+str(out),'-FU'+str(out),str(generated)]
    p = subprocess.run(cmd,capture_output=True,timeout=60)
    (out/'build.log').write_bytes(p.stdout+p.stderr)
    if p.returncode:
        raise RuntimeError('Pascal compilation failed')
    db = lambda v: struct.unpack('<Q', struct.pack('<d',v))[0]
    fb = lambda v: struct.unpack('<I', struct.pack('<f',v))[0]
    radius_bits = [db(v) for v in (-math.inf,-2.,-1.,-0.,0.,.5,1.,2.,256.,1e300,math.inf)]
    radius_bits += [db(1.)+d for d in (-1,1,(1<<32)-1,1<<32,(1<<32)+1)]
    radius_bits += [0x7ff8000000000001,0xfff8000000000001]
    rows = []
    for r in radius_bits:
        for old in radius_bits:
            rows.append([r,old,fb(.7),fb(.5),60])
    for r in (1.1,2.,2.75209,4.,256.,1e30):
        for delta in (-2,-1,0,1,2):
            for iteration in (-2147483648,-1,0,60,2147483647):
                for stop,power in ((0.,0.),(.7,.5),(-1.,2.),(1e30,-1.)):
                    rows.append([db(r),db(r)+delta,fb(stop),fb(power),iteration])
    stdin = ''.join(' '.join(map(str,row))+'\n' for row in rows)
    (out/'inputs.txt').write_text(stdin)
    pascal = out/'smooth_iterations_probe_generated.exe'
    results = []
    for side,exe,flags in [('pascal',pascal,[]),('cpp',a.cpp,['--smooth-iterations'])]:
        p = subprocess.run([str(exe.resolve()),*flags],input=stdin,text=True,capture_output=True,timeout=30)
        if p.returncode:
            raise RuntimeError(side+': '+p.stderr)
        (out/(side+'.txt')).write_text(p.stdout)
        values = [int(v) for v in p.stdout.splitlines()]
        if len(values)!=len(rows):
            raise ValueError('Incomplete '+side+' output')
        results.append(values)
    failures = [dict(row=i,input=rows[i],expected=e,actual=v)
                for i,(e,v) in enumerate(zip(*results)) if e!=v]
    write_json(out/'golden.json',[dict(input=x,expected=y) for x,y in zip(rows,results[0])])
    summary = dict(scope=__doc__,finished=True,passed=not failures,rows=len(rows),
        failures=len(failures),examples=failures[:20],build_command=cmd,
        cpp_sha256=digest(a.cpp.read_bytes()),pascal_sha256=digest(pascal.read_bytes()),
        source_sha256={str(p):digest(p.read_bytes()) for p in paths+[generated,Path(__file__).resolve()]})
    write_json(out/'summary.json',summary)
    if a.header:
        text = '// Extracted Pascal CalcSmoothIterations; see check_smooth_iterations.py.\n'
        text += 'struct SmoothIterationReference { uint64_t radius,oldRadius; uint32_t logStop,power; int32_t iterations; uint32_t result; };\n'
        text += 'static constexpr SmoothIterationReference smoothIterationReferences[] = {\n'
        text += ''.join('{'+f'{r}ULL,{old}ULL,{stop}U,{power}U,{it},{expected}U'+'},\n'
                        for (r,old,stop,power,it),expected in zip(rows,results[0]))
        a.header.write_text(text+'};\n')
    print(json.dumps({k:summary[k] for k in ('rows','failures','passed','examples')}))
    return bool(failures)


if __name__=='__main__':
    raise SystemExit(main())
