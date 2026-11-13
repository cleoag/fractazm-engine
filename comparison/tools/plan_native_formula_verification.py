#!/usr/bin/env python3
"""Partition every pinned ordinary external formula into bounded review groups.

This is a work inventory, not numerical or device evidence. Formula IDs present
in the 77 priority scenes sort before the rest. No scene or formula is omitted
because it lacks a convenient extall fixture.
"""
import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
from qualify_native_metal_all import sha, require

ROOT = Path(__file__).resolve().parents[2]
FIRST = ('_Translate', '_TranslateC4d', '_scaleC', '_ScaleC4d')
FAMILIES = ('Ads', '3D', '3Da', '4D', '4Da', 'dIFS', 'dIFSa', 'native')


def plan(root=ROOT, group_size=16):
    require(type(group_size) is int and 1 <= group_size <= 32, 'Group size must be 1..32')
    contract_path = root/'comparison/contracts/native-gpu-acceptance-v4/acceptance-contract.json'
    contract = json.loads(contract_path.read_text())
    inventory_path = contract_path.parent/contract['inventory']
    require(sha(inventory_path) == contract['inventory_sha256'], 'Pinned inventory changed')
    inventory = json.loads(inventory_path.read_text())
    external = {f['id']: f for f in inventory['formulas'] if f['namespace'] == 'external'}
    require(len(external) == 457, 'Ordinary external denominator changed')
    generated_path = root/'src/render/native_metal/generated/inventory.json'
    rows = json.loads(generated_path.read_text())['formulas']
    require(len(rows) == len(external) and len({f['id'] for f in rows}) == len(rows), 'Duplicate/missing generated formula')
    require({f['id']: f['name'] for f in rows} == {i: f['name'] for i,f in external.items()}, 'Registry identities differ')
    uses = defaultdict(set)
    for scene in inventory['scenes']:
        for slot in scene['slots']:
            if slot['iterations'] > 0 and slot['key'].startswith('external:'):
                uses[int(slot['key'].split(':')[1])].add(scene['id'])
    require(set(uses) <= set(external), 'Unknown phase-one formula')
    require({f['family'] for f in rows} <= set(FAMILIES), 'Unknown source family')
    by_name = {f['name']: f for f in rows}
    require(set(FIRST) <= set(by_name), 'First independent affine group changed')
    groups = []
    def append(label, members):
        groups.append({'id': f'{len(groups)+1:02d}-{label}',
                       'formulas': [{'namespace':'external','id':f['id'],'name':f['name'],
                                     'family':f['family'],'priority_scene_ids':sorted(uses.get(f['id'], ()))} for f in members],
                       'status':'OPEN_NO_DEVICE_CREDIT'})
    append('affine', [by_name[n] for n in FIRST])
    rest = [f for f in rows if f['name'] not in FIRST]
    for priority in (True, False):
        for family in FAMILIES:
            members = sorted((f for f in rest if bool(uses.get(f['id'])) == priority and f['family'] == family), key=lambda f:f['id'])
            for start in range(0,len(members),group_size):
                append(('phase1-' if priority else 'remaining-')+family, members[start:start+group_size])
    selected = [f['id'] for g in groups for f in g['formulas']]
    require(len(selected) == len(set(selected)) == 457 and set(selected) == set(external), 'Partition lost or repeated a formula')
    return {'schema':'mb3d.native-external-verification-plan.v1','qualification':False,
            'contract_sha256':sha(contract_path),'inventory_sha256':sha(inventory_path),
            'generated_inventory_sha256':sha(generated_path),'external_count':457,
            'internal_count':len(inventory['formulas'])-457,'phase1_external_count':len(uses),
            'family_counts':dict(sorted(Counter(f['family'] for f in rows).items())),
            'group_count':len(groups),'groups':groups}


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,required=True)
    p.add_argument('--group-size',type=int,default=16);a=p.parse_args()
    require(not a.output.exists(), 'Use a new output file')
    value=plan(group_size=a.group_size);a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(value,indent=2,ensure_ascii=False)+'\n')
    print(f"{value['external_count']} external formulas / {value['group_count']} groups; {value['phase1_external_count']} used by priority scenes; no qualification credit")

if __name__=='__main__':main()
