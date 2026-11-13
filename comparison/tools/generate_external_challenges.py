"""Deterministic, labelled challenges for every external asset.

Retains the old baseline and adds early execution / Julia / option probes.
These finite probes are NOT exhaustive branch coverage. Control influence and
Pascal parity must both be measured before claiming numerical coverage.
"""
import argparse, json, math, struct, re
from pathlib import Path
from external_validation import (ROOT, decode, slots, digest, write_json,
                                 saved_option_types, prepare_external_options)


def set_slot(data, i, name, metadata, count=1):
    off=848+i*188
    data[off:off+188]=bytes(188)
    types=saved_option_types(metadata)
    values=metadata['option_defaults']
    if len(types)>16 or len(values)>16: raise ValueError('option overflow: '+name)
    struct.pack_into('<iii',data,off,count,20,len(types))
    data[off+12:off+44]=name.encode('ascii').ljust(32,b'\0')
    data[off+44:off+60]=bytes(types).ljust(16,b'\0')
    struct.pack_into('<16d',data,off+60,*(values+[0.]*(16-len(values))))


CONDITIONAL_TARGET_OPTIONS = {
    '_CondItcountR': (2,), '_CondItcountSwitch': (1,),
    '_CondItcountSwX2': (1, 6), '_CondLyapunov': (1, 2),
    '_CondLyapunovEx': (1, 2),
}


def ensure_conditional_partners(data, name):
    """Keep a counted partner outside the conditional formula's write targets.

    A transform followed by a single power formula can loop forever in Pascal
    when that power's count is set to zero. Populate all six slots so every
    clamped target exists, and retain at least one untouched counted partner.
    Apply this after each option mutation: it can change the target slot itself.
    This changes generated inputs, never the renderer's formula semantics.
    """
    indices = CONDITIONAL_TARGET_OPTIONS.get(name)
    if indices is None:
        return None
    target = next(s['slot'] for s in slots(data) if s['name'] == name)
    off = 848 + 188 * target
    written = {max(1, min(6, int(struct.unpack_from('<d', data, off+60+8*i)[0])))-1
               for i in indices}
    power = bytearray(188)
    struct.pack_into('<iii', power, 0, 1, 0, 2)
    power[12:44] = b'Integer Power'.ljust(32, b'\0')
    struct.pack_into('<2d', power, 60, 2., -1.)
    for slot in range(6):
        if slot != target:
            data[848+188*slot:1036+188*slot] = power
    # Include every partner in both the first pass and the repeating loop.
    data[845] = 5
    protected = next(i for i in range(6) if i != target and i not in written)
    return dict(written_slots=sorted(written), protected_counted_slot=protected,
                partner='Integer Power 2', repeat_from_slot=0, end_slot=5)


