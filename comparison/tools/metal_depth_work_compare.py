"""Declared scheduling-only change for full depth-scene optimization."""
import copy
from run_m3parameter_gpu import require

def same_work(new,old,size):
    n=copy.deepcopy(new);o=copy.deepcopy(old)
    if n['execution']['ao_algorithm']==16:
        pixels=size[0]*size[1]
        require(o['execution']['ao_rng_advances']==n['execution']['ao_rng_advances']==0,'Random DE AO unexpectedly selected')
        # 6 render workers: CPU task_rows=floor(height/(6*4)); old one pixel
        # per task/command; new 4096 independent pixels plus its ABI marker.
        require(o['execution']['ao_dispatches']==2+max(1,size[1]//24)*size[0],'Unexpected old DE AO schedule')
        require(n['execution']['ao_dispatches']==3+(pixels+4095)//4096,'Unexpected new DE AO schedule')
        n['execution']['ao_dispatches']=o['execution']['ao_dispatches']
    require(n==o,'Old/new work changed beyond audited DE AO dispatch count')
    return True
