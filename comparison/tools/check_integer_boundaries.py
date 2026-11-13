"""Compact original-code checks for source-qualified INTEGER boundary families.

No arbitrary large recursion is executed. Coupled options rotate through a
small boundary list so each changed native field is checked independently. The
original x86 CODE and retained parameter buffer provide the numerical oracle;
loader correctness is a separate obligation. MC/JIT are excluded.
"""
import argparse
from collections import Counter
import json
import math
import os
from pathlib import Path
import struct

from external_validation import ROOT, decode, digest, prepare_external_options, slots, write_json
from generate_external_challenges import ensure_conditional_partners
from check_formula_scene_points import execute, sha
from gen_bulk_formula_table import parse_m3f_options

MIN, MAX = -(1 << 31), (1 << 31)-1


def plans():
    result=[]
    def add(name, indices, values, reason):
        for k in range(len(values)):
            result.append((name,{index:values[(k+j)%len(values)] for j,index in enumerate(indices)},reason))
    for name,indices,limit in [('_CondItcountR',[3,4,5],99999),
        ('_CondItcountSwitch',[0,2,3,4],100),('_CondItcountSwX2',[0,2,3,4,5,7,8,9],100),
        ('_CondLyapunovEx',[3,4],100)]:
        add(name,indices,[MIN,-1,0,1,2,limit-1,limit,limit+1,MAX],'signed clamp and coupled writes')
    for name in ['_CondLyapunov','_CondLyapunovEx']:
        add(name,[0],[0,*[1<<i for i in range(15)],32767,32768,MIN,MAX],'each selected bit plus ignored high bits')
    add('_recFold',[4],[MIN,-1,0,1,2,999,1000,1001,MAX],'bounded DEC count, including overflow skip')
    for name in ['_RotateRnd','_TranslateRnd']:
        add(name,[3,4,5],[MIN,MIN+1,-1,0,1,16382,16383,16384,MAX-1,MAX],'low14-bit modular product and counter wrap')
    add('ASurfSmoothFold',[4,6],[0,1,2,3,7,8,31,32,MIN,-1,MAX],'unsigned exponent zero/one, sparse/dense low/high bits')
    for name,indices in [('BT1Pinegon',[6,7,8,9,10]),('BT1Pinehedron',[6,7,8,9])]:
        add(name,indices,[MIN,-129,-128,-1,0,1,2,127,128,255,256,257,MAX],'low-byte equality, signed threshold, bottom-tested count')
    add('Knots2IFS',[11],[-1,2],'once versus repeated bottom-tested loop; large counts deliberately excluded')
    add('RandCubesIFS',[8,9],[MIN,MIN+1,-1,0,1,MAX-1,MAX],'wrapped seed hash with signed cell boundaries')
    add('RoomDoorsIFS',[11],[MIN,-1,MAX],'period bypass and unsigned large divisor')
    add('SphCageIFS',[7],[MIN,1,1001,MAX],'signed saturation to existing2/1000 classes')
    add('TrifoxComplexAngles',[0,3],[4,7,8,1<<30],'sparse/dense unsigned exponent loops; zero excluded')
    add('_e_3Dtess',[7],[MIN],'signed low depth; positive huge work deliberately excluded')
    add('_IFS_tess',[7],[1,2],'short unsigned depth limits; negative/large work deliberately excluded')
    # Append to keep previously retained boundary case identifiers stable.
    add('ASurfSmoothFold',[4,6],[4],'power-of-two exponent boundary')
    return result


