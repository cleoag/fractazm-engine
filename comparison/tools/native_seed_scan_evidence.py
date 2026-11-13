"""Derive the only permitted physical-work mapping for GPU seed preparation."""
import copy

LIMIT=262144


def scan_commands(width,height,owners,rows):
    assert 0<owners<=height and 0<rows<=height
    items=width*rows;levels=[];commands=1 # actual ABI marker dispatch
    while True:
        groups=(items+255)//256;levels.append(items)
        commands+=(owners*groups*256+LIMIT-1)//LIMIT
        if groups==1:break
        items=groups
    commands+=sum((owners*n+LIMIT-1)//LIMIT for n in levels[:-1])
    commands+=(owners*levels[0]+LIMIT-1)//LIMIT
    return commands


def normalize(work,size,threads):
    result=copy.deepcopy(work);e=result['execution'];width,height=size
    workers=min(threads,height)
    primary=e.pop('primary_seed_scan_commands');ao=e.pop('ao_seed_generation_commands')
    expected_primary=scan_commands(width,height,workers,(height+workers-1)//workers) if e['primary_random_mode']==2 else 0
    expected_ao=0
    if '|ao-de-seeded' in e['execution_plan']:
        assert e['ao_enabled'] and e['ao_algorithm']==16
        rows=height if workers==1 else max(1,height//(workers*4));owners=(height+rows-1)//rows
        expected_ao=(width*height+LIMIT-1)//LIMIT+scan_commands(width,height,owners,rows)
    assert type(primary) is int and primary==expected_primary,'Unexpected primary seed command count'
    assert type(ao) is int and ao==expected_ao,'Unexpected AO seed command count'
    assert e['ao_commands']>=ao and e['ao_dispatches']>=ao
    e['ao_commands']-=ao;e['ao_dispatches']-=ao
    return result,{'primary_seed_commands':primary,'ao_seed_commands':ao}
