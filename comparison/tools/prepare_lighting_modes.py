"""Finite ordinary lighting interventions with immutable independent references.

Uses real source light fields while preserving per-slot overlay bytes used for
global color settings. An unchanged image is not reported as reached coverage.
"""
from __future__ import annotations
import argparse
from collections import Counter
import json
from pathlib import Path
import struct

from external_validation import decode, slots, write_json
from prepare_effective_modes import ROOT, EXE, capture
from prepare_stage_a_modes import rel, sha
from image_diff import DEFAULT_THRESHOLDS, load_rgb
from compare_silight5 import read_dump, FIELD_NAMES
from audit_ordinary_source import Audit


def set_position(data, xyz):
    data[500] = 4
    for off, value in zip((509, 517, 525), xyz):
        data[off:off+7] = struct.pack('<d', value)[1:]


def move_light(data, index):
    # AdditionalByteEx/FreeByte are cross-light global settings, not position
    # light attributes. A whole-record swap would alter fog and color modes.
    source = bytes(data[500:532]); target = 500 + 32 * index
    for start, end in ((0, 16), (17, 24), (25, 32)):
        data[target+start:target+end] = source[start:end]
    if index: data[500] = (data[500] & ~3) | 1


def generate(out, visible_only=False):
    source = ROOT/'comparison/results/stage-a-20261018/effective-modes-v1/scenes/effective_control.m3p'
    base = decode(source); definitions = []
    def add(name, data, control=None, relation='intervention', purpose=''):
        definitions.append((name, data, control, relation, purpose))
    add('linear', bytearray(base), purpose='one active directional light')
    for squared in (0, 1):
        arithmetic = 'squared' if squared else 'linear'
        control = bytearray(base); control[439] = squared
        if squared: add(arithmetic, control, 'linear', purpose='squared color arithmetic')
        for function in range(3):
            data = bytearray(control); data[501] = (data[501] & ~48) | function << 4
            add(f'{arithmetic}_diffuse{function}', data, arithmetic, purpose=f'diffuse table {function}')
        for exponent in (0, 7):
            data = bytearray(control); data[501] = (data[501] & ~7) | exponent
            add(f'{arithmetic}_specular{exponent}', data, arithmetic, purpose=f'FastIntPow exponent {2 << exponent}')
    # The camera starts at z=-2: the light must be in front of it for the
    # visible-source compositor, independently of its illumination effect.
    positional = bytearray(base); set_position(positional, (0., 0., -1.5))
    positional[502:504] = bytes((10, 2))  # Lamp=100
    add('positional', positional, 'linear', purpose='active finite point light')
    for squared in (0, 1):
        control = bytearray(positional); control[439] = squared
        prefix = 'pos_squared' if squared else 'positional'
        if squared: add(prefix, control, 'positional', purpose='squared point-light control')
        for shape in (2, 4, 6, 8):
            data = bytearray(control)
            data[500] |= (shape & 6) << 2
            data[501] |= (shape & 8) << 4
            add(f'{prefix}_shape{shape}', data, prefix, purpose=f'visible light kernel {shape}')
    shadow = bytearray(base); shadow[133] = 7
    add('shadow0', shadow, 'linear', purpose='hard shadow active light 0')
    for index in range(1, 6):
        data = bytearray(shadow); move_light(data, index); data[133] = 3 | (4 << index)
        add(f'shadow{index}', data, 'shadow0', 'permutation', f'same source moved to light {index}')
    fog = bytearray(base); struct.pack_into('<i', fog, 452, 100)
    add('fog_far', fog, 'linear', purpose='nonzero dynamic fog')
    data = bytearray(fog); struct.pack_into('<I', data, 476, struct.unpack_from('<I', data, 476)[0] & ~0x40000)
    add('fog_near', data, 'fog_far', purpose='far-fog false branch')
    volume = bytearray(fog); volume[343] = 0x21
    add('volume1', volume, 'fog_far', purpose='directional volume map and integration')
    for index in range(1, 6):
        data = bytearray(volume); move_light(data, index); data[343] = 0x21 + index
        add(f'volume{index+1}', data, 'volume1', 'permutation', f'same volume source moved to light {index}')
    posvol = bytearray(volume); set_position(posvol, (-1., .4, -4.))
    posvol[502:504] = bytes((10, 0))  # Lamp=1 to keep integration finite and visible
    add('volume_pos1', posvol, 'volume1', purpose='six-face positional volume map')
    data = bytearray(posvol); move_light(data, 5); data[343] = 0x26
    add('volume_pos6', data, 'volume_pos1', 'permutation', 'positional source sorting and final light slot')
    if visible_only:
        definitions = [row for row in definitions if row[0] in ('positional', 'pos_squared') or '_shape' in row[0]]
        definitions = [(name, data, None if name == 'positional' else control, relation, purpose)
                       for name, data, control, relation, purpose in definitions]
    audit = Audit(); cases=[]
    for name, data, control, relation, purpose in definitions:
        path=out/'scenes'/('lighting_'+name+'.m3p'); path.parent.mkdir(parents=True, exist_ok=True);path.write_bytes(data)
        cases.append(dict(id=path.stem, scene=rel(path), sha256=sha(data), slots=slots(data), target_formulas=[],
            control='lighting_'+control if control else None, expected_relation=relation, purpose=purpose,
            requested_selectors=audit.decode(data)))
    manifest=dict(schema_version=1,scope='finite ordinary lighting interventions; not all real-valued inputs',
                  source=rel(source),source_sha256=sha(source.read_bytes()),cases=cases)
    write_json(out/'manifest.json',manifest);return manifest


