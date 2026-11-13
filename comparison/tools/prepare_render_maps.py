"""Small ordinary production-resource controls; no synthetic map callbacks."""
import argparse
from collections import Counter
import json
from pathlib import Path
import struct

from external_validation import decode, slots, write_json, prepare_external_options
from prepare_effective_modes import ROOT, capture
from prepare_stage_a_modes import rel, sha
from prepare_lighting_modes import qualify
from image_diff import DEFAULT_THRESHOLDS
from audit_ordinary_source import Audit


def stage_exact(path, payload):
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists() and path.read_bytes() != payload:
        raise ValueError('Refusing to replace unrelated resource: ' + str(path))
    path.write_bytes(payload)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--pascal-exe', type=Path, required=True)
    ap.add_argument('--render-reference', action='store_true')
    ap.add_argument('--preprocess-only', action='store_true')
    ap.add_argument('--formula-only', action='store_true')
    args = ap.parse_args(); out=args.output.resolve(); exe=args.pascal_exe.resolve()
    out.mkdir(parents=True, exist_ok=False)
    bitmap=ROOT/'comparison/fixtures/M3Maps/9001.bmp'
    resources={'9001.bmp':bitmap.read_bytes()}
    # A nonconstant 16-bit PGM enters the production 48-bit map buffer path.
    resources['9057.pgm']=b'P5\n32 16\n65535\n'+b''.join(struct.pack('>H',(x*1709+y*2741)%65536) for y in range(16) for x in range(32))
    resources['9058.pgm']=('P2\n32 16\n65535\n'+'\n'.join(' '.join(str((x*1709+y*2741)%65536) for x in range(32)) for y in range(16))+'\n').encode()
    identities=[]
    for name,payload in resources.items():
        for path in (out/'assets'/name, ROOT/'M3Maps'/name, exe.parent/'M3Maps'/name):
            stage_exact(path,payload)
        identities.append(dict(filename=name,sha256=sha(payload),bytes=len(payload)))
    bgname='stagea9001.bmp'
    for path in (ROOT/bgname,exe.parent/bgname):stage_exact(path,resources['9001.bmp'])
    base=decode(ROOT/'comparison/results/stage-a-20261018/effective-modes-v1/scenes/effective_control.m3p')
    definitions=[]
    def add(name,data,control=None,relation='intervention',purpose=''):
        definitions.append((name,data,control,relation,purpose))
    def diffuse(data,number,projection):
        data[435]=number&255;data[548]=number>>8;data[556]=(data[556]&~3)|projection
        data[580]=30 # map scale = 1
    for squared in (0,1):
        prefix='squared' if squared else 'linear'
        control=bytearray(base);control[439]=squared
        add(prefix,control)
        for projection in range(4):
            data=bytearray(control);diffuse(data,9001,projection)
            add(f'{prefix}_diffuse{projection}',data,prefix,purpose=f'loaded diffuse projection{projection}')
        data=bytearray(control);data[500]=2;struct.pack_into('<H',data,507,9001)
        data[509:516]=bytes(7);data[517:524]=bytes(7);data[525:532]=bytes(7)
        add(f'{prefix}_light',data,prefix,purpose='loaded directional light map')
        for screen in (0,1):
            data=bytearray(control);data[816:840]=bgname.encode().ljust(24,b'\0')
            tb=struct.unpack_from('<I',data,476)[0]&~(0x8000|0x80000)
            struct.pack_into('<I',data,476,tb|(0x8000 if screen else 0));data[644]=40
            add(f'{prefix}_background{screen}',data,prefix,purpose='loaded background '+('screen' if screen else 'sphere'))
        data=bytearray(control);diffuse(data,65530,0)
        add(f'{prefix}_missing_diffuse',data,prefix,'permutation','missing diffuse map falls back to gradient')
    for kind in ('diffuse','light'):
        data=bytearray(base)
        if kind=='diffuse':diffuse(data,9057,0)
        else:data[500]=2;struct.pack_into('<H',data,507,9057)
        add(f'pgm16_{kind}',data,'linear',purpose='16-bit PGM production decoding and '+kind+' consumption')
    if args.preprocess_only:
        definitions=[]
        control=bytearray(base);control[439]=0
        add('linear',control)
        for kind in ('diffuse','light'):
            data=bytearray(control)
            if kind=='diffuse':diffuse(data,9058,0)
            else:data[500]=2;struct.pack_into('<H',data,507,9058)
            add(f'p2_{kind}',data,'linear',purpose='Original Pascal ASCII P2 loading and '+kind+' consumption')
        background=bytearray(control);background[816:840]=bgname.encode().ljust(24,b'\0');background[644]=40
        tb=struct.unpack_from('<I',background,476)[0]&~(0x8000|0x80000)
        struct.pack_into('<I',background,476,tb)
        add('background_plain',background,'linear')
        for name,smooth,convert,border in [('smooth',1,0,0),('sphere',0,1,0),('border',0,0,1),('combined',1,1,1)]:
            data=bytearray(background);struct.pack_into('<I',data,476,tb|(0x80000 if smooth else 0));data[439]=(128 if convert else 0)|(16 if border else 0)
            add('background_'+name,data,'background_plain',purpose='Original background resource preprocessing '+name)
    if args.formula_only:
        definitions=[]
        sources=[('HeightMapIFS','comparison/scenes_validation/heightmap_real_asset.m3p'),
                 ('SphereHeightMap','comparison/scenes/extall_dIFS_SphereHeightMap.m3p')]
        for name,source in sources:
            form=ROOT/'mb3d/M3Formulas'/(name+'.m3f')
            stage_exact(exe.parent/'M3Formulas'/form.name,form.read_bytes())
            identities.append(dict(filename=form.name,sha256=sha(form.read_bytes()),bytes=form.stat().st_size))
            original=decode(ROOT/source);struct.pack_into('<ii',original,4,96,72)
            selected=next(slot for slot in slots(original) if slot['name']==name)
            prepare_external_options(original,selected)
            # External slots are loaded by name and canonicalized to number20.
            struct.pack_into('<i',original,848+selected['slot']*188+4,20)
            for kind,number in [('bmp',9001),('p2',9058)]:
                data=bytearray(original);struct.pack_into('<d',data,848+selected['slot']*188+60,float(number))
                add(name+'_'+kind,data,None if kind=='bmp' else name+'_bmp',
                    purpose='Production formula callback with independently loaded '+kind+' resource')
    audit=Audit();cases=[]
    for name,data,control,relation,purpose in definitions:
        path=out/'scenes'/('maps_'+name+'.m3p');path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(data)
        cases.append(dict(id=path.stem,scene=rel(path),sha256=sha(data),slots=slots(data),target_formulas=[slot['name'] for slot in slots(data) if slot['number']>=20],
            control='maps_'+control if control else None,expected_relation=relation,purpose=purpose,requested_selectors=audit.decode(data)))
    manifest=dict(schema_version=1,scope='finite production map-resource interventions',assets=identities,cases=cases)
    write_json(out/'manifest.json',manifest)
    if not args.render_reference:return
    exe_sha=sha(exe.read_bytes());report=dict(schema_version=1,finished=False,batches=[],pascal_sha256=exe_sha,
        manifest_sha256=sha((out/'manifest.json').read_bytes()),thresholds=DEFAULT_THRESHOLDS,results=[],
        assets=identities,provenance=dict(pascal_exe=str(exe),pascal_sha256=exe_sha,tool_sha256=sha(Path(__file__).read_bytes())))
    for case in cases:
        if sha(exe.read_bytes())!=exe_sha:raise ValueError('Reference changed')
        row=capture(out,case,exe);row['scene_sha256']=case['sha256'];report['results'].append(row)
        report['counts']=dict(Counter(r['status'] for r in report['results']))
        write_json(out/'summary.json',report);print(case['id'],row['status'],flush=True)
    qualify(out,manifest,report);report['finished']=True;write_json(out/'summary.json',report)
    print(json.dumps(dict(Counter(r['qualification'] for r in report['controls']))))


if __name__=='__main__':main()
