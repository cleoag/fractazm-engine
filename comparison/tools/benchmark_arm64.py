"""Sequential real-scene CPU scaling benchmark; checks exact RGB across workers."""
import argparse
import json
from pathlib import Path
import statistics
from check_arm64_scenes import ROOT, native_identity, render

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--renderer',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--width',type=int,default=320)
    parser.add_argument('--height',type=int,default=240)
    parser.add_argument('--repeats',type=int,default=3)
    parser.add_argument('--work-budget',type=int,default=None,
                        help='Enable shared admission accounting; 0 means unlimited counted work')
    args=parser.parse_args()
    if min(args.width,args.height) < 1: parser.error('Dimensions must be positive')
    if args.repeats < 2: parser.error('At least two measured repeats required')
    if args.work_budget is not None and args.work_budget < 0: parser.error('Work budget must be nonnegative')
    renderer=args.renderer.resolve(); identity=native_identity(renderer)
    args.output.mkdir(parents=True,exist_ok=False)
    scenes=['tests/reference/formula_0_IntPow8.m3p',
            'tests/reference/formula_4_AmazingBox.m3p',
            'comparison/scenes/extall_4D_Magnet4D.m3p',
            'comparison/scenes/extall_4Da_ABoxMod4d.m3p',
            'comparison/scenes/extall_3Da_ABoxSmoothFold.m3p']
    report={'identity':identity,'scope':'Full ordinary CPU renders; includes process startup, load and BMP save; no concurrent benchmark jobs',
            'work_budget':args.work_budget,'results':[],'rgb_identical':True,'status':'RUNNING'}
    for name in scenes:
        baseline=None
        for threads in [1,2,4,8,14]:
            runs=[]
            for repeat in range(args.repeats+1):
                out=args.output/(Path(name).stem+f'-t{threads}-r{repeat}.bmp')
                row=render(renderer,ROOT/name,out,args.width,args.height,threads,args.work_budget,120)
                if row['status']!='rendered': raise RuntimeError(row)
                baseline=baseline or row['rgb_sha256']
                report['rgb_identical'] &= row['rgb_sha256']==baseline
                row['warmup']=repeat==0; runs.append(row)
            report['results'].append({'scene':name,'threads':threads,
                'median_seconds':statistics.median(r['seconds'] for r in runs[1:]),
                'min_seconds':min(r['seconds'] for r in runs[1:]),
                'max_seconds':max(r['seconds'] for r in runs[1:]),'runs':runs})
            (args.output/'summary.json').write_text(json.dumps(report,indent=2)+'\n')
    report['status']='PASS_RGB_CONSISTENCY' if report['rgb_identical'] else 'FAIL'
    (args.output/'summary.json').write_text(json.dumps(report,indent=2)+'\n')
    return 0 if report['rgb_identical'] else 1

if __name__=='__main__': raise SystemExit(main())
