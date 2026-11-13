"""Check the packed MC codec and noise calculation against extracted Pascal.

Packing must match exactly and preserve adjacent bytes. Decoding and variance
use a fixed 2e-12 relative/absolute tolerance; bit mismatches are also reported.
This does not certify progressive scheduling, file resume, or full MC images.
"""
import argparse
import math
import random
import re
import struct
import subprocess
from pathlib import Path

from check_mc_sampling import extract
from external_validation import ROOT, digest, write_json


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--pascal-work-root', type=Path, required=True)
    ap.add_argument('--cpp', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--fpc', default='fpc')
    ap.add_argument('--exhaustive-decode', action='store_true')
    a = ap.parse_args()
    work, out = a.pascal_work_root.resolve(), a.output.resolve()
    immutable = (ROOT/'mb3d').resolve()
    if work == immutable or immutable in work.parents:
        raise ValueError('Immutable Pascal tree')
    out.mkdir(parents=True, exist_ok=False)
    divpath = ROOT/'mb3d/DivUtils.pas'
    typepath = ROOT/'mb3d/TypeDefinitions.pas'
    mathpath = ROOT/'mb3d/Math3D.pas'
    div = divpath.read_text(encoding='latin1')
    types = typepath.read_text(encoding='latin1')
    maths = mathpath.read_text(encoding='latin1')
    snippets = {
        'codec': extract(div, 'function MCRGBtoDouble(', ' { TMCrecord ='),
        'noise': extract(div, 'function NoiseOfMCRGB(', 'procedure SVecToMCRGB('),
        'clamp': extract(maths, 'function Min0MaxCD(', 'function MinMaxCD('),
        'maximum': extract(maths, 'function MaxCD(', 'function MaxAbsCD('),
    }
    constant = re.findall(r'\benat\s*:\s*Double\s*=\s*[^;]+;', types)
    if len(constant) != 1: raise ValueError('enat source changed')
    record = types[types.index('  TMCrecord = packed record'):types.index('  TMCrecordNew =')]
    source = '''program mc_codec_probe_generated;
{$mode delphi}
uses Math, SysUtils;
type TRGB = array[0..2] of Byte; TPRGB = ^TRGB;
''' + record + '\nconst ' + constant[0] + '\n' + '\n'.join(
        snippets[k] for k in ['clamp', 'maximum', 'codec', 'noise']) + '''
var value,decoded,noise: Double; packedValue,encoded,count,base,j: Cardinal;
    guard: array[0..4] of Byte; rec: TMCrecord;
    block: array[0..4095] of Double; outputFile: File;
begin
  if (ParamCount=2) and (ParamStr(1)='--decode-file') then begin
    AssignFile(outputFile,ParamStr(2)); Rewrite(outputFile,1);
    base := 0;
    while base < $1000000 do begin
      for j := 0 to 4095 do begin
        packedValue := base+j;
        block[j] := MCRGBtoDouble(TPRGB(@packedValue));
      end;
      BlockWrite(outputFile,block,SizeOf(block));
      Inc(base,4096);
    end;
    CloseFile(outputFile); Halt(0);
  end;
  while not EOF(Input) do begin
    ReadLn(value,packedValue,count);
    if (packedValue > $FFFFFF) or (count > 65535) then Halt(2);
    FillChar(rec,SizeOf(rec),0); FillChar(guard,SizeOf(guard),0);
    guard[0] := $A5; guard[4] := $5A;
    DoubleToMCRGBv(value,TPRGB(@guard[1]));
    encoded := PCardinal(@guard[1])^ and $FFFFFF;
    Move(packedValue,rec.Ysum,3); Move(guard[1],rec.Ysqr,3);
    rec.RayCount := count;
    decoded := MCRGBtoDouble(@rec.Ysum);
    noise := NoiseOfMCRGB(@rec);
    WriteLn(encoded,' ',PQWord(@decoded)^,' ',PQWord(@noise)^,' ',guard[0],' ',guard[4]);
  end;
end.
'''
    generated = work/'mc_codec_probe_generated.pas'
    if generated.exists() and not generated.read_text().startswith('program mc_codec_probe_generated;'):
        raise ValueError('Unrelated generated file')
    generated.write_text(source)
    command = [a.fpc, '-Mdelphi', '-O2', '-CfSSE2', '-FE'+str(out), '-FU'+str(out), str(generated)]
    p = subprocess.run(command, capture_output=True, timeout=60)
    (out/'build.log').write_bytes(p.stdout+p.stderr)
    if p.returncode: raise RuntimeError('Pascal build failed; see build.log')
    rng = random.Random(0x24563487)
    values = [-1e300,-2.,-1.,-.5,0.,.01,.3,1.,2.,3.,7.,400.,1e300]
    for v in list(values): values += [math.nextafter(v,-math.inf),math.nextafter(v,math.inf)]
    # Quantization midpoints and their adjacent Double values, on both branches.
    for code in [0,1,2,2097151,2097152,4194303,6291454,6291455,6291456,0xfffffe]:
        t = (code+.5)/2097151.875
        v = t-1 if t <= 3 else math.exp(t-2)-math.e+2
        values += [math.nextafter(v,-math.inf),v,math.nextafter(v,math.inf)]
    codes = [0,1,2,2097151,2097152,4194303,6291454,6291455,6291456,0xfffffe,0xffffff]
    rows = [(v,c,n) for v in values for c in codes for n in [0,1,2,4,65535]]
    rows += [(rng.uniform(-2,405),rng.randrange(1<<24),rng.choice([0,1,4,32,65535])) for _ in range(20000)]
    stdin = ''.join(f'{v:.17g} {c} {n}\n' for v,c,n in rows)
    (out/'inputs.txt').write_text(stdin)
    pascal = out/'mc_codec_probe_generated.exe'
    def run(exe, flags):
        p = subprocess.run([str(exe.resolve()),*flags],input=stdin,text=True,
                           capture_output=True,timeout=60,cwd=out)
        if p.returncode: raise RuntimeError(f'{exe} exited {p.returncode}: {p.stderr[:500]}')
        (out/(exe.stem+'.txt')).write_text(p.stdout)
        result = [[int(x) for x in line.split()] for line in p.stdout.splitlines()]
        if len(result)!=len(rows) or any(len(r)!=5 for r in result): raise ValueError('Incomplete output')
        return result
    expected,actual = run(pascal,[]),run(a.cpp,['--codec'])
    failures,bit_mismatches,examples = [0]*5,[0]*5,[]
    maximum_absolute = [0.,0.]
    def double(bits): return struct.unpack('<d',struct.pack('<Q',bits))[0]
    for i,(ref,cpp) in enumerate(zip(expected,actual)):
        for lane,(r,c) in enumerate(zip(ref,cpp)):
            bit_mismatches[lane] += r != c
            ok = r == c
            if lane in [1,2]:
                rd,cd = double(r),double(c)
                maximum_absolute[lane-1] = max(maximum_absolute[lane-1],abs(rd-cd))
                ok = math.isfinite(rd) and math.isfinite(cd) and math.isclose(rd,cd,abs_tol=2e-12,rel_tol=2e-12)
            if not ok:
                failures[lane] += 1
                if len(examples)<30: examples.append(dict(row=i,input=rows[i],lane=lane,reference=r,cpp=c))
    report = dict(scope=__doc__,rows=len(rows),passed=not any(failures),
        lanes=['packed24','decoded_double_bits','noise_double_bits','guard_before','guard_after'],
        failures=failures,bit_mismatches=bit_mismatches,maximum_absolute=maximum_absolute,
        tolerances=dict(abs=2e-12,rel=2e-12),examples=examples,command=command,
        sources={str(p):digest(p.read_bytes()) for p in [divpath,typepath,mathpath,generated,Path(__file__),
            ROOT/'src/core/calc_monte_carlo.cpp',ROOT/'src/core/calc_monte_carlo.h',ROOT/'comparison/tools/mc_sampling_probe.cpp']},
        extracted={k:digest(v.encode()) for k,v in snippets.items()},
        binaries={str(p):digest(p.read_bytes()) for p in [pascal,a.cpp]},inputs_sha256=digest(stdin.encode()))
    if a.exhaustive_decode:
        import numpy as np
        paths = [out/'pascal-decode.bin',out/'cpp-decode.bin']
        for exe,path in zip([pascal,a.cpp],paths):
            p = subprocess.run([str(exe.resolve()),'--decode-file',str(path)],
                               capture_output=True,timeout=60,cwd=out)
            if p.returncode: raise RuntimeError(f'Exhaustive decode exit {p.returncode}')
            if path.stat().st_size != (1<<24)*8: raise ValueError('Incomplete exhaustive output')
        ref,cpp = [np.memmap(p,dtype='<f8',mode='r') for p in paths]
        bits,errors,max_abs,max_scaled,first = 0,0,0.,0.,[]
        for start in range(0,1<<24,65536):
            r,c = ref[start:start+65536],cpp[start:start+65536]
            delta = np.abs(r-c)
            bad = (~np.isfinite(r)) | (~np.isfinite(c)) | (delta > np.maximum(2e-12,2e-12*np.maximum(np.abs(r),np.abs(c))))
            bits += int(np.count_nonzero(r.view('<u8')!=c.view('<u8')))
            errors += int(np.count_nonzero(bad))
            max_abs = max(max_abs,float(delta.max()))
            max_scaled = max(max_scaled,float((delta/np.maximum(1.,np.maximum(np.abs(r),np.abs(c)))).max()))
            if len(first)<20: first.extend((start+np.flatnonzero(bad)[:20-len(first)]).tolist())
        report['exhaustive_decode'] = dict(codes=1<<24,failures=errors,bit_mismatches=bits,
            maximum_absolute=max_abs,maximum_scaled=max_scaled,first_failed_codes=first,
            outputs={str(p):digest(p.read_bytes()) for p in paths})
        report['passed'] = report['passed'] and errors==0
    write_json(out/'summary.json',report)
    print({k:report[k] for k in ['rows','passed','failures','bit_mismatches','exhaustive_decode'] if k in report})
    return int(not report['passed'])


if __name__ == '__main__': raise SystemExit(main())
