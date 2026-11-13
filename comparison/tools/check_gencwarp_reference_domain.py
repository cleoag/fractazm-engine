"""Prove the original _GenCWarp selector domain with native x87 witnesses.

Seeds are diagnostic perturbations, NOT a claim about actual application callers.
Plain runs execute unchanged M3F CODE; traced copies replace two five-byte sites
with FXSAVE tail shims and replay the replaced instructions. Pascal is built only
in the external editable tree. No undefined selector receives a numerical PASS.
"""
import argparse
import hashlib
import json
import math
import os
import struct
import subprocess
from pathlib import Path

from capstone import Cs, CS_ARCH_X86, CS_MODE_32

ROOT = Path(__file__).resolve().parents[2]
EDITABLE = Path('C:/work/_c/mb3d_pascal_work/mb3d_work')
ASSET = ROOT/'mb3d/M3Formulas/_GenCWarp.m3f'
SITES = ((0x6c, 0x390, -0x40), (0x3a1, 0x6c5, -0x44))


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def prove_domain(code):
    """Follow every all-not-equal branch; no arithmetic or floating branch guessed."""
    insns = {i.address: i for i in Cs(CS_ARCH_X86, CS_MODE_32).disasm(code, 0)}
    require(sum(i.size for i in insns.values()) == len(code), 'Incomplete disassembly')
    proofs = []
    for start, end, offset in SITES:
        pc = start
        constants, path = [], []
        while pc != end:
            require(pc in insns and len(path) < 100, 'Invalid selector chain')
            cmp = insns[pc]
            if cmp.mnemonic == 'nop':
                path.append(pc)
                pc += cmp.size
                continue
            require(cmp.mnemonic == 'cmp' and cmp.op_str.startswith(f'dword ptr [edi - 0x{-offset:x}], '), 'Unexpected selector operand')
            constants.append(int(cmp.op_str.split(', ')[1], 0))
            branch = insns[pc+cmp.size]
            require(branch.mnemonic == 'jne', 'Unexpected selector branch')
            path.extend([pc, branch.address])
            pc = int(branch.op_str, 0)
        require(constants == list(range(24)), 'Expected exactly selectors 0..23')
        require(insns[end].mnemonic == 'fpatan', 'Missing original FPATAN')
        proofs.append(dict(parameter_offset=offset, compared_values=constants,
                           out_of_domain_path=path, fpatan_offset=end,
                           x87_instructions_before_fpatan_on_this_path=0,
                           implication='Every other int32 reaches FPATAN without loading operands'))
    require(code[0x390:0x395] == bytes.fromhex('D9F3DC4FCC'), 'Theta trace site changed')
    require(code[0x6c5:0x6ca] == bytes.fromhex('D9F3DC4FC4'), 'Phi trace site changed')
    return proofs


