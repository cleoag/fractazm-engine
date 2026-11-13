#!/usr/bin/env python3
"""Experimental source-bound continuation cuts, never production formula output.

Recipes describe audited live fields/cut positions, not mathematical bodies.
Every arithmetic statement is extracted from the existing generated evaluator.
Unsupported identities/patterns fail closed; this is not a corpus capability claim.
"""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import re
from gen_native_metal_formulas import tokens, matching, emit_text

ROOT = Path(__file__).resolve().parents[1]
# Audited P1 source shapes/helper semantics. Changes require a fresh cut audit.
AUDITED_SOURCE_SHA256 = {10089: '99d9004e0bf411f53bab07d56f583fa363e3fe9d1327e6c0c3cb6bfab88eef13', 10274: 'e94527ca7a309be5b560e55a2ccd0f8198d55da560345f4bad796cd71d583399', 10265: '97a1e941ee46cab924179702da0544b24747ad025acffe3fd426fc7f1b7665d8', 10220: '55883c52ea7c8a504bbdddd08871ff3a168a652c6b9afb90034d6942ddefac6e', 10321: 'f9864bdfc29243fa3d14b3efe3e96034404baf609bae4f96f74c40a0d4377ced'}
REGION_ORDER = (10274,10265,10220,10321,10089)
AUDITED_MATH_SHA256 = '291c7943394835e6537d0ec1b87dba1f6a55e33e9d5b907591802c2f1766aaad'


def locate(v, needle):
    positions = [i for i in range(len(v)-len(needle)+1) if v[i:i+len(needle)] == needle]
    if len(positions) != 1:
        raise ValueError(f'Expected one audited pattern {needle}, found {len(positions)}')
    return positions[0]


def body(source):
    v = tokens(source)
    start = locate(v, ['template', '<', '>', 'FormulaVec3', 'execute_formula', '<'])
    begin = v.index('{', start)
    return v[begin+1:matching(v, begin)]


def finish(v, save=''):
    # These audited recipes have one expression return per region, no nested
    # helper/lambda return. Refuse any expansion of that contract.
    i = locate(v, ['return']); end = v.index(';', i)
    return emit_text(v[:i]) + '{ s.result = ' + emit_text(v[i+1:end]) + '; ' + save + ' s.pc=99; return; }' + emit_text(v[end+1:])


def fields_code(fields):
    declarations=[]; aliases=[]; copies=[]
    for type_, name, count in fields:
        declarations.append(f'{type_} {name}' + (f'[{count}]' if count else '') + ';')
        aliases.append(f'[[maybe_unused]] {type_} ' + (f'{name}[{count}]; for(uint k=0;k<{count};++k){name}[k]=s.{name}[k];' if count else f'{name}=s.{name};'))
        copies.append(f'for(uint k=0;k<{count};++k)s.{name}[k]={name}[k];' if count else f's.{name}={name};')
    return '\n'.join(declarations), '\n'.join(aliases), '\n'.join(copies)


def observer(fields, extras):
    """Canonical typed frame (conservative live superset), never struct padding."""
    statements=['out[0]=s.pc;']; offset=1
    for type_,name,count in fields+extras+[('FormulaVec3','result',0)]:
        names=([f'{name}[{i}]' for i in range(count)] if count else
               [f'{name}.{axis}' for axis in 'xyz'] if type_=='FormulaVec3' else [name])
        for name in names:
            value=f'as_type<uint>(s.{name})' if type_ in ('float','numeric::X87Real','FormulaVec3') else f'uint(s.{name})'
            statements.append(f'out[{offset}]={value};');offset+=1
    return 'inline void observe(thread const Frame& s,thread uint* out){'+''.join(statements)+'}'


