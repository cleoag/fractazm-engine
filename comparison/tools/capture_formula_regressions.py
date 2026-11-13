"""Capture original x86 outputs for small, durable C++ regression fixtures.

The C++ probe supplies parameter expansion only. Expected outputs always come
from the shipped formula bytes, executed by the independent 32-bit oracle.
"""
import argparse
import json
import math
import os
from pathlib import Path
from check_formula_scene_points import execute, sha, parse_m3f_options, rounding_coordinates, nonfinite_coordinates, orbit_scale_coordinates, quaternion_boundary_coordinates, require_independent_point_context
from external_validation import ROOT, decode, slots, write_json


def literal(value):
    x = float(value)
    if math.isnan(x): return 'NAN'
    if math.isinf(x): return '-INFINITY' if x < 0 else 'INFINITY'
    if x == 0.0 and math.copysign(1.0, x) < 0: return "-0.0"
    return format(x, '.17g')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    for key in ('cpp', 'oracle', 'manifest', 'output', 'header'):
        ap.add_argument('--' + key, type=Path, required=True)
    ap.add_argument('--case', action='append', required=True)
    ap.add_argument('--sine-inputs', action='store_true', help='Include large coordinates and period boundaries')
    ap.add_argument('--cancellation-inputs', action='store_true', help='Include signed-weight power cancellation coordinates')
    ap.add_argument('--counter-inputs', action='store_true', help='Include persistent bFirstIt counter boundaries')
    ap.add_argument('--rout-inputs', action='store_true', help='Stored radius values independent of XYZ')
    ap.add_argument('--spheremap-callback', action='store_true', help='_MapTranslate analytic callback RGB=(z,x,mapNr)')
    ap.add_argument('--heightmap-callback', action='store_true', help='HeightMapIFS analytic map callback, Rold/trap state and UV outputs')
    ap.add_argument('--origin-input', action='store_true', help='Include the singular all-zero XYZ point')
    ap.add_argument('--rounding-inputs', action='store_true', help='Half-tile and integer conversion boundary neighbors')
    ap.add_argument('--nonfinite-inputs', action='store_true', help='All XYZ combinations of finite, NaN and signed infinities')
    ap.add_argument('--orbit-scale-inputs', action='store_true', help='Powers of two across the Double exponent range and FSINCOS range boundaries')
    ap.add_argument('--map-context-inputs',action='store_true',help='Vary sphere callback Rold and trap state')
    ap.add_argument('--quaternion-inputs', action='store_true', help='QuatPow signed-zero, epsilon-neighbor and exceptional XYZW inputs')
    a = ap.parse_args()
    a.output.mkdir(parents=True, exist_ok=False)
    manifest = json.loads(a.manifest.read_bytes())
    by_id = {c['id']: c for c in manifest['cases']}
    os.environ['MB3D_POINT_DERIVATIVES'] = '1'
    inputs = [[-2., -.125, .5, .375], [.125, -.25, 0., 3.],
              [0., 0., 2., .375], [.125, .25, 0.01, .375],
              [-1e-12, 0., -2., .375], [0., -1e-12, -1e-12, .375]]
    if a.quaternion_inputs:
        inputs += quaternion_boundary_coordinates()
    if a.orbit_scale_inputs:
        inputs += orbit_scale_coordinates()
    if a.nonfinite_inputs:
        inputs += nonfinite_coordinates()
    if a.rounding_inputs:
        inputs += rounding_coordinates()
    if a.origin_input:
        inputs += [[0., 0., 0., .375]]
    if a.sine_inputs:
        inputs += [[v, -v, v, v] for v in
                   (1e12, -1e12, math.pi, -math.pi,
                    math.nextafter(math.pi/2, -math.inf),
                    math.nextafter(math.pi/2, math.inf))]
    if a.cancellation_inputs:
        inputs += [[-2., -1e-12, -1e-12, .375],
                   [-1e-12, -2., -1e-12, .375],
                   [-1e-12, -1e-12, -2., .375]]
    if a.rout_inputs:
        if a.counter_inputs or a.heightmap_callback: raise ValueError('Incompatible input lanes')
        os.environ['MB3D_POINT_ROUT']='1'
        inputs=[xyz+[radius] for xyz in inputs for radius in (-float('inf'),-4.,-2.,-1.,-0.,0.,1e-30,.25,2.,4.,13.5,float('inf'),float('nan'))]
    if a.counter_inputs:
        os.environ['MB3D_POINT_FIRST'] = '1'
        # The probe shares the first-counter and Deriv1 input lane. Preserve
        # that same value explicitly in the fixture instead of hiding it.
        inputs = [xyz + [count, count, .375, 3.] for xyz in inputs
                  for count in (-2147483648, -1, 0, 1, 2, 2147483647)]
    if a.heightmap_callback:
        if a.counter_inputs: raise ValueError('Incompatible input lanes')
        os.environ.pop('MB3D_POINT_DERIVATIVES', None)
        os.environ['MB3D_POINT_HEIGHTMAP_CALLBACK']='1'
        # Exact clipping planes for Hscale=-.5,Hoffset=.375,identity matrix.
        for plane in (-1.125,-.125):
            inputs += [[.125,-.25,z,.375] for z in
                       (math.nextafter(plane,-math.inf),plane,math.nextafter(plane,math.inf))]
        inputs=[xyz+[rold,trap] for xyz in inputs for rold in (-1.,0.,.001,1.,float('nan')) for trap in (0.,-.375)]
    if a.map_context_inputs:
        if not a.spheremap_callback or a.counter_inputs or a.rout_inputs or a.heightmap_callback:raise ValueError('Map context requires sphere callback only')
        os.environ.pop('MB3D_POINT_DERIVATIVES',None)
        os.environ['MB3D_POINT_MAP_CONTEXT']='1'
        inputs=[xyz+[rold,trap] for xyz in inputs for rold in (-1.,0.,.001,1.,float('nan')) for trap in (0.,-.375)]
    stdin = ''.join(' '.join(format(v, '.17g') for v in xyz[:4] + [.2, -.3, .4, 1.]
                    + (xyz[5:8] if a.counter_inputs else [-2., .375, 3.])) + '\n'
                    for xyz in inputs)
    if a.rout_inputs:
        stdin=''.join(' '.join(format(v,'.17g') for v in xyz[:4]+[.2,-.3,.4,xyz[4],-2.,.375,3.])+'\n' for xyz in inputs)
    if a.heightmap_callback or a.map_context_inputs:
        stdin=''.join(' '.join(format(v,'.17g') for v in xyz[:4]+[.2,-.3,.4]+xyz[4:6]+[1.,1.])+'\n' for xyz in inputs)
    if a.spheremap_callback:
        if a.heightmap_callback: raise ValueError('Incompatible callbacks')
        os.environ['MB3D_POINT_SPHEREMAP_CALLBACK']='1'
    lanes=25 if a.spheremap_callback else (24 if a.heightmap_callback else 20)
    rows = []
    for case_id in a.case:
        case = by_id[case_id]
        scene = ROOT / case['scene']
        if sha(scene) != case['sha256']:
            raise ValueError('Scene changed: ' + case_id)
        name = case['target_formulas'][0]
        if name == 'ManowarBulb':
            raise ValueError('ManowarBulb requires explicit SSE state; use check_formula_sse_state.py')
        asset = ROOT / 'mb3d/M3Formulas' / (name + '.m3f')
        code = a.output / (name + '.bin')
        code.write_bytes(bytes.fromhex(asset.read_text(encoding='latin1').split('[CODE]')[1].split('[END]')[0]))
        blob = a.output / (case_id + '.params')
        execute([a.cpp.resolve(), name, scene.resolve(), blob.resolve()], stdin, lanes)
        require_independent_point_context(name,blob.read_bytes())
        meta = parse_m3f_options(asset)
        expected = execute([a.oracle.resolve(), code.resolve(), blob.resolve()] +
                           (['difs'] if meta['de_option'] in (20, 21) else []), stdin, lanes)
        if len(expected) != len(inputs) or any(len(row) != lanes for row in expected):
            raise ValueError('Incomplete original output')
        slot = next(s for s in slots(decode(scene)) if s['name'] == name)
        options = slot['options']
        for xyz, output in zip(inputs, expected):
            rows.append(dict(case=case_id, name=name, options=options, option_count=slot['option_count'], input=xyz,
                             output=[format(x, '.17g') for x in output],
                             scene_sha256=sha(scene), asset_sha256=sha(asset),
                             parameter_sha256=sha(blob)))
    report = dict(scope=__doc__, cpp_sha256=sha(a.cpp), oracle_sha256=sha(a.oracle),
                  manifest_sha256=sha(a.manifest), tool_sha256=sha(Path(__file__)),
                  input_profile_sha256=sha(Path(__file__).with_name('check_formula_scene_points.py')),
                  spheremap_callback=a.spheremap_callback, heightmap_callback=a.heightmap_callback, stored_radius=a.rout_inputs,
                  map_context_inputs=a.map_context_inputs,
                  map_reference=('RGB=(z,x,mapNr); projection/interpolation excluded' if a.spheremap_callback else ('RGB=(u,v,mapNr); texture interpolation excluded' if a.heightmap_callback else None)), rows=rows)
    write_json(a.output / 'golden.json', report)

    header = '// Original x86 outputs; generated by capture_formula_regressions.py.\n'
    width = 6 if a.heightmap_callback or a.map_context_inputs else (8 if a.counter_inputs else (5 if a.rout_inputs else 4))
    header += f'struct FormulaRegression {{ const char* name; int option_count; double options[16], input[{width}], output[{lanes}]; const char* case_id; }};\n'
    header += 'static const FormulaRegression formulaRegressions[] = {\n'
    for row in rows:
        arrays = [row['options'], row['input'], row['output']]
        header += '    {"' + row['name'] + '", ' + str(row['option_count']) + ', ' + ', '.join('{' + ', '.join(map(literal, xs)) + '}' for xs in arrays) + ', ' + json.dumps(row['case']) + '},\n'
    header += '};\n'
    a.header.write_text(header, encoding='utf-8')
    print(len(rows), 'original-code rows')


if __name__ == '__main__':
    main()
