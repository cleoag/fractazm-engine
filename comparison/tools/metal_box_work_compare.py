"""Compare deterministic work; allow only the proved HiQ dispatch regrouping."""
import copy
from run_m3parameter_gpu import require

def same_work(candidate, baseline):
 c=copy.deepcopy(candidate);b=copy.deepcopy(baseline)
 e=c['execution'];old=b['execution']
 if e.get('ao_enabled') and e.get('ao_algorithm')==2:
  n=e['primary_records'];records=e['ao_records'];repeats=e['ao_requested_repeats']
  require(n>0 and repeats>0 and e['ao_levels']%(n*repeats)==0,'Invalid HiQ level accounting')
  levels=e['ao_levels']//(n*repeats)
  def count(batch):return 2+repeats*((records+batch-1)//batch+(3*levels-2)*((n+batch-1)//batch))
  require(old['ao_dispatches']==count(1024) and e['ao_dispatches']==count(16384),'Unexpected HiQ dispatch count')
  # Explicitly explained scheduling count only. No sampling/work fields dropped.
  e['ao_dispatches']=old['ao_dispatches']
 require(c==b,'Image-preserving scheduling changed non-dispatch work')
 return True