def generate(manifest, output):
    from gen_bulk_formula_table import parse_m3f_options
    m=json.loads(manifest.read_text()); output.mkdir(parents=True,exist_ok=True)
    cases=[]
    for f in m['formulas']:
        meta=parse_m3f_options(str(ROOT/f['asset']))
        old=next((c for c in m['cases'] if c['id'] in f['cases'] and c['id'].endswith('_'+re.sub(r'[^A-Za-z0-9_]', '_', f['name']))),None)
        if old: base=decode(ROOT/old['scene']); source=old['scene']
        else:
            source='comparison/scenes/extall_Ads__Folding1.m3p' if meta['de_option']<0 or f['name']=='JITTrigExample' else 'comparison/scenes/extall_3D_BristorBrot.m3p'
            base=decode(ROOT/source)
            set_slot(base,1 if 'Ads' in source else 0,f['name'],meta)
        struct.pack_into('<ii',base,4,64,48)
        active=slots(base)
        target=next(s for s in active if s['name'].casefold()==f['name'].casefold())
        target=prepare_external_options(base, target)
        active=slots(base)
        # Put transforms before the escaping fractal, with one iteration each.
        blocks=[bytes(base[848+s['slot']*188:1036+s['slot']*188]) for s in (active if meta['de_option']>=20 else sorted(active,key=lambda s:s['slot']!=target['slot']))]
        for i,block in enumerate(blocks):
            base[848+i*188:1036+i*188]=block
            struct.pack_into('<i',base,848+i*188,1)
        for i in range(len(blocks),6):struct.pack_into('<i',base,848+i*188,0)
        target_offset=848+next(s['slot'] for s in slots(base) if s['name'].casefold()==f['name'].casefold())*188
        variants=[('early',bytearray(base),{'order':'preserved-dIFS' if meta['de_option']>=20 else 'target-first','repeat':1})]
        if meta['de_option'] < 0 and len(blocks) > 1:
            # Apply an auxiliary transform once, then repeat the fractal.
            # bHybOpt1 packs end-slot in the low nibble and repeat-from in high.
            once=bytearray(base);once[845]=(len(blocks)-1) | 0x10
            variants.append(('once',once,{'order':'target-first','repeat_from_slot':1}))
        julia=bytearray(base);julia[190]=1;struct.pack_into('<3d',julia,191,.173,-.231,.097)
        variants.append(('julia',julia,{'julia':[.173,-.231,.097]}))
        # One-at-a-time parameter probes leave map identifiers untouched.
        # Angles and compound values are the saved user values, not PVar bytes.
        value_types=saved_option_types(meta)
        for i,value in enumerate(meta['option_defaults']):
            if meta['map_value_mask'] & (1<<i):continue
            if not math.isfinite(value):continue
            for label,new in [('zero',0.),('alternate',1. if value==0 else -value if value<0 else value*.75)]:
                if new==value:continue
                # Integer option schemas use exact integers; compound integer
                # packing requires a dedicated fixture and is left explicit.
                if i<len(value_types) and value_types[i]==2:
                    new=0. if label=='zero' else float(int(value)+1)
                probe=bytearray(base);struct.pack_into('<d',probe,target_offset+60+i*8,new)
                variants.append((f'p{i}_{label}',probe,{'parameter':i,'from':value,'to':new}))
        for label,data,changes in variants:
            # The once case intentionally skips the conditional on repeats.
            previous_repeat = data[845] & 0xf0
            partners = ensure_conditional_partners(data, f['name'])
            if partners:
                if label == 'once':
                    data[845] |= previous_repeat
                    partners['repeat_from_slot'] = previous_repeat >> 4
                changes = {**changes, 'conditional_partners': partners}
            name='challenge_'+f['name']+'_'+label
            path=output/(name+'.m3p');path.write_bytes(data)
            cases.append({'id':name,'target_formulas':[f['name']],'scene':str(path.resolve().relative_to(ROOT)),
                          'sha256':digest(data),'source':source,'source_sha256':digest((ROOT/source).read_bytes()),
                          'kind':'finite-challenge','changes':changes,'slots':slots(data)})
    report={'schema_version':1,'scope':__doc__,'source_manifest_sha256':digest(manifest.read_bytes()),'cases':cases}
    write_json(output/'manifest.json',report)
    # The fast first pass selects early/Julia only. Parameter cases remain
    # inventoried, not silently claimed as tested.
    write_json(output/'activation.json',{**report,'cases':[c for c in cases if c['id'].endswith(('_early','_julia'))]})
    print('generated',len(cases),'cases for',len(m['formulas']),'assets')


def downloaded(study_path, output):
    """Preserve downloaded hybrid settings/camera; reduce only image dimensions."""
    study=json.loads(study_path.read_text());output.mkdir(parents=True,exist_ok=True)
    cases=[]
    for i,record in enumerate(study['scenes']):
        source=Path(record['path']); relative=source.relative_to('mb3d/M3Parameter')
        normalized=study_path.parent/'normalized'/relative
        data=decode(normalized);w,h=struct.unpack_from('<ii',data,4)
        scale=64/max(w,h);width=max(16,round(w*scale));height=max(16,round(h*scale))
        struct.pack_into('<ii',data,4,width,height)
        name=f'downloaded_{i:03}_'+re.sub(r'[^A-Za-z0-9_]', '_',source.stem)
        path=output/(name+'.m3p');path.write_bytes(data)
        cases.append({'id':name,'target_formulas':sorted({s['name'] for s in slots(data) if s['number']>=20 and s['name']}),
                      'scene':str(path.resolve().relative_to(ROOT)), 'sha256':digest(data),
                      'source':str(source),'source_sha256':record['sha256'],
                      'normalized_source_sha256':record['normalized_sha256'],
                      'changes':{'width':width,'height':height},'slots':slots(data),
                      'kind':'downloaded-native-normalized'})
    write_json(output/'manifest.json',{'schema_version':1,'scope':__doc__,'cases':cases})
    print('prepared',len(cases),'downloaded scenes')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--manifest',type=Path);p.add_argument('--study',type=Path);p.add_argument('--output',type=Path,required=True)
    a=p.parse_args()
    if a.study:downloaded(a.study,a.output)
    elif a.manifest:generate(a.manifest,a.output)
    else:p.error('--manifest or --study required')
