"""Map MC Pascal implementation declarations to narrowly scoped measured evidence.

This is a routine inventory and evidence map, not executed line/branch coverage.
Unmapped methods remain unverified even when broad MC image tests exercise them.
"""
import argparse
from collections import Counter
import re
from pathlib import Path
from external_validation import ROOT, digest, write_json
from inventory_pascal_surface import mask_comments_strings
import json


def verified_report(raw, source_path, source_hash):
    document=json.loads(raw)
    report=document.get('evidence',document)
    if report.get('sources',{}).get(str(source_path.resolve())) != source_hash:
        raise ValueError('Reference source hash mismatch')
    if not (report.get('exact') is True or report.get('passed') is True):
        raise ValueError('Unsuccessful evidence')
    if not isinstance(report.get('rows'),int) or report['rows']<=0:
        raise ValueError('Missing measured rows')
    if not report.get('failures') or any(report['failures']):
        raise ValueError('Missing or failing measurements')
    return report


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--evidence',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    a=ap.parse_args(); a.output.mkdir(parents=True,exist_ok=True)
    mapping={
        ('CalcMonteCarlo.pas','TMCCalcThread.CalcRay'):('mc-thresholds.json','partial-refinement-predicate-exact','8,360 inputs; typed tolerance constants and binary refinement predicate only, not complete ray transport'),
        ('CalcMonteCarlo.pas','GetHalton2Dshifts'):('mc-halton-shifts.json','finite-input-bit-exact','335,872 rows; pixel shifts, original seed arithmetic and typed constants'),
        ('CalcMonteCarlo.pas','TMCCalcThread.GetRand'):('mc-halton-shifts.json','finite-input-bit-exact','Output Double bits and updated 32-bit seed; not every seed'),
        ('CalcMonteCarlo.pas','TMCCalcThread.CalculateVgradsFOV'):('mc-camera-angles.json','partial-angle-expressions-exact','8,312 inputs; CAFX/CAFY only, not normalization, view vector or rotation'),
        ('CalcMonteCarlo.pas','halton'):('mc-halton.json','finite-input-bit-exact','272,630 index/base inputs; bases >= 2'),
        ('CalcMonteCarlo.pas','PreComputeHaltonSequence'):('mc-halton.json','exhaustive-table-exact','All 65,536 table entries, all four Word fields'),
        ('CalcMonteCarlo.pas','CalcBokeh'):('mc-sampling.json','finite-input-bit-exact','Disc/bokeh corpus, all eight mode values'),
        ('CalcMonteCarlo.pas','MakeDiscFromHalton'):('mc-sampling.json','finite-input-bit-exact','Disc/bokeh corpus; all measured output float bits'),
        ('CalcMonteCarlo.pas','TMCCalcThread.GetSubPixelShift'):('mc-subpixel.json','finite-input-bit-exact','Uniform/Gaussian inputs, rejection and random consumption'),
        ('CalcMonteCarlo.pas','TMCCalcThread.CalcN'):('mc-adaptive-kernel.json','finite-input-exact-decision','Explicit tail bytes; count and random consumption, not scheduler'),
        ('DivUtils.pas','MCRGBtoDouble'):('mc-codec.json','exhaustive-code-tolerance','All 24-bit codes; 2e-12 relative/absolute, bit differences retained'),
        ('DivUtils.pas','DoubleToMCRGBv'):('mc-codec.json','finite-input-exact-code','Finite packing inputs and neighbor-byte guards'),
        ('DivUtils.pas','NoiseOfMCRGB'):('mc-codec.json','finite-input-tolerance','Same finite corpus; 2e-12 relative/absolute'),
    }
    reports={}; files={}; rows=[]; seen=set()
    for filename in ['CalcMonteCarlo.pas','DivUtils.pas']:
        path=ROOT/'mb3d'/filename; raw=path.read_bytes(); source=raw.decode('latin1')
        files[str(path.relative_to(ROOT))]=digest(raw)
        masked=mask_comments_strings(source); begin=re.search(r'\bimplementation\b',masked,re.I).end()
        for m in re.finditer(r'\b(procedure|function)\s+([A-Za-z_][\w.]*)',masked[begin:],re.I):
            name=m[2]
            if filename=='DivUtils.pas' and not ('MCR' in name or name=='NoiseOfMCRGB'): continue
            key=(filename,name); seen.add(key)
            row=dict(source=str(path.relative_to(ROOT)),line=source.count('\n',0,begin+m.start())+1,
                     name=name,status='unverified',complete=False)
            if key in mapping:
                report_name,status,scope=mapping[key]; evidence_path=a.evidence/report_name
                raw=evidence_path.read_bytes()
                # Every mapping is tied to the immutable source actually used.
                report=verified_report(raw,ROOT/'mb3d'/filename,files[row['source']])
                reports[report_name]=dict(sha256=digest(raw),rows=report['rows'])
                row.update(status=status,evidence=report_name,scope=scope)
            rows.append(row)
    if set(mapping)-seen: raise ValueError('Mapped routine absent from source')
    totals=dict(routines=len(rows),statuses=dict(Counter(r['status'] for r in rows)),complete=False)
    write_json(a.output/'mc-function-coverage.json',dict(scope=__doc__,sources=files,evidence=reports,totals=totals,routines=rows))
    lines=['# Monte Carlo routine evidence','',__doc__.strip(),'',str(totals),'',
           '| Pascal routine | Status | Evidence / boundary |','|---|---|---|']
    for r in rows:
        detail=(f"[{r['evidence']}]({r['evidence']}) — {r['scope']}" if 'evidence' in r else 'No independent routine-level acceptance measurement')
        lines.append(f"| {r['source']}:{r['line']} `{r['name']}` | {r['status']} | {detail} |")
    (a.output/'mc-function-coverage.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')
    print(totals)


if __name__=='__main__':main()