HARNESS = r'''
var CodePtr, ThetaResume, PhiResume: Pointer;
    BeforeState, AfterState, ThetaState, PhiState: Pointer;
    RawStates: array[0..4*512+15] of Byte;
    Params: array[0..1023] of Byte;
    It: TIteration3Dext;
    PX,PY,PZ,PW,PI: Pointer;
    Stream: TFileStream;
    Original: array of Byte;
    Mode, Theta, Phi, Seeds, I: LongInt;
    A,B,C,D,X,Y,Z: Double;
    CW: Word = $133f;

procedure ThetaTap; assembler; nostackframe;
asm
  push eax
  mov eax,[ThetaState]
  fxsave [eax]
  pop eax
  fpatan
  fmul qword ptr [edi-$34]
  jmp dword ptr [ThetaResume]
end;

procedure PhiTap; assembler; nostackframe;
asm
  push eax
  mov eax,[PhiState]
  fxsave [eax]
  pop eax
  fpatan
  fmul qword ptr [edi-$3c]
  jmp dword ptr [PhiResume]
end;

procedure Execute; assembler; nostackframe;
asm
  push ebx
  push esi
  push edi
  fninit
  fldcw [CW]
  cmp dword ptr [Seeds],0
  je @ready
  fld qword ptr [A]
  fld qword ptr [B]
  cmp dword ptr [Seeds],4
  jne @ready
  fld qword ptr [C]
  fld qword ptr [D]
@ready:
  mov esi,[BeforeState]
  fxsave [esi]
  mov eax,[PX]
  mov edx,[PY]
  mov ecx,[PZ]
  push dword ptr [PW]
  push dword ptr [PI]
  call dword ptr [CodePtr]
  mov esi,[AfterState]
  fxsave [esi]
  fninit
  fldcw [CW]
  pop edi
  pop esi
  pop ebx
end;

procedure Patch(Offset: LongInt; Target: Pointer);
begin
  PByte(PtrUInt(CodePtr)+Offset)^:=$e9;
  PLongInt(PtrUInt(CodePtr)+Offset+1)^:=LongInt(PtrUInt(Target)-PtrUInt(CodePtr)-Offset-5);
end;

procedure State(P: Pointer);
begin Write(PWord(PtrUInt(P)+2)^,' ',PByte(PtrUInt(P)+4)^,' '); end;
procedure Number(V: Double);
begin Write(FloatToStrF(V,ffGeneral,17,0),' '); end;
begin
  if ParamCount<>2 then Halt(2);
  if (SizeOf(Pointer)<>4) or (PtrUInt(@It.C1)-PtrUInt(@It)<>56) then Halt(3);
  DefaultFormatSettings.DecimalSeparator:='.';
  SetExceptionMask([exInvalidOp,exDenormalized,exZeroDivide,exOverflow,exUnderflow,exPrecision]);
  Stream:=TFileStream.Create(ParamStr(1),fmOpenRead);
  SetLength(Original,Stream.Size); Stream.ReadBuffer(Original[0],Stream.Size); Stream.Free;
  CodePtr:=VirtualAlloc(nil,Length(Original),MEM_COMMIT or MEM_RESERVE,PAGE_EXECUTE_READWRITE);
  if CodePtr=nil then Halt(4);
  Stream:=TFileStream.Create(ParamStr(2),fmOpenRead);
  if Stream.Size<>1024 then Halt(5);
  Stream.ReadBuffer(Params,1024); Stream.Free;
  BeforeState:=Pointer((PtrUInt(@RawStates[0])+15) and not PtrUInt(15));
  AfterState:=Pointer(PtrUInt(BeforeState)+512);
  ThetaState:=Pointer(PtrUInt(BeforeState)+1024);
  PhiState:=Pointer(PtrUInt(BeforeState)+1536);
  ThetaResume:=Pointer(PtrUInt(CodePtr)+$395); PhiResume:=Pointer(PtrUInt(CodePtr)+$6ca);
  PX:=@It.x; PY:=@It.y; PZ:=@It.z; PW:=@It.w; PI:=@It.C1;
  while not EOF(Input) do begin
    ReadLn(Mode,Theta,Phi,Seeds,A,B,C,D,X,Y,Z);
    if not (Seeds in [0,2,4]) then Halt(6);
    FillChar(It,SizeOf(It),0); FillChar(RawStates,SizeOf(RawStates),0);
    It.x:=X;It.y:=Y;It.z:=Z;It.C1:=X+0.125;It.C2:=Y-0.25;It.C3:=Z+0.5;
    It.J1:=0.325;It.J2:=-0.15;It.J3:=0.55;
    It.PVar:=@Params[512];It.VaryScale:=1;It.Deriv1:=1;
    PLongInt(@Params[512-$40])^:=Theta; PLongInt(@Params[512-$44])^:=Phi;
    Move(Original[0],CodePtr^,Length(Original));
    if Mode=1 then begin Patch($390,@ThetaTap);Patch($6c5,@PhiTap);end
    else if Mode<>0 then Halt(7);
    Execute;
    State(BeforeState);State(ThetaState);State(PhiState);State(AfterState);
    Number(It.x);Number(It.y);Number(It.z);Number(It.w);
    Number(It.J1);Number(It.J2);Number(It.J3);WriteLn;
  end;
  VirtualFree(CodePtr,0,MEM_RELEASE);
end.
'''


