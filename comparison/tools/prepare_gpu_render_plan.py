#!/usr/bin/env python3
"""Freeze the common-plan candidate over the last retained GPU package."""
import argparse
import shutil
from run_m3parameter_gpu import ROOT, read, write, sha, require, verify_inputs

CHANGES = [
    'tests/test_native_ao_certificate_gpu.mm',
    'src/render/native_metal/native_ao_certificate_abi.h',
    'src/render/native_metal/native_ao_certificate_math.h',
    'src/render/native_metal/native_ao_certificate.metal.h',
    'src/render/native_metal/native_ao_screen.metal.h',
    'src/render/native_metal/native_ao_readback.h',
    'src/render/native_metal/command_policy.h','src/render/native_metal/command_batch.h',
    'tests/test_native_command_gpu.mm',
    'src/render/native_metal/execution_policy.h',
    'src/render/native_metal/image_execution_policy.h',
    'src/render/native_metal/restricted_execution_policy.h',
    'src/render/native_metal/native_ao_dispatch.h','src/render/native_metal/native_ao_dispatch.mm',
    'tests/test_native_ao_seed_gpu.mm','tests/test_native_dof_gpu.mm','tests/test_native_zbuffer_gpu.mm',
    'comparison/tools/test_native_de_ao_pixels.mm',
    'comparison/tools/backend_render_benchmark.cpp',
    'src/render/native_metal/pixel_state_contract.h',
    'src/render/native_metal/soft_shadow_state_contract.h','src/render/native_metal/camera_soft_shadow.metal.h',
    'src/render/native_metal/smoothing_readback.h',
    'src/render/native_metal/numerical_gradient.metal.h','src/render/native_metal/orbit_driver.metal.h',
    'src/render/native_metal/surface_color_abi.h','src/render/native_metal/surface_color.metal.h',
    'src/render/native_metal/general_geometry_abi.h','src/render/native_metal/general_geometry.metal.h',
    'src/render/native_metal/general_scene_host.h','src/render/native_metal/general_scene_host.cpp',
    'src/render/native_metal/primary_inside_bridge.metal.h','src/render/native_metal/camera_kernels.metal.h',
    'src/render/native_metal/generic_shadow_abi.h','src/render/native_metal/generic_reflection_abi.h',
    'src/render/native_metal/soft_shadow_abi.h',
    'src/render/native_metal/reflection_work_readback.h',
    'src/render/native_metal/generic_reflection_math.metal.h','src/render/native_metal/generic_reflection_kernel.metal.h',
    'tests/test_native_smoothing_readback.cpp','src/render/backend.h','src/cli/main.cpp',
    'tests/native_multi_shadow_cases.h',
    'CMakeLists.txt',
    'src/render/native_metal/postprocess_plan.h',
    'src/render/native_metal/render_plan.h', 'src/render/native_metal/render_plan.cpp',
    'src/render/native_metal/general_renderer.mm',
    'src/render/native_metal/dof_dispatch.h', 'src/render/native_metal/dof_dispatch.mm',
    'src/render/native_metal/zbuffer_dispatch.h', 'src/render/native_metal/zbuffer_dispatch.mm',
    'comparison/tools/phase1_admission.cpp',
    'tests/test_native_postprocess_plan.cpp', 'tests/test_native_render_plan.cpp',
    'src/render/native_metal/material_palette.h','tests/test_native_material_palette.cpp',
    'src/render/native_metal/resource_requirements.h',
    'src/render/native_metal/generic_color_base.cpp',
    'src/render/metal_renderer.mm','tests/test_native_resource_entry.cpp',
    'src/render/native_metal/formula_resources.h','src/render/native_metal/formula_host.cpp',
    'src/core/external_formulas.h',
    'tests/test_native_formula_resources.cpp',
    'comparison/tools/audit_native_formula_resources.py',
    'src/render/native_metal/generic_color_abi.h',
    'src/render/native_metal/generic_color_base.h',
    'src/render/native_metal/generic_color_host.cpp',
    'src/render/native_metal/material_coordinates.metal.h',
    'src/render/native_metal/native_directional_color.metal.h',
    'src/render/native_metal/camera_inside_reflection_math.metal.h',
    'src/render/native_metal/camera_inside_reflection_kernel.metal.h',
    'tests/test_native_logical_lanes_gpu.mm',
    'src/render/native_metal/inside_palette_math.h',
    'tests/test_phase1_color_gpu.mm',
    'comparison/metal_exact/native_reflection_host.cpp',
    'comparison/tools/test_native_reflection_material.mm',
]

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--baseline', type=lambda s: ROOT / s, required=True)
    p.add_argument('--output', type=lambda s: ROOT / s, required=True)
    p.add_argument('--material', action='store_true')
    a = p.parse_args();base=a.baseline.resolve();out=a.output.resolve()
    verify_inputs(base)
    require(not out.exists(), 'Fresh output required')
    require(shutil.disk_usage(out.parent).free >= 13 * 1024**3, 'Disk reserve')
    out.mkdir()
    for name in ['source', 'inputs', 'runtime', 'resources']:
        shutil.copytree(base / name, out / name)
    (out / 'provenance').mkdir()
    shutil.copyfile(base / 'provenance/capture.py', out / 'provenance/capture.py')
    shutil.copyfile(__file__,out/'provenance/prepare.py')
    delta=[]
    overlays=CHANGES+(['src/render/native_metal/generic_exterior_color.metal.h',
        'src/render/native_metal/generic_color.metal.h',
        'src/render/native_metal/camera_color.metal.h',
        'src/render/native_metal/inside_material.metal.h'] if a.material else [])
    for rel in overlays:
        source=ROOT/rel;target=out/'source'/rel
        before=sha(target) if target.exists() else None
        target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(source,target)
        delta.append({'path':rel,'before':before,'after':sha(target)})
    # CTest's source-root arguments must resolve inside the frozen snapshot.
    # Preserve immutable data with explicit hashes rather than reading live fixtures.
    fixture_dirs=['tests/fixtures','tests/reference','comparison/fixtures','comparison/benchmark_scenes']
    fixture_dirs += [str(p.relative_to(ROOT)) for p in (ROOT/'comparison').glob('scenes*') if p.is_dir()]
    fixtures={}
    for rel in fixture_dirs:
        for source in sorted((ROOT/rel).rglob('*')):
            if not source.is_file():continue
            path=source.relative_to(ROOT);target=out/'source'/path
            target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(source,target)
            fixtures[str(path)]=sha(target)
    # The backend negative test deliberately opens this existing non-library.
    shutil.copyfile(ROOT/'README.md',out/'source/README.md')
    fixtures['README.md']=sha(out/'source/README.md')
    write(out/'fixture-manifest.json',{'source':'repository fixtures','files':fixtures})
    manifest=read(base/'source-manifest.json')
    manifest['parent_manifest_sha256']=sha(base/'source-manifest.json')
    manifest['selection']='Frozen Native pipeline candidate with explicitly hashed source overlays'
    manifest['source_files']={str(f.relative_to(out/'source')):sha(f) for f in (out/'source').rglob('*') if f.is_file()}
    write(out/'source-manifest.json',manifest)
    contract=read(base/'contract.json');contract['source_manifest_sha256']=sha(out/'source-manifest.json')
    write(out/'contract.json',contract);write(out/'source-delta.json',delta)
    print('Prepared',out,flush=True)

if __name__=='__main__':main()