def inputs(name):
    conditional=name.startswith('_Cond')
    if conditional:
        return [[*xyz,.375,.2,-.3,.4,counter,mask,7,1]
            for xyz in [(0.,0.,0.),(.125,-.25,.5),(3.,-2.,1.)]
            for counter in [MIN,-2,-1,*range(18),98,99,100,101,MAX]
            for mask in (0,21,63)]
    counters=range(256) if name.startswith('BT1Pine') else [MIN,-1,0,1,2,127,128,255,256,32767,32768,65535,65536,MAX]
    xyzs=[(.125,-.25,.5),(0.,0.,0.),(1.,-1.,2.),(3.,-2.,1.)]
    if name in ('RandCubesIFS','RoomDoorsIFS'):
        xyzs += [(x,-x,x) for x in [math.nextafter(.5,0),.5,math.nextafter(.5,1),-2147483648.,2147483647.,4294967295.]]
    return [[*xyz,.375,.2,-.3,.4,1,counter,1,1] for xyz in xyzs for counter in counters]


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--cpp',type=Path,required=True);ap.add_argument('--oracle',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True);ap.add_argument('--only',default='')
    a=ap.parse_args();a.output=a.output.resolve();a.output.mkdir(parents=True,exist_ok=True)
    audit=json.loads((ROOT/'comparison/results/stage-a-20261018/integer-domain-ledger-v2.json').read_bytes())
    metadata={(r['formula'],r['stored_value_index']):r for r in audit['options']}
    ledger=json.loads((ROOT/'comparison/results/stage-a-20261018/coverage-ledger-v1.json').read_bytes())
    bases={r['case']:r for r in ledger['external_cases']}
    result=dict(scope=__doc__,cpp_sha256=sha(a.cpp),oracle_sha256=sha(a.oracle),tool_sha256=sha(Path(__file__)),
                relative_tolerance=2e-12,results=[],finished=False)
    for n,(name,changes,reason) in enumerate(plans()):
        if a.only and name not in a.only.split(','):continue
        source=ROOT/bases['challenge_'+name+'_early']['scene'];data=decode(source)
        slot=prepare_external_options(data,next(s for s in slots(data) if s['name']==name))
        for index,value in changes.items():struct.pack_into('<d',data,908+188*slot['slot']+8*index,float(value))
        # Activate all three random-transform axes, otherwise a seed could be
        # nominally selected while its arithmetic remains dormant.
        if name in ('_RotateRnd','_TranslateRnd'):
            for i,v in enumerate((.75,-.5,1.25)):struct.pack_into('<d',data,908+188*slot['slot']+8*i,v)
        partners=ensure_conditional_partners(data,name)
        case=f'boundary_{n:03}_{name}';scene=a.output/(case+'.m3p');scene.write_bytes(data)
        blob=a.output/(case+'.params');code=a.output/(name+'.bin')
        asset=ROOT/'mb3d/M3Formulas'/f'{name}.m3f';meta=parse_m3f_options(asset)
        code.write_bytes(bytes.fromhex(asset.read_text(encoding='latin1').split('[CODE]')[1].split('[END]')[0]))
        conditional=name.startswith('_Cond');lanes=26 if conditional else 20
        os.environ.pop('MB3D_POINT_COUNTERS',None);os.environ.pop('MB3D_POINT_FIRST',None)
        os.environ['MB3D_POINT_COUNTERS' if conditional else 'MB3D_POINT_FIRST']='1'
        rows=inputs(name);stdin=''.join(' '.join(format(v,'.17g') for v in r)+'\n' for r in rows)
        row=dict(case=case,name=name,changes=changes,reason=reason,scene=str(scene.relative_to(ROOT)),
                 scene_sha256=sha(scene),source=str(source.relative_to(ROOT)),source_sha256=sha(source),
                 code_sha256=sha(code),inputs_sha256=digest(stdin.encode()),rows=len(rows),lanes=lanes,status='error')
        (a.output/(case+'.input')).write_text(stdin,encoding='ascii')
        try:
            actual=execute([a.cpp.resolve(),name,scene,blob],stdin,lanes)
            p=blob.read_bytes();row['parameter_sha256']=sha(blob);checks=[]
            for index,value in changes.items():
                offset=metadata[(name,index)]['pvar_offset'];offset=int(offset,0) if isinstance(offset,str) else offset
                loaded=struct.unpack_from('<i',p,512+offset)[0]
                checks.append(dict(index=index,offset=offset,expected=value,loaded=loaded))
            row['native_parameter_checks']=checks
            if any(x['expected']!=x['loaded'] for x in checks):raise ValueError('effective parameter mismatch')
            expected=execute([a.oracle.resolve(),code,blob]+(['difs'] if meta['de_option'] in (20,21) else []),stdin,lanes)
            if len(actual)!=len(rows) or len(expected)!=len(rows) or any(len(r)!=lanes for r in actual+expected):raise ValueError('incomplete output')
            errors=[];max_error=0.;exact=0
            for i,(xs,ys) in enumerate(zip(actual,expected)):
                for k,(x,y) in enumerate(zip(xs,ys)):
                    equal=(math.isnan(x) and math.isnan(y)) or (x==y and (x!=0 or math.copysign(1,x)==math.copysign(1,y)))
                    exact+=equal
                    err=abs(x-y)/max(1.,abs(y)) if math.isfinite(x) and math.isfinite(y) else 0 if equal else math.inf
                    max_error=max(max_error,err)
                    if err>2e-12 and len(errors)<8:errors.append(dict(row=i,lane=k,cpp=str(x),original=str(y),input=rows[i]))
            row.update(status='fail' if errors else 'pass',max_scaled_error=max_error,exact_or_nan_class_lanes=exact,examples=errors)
            write_json(a.output/(case+'.original.json'),expected);write_json(a.output/(case+'.cpp.json'),actual)
        except Exception as exc:row['error']=str(exc)
        result['results'].append(row);result['counts']=dict(Counter(r['status'] for r in result['results']))
        write_json(a.output/'summary.json',result);print(case,row['status'],row.get('error',row.get('examples',[])),flush=True)
    result['finished']=True;write_json(a.output/'summary.json',result);print(result['counts'])


if __name__=='__main__':main()
