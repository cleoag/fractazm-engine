"""Vendor independent original real-driver outputs; never generate golden C++ data."""
import argparse,hashlib,json,math,shutil,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--reference',type=Path,required=True);ap.add_argument('--manifest',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
 summary=json.loads((a.reference/'summary.json').read_bytes());assert summary['finished']
 for n,h in summary['artifact_sha256'].items():assert sha(a.reference/n)==h
 cases={c['id']:c for c in json.loads(a.manifest.read_bytes())['cases']};inputs=[[float(x)for x in l.split()]for l in(a.reference/'inputs.txt').read_text().splitlines()]
 a.output.mkdir(parents=True,exist_ok=True);data=bytearray(struct.pack('<8sI',b'MANOHYB1',len(summary['results'])))
 for row in summary['results']:
  assert row['plain_trace_repeat_invariant'] and all(x['status']=='PASS'for x in row['independent_loader'])
  c=cases[row['id']];scene=ROOT/c['scene'];assert sha(scene)==c['sha256'];shutil.copy2(scene,a.output/(c['id']+'.m3p'))
  rt=row['runtime'];data+=struct.pack('<64s8i',c['id'].encode(),rt['driver_id'],rt['DEoption'],rt['start'],rt['repeat'],rt['end'],*rt['counts'],len(inputs))
  rows=[[math.nan if 'nan'in x.lower()else float(x)for x in l.split()]for l in(a.reference/c['id']/'pas_trace.stdout').read_text().splitlines()]
  for inp,r in zip(inputs,rows):
   data+=struct.pack('<9d12d3i',*inp,*r[:12],*(int(v)for v in r[12:15]))
   for i in range(int(r[14])):
    j=15+i*5;data+=struct.pack('<i4d',int(r[j]),*r[j+1:j+5])
 (a.output/'original.bin').write_bytes(data)
 (a.output/'identity.json').write_text(json.dumps({'scope':__doc__,'reference_summary':str(a.reference/'summary.json'),'reference_summary_sha256':sha(a.reference/'summary.json'),'source_identities':summary['identities'],'files':{p.name:sha(p)for p in a.output.iterdir()if p.is_file()and p.name!='identity.json'},'cases':len(summary['results']),'orbits':len(summary['results'])*len(inputs),'comparison_tolerance':2e-12},indent=2))
if __name__=='__main__':main()