def generate(formula_id, source):
    if formula_id not in AUDITED_SOURCE_SHA256:
        raise ValueError(f'No audited continuation cuts for formula {formula_id}')
    if hashlib.sha256(source.encode()).hexdigest()!=AUDITED_SOURCE_SHA256[formula_id]:
        raise ValueError('Evaluator changed: repeat the liveness/control-flow cut audit')
    v = body(source)
    manual_copy=False
    if formula_id == 10274:
        begin=locate(v,['do','{']); end=matching(v,begin+1)
        if v[end+1:end+6] != ['while','(','true',')',';']:
            raise ValueError('Unexpected tess loop condition')
        prefix=v[:begin]; loop=v[begin+2:end]; tail=v[end+6:]
        if loop.count('break') != 2 or loop.count('return') != 1:
            raise ValueError('Unexpected tess exit structure')
        fields=[('float','v',3),('int','maxDepth',0),('float','axiom',0),('float','threshold',0),('int','count',0)]
        decl,alias,copy=fields_code(fields)
        loop_text=re.sub(r'\bbreak\s*;', '{ s.pc=2; s.work_steps=work.steps; return; }', finish(loop,'s.work_steps=work.steps;'))
        if 'break' in loop_text:raise ValueError('Unlowered tess break')
        cases=f'''case 0: {{ {emit_text(prefix)} {copy} s.work_steps=0; s.pc=1; return; }}
case 1: {{ {alias}
FormulaWorkBudget work(params,"continuation"); work.steps=s.work_steps;
{loop_text}
s.work_steps=work.steps; return; }}
case 2: {{ {alias} {finish(tail)} }}'''
        extra='uint work_steps;'; observe=observer(fields,[('uint','work_steps',0)])
        regions=['initialize','guarded_depth_iteration','final_coordinate_transform']
    elif formula_id == 10265:
        begin=locate(v,['for','(','unsigned','i','=']); opening=v.index('{',begin); end=matching(v,opening)
        condition=v[begin:opening]
        if condition != ['for','(','unsigned','i','=','0',';','i','<','T1_Count',';','i','++',')']:
            raise ValueError('Unexpected Pine T1 loop')
        rotate=locate(v,['if','(','rotate',')']); polygon=locate(v,['if','(','polygon',')'])
        po=polygon+4; pe=matching(v,po)
        wh=locate(v,['while','(','theta','>=','half_angle',')']); wo=wh+6; we=matching(v,wo)
        if not end<rotate<polygon<wh<we<pe:raise ValueError('Unexpected Pine region order')
        floats='Scale xOff pixel_x pixel_y pixel_z yz_angle Sides s23 s13 s12 pi_const Cx Cy Cz x y z'.split()
        fields=[('float',n,0) for n in floats]+[('uint','T1_Count',0),('bool','rotate',0),('bool','polygon',0)]
        decl,alias,copy=fields_code(fields)
        extra='uint i; float half_angle,double_half,theta;'; observe=observer(fields,[("float",n,0) for n in ('half_angle','double_half','theta')])
        # Keep bounded arithmetic in its original local scope; serialize only
        # at the genuinely unbounded angle-loop back edge. Do not introduce
        # artificial stores between rotation, factor and final polynomial.
        checkpoint=copy+' s.theta=theta; s.half_angle=half_angle; s.double_half=double_half; s.pc=3; return;'
        bounded_loop='uint remaining=1; while(theta>=half_angle) { if(!remaining){'+checkpoint+'} --remaining; '+emit_text(v[wo+1:we])+' }'
        tail=emit_text(v[pe+1:])
        final='s.result=finish_value(x,y,z,Cx,Cy,Cz,pixel_x,pixel_y,pixel_z); s.pc=99; return;'
        begin_code=emit_text(v[:polygon])+' if(polygon){ '+emit_text(v[po+1:wh])+bounded_loop+emit_text(v[we+1:pe])+' } '+final
        resume_code=alias+' float theta=s.theta,half_angle=s.half_angle,double_half=s.double_half; '+bounded_loop+emit_text(v[we+1:pe])+final
        code=f'''// Generated experimental loop cut, not production dispatch.
namespace mb3d_native {{ namespace continuation_10265 {{
// Preserve the original by-value return context for contraction selection.
// The complete polynomial below is extracted verbatim, not recomputed.
inline FormulaVec3 finish_value(float x,float y,float z,float Cx,float Cy,float Cz,float pixel_x,float pixel_y,float pixel_z) {{ {tail} }}
#ifndef MB3D_P2_SHARED_FRAME_TYPES
struct Frame {{ uint pc; {decl} {extra} FormulaVec3 result; }};
#endif
{observe}
inline void step(thread Frame& s,FormulaVec3 p,FormulaParams params,thread float& de) {{
if(s.pc==0){{ {begin_code} }}
else if(s.pc==3){{ {resume_code} }}
else if(s.pc!=99){{params.fault->code=InvalidFormulaAbi;s.pc=99;}}
}}
}} }}'''
        return code, {'formula_id':formula_id,'source_sha256':hashlib.sha256(source.encode()).hexdigest(),'regions':['bounded_prefix_and_first_fold','angle_loop_backedge','bounded_finish'],'live_fields':fields,'scope':'raw body proof only; wrapper/scene continuation unsupported'}
    elif formula_id == 10089:
        begin=locate(v,['for','(','int','i','=','0',';','i','<','iter_count',';','i','++',')'])
        opening=begin+14;end=matching(v,opening)
        # Restore yy before xx, matching the original loop's yy_new/xx_new
        # value-definition order. This preserves the pinned compiler's
        # contraction selection in sqrt(xx*xx+yy*yy); expression tokens stay
        # unchanged. Frozen-control and canonical-state proofs guard the cut.
        names='Beta Scale Amplitude Alpha Step Gamma OTrapClrMul x y z yy xx ref_axis'.split()
        fields=[('float',n,0) for n in names]+[('int',n,0) for n in ('Apply_scale_add','Flip_XZ','iter_count')]
        decl,alias,copy=fields_code(fields)
        # The original early exit precedes iter_count's declaration. No loop
        # locals are initialized/read on that return; only the result is live.
        cases=f'''case 0: {{ {finish(v[:begin])} {copy} s.i=0;s.pc=1;return; }}
case 1: {{ {alias} {emit_text(v[opening+1:end])} {copy} ++s.i;s.pc=s.i==uint(iter_count)?2:1;return; }}
case 2: {{ {alias} {finish(v[end+1:]).replace('return;',copy+' return;')} }}'''
        extra='uint i;';observe=observer(fields,[('uint','i',0)])
        regions=['initialize_or_early_exit','gnarly_iteration','distance_and_trap_writeback']
        manual_copy=True
    elif formula_id == 10220:
        begin=locate(v,['if','(','d1','>','0','&&','count','>','0','&&','count','<=','999',')'])
        loop=locate(v,['for','(','uint','i','=','0',';','i','<','count',';','++','i',')'])
        start=loop+14; end=v.index(';',start)
        if loop!=begin+14:raise ValueError('Unexpected columns loop position')
        fields=[('int','apply',0),('FormulaVec3','local',0),('numeric::X87Real','d1',0),('numeric::X87Real','d2',0),('uint','count',0)]
        decl,alias,copy=fields_code(fields)
        cases=f'''case 0: {{ {emit_text(v[:begin])} {copy} s.i=0;
 s.pc=(d1>0&&count>0&&count<=999)?1:2; return; }}
case 1: {{ {alias} {emit_text(v[start:end+1])} ++s.i; if(s.i==count)s.pc=2; return; }}
case 2: {{ {alias} {finish(v[end+1:])} }}'''
        extra='uint i;'; observe=observer(fields,[('uint','i',0)])
        regions=['initialize','column_fold_iteration','distance_and_writeback']
    elif formula_id == 10321:
        begin=locate(v,['for','(','int','i','=','0',';','i','<','8',';','i','++',')'])
        opening=begin+14;end=matching(v,opening)
        fields=[('int','pvals',8),('numeric::X87Real','terms',8),('float','results',4)]
        decl,alias,copy=fields_code(fields)
        cases=f'''case 0: {{ {emit_text(v[:begin])} {copy} s.i=0; s.pc=1; return; }}
case 1: {{ {alias} int i=int(s.i); {emit_text(v[opening+1:end])} ++s.i; if(s.i==8)s.pc=2; return; }}
case 2: {{ {alias} {finish(v[end+1:])} }}'''
        extra='uint i;'; observe=observer(fields,[('uint','i',0)])
        regions=['initialize','quaternion_term','final_writeback']
    else:
        raise ValueError(f'No audited continuation cuts for formula {formula_id}')
    if not manual_copy:cases=cases.replace('return;',copy+' return;')
    code=f'''// Generated experimental cuts. Never include in production dispatch.
namespace mb3d_native {{ namespace continuation_{formula_id} {{
#ifndef MB3D_P2_SHARED_FRAME_TYPES
struct Frame {{ uint pc; {decl} {extra} FormulaVec3 result; }};
#endif
{observe}
inline void step(thread Frame& s,FormulaVec3 p,FormulaParams params,thread float& de) {{
switch(s.pc) {{ {cases}
case 99: return;
default: params.fault->code=InvalidFormulaAbi; s.pc=99; return;
}} }}
}} }}
'''
    return code, {'formula_id':formula_id,'source_sha256':hashlib.sha256(source.encode()).hexdigest(),'regions':regions,'live_fields':fields,'scope':'raw body proof only; wrapper/scene continuation unsupported'}


