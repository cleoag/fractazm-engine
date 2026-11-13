"""Bounded original-CODE witnesses for the fixed 24 original-reference gaps.

No unguarded full application is run on a known hanging input. The standalone
i386 program extracts immutable Pascal dispatch/types/math and adds observation
hooks and a hard 512-call/32-scan stop. An optional, explicitly logged counter
fast-forward is a source-invariant witness, never a numerical reference image.
"""
from __future__ import annotations
import argparse
import copy
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

from external_validation import ROOT, decode, slots, write_json

BASE = ROOT / 'comparison/runs/compatibility-final-20261018'
WORK = Path('C:/work/_c/mb3d_pascal_work/mb3d_work/_reference_gaps_nontermination')
FPC = Path('C:/FPC/3.2.2/bin/i386-Win32/fpc.exe')
SUFFIXES = {
 'TrifoxComplexAngles': ('p0_zero','p3_zero'),
 '_CondItcountR': ('early','julia','once','p1_alternate','p1_zero','p4_alternate','p4_zero','p5_alternate','p5_zero'),
 '_CondItcountSwX2': ('once','p4_zero'),
 '_CondItcountSwitch': ('early','julia','once','p0_zero','p2_alternate','p2_zero','p3_alternate'),
 '_CondLyapunov': ('once','p0_zero'),
 '_CondLyapunovEx': ('once','p0_zero'),
}
CASE_IDS = frozenset('challenge_'+n+'_'+s for n, ss in SUFFIXES.items() for s in ss)
ASSET_SHA = {
 'TrifoxComplexAngles':'93027fbc697a22022763f5b86a832684b37ef7aa1d7fbb45b3d8f1772ee18a35',
 '_CondItcountR':'8246faa30b8cb381db3dbd21df3e73d7730bbdbdefb4ea43b12fc672b734ad9c',
 '_CondItcountSwX2':'9d37c0190ee52918e25c9daf87397b5d44af85af0460a9c92bc46f1ee8394b6a',
 '_CondItcountSwitch':'1277be3d35c9c15509910c024b28aa5b34b1d37e8c9d0fe0b4b95db0689271e3',
 '_CondLyapunov':'7b9326eedbae74e83e3bf2f76309aa90165b302186c4bfbdfce9a68d2a5ab395',
 '_CondLyapunovEx':'aa0c9c8c9c1bf2c8c0e7065f8c0a1c938dfa4fcb82877aad6be55731fa185149',
}


