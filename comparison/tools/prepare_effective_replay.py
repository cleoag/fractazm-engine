"""Adapt immutable effective-mode evidence to the shared replay contract."""
import json
import shutil
from pathlib import Path
from prepare_effective_modes import ROOT, OUT, sha
from external_validation import write_json

def main():
    target=OUT/'replay-reference'
    if target.exists():raise ValueError('Replay adapter already exists')
    manifest=json.loads((OUT/'manifest.json').read_text());summary=json.loads((OUT/'summary.json').read_text())
    assert summary['finished'] and len(manifest['cases'])==len(summary['results'])
    target.mkdir();(target/'pascal').mkdir()
    report=dict(summary,batches=[],provenance=dict(original_summary=str(OUT/'summary.json'),original_summary_sha256=sha((OUT/'summary.json').read_bytes()),
        original_manifest_sha256=sha((OUT/'manifest.json').read_bytes()),adapter_sha256=sha(Path(__file__).read_bytes()),
        note='Metadata-only adaptation. Original status/PNG/log bytes preserved. Trace-instability remains explicit in original summary.'))
    for case,row in zip(manifest['cases'],report['results']):
        assert case['id']==row['scene']
        row['scene_sha256']=case['sha256']
        image=OUT/'pascal'/(case['id']+'.png');log=OUT/row['pascal_log']
        assert sha(image.read_bytes())==row['pascal_image_sha256'] and sha(log.read_bytes())==row['pascal_log_sha256']
        shutil.copyfile(image,target/'pascal'/image.name);shutil.copyfile(log,target/log.name)
    write_json(target/'summary.json',report)
    frozen=ROOT/'comparison/bin/stage-a-20261018/v7/mb3d_render.exe'
    frozen.parent.mkdir(parents=True,exist_ok=True)
    if frozen.exists():raise ValueError('Frozen candidate exists')
    source=ROOT/'build-vs-fixes/Release/mb3d_render.exe';before=sha(source.read_bytes());shutil.copyfile(source,frozen)
    assert before==sha(frozen.read_bytes())==sha(source.read_bytes())
    write_json(frozen.parent/'identity.json',dict(source=str(source),frozen=str(frozen),sha256=before))
    print(frozen, before)

if __name__=='__main__':main()