def qualify(out, manifest, report):
    ready={r['scene'] for r in report['results'] if r['status']=='reference_ready'}
    results=[]
    for case in manifest['cases']:
        control=case['control']
        if not control or case['id'] not in ready or control not in ready:continue
        image=load_rgb(out/'pascal'/(case['id']+'.png')); baseline=load_rgb(out/'pascal'/(control+'.png'))
        _,_,packed=read_dump(out/'pascal'/(case['id']+'.silight'))
        _,_,old=read_dump(out/'pascal'/(control+'.silight'))
        changed={name:sum(a[i]!=b[i] for a,b in zip(packed,old)) for i,name in enumerate(FIELD_NAMES)}
        pixels=int((image!=baseline).any(axis=2).sum())
        permutation=case['expected_relation']=='permutation'
        results.append(dict(scene=case['id'],control=control,relation=case['expected_relation'],rgb_changed_pixels=pixels,
            primary_changed_fields=changed,
            qualification='permutation_rgb_exact' if permutation and not pixels else
                          'permutation_differs' if permutation else
                          'image_influence_observed' if pixels else 'unqualified_no_observed_image_effect',
            limitation='Source gate plus controlled intervention, not instrumented branch or all-boundary coverage.'))
    report['controls']=results


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',required=True,type=Path)
    ap.add_argument('--render-reference',action='store_true')
    ap.add_argument('--pascal-exe',type=Path,default=EXE)
    ap.add_argument('--visible-only',action='store_true',help='Ten foreground light controls, including both color arithmetic modes')
    args=ap.parse_args();out=args.output.resolve()
    if out.exists():raise ValueError('Use a new directory; historical evidence is immutable')
    out.mkdir(parents=True);manifest=generate(out,args.visible_only)
    if not args.render_reference:return
    exe=args.pascal_exe.resolve();exesha=sha(exe.read_bytes())
    report=dict(schema_version=1,finished=False,pascal_sha256=exesha,batches=[],thresholds=DEFAULT_THRESHOLDS,
        manifest_sha256=sha((out/'manifest.json').read_bytes()),results=[],
        provenance=dict(pascal_exe=str(exe),pascal_sha256=exesha,tool_sha256=sha(Path(__file__).read_bytes())))
    for case in manifest['cases']:
        if sha(exe.read_bytes())!=exesha:raise ValueError('Reference executable changed during run')
        row=capture(out,case,exe);row['scene_sha256']=case['sha256'];report['results'].append(row)
        report['counts']=dict(Counter(r['status'] for r in report['results']))
        write_json(out/'summary.json',report);print(case['id'],row['status'],row.get('hit_pixels'),flush=True)
    qualify(out,manifest,report);report['finished']=True;write_json(out/'summary.json',report)
    print(json.dumps(dict(Counter(r['qualification'] for r in report['controls']))))


if __name__=='__main__':main()