def build(output, work, env):
    from build_original_hybrid_oracle import HEADER
    require(work.resolve().is_relative_to(EDITABLE.resolve()), 'Pascal build must be in editable tree')
    work.mkdir(parents=True, exist_ok=True)
    types = (ROOT/'mb3d/TypeDefinitions.pas').read_text()
    types = types[types.index('  ThybridIteration2 ='):types.index('  ThybridIteration =')]
    pas = work/'gencwarp_x87_oracle.pas'
    pas.write_text(HEADER.replace('original_hybrid_oracle','gencwarp_x87_oracle')+types+HARNESS)
    fpc=Path('C:/FPC/3.2.2/bin/i386-Win32/fpc.exe')
    command=[str(fpc),'-Mdelphi','-CfSSE2','-O2','-al',str(pas.resolve())]
    run=subprocess.run(command,cwd=work,env=env,capture_output=True,text=True,timeout=60)
    (output/'build.log').write_text(run.stdout+run.stderr)
    run.check_returncode()
    exe=pas.with_suffix('.exe')
    for path in (pas,exe,pas.with_suffix('.s')):
        (output/path.name).write_bytes(path.read_bytes())
    return exe, dict(command=command, exit_code=run.returncode, sha256={str(p):sha(p) for p in
        (fpc,fpc.with_name('ppc386.exe'),pas,exe,ROOT/'mb3d/TypeDefinitions.pas',Path(__file__))})


def run_process(command, stdin, prefix, env):
    result=subprocess.run(list(map(str,command)),input=stdin,text=True,capture_output=True,timeout=30,env=env)
    prefix.with_suffix('.stdin.txt').write_text(stdin)
    prefix.with_suffix('.stdout.txt').write_text(result.stdout)
    prefix.with_suffix('.stderr.txt').write_text(result.stderr)
    prefix.with_suffix('.command.json').write_text(json.dumps(dict(command=list(map(str,command)),exit_code=result.returncode),indent=2))
    return result


def number(text):
    return math.nan if 'nan' in text.lower() else float(text)


