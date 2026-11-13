"""Adapt 31 retained independent Pascal captures to the common image replayer.

Only metadata changes: scene, original PNG and load-log hashes are checked and
preserved. This does not execute either renderer or certify any numerical result.
"""
import argparse
import json
from pathlib import Path
import shutil

from external_validation import ROOT, decode, digest, slots, write_json
from pascal_formula_identity import validate_formula_load


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output',type=Path,required=True)
    a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=False)
    (a.output/'pascal').mkdir()
    base=ROOT/'comparison/runs/stage-a-20261018'
    cases,rows,identities,executables=[],[],{},set()
    for folder,subfolder,direct in [('selectors-27-v1','pascal-reference',False),
                                    ('original-direct-four-v1','images-v8',True)]:
        manifest=base/folder/'scenes/manifest.json'
        reference=base/folder/subfolder/'summary.json'
        original=json.loads(reference.read_bytes())
        identities[str(reference.relative_to(ROOT))]=digest(reference.read_bytes())
        identities[str(manifest.relative_to(ROOT))]=digest(manifest.read_bytes())
        executables.add(original['pascal_sha256'])
        by_id={r['case']:r for r in original['results']}
        for case in json.loads(manifest.read_bytes())['cases']:
            name=case['id'];r=by_id[name];scene=ROOT/case['scene']
            if digest(scene.read_bytes())!=case['sha256'] or case['sha256']!=r['scene_sha256']:
                raise ValueError('Changed original scene: '+name)
            png=reference.parent/(name+('-pascal' if direct else '')+'.png')
            log=reference.parent/(name+('-pascal' if direct else '')+'.log')
            png_sha=r['pascal_image_sha256' if direct else 'image_sha256']
            log_sha=r['pascal_log_sha256' if direct else 'log_sha256']
            if digest(png.read_bytes())!=png_sha or digest(log.read_bytes())!=log_sha:
                raise ValueError('Changed original capture: '+name)
            case=dict(case,slots=slots(decode(scene)))
            loaded=validate_formula_load(log.read_text(errors='replace'),scene,case['slots'])
            shutil.copyfile(png,a.output/'pascal'/(name+'.png'))
            shutil.copyfile(log,a.output/(name+'.log'))
            cases.append(case)
            rows.append(dict(scene=name,scene_sha256=case['sha256'],status='reference_captured',
                pascal_image_sha256=png_sha,pascal_log=name+'.log',pascal_log_sha256=log_sha,
                pascal_formula_load=loaded,source_summary=str(reference.relative_to(ROOT))))
    if len(executables)!=1 or len(cases)!=31:
        raise ValueError('Unexpected reference set')
    manifest=a.output/'manifest.json';write_json(manifest,dict(cases=cases))
    write_json(a.output/'summary.json',dict(scope=__doc__,finished=True,batches=[],
        manifest_sha256=digest(manifest.read_bytes()),pascal_sha256=executables.pop(),results=rows,
        provenance=dict(original_inputs=identities,metadata_adapter_sha256=digest(Path(__file__).read_bytes()),
            scope='Original PNG/log bytes unchanged; this adaptation is not numerical execution.')))
    print('Prepared',len(cases),'verified original references')


if __name__=='__main__':main()