def shared_frame_header(codes):
    text = ['''#pragma once
// Generated from the audited raw recipes. Private, replaceable layout.
#include "formula_abi.h"
#define MB3D_P2_SHARED_FRAME_TYPES 1
namespace mb3d_native {
#if defined(MB3D_METAL_NATIVE_FLOAT)
// Compute callers include formula_math.metal.h before this header.
using ContinuationFrameVector=FormulaVec3;
#else
struct ContinuationFrameVector { float x,y,z; };
#endif
static_assert(sizeof(ContinuationFrameVector)==12,"continuation vector layout");
''']
    for id_,code in codes:
        frame=re.search(r'struct Frame \{.*?\};',code,re.S).group()
        frame=frame.replace('numeric::X87Real','float').replace('FormulaVec3','ContinuationFrameVector')
        frame=re.sub(r'\buint\b','U32',frame);frame=re.sub(r'\bint\b','I32',frame)
        text.append(f'namespace continuation_{id_} {{ {frame} }}\n')
    return ''.join(text)+'} // namespace mb3d_native\n'


def original_observer(id_, source):
    """Instrument unchanged original control flow, without resume/replay."""
    v=body(source)
    def end(v, copy=''):
        # An observer may write typed snapshots, but original return expressions
        # and mathematical statement order are not regenerated from the stepper.
        result=emit_text(v)
        def ret(m):
            return '{ auto result='+m[1]+'; s.result=result; s.pc=99; '+copy+' o.record(s,params,de); return result; }'
        return re.sub(r'\breturn\s+([^;]+);',ret,result)
    def record(pc,copy):return copy+f' s.pc={pc}; o.record(s,params,de);'
    if id_==10274:
        b=locate(v,['do','{']);e=matching(v,b+1)
        copy='for(uint k=0;k<3;++k)s.v[k]=v[k];s.maxDepth=maxDepth;s.axiom=axiom;s.threshold=threshold;s.count=count;s.work_steps=work.steps;'
        loop=end(v[b+2:e],copy)
        loop=re.sub(r'\bbreak\s*;', '{ '+record(2,copy)+' break; }',loop)
        text=emit_text(v[:b])+record(1,copy)+'do {'+loop+record(1,copy)+'}while(true);'+end(v[e+6:],copy)
    elif id_==10265:
        polygon=locate(v,['if','(','polygon',')']);po=polygon+4;pe=matching(v,po)
        wh=locate(v,['while','(','theta','>=','half_angle',')']);wo=wh+6;we=matching(v,wo)
        names='Scale xOff pixel_x pixel_y pixel_z yz_angle Sides s23 s13 s12 pi_const Cx Cy Cz x y z T1_Count rotate polygon theta half_angle double_half'.split()
        copy=''.join(f's.{n}={n};' for n in names)
        text=emit_text(v[:wo+1])+emit_text(v[wo+1:we])+'if(theta>=half_angle){'+record(3,copy)+'}'+emit_text(v[we:pe+1])
        text+='auto result=finish_value(x,y,z,Cx,Cy,Cz,pixel_x,pixel_y,pixel_z);s.result=result;s.pc=99;o.record(s,params,de);return result;'
    elif id_==10089:
        b=locate(v,['for','(','int','i','=','0',';','i','<','iter_count',';','i','++',')']);op=b+14;e=matching(v,op)
        names='Beta Scale Amplitude Alpha Step Gamma OTrapClrMul x y z yy xx ref_axis Apply_scale_add Flip_XZ iter_count'.split()
        copy=''.join(f's.{n}={n};' for n in names)
        text=end(v[:b])+record(1,copy)+emit_text(v[b:e])+copy+'s.i=i+1;s.pc=s.i==uint(iter_count)?2:1;o.record(s,params,de);'+emit_text(v[e:e+1])+end(v[e+1:],copy)
    elif id_==10220:
        b=locate(v,['if','(','d1','>','0','&&','count','>','0','&&','count','<=','999',')'])
        loop=locate(v,['for','(','uint','i','=','0',';','i','<','count',';','++','i',')']);start=loop+14;e=v.index(';',start)
        copy='s.apply=apply;s.local=local;s.d1=d1;s.d2=d2;s.count=count;'
        text=emit_text(v[:b])+copy+'s.pc=(d1>0&&count>0&&count<=999)?1:2;o.record(s,params,de);'
        text+=emit_text(v[b:start])+'{'+emit_text(v[start:e+1])+copy+'s.i=i+1;s.pc=(s.i==count)?2:1;o.record(s,params,de);}'
        text+=end(v[e+1:],copy)
    elif id_==10321:
        b=locate(v,['for','(','int','i','=','0',';','i','<','8',';','i','++',')']);op=b+14;e=matching(v,op)
        copy='for(uint k=0;k<8;++k){s.pvals[k]=pvals[k];s.terms[k]=terms[k];}for(uint k=0;k<4;++k)s.results[k]=results[k];'
        text=emit_text(v[:b])+record(1,copy)+emit_text(v[b:e])+copy+'s.i=i+1;s.pc=s.i==8?2:1;o.record(s,params,de);'+emit_text(v[e:e+1])+end(v[e+1:],copy)
    else:raise ValueError('No old observer for unaudited body')
    return f'''namespace mb3d_native {{ namespace continuation_{id_} {{
inline FormulaVec3 original_observed(FormulaVec3 p,FormulaParams params,thread float& de,thread ProbeObserver& o) {{
using namespace formula_{id_}; Frame s{{}}; {text}
}}
}} }}'''


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--source-root',type=Path,default=ROOT)
    ap.add_argument('--output',type=Path,required=True)
    args=ap.parse_args()
    # Never overwrite production lowering outputs, including through symlinks.
    output=args.output.resolve();production=(ROOT/'src/render/native_metal/generated').resolve()
    if output==production or output.is_relative_to(production):ap.error('Separate experimental output required')
    output.mkdir(parents=True,exist_ok=True)
    math_path=args.source_root/'src/render/native_metal/formula_math.metal.h'
    if hashlib.sha256(math_path.read_bytes()).hexdigest()!=AUDITED_MATH_SHA256:
        ap.error('Formula helper changed: repeat the atomic-region audit')
    rows=[];codes=[]
    for id_ in REGION_ORDER:
        p=args.source_root/f'src/render/native_metal/generated/formula_{id_}.metal.h'
        code,row=generate(id_,p.read_text());codes.append((id_,code));(output/f'continuation_{id_}.metal.h').write_text(code);rows.append(row)
        (output/f'observer_{id_}.metal.h').write_text(original_observer(id_,p.read_text()))
    (output/'continuation_frames.h').write_text(shared_frame_header(codes))
    (output/'manifest.json').write_text(json.dumps({'status':'EXPERIMENTAL_NOT_QUALIFIED','rows':rows},indent=2)+'\n')

if __name__=='__main__':main()
