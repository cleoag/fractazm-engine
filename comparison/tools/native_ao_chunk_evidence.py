"""Remove only the exactly derived physical AO chunk-preparation overhead."""
import copy
import re


def normalize(work,size,threads):
    result=copy.deepcopy(work);e=result['execution'];plan=e['execution_plan']
    flags={name:re.findall(r','+name+r'=([^,|]+)',plan) for name in ('chunk-prefix','base-table-reuse')}
    assert all(v in ([],['0'],['1']) for v in flags.values()),'Invalid/repeated policy marker'
    enabled=flags['chunk-prefix']==['1']
    assert flags['base-table-reuse']!=['1'] or enabled,'Reuse outside chunk planner'
    extra=0
    if enabled:
        width,height=size;owners=min(threads,height)
        assert 1<=owners<=32 and e['ao_certificate_version']==1 and e['ao_algorithm'] in (4,5,8,9)
        levels=e['ao_certificate_levels'];assert type(levels) is int and levels>0
        assert e['ao_certificate_pixels']==width*height*levels,'Noncanonical AO extent'
        assert re.findall(r'\|rng:[^|]*,chunks=(\d+)(?=\||$)',plan)==['8'],'Unexpected continuation policy'
        calls=(width*((height+owners-1)//owners)+127)//128
        extra=1+levels*((calls*owners+1023)//1024+(calls+7)//8)
        assert e['ao_commands']>=extra and e['ao_dispatches']>=extra
        e['ao_commands']-=extra;e['ao_dispatches']-=extra
    e['execution_plan']=plan.replace(',chunk-prefix=1','').replace(',chunk-prefix=0','').replace(',base-table-reuse=1','').replace(',base-table-reuse=0','')
    return result,{'ao_chunk_preparation_commands':extra}
