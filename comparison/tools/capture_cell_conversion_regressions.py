"""Capture immutable original CODE around masked FIST32 conversion limits."""
import argparse
import json
import math
import os
from pathlib import Path
import struct
from capture_formula_regressions import literal
from check_formula_scene_points import execute,sha
from external_validation import ROOT,decode,prepare_external_options,slots,write_json
from gen_bulk_formula_table import parse_m3f_options


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    for k in ('cpp','oracle','output','header'):ap.add_argument('--'+k,type=Path,required=True)
    a=ap.parse_args();a.output=a.output.resolve();a.output.mkdir(parents=True,exist_ok=False)
    ledger=json.loads((ROOT/'comparison/results/stage-a-20261018/coverage-ledger-v1.json').read_bytes())
    bases={r['case']:r for r in ledger['external_cases']}
    values=[0.,-.0,.5,-.5,1.5,-1.5,-2147483648.,2147483647.,2147483648.,-2147483649.,4294967295.]
    for x in (-2147483648.5,2147483647.5):values.extend([math.nextafter(x,-math.inf),x,math.nextafter(x,math.inf)])
    inputs=[[v,-v,.5,.375,.2,-.3,.4] for v in values]
    inputs += [[v,.25,-.5,.375,.2,-.3,.4] for v in values]
    inputs += [[.25,v,-.5,.375,.2,-.3,.4] for v in values]
    os.environ.pop('MB3D_POINT_FIRST',None);os.environ.pop('MB3D_POINT_COUNTERS',None)
    os.environ['MB3D_POINT_DERIVATIVES']='1'
    stdin=''.join(' '.join(format(v,'.17g') for v in r+[1.,-2.,.375,3.])+'\n' for r in inputs)
    (a.output/'inputs.txt').write_text(stdin,encoding='ascii')
    report=dict(scope=__doc__,oracle_sha256=sha(a.oracle),cpp_parameter_probe_sha256=sha(a.cpp),generator_sha256=sha(Path(__file__)),inputs_sha256=sha(a.output/'inputs.txt'),cases=[],rows=[])
    for name in ['RandCubesIFS','hextgrid4IFS','tritgrid2IFS','RoomDoorsIFS','_recFold']:
        source=ROOT/bases['challenge_'+name+'_early']['scene'];base=decode(source)
        slot=prepare_external_options(base,next(s for s in slots(base) if s['name']==name));offset=908+slot['slot']*188
        variants=range(7) if name=='RandCubesIFS' else [-2147483648,1,2,1000,1001,2147483647] if name=='_recFold' else [0]
        asset=ROOT/'mb3d/M3Formulas'/f'{name}.m3f';meta=parse_m3f_options(asset)
        code=a.output/(name+'.bin');code.write_bytes(bytes.fromhex(asset.read_text(encoding='latin1').split('[CODE]')[1].split('[END]')[0]))
        for variant in variants:
            data=bytearray(base)
            if name=='RandCubesIFS':
                for index in (10,11):struct.pack_into('<d',data,offset+index*8,float(variant))
            if name=='_recFold':struct.pack_into('<d',data,offset+4*8,float(variant))
            case=f'cell_{name}_{variant}';scene=a.output/(case+'.m3p');scene.write_bytes(data)
            blob=a.output/(case+'.params');execute([a.cpp.resolve(),name,scene,blob,'--dump-only'],'')
            expected=execute([a.oracle.resolve(),code,blob]+(['difs'] if meta['de_option'] in (20,21) else []),stdin)
            if len(expected)!=len(inputs) or any(len(r)!=20 for r in expected):raise ValueError('incomplete original output')
            options=list(struct.unpack_from('<16d',data,offset))
            report['cases'].append(dict(id=case,name=name,scene=str(scene.relative_to(ROOT)),scene_sha256=sha(scene),parameter_sha256=sha(blob),code_sha256=sha(code)))
            for xyz,out in zip(inputs,expected):report['rows'].append(dict(name=name,option_count=slot['option_count'],options=options,input=xyz,output=[format(v,'.17g') for v in out],case_id=case))
    write_json(a.output/'golden.json',report)
    header='// Immutable original x86 CODE outputs; capture_cell_conversion_regressions.py.\nstruct FormulaRegression { const char* name; int option_count; double options[16], input[7], output[20]; const char* case_id; };\nstatic const FormulaRegression formulaRegressions[] = {\n'
    for row in report['rows']:
        arrays=', '.join('{'+', '.join(map(literal,row[k]))+'}' for k in ('options','input','output'))
        header+='    {'+json.dumps(row['name'])+', '+str(row['option_count'])+', '+arrays+', '+json.dumps(row['case_id'])+'},\n'
    a.header.write_text(header+'};\n',encoding='utf-8');print(len(report['rows']),'original rows')


if __name__=='__main__':main()
