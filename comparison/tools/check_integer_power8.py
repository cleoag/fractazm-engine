"""Verify builtin power 8 against executable assembly extracted from Pascal.

Inputs cover singular, cancellation, seeded and extreme coordinates, Julia C
and Z multiplier. This is finite native x87 coverage.
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
    start = source.rindex('procedure HybridIntP8(')
    start = source.index('\nasm', start)+4
    block = re.sub(r'//[^\n]*', '', source[start:source.index('\nend;', start)])
    block = '\n'.join(l for l in block.splitlines() if not re.match(r'\s*(push|pop|mov|add)\b',l))
    mappings = {'eax':0,'edx':8,'ecx':16,'esi+24':24,'esi+32':32,'esi+40':40,
                'edi-104':48,'edi+56':56,'edi+64':64,'edi+72':72,'edi+104':80,
                'edi+120':88,'edi-56':96,'edi-64':104}
    def address(match):
        value = re.sub(r'\s+', '', match[1])
        if value not in mappings:
            raise ValueError('Unknown reference address: '+value)
        return '[eax+'+str(mappings[value])+']'
    block = re.sub(r'\[([^\]]+)\]', address, block)
    generated = work/'integer_power8_probe_generated.pas'
    pas = '''program integer_power8_probe_generated;
{$mode delphi}
uses Math;
type TValues=array[0..13] of QWord;
procedure Power8(p:Pointer); assembler;
asm
'''+block+'''
end;
var data:TValues; i:Integer;
begin
Set8087CW($137f);
while not EOF(Input) do begin
for i:=0 to 12 do Read(data[i]); ReadLn(data[13]);
Power8(@data);
WriteLn(data[0],' ',data[1],' ',data[2]);
end; end.
'''
    if generated.exists() and not generated.read_text().startswith('program integer_power8_probe_generated;'):
        raise ValueError('Unrelated source')
    generated.write_text(pas)
    (out/'reference.pas').write_text(pas)
    cmd = [a.fpc,'-Mdelphi','-O2','-CfSSE2','-al','-FE'+str(out),'-FU'+str(out),str(generated)]
    p = subprocess.run(cmd,capture_output=True,timeout=60)
    (out/'build.log').write_bytes(p.stdout+p.stderr)
    if p.returncode:
        raise RuntimeError('Pascal compilation failed')
    db = lambda v: struct.unpack('<Q', struct.pack('<d',v))[0]
    import random
    from check_formula_scene_points import orbit_scale_coordinates, nonfinite_coordinates
    rng = random.Random(0x5038)
    coordinates = [r[:3] for r in orbit_scale_coordinates()+nonfinite_coordinates()]
    coordinates += [[0.,0.,0.], [1.,1.,1.], [1.,math.nextafter(1.,2.),-1.]]
    coordinates += [[rng.uniform(-2,2) for _ in range(3)] for _ in range(128)]
    rows = []
    for xyz in coordinates:
        for julia,mul in (([0.,0.,0.],-1.),([.2,-.3,.4],1.),([-1.,2.,-3.],0.)):
            rows.append([db(v) for v in xyz+julia+[mul,6.,7.,8.,28.,70.,1.,1e-40]])
    stdin = ''.join(' '.join(map(str,row))+'\n' for row in rows)
    (out/'inputs.txt').write_text(stdin)
    pascal = out/'integer_power8_probe_generated.exe'
    results = []
    for side,exe,flags in [('pascal',pascal,[]),('cpp',a.cpp,['--integer-power8'])]:
        p = subprocess.run([str(exe.resolve()),*flags],input=stdin,text=True,capture_output=True,timeout=30)
        if p.returncode:
            raise RuntimeError(side+': '+p.stderr)
        (out/(side+'.txt')).write_text(p.stdout)
        values = [[int(w) for w in v.split()] for v in p.stdout.splitlines()]
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
        text = '// Extracted Pascal HybridIntP8; see check_integer_power8.py.\n'
        text += 'struct IntegerPower8Reference { uint64_t input[14], expected[3]; };\n'
        text += 'static constexpr IntegerPower8Reference integerPower8References[] = {\n'
        text += ''.join('{{'+','.join(str(v)+'ULL' for v in row)+'},{'+
                        ','.join(str(v)+'ULL' for v in expected)+'}},\n'
                        for row,expected in zip(rows,results[0]))
        a.header.write_text(text+'};\n')
    print(json.dumps({k:summary[k] for k in ('rows','failures','passed','examples')}))
    return bool(failures)


if __name__=='__main__':
    raise SystemExit(main())