def sha(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def require(b, s):
    if not b: raise ValueError(s)
def ref(p):
    p = Path(p).resolve()
    return dict(path=str(p), sha256=sha(p))
def checked(p, h):
    p = Path(p); require(sha(p) == h, 'Changed retained artifact: '+str(p)); return p
def extract(p, start, end):
    text = p.read_text(encoding='latin1')
    a = text.index(start, text.index('implementation') if 'implementation' in text else 0)
    b = text.index(end, a)
    return text[a:b].rstrip()+'\n'


HOOKS = r'''
var Calls, Scans, Seed, Accelerate, Jumped: Integer;
procedure Emit(const Stage: string; N: Integer; P: TPIteration3D);
var I: Integer; T: TPIteration3Dext;
begin
  T:=TPIteration3Dext(PtrUInt(P)-56);
  Write(Stage,'|',Calls,'|',Scans,'|',N,'|',T.bFirstIt,'|',T.ItResultI,'|',T.bTmp,
    '|',IntToHex(PInt64(@T.Rout)^,16),'|',IntToHex(PInt64(@T.x)^,16),
    '|',IntToHex(PInt64(@T.y)^,16),'|',IntToHex(PInt64(@T.z)^,16));
  for I:=0 to 5 do Write('|',T.nHybrid[I]);
  WriteLn;
end;
procedure Observe(const Stage: string; N: Integer; P: TPIteration3D);
begin
  if Stage='before' then begin
    if Calls>=512 then raise Exception.Create('bounded_call_stop');
    if (Accelerate=1) and (Jumped=0) and (Calls=64) then begin
      Emit('jump_from',N,P); P.bFirstIt:=2147483647; Jumped:=1; Emit('jump_to',N,P);
    end;
    Inc(Calls); Scans:=0;
  end;
  if Stage='scan' then begin
    Inc(Scans);
    if Scans>32 then raise Exception.Create('bounded_scan_stop');
  end;
  Emit(Stage,N,P);
end;
procedure CalcSmoothIterations(P: TPIteration3D; N: Integer);
begin raise Exception.Create('unexpected_smoothing'); end;
'''

MAIN = r'''
var CodePtr: Pointer;
    It: TIteration3Dext;
    Params, Partner: array[0..1023] of Byte;
    S: TFileStream;
    I, EndSlot, RepeatSlot, Power, JuliaFlag: Integer;
    V: Double;
begin
  SetErrorMode(SEM_FAILCRITICALERRORS or SEM_NOGPFAULTERRORBOX);
  DefaultFormatSettings.DecimalSeparator:='.';
  SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
  Set8087CW($133F);
  if ParamCount<>4 then Halt(2);
  S:=TFileStream.Create(ParamStr(1),fmOpenRead);
  CodePtr:=VirtualAlloc(nil,4096,MEM_COMMIT or MEM_RESERVE,PAGE_EXECUTE_READWRITE);
  if (S.Size>4096) or (CodePtr=nil) then Halt(3);
  S.ReadBuffer(CodePtr^,S.Size);S.Free;
  S:=TFileStream.Create(ParamStr(2),fmOpenRead);S.ReadBuffer(Params,SizeOf(Params));S.Free;
  S:=TFileStream.Create(ParamStr(3),fmOpenRead);S.ReadBuffer(Partner,SizeOf(Partner));S.Free;
  FillChar(It,SizeOf(It),0);
  ReadLn(EndSlot,RepeatSlot,Power,Seed,Accelerate,It.maxIt,JuliaFlag);
  ReadLn(It.C1,It.C2,It.C3,It.Ju1,It.Ju2,It.Ju3,It.RStop);
  for I:=0 to 5 do Read(It.nHybrid[I]); ReadLn;
  It.EndTo:=EndSlot;It.iRepeatFrom:=RepeatSlot;It.DoJulia:=JuliaFlag<>0;
  for I:=0 to 5 do begin
    It.fHPVar[I]:=@Partner[512];
    if Power=8 then It.fHybrid[I]:=ThybridIteration2(@HybridIntP8)
      else It.fHybrid[I]:=ThybridIteration2(@HybridItIntPow2);
  end;
  It.fHybrid[0]:=ThybridIteration2(CodePtr);It.fHPVar[0]:=@Params[512];
  try
    if ParamStr(4)='single' then begin
      It.x:=It.C1;It.y:=It.C2;It.z:=It.C3;
      It.J1:=It.C1;It.J2:=It.C2;It.J3:=It.C3;
      It.Rout:=It.x*It.x+It.y*It.y+It.z*It.z;
      It.PVar:=@Params[512];It.bFirstIt:=Seed;
      Observe('before',0,TPIteration3D(@It.C1));
      It.fHybrid[0](It.x,It.y,It.z,It.w,@It.C1);
      Observe('after',0,TPIteration3D(@It.C1));
    end else doHybridPas(TPIteration3D(@It.C1));
    Emit('returned',-1,TPIteration3D(@It.C1));
  except
    on E: Exception do begin WriteLn('STOP|',E.Message);Emit('stopped',-1,TPIteration3D(@It.C1));end;
  end;
end.
'''


def build(output):
    WORK.mkdir(exist_ok=True)
    generated=WORK/output.name;generated.mkdir(exist_ok=True)
    require(not (generated/'nontermination_witness.exe').exists(), 'Use a fresh witness workspace')
    types = ROOT/'mb3d/TypeDefinitions.pas'
    t=types.read_text(encoding='latin1')
    t=t[t.index('  ThybridIteration2 ='):t.index('  ThybridIteration =')]
    formulas=ROOT/'mb3d/formulas.pas'
    driver=extract(formulas,'procedure doHybridPas(', 'function doHybrid4DDEPas(')
    # Only these explicit observation substitutions alter the extracted driver.
    observed=driver.replace('bFirstIt  := 0;', 'bFirstIt  := Seed;')
    observed=observed.replace('          Inc(n);', "          Observe('scan',n,PIteration3D);\n          Inc(n);")
    call='        fHybrid[n](x, y, z, w, PIteration3D);'
    require(observed.count(call)==1,'Driver extraction changed')
    observed=observed.replace(call,"        Observe('before',n,PIteration3D);\n"+call+"\n        Observe('after',n,PIteration3D);")
    source=('program nontermination_witness;\n{$mode delphi}{$asmmode intel}\n'
            'uses Windows,SysUtils,Classes,Math;\ntype TVec3D=array[0..2] of Double; TPVec3D=^TVec3D;\n'
            'TSMatrix4=array[0..3,0..3] of Single;\n'+t+
            extract(ROOT/'mb3d/Math3D.pas','procedure mCopyVec(', 'procedure CopyVecSSE2(')+HOOKS+
            extract(formulas,'procedure HybridItIntPow2(', 'procedure HybridItIntPow2SSE2(')+
            extract(formulas,'procedure HybridIntP8(', 'procedure HybridCubeSSE2(')+observed+MAIN)
    pas=generated/'nontermination_witness.pas';pas.write_text(source,encoding='latin1')
    (output/'original-doHybridPas.pas.txt').write_text(driver)
    (output/'observed-doHybridPas.pas.txt').write_text(observed)
    cmd=[str(FPC),'-B','-Mdelphi','-CfSSE2','-O2','-FU'+str(generated),'-FE'+str(generated),str(pas)]
    r=subprocess.run(cmd,cwd=generated,capture_output=True,text=True,timeout=30)
    (output/'build.log').write_text(r.stdout+r.stderr)
    require(r.returncode==0,'Standalone witness compilation failed; see build.log')
    return generated/'nontermination_witness.exe',dict(command=cmd,source=ref(pas),binary=ref(generated/'nontermination_witness.exe'),
       immutable_sources=[ref(p) for p in (types,formulas,ROOT/'mb3d/Math3D.pas',ROOT/'mb3d/CustomFormulas.pas',ROOT/'mb3d/DivUtils.pas')],
       modifications=['bounded observation hooks only','optional initial bFirstIt seed','explicit optional stable-counter fast-forward; no reference RGB claim'])


def parameters(slot):
    """Independent subset of Pascal FillCustomVBufWithVars for these six assets."""
    name=slot['name']; vals=slot['options']; buf=bytearray(1024);struct.pack_into('<d',buf,504,.5)
    if name=='_CondItcountR':
        struct.pack_into('<2d',buf,488,vals[1]*vals[1],vals[0]*vals[0])
        for i,v in enumerate(vals[2:6]):struct.pack_into('<i',buf,484-4*i,round(v))
    elif name.startswith('_Cond'):
        for i,v in enumerate(vals[:slot['option_count']]):struct.pack_into('<i',buf,500-4*i,round(v))
    elif name=='TrifoxComplexAngles':
        for off,v in zip((-12,-24,-28,-32),(vals[0],vals[2],vals[3],vals[4])):struct.pack_into('<i',buf,512+off,round(v))
        for off,v in zip((-16,-20,-36,-40),(vals[1],1/vals[1],vals[5],vals[6])):struct.pack_into('<f',buf,512+off,v)
    else:raise ValueError(name)
    return buf


def partner_buffer(zmul):
    buf=bytearray(1024);struct.pack_into('<d',buf,496,zmul);struct.pack_into('<d',buf,504,.5)
    # Exact independent original DivUtils.Initconstants PAligned16 literals.
    for off,v in {16:-2.,24:1e-100,32:1.,40:1.,48:-1.,56:-1.,64:2.,72:2.,96:-1.,104:2.,112:.5,120:3.,128:4.,136:5.,144:6.,152:7.,160:8.,168:10.,176:15.,184:21.,192:28.,200:35.,208:70.}.items():struct.pack_into('<d',buf,512+off,v)
    for off,v in ((0,0x7fffffffffffffff),(8,0x7fffffffffffffff),(80,0x8000000000000000),(88,0x8000000000000000)):struct.pack_into('<Q',buf,512+off,v)
    return buf


def parse_trace(text):
    rows=[];stop=None
    for line in text.splitlines():
        a=line.split('|')
        if a[0]=='STOP':stop=a[1];continue
        require(len(a)==17,'Malformed witness trace: '+line)
        rows.append(dict(stage=a[0],calls=int(a[1]),scans=int(a[2]),slot=int(a[3]),first=int(a[4]),counted=int(a[5]),remaining=int(a[6]),
                         radius_bits=a[7],xyz_bits=a[8:11],counts=list(map(int,a[11:]))))
    return dict(rows=rows,stop=stop,returned=bool(rows and rows[-1]['stage']=='returned'))


def run(exe,output,label,scene,point=(.1,.2,.3),seed=0,accelerate=0,single=False):
    d=decode(scene);ss=slots(d); s=ss[0];name=s['name']
    asset=ROOT/'mb3d/M3Formulas'/f'{name}.m3f';code=bytes.fromhex(asset.read_text().split('[CODE]')[1].split('[END]')[0])
    checked(asset,ASSET_SHA[name])
    codepath=output/(name+'.bin')
    if not codepath.exists():codepath.write_bytes(code)
    par=output/(label+'.params');par.write_bytes(parameters(s))
    pbuf=output/(label+'.partner');pbuf.write_bytes(partner_buffer(ss[1]['options'][1] if len(ss)>1 else -1.))
    counts=[0]*6
    for item in ss:counts[item['slot']]=item['iterations']
    if name.startswith('_Cond'):counts[0]=-2147483648|counts[0]
    end=max(x['slot'] for x in ss);repeat=min(end,d[845]>>4)
    maxit=struct.unpack_from('<i',d,12)[0];rstop=struct.unpack_from('<d',d,92)[0]
    julia=struct.unpack_from('<3d',d,191)
    inp=f'{end} {repeat} {int(ss[1]["options"][0]) if len(ss)>1 else 2} {seed} {accelerate} {maxit} {d[190]}\n'
    inp+=' '.join(map(str,(*point,*julia,rstop)))+'\n'+' '.join(map(str,counts))+'\n'
    (output/(label+'.input')).write_text(inp)
    cmd=[str(exe),str(codepath),str(par),str(pbuf),'single' if single else 'driver']
    r=subprocess.run(cmd,input=inp,capture_output=True,text=True,timeout=5,cwd=WORK)
    log=output/(label+'.trace');log.write_text(r.stdout)
    require(r.returncode==0 and not r.stderr, f'Witness process failed: {label}: {r.stderr}')
    result=parse_trace(r.stdout)
    result.update(scene=ref(scene),params=ref(par),code=ref(codepath),trace=ref(log),input=ref(output/(label+'.input')),
                  command=cmd,point=list(point),seed=seed,accelerated=bool(accelerate),initial_counts=counts,end=end,repeat=repeat,maxit=maxit)
    return result


def expected_conditional(name, opts, before):
    """Source-reviewed integer/radius branches; checked against unchanged CODE."""
    out=before['counts'][:];first=before['first'];selected=[]
    def signed(v):return ((v+2**31)%2**32)-2**31
    def put(slot,value):
        ix=max(1,min(6,round(slot)))-1
        out[ix]=signed(value | (0x80000000 if out[ix]<0 else 0))
    if name=='_CondItcountR':
        radius=struct.unpack('<d',bytes.fromhex(before['radius_bits'])[::-1])[0]
        branch=0 if radius<opts[0]**2 else 1 if radius<opts[1]**2 else 2
        put(opts[2],max(0,min(99999,round(opts[3+branch]))));selected.append(('radius',branch))
    else:
        first=signed(first+1)
        if name in ('_CondItcountSwitch','_CondItcountSwX2'):
            for base in ([0,5] if name=='_CondItcountSwX2' else [0]):
                cutoff=max(1,min(100,round(opts[base])))
                branch=0 if first<cutoff else 1 if first==cutoff else 2
                put(opts[base+1],max(0,min(100,round(opts[base+2+branch]))));selected.append(('switch'+str(base),branch))
        else:
            # Original CODE decrements masked count before shifting: bit0 is
            # selected for first&15 == 0 or1, bit14 for masked15.
            bit=(round(opts[0]) >> max(0,(first&15)-1))&1
            low=0 if name=='_CondLyapunov' else max(0,min(100,round(opts[3])))
            high=1 if name=='_CondLyapunov' else max(1,min(100,round(opts[4])))
            if bit==0:put(opts[1],low);put(opts[2],high)
            else:put(opts[2],low);put(opts[1],high)
            selected.append(('sequence',bit))
    return out,first,selected


def check_conditional_trace(witness,name,opts):
    rows=witness['rows'];selected=[]
    for i,r in enumerate(rows):
        if r['stage']!='after' or r['slot']!=0:continue
        before=rows[i-1]
        require(before['stage']=='before' and before['slot']==0,'Missing callback input')
        counts,first,branch=expected_conditional(name,opts,before)
        require(r['counts']==counts and r['first']==first,'Original conditional branch mismatch')
        require(r['xyz_bits']==before['xyz_bits'] and r['radius_bits']==before['radius_bits'],'Unexpected orbit mutation')
        selected.extend(branch)
    require(selected,'No original conditional callback reached')
    return sorted(set(selected))


def exact_rgb(a,b):
    from PIL import Image
    with Image.open(a) as aa, Image.open(b) as bb:
        return aa.size==bb.size and aa.convert('RGB').tobytes()==bb.convert('RGB').tobytes()


def invariant(case):
    n=case['formula'];cid=case['id']
    if n=='TrifoxComplexAngles':return 'zero_shift_fixed_point'
    if cid.endswith('_once'):return 'empty_repeat_range_excludes_start'
    if n=='_CondItcountR':return 'radius_fixed_point_disables_only_counted_slot'
    if n in ('_CondLyapunov','_CondLyapunovEx'):return 'zero_sequence_only_enables_out_of_range_slot'
    if n=='_CondItcountSwitch' and cid.endswith('_p2_zero'):return 'all_switch_branches_disable_only_counted_slot'
    return 'excessive_finite_signed_counter_wrap'


def validate(report):
    require(report.get('finished') is True,'Incomplete qualification')
    rows=report.get('cases',[])
    require(len(rows)==24 and len({x['id'] for x in rows})==24 and {x['id'] for x in rows}==CASE_IDS,'Wrong/duplicate case set')
    for c in rows:
        require(c['classification']==invariant(c),'Wrong source classification: '+c['id'])
        require(c.get('guarded_rejection_verified') is True,'Missing guarded behavioral evidence')
        require(c.get('replacement_original_rgb_exact') is True,'Missing finite original control')
        require(c.get('replacement_branch_reached') is True,'Unreached replacement branch')
        if c['classification']=='excessive_finite_signed_counter_wrap':
            require(c['proof']['wrap_returned'] is True and c['proof']['counter_wrap_observed'] is True,'Missing signed-wrap witness')
            ww=c['proof']['wrap_witness'];wr=ww['rows']
            require(ww['returned'] and any(x['stage']=='after' and x['first']==-2147483648 for x in wr),'Unobserved counter wrap')
            before=next(x for x in wr if x['stage']=='jump_from');after=next(x for x in wr if x['stage']=='jump_to')
            require(before['counts'][1]==0 and before['slot']==0 and before['first']>3 and after['first']==2147483647,'Invalid accelerated segment')
            require(all(before[k]==after[k] for k in before if k not in ('stage','first')),'Acceleration altered other state')
        elif c['classification']=='zero_shift_fixed_point':
            require(c['proof']['zero_preserved'] and c['proof']['carry_always_zero'] and c['proof']['back_edge_taken'],'Missing SHR fixed point')
        else:require(c['proof']['invariant_verified'] is True,'Missing source invariant')
        replacement=c['replacement'];w=replacement['witness']
        require(w['returned'] and w['rows'][-1]['stage']=='returned','Replacement did not return')
        require(any(r['stage']=='after' and r['slot']==0 for r in w['rows']),'Replacement skipped target')
        if c['formula'].startswith('_Cond'):
            require(w['rows'][-1]['counted']>0,'Replacement has no counted progression')
            check_conditional_trace(w,c['formula'],slots(decode(Path(replacement['scene']['path'])))[0]['options'])
        for key in ('original_image','current_cpp_image','current_pascal_image'):
            require(key in replacement,'Missing image identity')
            checked(replacement[key]['path'],replacement[key]['sha256'])
        require(exact_rgb(replacement['original_image']['path'],replacement['current_cpp_image']['path']) and
                exact_rgb(replacement['original_image']['path'],replacement['current_pascal_image']['path']),'Replacement RGB mismatch')
    # All nested path+hash receipts are checked, including traces and executable.
    def visit(x):
        if isinstance(x,dict):
            if 'path' in x and 'sha256' in x:checked(x['path'],x['sha256'])
            if 'trace' in x and 'rows' in x:
                actual=parse_trace(Path(x['trace']['path']).read_text())
                require(all(x[k]==actual[k] for k in ('rows','stop','returned')),'Trace/summary mismatch')
            for y in x.values():visit(y)
        elif isinstance(x,list):
            for y in x:visit(y)
    visit(report)
    return True


def negative_controls(report):
    """Exercise fail-closed acceptance without changing any retained artifact."""
    def wrap_row(r):return next(c for c in r['cases'] if c['classification']=='excessive_finite_signed_counter_wrap')
    mutations=[
      ('missing_case',lambda r:r['cases'].pop()),
      ('duplicate_case',lambda r:r['cases'].__setitem__(-1,copy.deepcopy(r['cases'][0]))),
      ('budget_mislabelled_nontermination',lambda r:wrap_row(r).__setitem__('classification','all_switch_branches_disable_only_counted_slot')),
      ('missing_wrap',lambda r:wrap_row(r)['proof'].__setitem__('counter_wrap_observed',False)),
      ('replacement_unreached',lambda r:r['cases'][0].__setitem__('replacement_branch_reached',False)),
      ('replacement_skips_target',lambda r:r['cases'][0]['replacement']['witness'].__setitem__('rows',[r['cases'][0]['replacement']['witness']['rows'][-1]])),
      ('trace_row_tampered',lambda r:r['cases'][0]['replacement']['witness']['rows'][0].__setitem__('counted',999)),
      ('image_hash_tampered',lambda r:r['cases'][0]['replacement']['original_image'].__setitem__('sha256','0'*64)),
      ('acceleration_changes_radius',lambda r:next(x for x in wrap_row(r)['proof']['wrap_witness']['rows'] if x['stage']=='jump_to').__setitem__('radius_bits','0000000000000000')),
      ('unfinished',lambda r:r.__setitem__('finished',False)),
    ]
    results=[]
    for name,mutate in mutations:
        changed=copy.deepcopy(report);mutate(changed)
        try:validate(changed)
        except (ValueError,StopIteration) as e:results.append(dict(case=name,rejected=True,diagnostic=str(e)))
        else:raise ValueError('Validator false PASS: '+name)
    return results


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
    output=a.output.resolve();output.mkdir(parents=True,exist_ok=True)
    require(not (output/'summary.json').exists(),'Refusing overwrite')
    hang=json.loads((BASE/'hang-app-v3/summary.json').read_bytes())
    cases=[c for c in hang['cases'] if c['id'] in CASE_IDS]
    require({c['id'] for c in cases}==CASE_IDS,'Retained case set differs')
    fm=ROOT/'comparison/runs/external-complete/conditional-finite-scenes-v1/manifest.json'
    finite={c['id']:c for c in json.loads(fm.read_bytes())['cases']}
    fs=ROOT/'comparison/runs/external-complete/conditional-finite-images-v1/summary.json'
    fr={c['scene']:c for c in json.loads(fs.read_bytes())['results']}
    tf=ROOT/'comparison/runs/hang-fixes-20261018/cpp-guarded/finite-reference-manifest.json'
    trifox=next(c for c in json.loads(tf.read_bytes())['cases'] if c['id']=='challenge_TrifoxComplexAngles_early')
    exe,built=build(output)
    report=dict(scope='Fixed 24 original-reference exclusions; bounded source/CODE witnesses, not all-input parity',finished=False,
                build=built,retained=[ref(p) for p in (BASE/'hang-app-v3/summary.json',fm,fs,tf,BASE/'pascal-finite-v3/summary.json')],cases=[],
                limitations=['No original application was run on excluded inputs.','Counter acceleration is a source-invariant witness, not elapsed-time execution of billions of calls.',
                             'Admissible orbit-state witnesses and retained guarded image failure are separate; these traces do not locate a failing render pixel.',
                             'witness-v1 was exploratory: an independent prepared-buffer comparison found its Trifox SRECI2 pair reversed. This run corrects that harness-only error; v1 is not accepted.'])
    points=json.loads((BASE/'points/summary.json').read_bytes())
    pointrows={r['case']:r for r in points['results']}
    for n in SUFFIXES:
        from capstone import Cs, CS_ARCH_X86, CS_MODE_32
        asset=checked(ROOT/'mb3d/M3Formulas'/f'{n}.m3f',ASSET_SHA[n]);code=bytes.fromhex(asset.read_text().split('[CODE]')[1].split('[END]')[0])
        (output/(n+'.disassembly.txt')).write_text('\n'.join(f'{i.address:04x}: {i.bytes.hex():24} {i.mnemonic} {i.op_str}' for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(code,0))+'\n')
    for c in cases:
        cid=c['id'];name=c['formula'];scene=checked(c['scene'],c['sha256']);kind=invariant(c)
        prepared=list((BASE/'points').glob('*/'+cid+'.params'));require(len(prepared)==1,'Missing prepared comparison')
        checked(prepared[0],pointrows[cid]['parameter_sha256'])
        require(parameters(slots(decode(scene))[0])[256:512]==prepared[0].read_bytes()[256:512], 'Independent parameter preparation differs')
        rr=[r for r in hang['results'] if r['case']==cid and r['threads']==1]
        require(len(rr)==2 and all(r['contract_pass'] and not r.get('output_exists',True) for r in rr),'Missing rejection pair')
        row=dict(id=cid,formula=name,scene=ref(scene),classification=kind,guarded_rejection_verified=True,
                 prepared_parameter_comparison=ref(prepared[0]),independent_negative_buffer_exact=True,
                 disassembly=ref(output/(name+'.disassembly.txt')),image_claim='Retained guarded rejection only; original numerical image remains excluded')
        if name=='TrifoxComplexAngles':
            # Byte-exact SHR EAX,1/JNC backward edges: EAX=0, CF=0 is invariant.
            asset=ROOT/'mb3d/M3Formulas/TrifoxComplexAngles.m3f';code=bytes.fromhex(asset.read_text().split('[CODE]')[1].split('[END]')[0])
            needle=bytes.fromhex('d1e873fa' if cid.endswith('p0_zero') else 'd1e873ec')
            off=code.index(needle);steps=[dict(eax_before=0,eax_after=0,carry=0,jnc_taken=True) for _ in range(4)]
            row['proof']=dict(asset=ref(asset),code_sha256=hashlib.sha256(code).hexdigest(),offset=off,instructions=needle.hex(),steps=steps,
                zero_preserved=True,carry_always_zero=True,back_edge_taken=True,argument='Logical right shift maps zero to zero and clears CF. JNC returns to the same loop; intervening x87 arithmetic cannot modify EAX/CF.')
            replacement=checked(ROOT/trifox['scene'],trifox['sha256'])
            w=run(exe,output,cid+'-replacement',replacement,single=True)
            require(w['returned'] and len([r for r in w['rows'] if r['stage']=='after'])==1,'Finite Trifox callback did not return')
            image=checked(trifox['pascal_reference_image'],trifox['pascal_reference_image_sha256'])
            cppimages=list((BASE/'images').glob('*/cpp/external-complete/'+trifox['id']+'.png'));require(len(cppimages)==1,'Missing current Trifox image')
            row['replacement']=dict(scene=ref(replacement),original_image=ref(image),witness=w,retained_control_id=trifox['id'],
                current_cpp_image=ref(cppimages[0]),current_pascal_image=ref(BASE/'pascal-finite-v3'/('pascal_t1_'+trifox['id']+'.png')))
            # Image equality is also checked from decoded pixels, independent of result labels.
            from PIL import Image
            old=Image.open(image).convert('RGB');v20=Image.open(checked(trifox['cpp_v20_image'],trifox['cpp_v20_image_sha256'])).convert('RGB')
            require(old.size==v20.size and old.tobytes()==v20.tobytes(),'Finite Trifox reference changed')
            row.update(replacement_original_rgb_exact=True,replacement_branch_reached=True)
        else:
            w=run(exe,output,cid+'-original-bounded',scene)
            require(not w['returned'],'Expected bounded pathological orbit')
            opts=slots(decode(scene))[0]['options']
            branches=check_conditional_trace(w,name,opts)
            proof=dict(witness=w,invariant_verified=True,original_branches=branches)
            if kind=='excessive_finite_signed_counter_wrap':
                accelerated=run(exe,output,cid+'-counter-wrap',scene,accelerate=1)
                check_conditional_trace(accelerated,name,opts)
                after=[r for r in accelerated['rows'] if r['stage']=='after' and r['slot']==0]
                proof.update(wrap_witness=accelerated,wrap_returned=accelerated['returned'],counter_wrap_observed=any(r['first']==-2147483648 for r in after),
                    argument='Only bFirstIt changes during disabled-partner high-branch cycles. INC wraps modulo 2^32; signed JL selects the positive Lo count after INT_MAX -> INT_MIN. Jump preserves all other state; actual original CODE then resumes counted work and returns.',
                    conditional_calls_before_first_signed_wrap=2147483648,
                    image_claim='Finite projected orbit demonstrated with exact counter acceleration; not a completed original image')
            elif kind=='empty_repeat_range_excludes_start':
                require(w['stop']=='bounded_scan_stop' and w['repeat']==1 and w['end']==1 and w['rows'][-1]['counts'][1]==0,'Wrong empty-cycle invariant')
                proof['argument']='Slot 0 executes once, zeroes slot 1; repeat=end=1 excludes start=0. n stays 1, bTmp stays zero; the original n=iStartFrom escape is unreachable.'
            elif kind=='radius_fixed_point_disables_only_counted_slot':
                after=[r for r in w['rows'] if r['stage']=='after'];require(all(r['slot']==0 and r['counts'][1]==0 and r['counted']==0 for r in after),'Radius invariant failed')
                require(len({(r['radius_bits'],tuple(r['xyz_bits']),tuple(r['counts']),r['first']) for r in after})==1,'Not a radius fixed point')
                proof['argument']='Rout<R1^2 disables slot 1. Original conditional CODE changes no coordinates/radius/first counter. Slot 0 retains its non-counting sign bit. The same complete loop state repeats and the termination predicate stays false.'
            elif kind=='zero_sequence_only_enables_out_of_range_slot':
                after=[r for r in w['rows'] if r['stage']=='after'];require(all(r['counts'][1]==0 and r['counts'][2]>0 and r['counted']==0 for r in after),'Sequence invariant failed')
                proof['argument']='Sequential code zero stays zero under every arithmetic shift; slot A=1 is always zero, slot B=2 is outside EndTo=1. Counter modulo 2^32 cannot change the selected bit. No counted callback is reachable.'
            else:
                require(c['id'].endswith('p2_zero'),'Unexpected invariant')
                proof['argument']='Clamped Lo/Eq/Hi counts are all zero, so signed counter wrap cannot enable slot 1; slot 0 stays non-counted and coordinates/radius remain unchanged.'
            row['proof']=proof
            fc=finite['finite_'+cid];replacement=checked(ROOT/fc['scene'],fc['sha256']);r=fr[fc['id']]
            require(r['status']=='pass' and r['exact_rgb'] and r['scene_sha256']==fc['sha256'],'Invalid retained finite image')
            image=checked(fs.parent/'pascal'/(fc['id']+'.png'),r['pascal_image_sha256'])
            fw=run(exe,output,cid+'-replacement',replacement)
            replacement_opts=slots(decode(replacement))[0]['options']
            reached=check_conditional_trace(fw,name,replacement_opts)
            callbacks=[x for x in fw['rows'] if x['stage']=='after' and x['slot']==0]
            require(fw['returned'] and callbacks and fw['rows'][-1]['counted']>0,'Finite replacement did not reach callback/count progression')
            # Separate one-call controls span comparison/counter boundaries while
            # the actual six-slot replacement above proves finite progression.
            extras=[]
            for k,(pt,seed) in enumerate([((1.2,.1,.1),0),((2.2,.1,.1),0)] if name=='_CondItcountR' else [((.1,.2,.3),2147483647),((.1,.2,.3),2)]):
                ew=run(exe,output,cid+'-branch-'+str(k),replacement,point=pt,seed=seed,single=True)
                extras.append(dict(witness=ew,branches=check_conditional_trace(ew,name,replacement_opts)))
            row['replacement']=dict(scene=ref(replacement),original_image=ref(image),retained_comparison=ref(fs),witness=fw,
                current_cpp_image=ref(BASE/'hang-app-v3'/('cpp_t1_'+fc['id']+'.png')),current_pascal_image=ref(BASE/'hang-app-v3'/('pascal_t1_'+fc['id']+'.png')),
                reached_branches=reached,boundary_callbacks=extras,
                protected_counted_slot=fc['changes']['protected_counted_slot'],conditional_calls=len(callbacks),counted_at_return=fw['rows'][-1]['counted'])
            row.update(replacement_original_rgb_exact=True,replacement_branch_reached=True)
        report['cases'].append(row);write_json(output/'progress.json',report)
        print(cid,kind,'OK',flush=True)
    report['finished']=True;validate(report)
    report['validator_negative_controls']=negative_controls(report)
    from collections import Counter
    report['classifications']=dict(Counter(c['classification'] for c in report['cases']))
    report['status']='pass';report['tool']=ref(Path(__file__))
    write_json(output/'summary.json',report)


if __name__=='__main__':main()