def same(left,right):
    return len(left)==len(right) and all(a==b or math.isnan(a) and math.isnan(b) for a,b in zip(left,right))


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--work',type=Path,default=EDITABLE/'_reference_gaps_gencwarp')
    ap.add_argument('--probe',type=Path,default=ROOT/'comparison/runs/compatibility-final-20261018/candidate/mb3d_formula_point_probe.exe')
    a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=False)
    env={k.upper():v for k,v in os.environ.items() if not k.upper().startswith(('MB3D_POINT_','MB3D_PARITY_','MB3D_HEADLESS_'))}
    code=bytes.fromhex(ASSET.read_text().split('[CODE]')[1].split('[END]')[0])
    proofs=prove_domain(code)
    binary=a.output/'_GenCWarp.bin';binary.write_bytes(code)
    manifest=ROOT/'comparison/runs/compatibility-final-20261018/selectors/generic.json'
    cases=[r for r in json.loads(manifest.read_bytes())['cases'] if r['target_formulas']==['_GenCWarp']]
    wanted={'selector_complete__GenCWarp_5_-1','selector_complete__GenCWarp_6_-1'}
    rejected=[];params=None
    for case in cases:
        if case['id'] not in wanted: continue
        scene=ROOT/case['scene'];require(sha(scene)==case['sha256'],'Scene identity mismatch')
        params=a.output/(case['id']+'.params.bin')
        result=run_process([a.probe,'_GenCWarp',scene,params],'.2 .1 .05 0 0 0 0 0 0 0 0\n',a.output/case['id'],env)
        require(result.returncode==12 and not result.stdout.strip() and 'FORMULA_INPUT_REJECTED' in result.stderr,'Invalid input was not rejected')
        native=struct.unpack_from('<i',params.read_bytes(),512+case['selector']['pvar_offset'])[0]
        require(native==-1,'Invalid selector not loaded as -1')
        rejected.append(dict(case=case['id'],status='rejected_original_undefined',scene_sha256=sha(scene),native_selector=native,exit_code=result.returncode))
    require({r['case'] for r in rejected}==wanted,'Missing invalid cases')
    exe,identity=build(a.output,a.work,env)
    selectors=[(t,p) for t in range(24) for p in range(24)]
    selectors += [(t,p) for bad in (-2147483648,-1,24,2147483647) for t,p in ((bad,8),(2,bad),(bad,bad))]
    experiments=[]
    for theta,phi in selectors:
        for seeds in (0,2):
            # Distinct atan2 angles, neither a numerical result nor caller provenance assumed.
            for seed_values in ((1,2,3,5),(-2,3,5,-7)):
                experiments.append((theta,phi,seeds,*seed_values,.2,.1,.05))
    rows={}
    for label,mode in (('plain',0),('trace',1),('repeat',1)):
        stdin=''.join(' '.join(map(str,(mode,*row)))+'\n' for row in experiments)
        result=run_process([exe,binary,params],stdin,a.output/label,env);result.check_returncode()
        rows[label]=[[number(v) for v in line.split()] for line in result.stdout.splitlines()]
        require(len(rows[label])==len(experiments) and all(len(r)==15 for r in rows[label]),'Incomplete native results')
    counts=dict(valid_controls=0,invalid_empty_stack_faults=0,invalid_sufficient_stack_without_fault=0,seed_dependent_pairs=0)
    witnesses=[]
    for i,(experiment,plain,trace,repeat) in enumerate(zip(experiments,rows['plain'],rows['trace'],rows['repeat'])):
        require(same(plain[:2]+plain[6:],trace[:2]+trace[6:]),f'Instrumentation changed result at {i}')
        require(same(trace,repeat),f'Nonrepeatable native experiment at {i}')
        theta,phi,seeds=experiment[:3];invalid=int(theta not in range(24))+int(phi not in range(24))
        depth=lambda n:int(trace[n]).bit_count()
        require(depth(1)==seeds,'Wrong incoming x87 depth')
        require(depth(3)==seeds+(2 if theta in range(24) else 0),'Wrong theta FPATAN operand depth')
        if theta in range(24) or seeds==2:
            require(depth(5)==seeds-2*int(theta not in range(24))+(2 if phi in range(24) else 0),
                    'Wrong phi FPATAN operand depth')
        if not invalid:
            require(not int(trace[6])&0x41 and depth(7)==seeds,'Valid selector corrupted x87 stack')
            require(same(trace[8:],rows['trace'][i-i%4][8:]),'Valid result depended on unused incoming x87 values')
            counts['valid_controls']+=1
        elif seeds==0:
            require(int(trace[6])&0x41==0x41,'Missing x87 stack underflow')
            counts['invalid_empty_stack_faults']+=1
        elif seeds>=2*invalid:
            require(not int(trace[6])&0x41 and depth(7)==seeds-2*invalid,'Invalid path did not consume incoming x87 values')
            counts['invalid_sufficient_stack_without_fault']+=1
            if i%2:
                require(not same(trace[8:],rows['trace'][i-1][8:]),'Invalid path did not respond to changed operands')
                counts['seed_dependent_pairs']+=1
        else:
            require(invalid==2 and seeds==2 and int(trace[6])&0x41==0x41,
                    'Two invalid angles should exhaust two incoming values')
        if invalid:
            witnesses.append(dict(theta=theta,phi=phi,seeds=seeds,seed_values=list(experiment[3:7]),
                before_depth=depth(1),theta_depth=depth(3),phi_depth=depth(5),after_depth=depth(7),
                status_word=int(trace[6]),outputs=[v if math.isfinite(v) else str(v) for v in trace[8:]]))
    report=dict(schema_version=1,finished=True,status='PASS_REFERENCE_DOMAIN_PROOF',
        scope=__doc__,numerical_parity_for_invalid_selectors=False,
        original_code_sha256=sha(binary),asset_sha256=sha(ASSET),probe_sha256=sha(a.probe),
        selector_manifest_sha256=sha(manifest),build=identity,static_proofs=proofs,
        rejected_cases=rejected,native_experiments=len(experiments),native_processes=3,
        instrumentation_and_repeat_invariant=True,counts=counts,invalid_witnesses=witnesses)
    report['artifact_sha256']={str(p.relative_to(a.output)):sha(p) for p in sorted(a.output.rglob('*')) if p.is_file()}
    (a.output/'summary.json').write_text(json.dumps(report,indent=2,allow_nan=False))
    print(json.dumps(dict(status=report['status'],experiments=len(experiments),counts=counts)))


if __name__=='__main__': main()
