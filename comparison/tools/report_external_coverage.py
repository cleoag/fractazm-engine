"""Join image, influence and point evidence without conflating their claims."""
import argparse, csv, json
from collections import Counter
from pathlib import Path
from external_validation import ROOT, write_json, digest, compact_image_evidence, merge_provenance_records

def latest_image_cases(images, expected):
    """Merge only the named scene universe, preserving failures and identity errors."""
    latest = {}
    for results in images.values():
        for result in results:
            name = result['scene']
            if name not in expected:
                continue
            item = dict(result)
            if item.get('scene_sha256') != expected[name]:
                item.update(status='error', error='Scene hash differs from current manifest')
            latest[name] = item
    return latest

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--run',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
    a.output.mkdir(parents=True,exist_ok=True)
    evidence={}
    def read(rel):
        p=a.run/rel
        if not p.exists():return {}
        raw = p.read_bytes()  # Match the digest to one atomic checkpoint read.
        evidence[str(p.relative_to(ROOT) if p.is_absolute() else p)] = digest(raw)
        return json.loads(raw)
    m=read('manifest.json');ch=read('challenges-v4/manifest.json');study=read('native-study/study.json');points=read('points-expanded-fixed2/summary.json')
    downloaded=read('downloaded-scenes-native/manifest.json')
    images={}
    provenance_records={}
    phases=['baseline','active-parity','corrected-parity','julia-fix-parity','once-parity','downloaded-parity','active-replay-v2','once-replay-v2','parameter-parity-batch','parameter-replay-final','parameter-replay-first-state-v3','parameter-replay-iteration-trap-v2','five-formula-images-v1','pinehedron-images-v1','smooth-fold-images-v1','vsshapes-images-v1','parameter-replay-20261015-v1','complex-power-images-v1','heightfield-images-v1','fold-stairs-images-v2','julia-state-images-v2','partly-julia-images-v1','transform-state-images-v2','sine-approx-images-v2','state-writeback-images-v4','packed-normals-images-v1','cayley-images-v2','ifs-images-v2','ifs-reference-retry-v1','gear-foldcut-images-v2','logspiral-images-v1','iteration-rotation-images-v2','random-transform-images-v1','gear2-images-v1','hexgrid-images-v2','hexgrid-family-images-v1','discoball-images-v1','sphcage-images-v1','riemann2-images-v1','hycinv-images-v1','mag-sign-images-v1','tiling-rucker-images-v1','tiling-rucker-images-v2','boxifs-tiling-images-v1','rucker-limiter-images-v1','boxifs-twist-images-v1','boxifs-camera-images-v1','menger-original-images-v1','mandalex-images-v1','mandalay-images-v1','parameter-replay-mandalay-v1','mandalex-reference-retry-v1','bulbox-images-v1','celtic-images-v1','sphereoffset4d-images-v1','platinum-b-images-v2','knots-images-v1','kleinbot-images-v1','general-images-v1','ifs-tess-images-v4','priority-images-v1','priority-images-v2','amazing-surf-images-v2','heightmap-images-v1','aboxsmooth-images-v1','spherefold-images-v2','state-batch-images-v2','fold-rounding-images-v1','rootcause-images-v2','contract-batch-images-v1','jit-reference-audit-v1','shape-state-images-v1','shape-missing-reference-image-v1','orbit-shapes-three-images-v3','orbit-missing-images-v2','manowar-images-v1','room-triplex-images-v1','fold-cut-images-v1','room-fold-current-images-v1','crossbifold-missing-images-v1','remaining-images-radius-order-v1','box-tetra-pine-images-v1','gnarly-family-images-v2','gnarly-missing-image-v1','dodeca-wavy-images-v1','dodeca-wavy-missing-images-v1','axis-wave-box-images-v1','map-icen-images-v1','iq-hex-images-v1','iq-hex-missing-images-v1','slice-coord-images-v1','slice-coord-missing-images-v1','angle-impact-images-v1','angle-missing-images-v1','fold-kali-tube-images-v1','fold-kali-tube-missing-images-v1','fold-kali-tube-images-v2','fold-kali-tube-missing-images-v2','iteration-depth-impact-images-v2','varyscale-pair-images-v1','amazingbox-sse2-images-v1','projection-reciprocal-images-v1','devil-citrus-images-v1','magnet-bicomplex-images-v2','angular-radius-images-v1','heightmap-family-images-v1','branch-julia-images-v2','ifs-apply-images-v1','fold-order-images-v2','knots-weird-images-v1','fold-poincare-images-v1','last-image-images-v1','remaining-points-images-v1']
    runs={}
    expected_hashes={c['id']:c['sha256'] for c in ch['cases']+downloaded.get('cases',[])}
    for phase in phases:
        by_case={}
        for p in sorted((a.run/phase).glob('*/summary.json'))+list((a.run/phase).glob('summary.json')):
            report=read(str(p.relative_to(a.run)))
            merge_provenance_records(report.get('provenance_records',{}),provenance_records)
            for r in report.get('results',[]):
                item={**r,'evidence':str(p.relative_to(a.run)), 'binary_sha256':report.get('cpp_sha256') or report.get('provenance',{}).get('cpp_sha256')}
                item=compact_image_evidence(item,provenance_records)
                if r['scene'] in expected_hashes and r.get('scene_sha256')!=expected_hashes[r['scene']]:
                    item.update(status='error',error='Scene hash differs from current manifest')
                by_case[r['scene']]=item
            runs[str(p.relative_to(a.run))]={'finished':bool(report.get('finished') or report.get('ended')),'counts':dict(Counter(r['status'] for r in report.get('results',[])))}
            for key in ['scope','requested_formulas','selected_case_count','unselected_case_count','pending_reference_cases','reference_finished','requested_cases','case_list_sha256']:
                if key in report: runs[str(p.relative_to(a.run))][key]=report[key]
        images[phase]=list(by_case.values())
    influence={r['case']:r for r in read('parameter-influence/summary.json').get('results',[])}
    influence.update({r['case']:r for r in read('parameter-influence-corrected/summary.json').get('results',[])})
    influence.update({r['case']:r for r in read('once-influence/summary.json').get('results',[])})
    pointmap={r['name']:r for r in points.get('formulas',[])}
    broad={}
    for phase in ['scene-points-v4','four-d-points','transform-points','scene-points-final','gencwarp-points-v2','cstate-points-v1','scene-points-cstate-v1','folding-c-points-v2','intpow-c-points-v3','difs-state-points-v1','menger-points-v2','tetratube-points-v1','deriv-trap-points-v1','iteration-trap-default-v2','grid-wave-points-v2','orbit-color-default-v2','menger4ex-points-v2','four-d-default-v3','fern-points-v1','pinehedron-points-v1','smooth-fold-points-v1','scene-points-current-v1','vsshapes-points-v1','complex-power-points-v1','heightfield-points-v1','fold-stairs-points-v2','scene-points-current-v2','julia-state-points-v2','partly-julia-points-v1','scene-points-current-v3','transform-state-points-v2','sine-approx-points-v2','state-writeback-points-v2','pinegon-points-v1','cayley-points-v1','ifs-points-v2','scene-points-current-v4','gear-points-v2','logspiral-points-v1','iteration-rotation-points-v2','random-transform-points-v1','gear2-points-v1','hexgrid-points-v2','hexgrid-family-points-v1','scene-points-current-v5','discoball-points-v1','sphcage-points-v1','riemann2-points-v1','hycinv-points-v1','scene-points-current-v6','mag-sign-points-v1','tiling-rucker-rounding-v2','boxifs-nonfinite-v2','boxifs-twist-points-v1','menger-original-points-v1','mandalex-points-v1','mandalay-points-v1','scene-points-current-v7','scene-points-current-v8','bulbox-points-v1','celtic-points-v1','sphereoffset4d-points-v1','platinum-b-points-v1','knots-points-v1','kleinbot-points-v1','scene-points-current-v9','ifs-tess-points-v2','cantor-points-v1','amazing-surf2-points-v1','amazing-surf-priority-before-v1','amazing-surf-points-v1','heightmap-points-v1','aboxsmooth-points-v1','spherefold-points-v2','state-batch-expanded-v2','fold-rounding-points-v1','rootcause-points-v2','contract-batch-points-v1','contract-map-points-v1','shape-state-points-v1','orbit-shapes-three-points-v1','manowar-default-preconditions-v1','room-triplex-points-v1','fold-cut-points-v1','box-tetra-pine-points-v1','gnarly-family-points-v1','dodeca-wavy-points-v1','axis-wave-box-points-v1','map-icen-points-v1','map-icen-map-points-v1','iq-hex-points-v1','slice-coord-points-v1','angle-impact-after-v1','fold-kali-tube-points-v1','varyscale-pair-points-v1','amazingbox-sse2-points-v1','projection-reciprocal-points-v1','devil-citrus-points-v1','magnet-bicomplex-points-v2','angular-radius-points-v1','heightmap-family-planar-points-v1','heightmap-family-sphere-points-v1','branch-julia-points-v2','ifs-apply-points-v1','fold-order-points-v2','knots-weird-points-v1','fold-poincare-points-v1','last-image-points-v2','remaining-points-points-v1']:
        report=read(phase+'/summary.json')
        if report:
            runs[phase]={'finished':report.get('finished',False),'counts':report.get('counts',{})}
        for r in report.get('results',[]):
            item={**r,'evidence':phase,'cpp_sha256':report['cpp_sha256'],'oracle_sha256':report['oracle_sha256']}
            if report.get('heightmap_callback') or report.get('spheremap_callback'):
                item['reference_scope'] = report.get('map_reference', 'Analytic RGB=(u,v,mapNr) callback; texture interpolation is not covered')
            if r.get('scene_sha256')!=expected_hashes.get(r['case']):item.update(status='error',error='Scene hash differs from current manifest')
            broad[r['case']]=item
    # These profiles vary persistent inputs or exhaust a selector domain. Keep
    # them separate from default-input tests even when scene IDs coincide.
    profiles={}
    for profile, phases_for_profile in {
        'tiling_rucker_state':['tiling-rucker-state-v1'],
        'tiling_rucker_parameters':['tiling-rucker-parameters-v2'],
        'tiling_rucker_rounding':['tiling-rucker-rounding-v2'],
        'general_state':['general-state-v1'],
        'general_parameters':['general-parameters-v1'],
        'ifs_tess_state':['ifs-tess-state-v2'],
        'ifs_tess_parameters':['ifs-tess-parameters-v2'],
        'amazing_surf_first':['amazing-surf-first-v1'],
        'amazing_surf_state':['amazing-surf-state-v1'],
        'amazing_surf_parameters':['amazing-surf-parameters-v1'],
        'heightmap_callback':['heightmap-points-v1'],
        'heightmap_parameters':['heightmap-parameters-v1'],
        'spherefold_parameters':['spherefold-parameters-v1'],
        'aexion_first':['aexion-first-v1'],
        'rootcause_selectors':['rootcause-selectors-points-v1'],
        'shape_state_selectors':['shape-selector-points-v1'],
        'manowar_sse_state':['manowar-sse-points-v1'],
        'orbit_shape_selectors':['orbit-selector-points-v1'],
        'room_triplex_selectors':['room-triplex-selectors-points-v1'],
        'fold_cut_selectors':['fold-cut-selectors-points-v1'],
        'box_tetra_selectors':['box-tetra-selectors-points-v1'],
        'gnarly_even_selectors':['gnarly-selectors-points-v1'],
        'axis_wave_box_selectors':['axis-wave-box-selector-points-v1'],
        'slice_coord_selectors':['slice-coord-selector-points-v1'],
        'fold_kali_tube_selectors':['fold-kali-tube-selector-points-v1'],
        'fold_kali_tube_stored_radius':['fold-kali-tube-rout-v1'],
        'logsp_full_options':['logsp-selector-points-v2'],
        'amazingbox_sse2_selectors':['amazingbox-sse2-selector-points-v1'],
        'magnet_bicomplex_selectors':['magnet-bicomplex-selector-points-v2'],
        'magnet_bicomplex_first_counter':['magnet-bicomplex-first-points-v2'],
        'branch_julia_selectors':['branch-julia-selectors-points-v2'],
        'ifs_apply_selectors':['ifs-apply-selectors-points-v2'],
        'fold_poincare_parameters':['fold-poincare-parameters-points-v1'],'last_image_parameters':['last-image-parameters-points-v1'],'remaining_points_parameters':['remaining-points-parameters-points-v1'],
        'knots_weird_selectors':['knots-weird-selectors-points-v1'],
        'fold_order_selectors':['fold-order-selectors-points-v2'],
        'ifs_apply_legacy_options':['ifs-apply-selectors-points-v1'],
        'heightmap_family_selectors':['heightmap-family-planar-selectors-points-v1','heightmap-family-sphere-selectors-points-v1'],
        'angular_radius_selectors':['angular-radius-selector-points-v1'],
        'angular_radius_stored_radius':['angular-radius-stored-radius-points-v1'],
        'devil_citrus_selectors':['devil-citrus-selector-points-v1'],
        'projection_reciprocal_selectors':['projection-reciprocal-selector-points-v1'],
        'varyscale_pair_first_state':['varyscale-pair-first-v1'],
        'varyscale_pair_selectors':['varyscale-pair-selector-points-v1'],
        'iq_hex_selectors':['iq-hex-selector-points-v1'],
        'fuzzy_radius':['iq-hex-fuzzy-radius-v1'],
        'fuzzy_iteration':['iq-hex-fuzzy-iteration-v1'],
        'vary2_first':['iq-hex-vary2-first-v1'],
        'sin4_range':['iq-hex-sin4-range-v1'],
        'map_icen_selectors':['map-icen-selector-points-v1','map-icen-map-selector-points-v1'],
        'dodeca_wavy_selectors':['dodeca-wavy-selector-points-v1'],
        'bt1pine_byte_counts':['bt1pine-selectors-points-v1'],
        'cubetube_selectors':['cubetube-points-v1'],
        'maptranslate_callback':['maptranslate-points-v1'],
        'kali_first_and_powers':['kali-points-v1'],
        'amazingbox_transform_selectors':['amazingboxtransform-points-v1'],
        'aboxsmooth_state':['aboxsmooth-state-v1'],
        'aboxsmooth_parameters':['aboxsmooth-parameters-v1'],
        'cantor_state':['cantor-state-v1'],
        'amazing_surf2_state':['amazing-surf2-state-v1'],
        'priority_parameters':['priority-parameters-v1'],
        'kleinbot_state':['kleinbot-state-v1'],
        'kleinbot_parameters':['kleinbot-parameters-v1'],
        'knots_state':['knots-state-v1'],
        'knots_parameters':['knots-parameters-v1'],
        'platinum_b_state':['platinum-b-state-v1'],
        'platinum_b_parameters':['platinum-b-parameters-v1'],
        'sphereoffset4d_state':['sphereoffset4d-state-v1'],
        'sphereoffset4d_parameters':['sphereoffset4d-parameters-v1'],
        'bulbox_state':['bulbox-state-v1'],
        'bulbox_parameters':['bulbox-parameters-v1'],
        'celtic_state':['celtic-state-v1'],
        'celtic_parameters':['celtic-parameters-v1'],
        'mandalay_state':['mandalay-state-v1'],
        'mandalay_parameters':['mandalay-parameters-v1'],
        'mandalex_state':['mandalex-state-v1'],
        'mandalex_parameters':['mandalex-parameters-v1'],
        'menger_original_state':['menger-original-state-v1'],
        'menger_original_parameters':['menger-original-parameters-v1'],
        'boxifs_orbit_scale':['boxifs-twist-points-v1'],
        'boxifs_nonfinite':['boxifs-nonfinite-v2'],
        'mag_sign_state':['mag-sign-state-v1'],
        'mag_sign_parameters':['mag-sign-parameters-v1'],
        'first_iteration':['first-state-points','first-state-points-v2','first-state-points-v3','first-state-points-v4'],
        'stored_radius':['rout-points'],
        'iteration_count':['iteration-inputs-v1','iteration-inputs-v2'],
        'iteration_trap_variants':['iteration-trap-points-v2'],
        'derivatives':['derivative-inputs-v1'],
        'cwarp_selectors':['gencwarp-selector-points-v3'],
        'intpow_c_selectors':['intpow-c-selector-points-v3'],
        'grid_wave_state':['grid-wave-state-v3'],
        'orbit_color_state':['orbit-color-state-v3'],
        'randcubes_selectors':['randcubes-points-v1'],
        'menger4ex_state':['menger4ex-state-v2'],
        'four_d_state':['four-d-state-v3'],
        'four_d_rotation_pairs':['four-d-ex-state-v3'],
        'fern_state':['fern-state-v1'],
        'pinehedron_state':['pinehedron-state-v1'],
        'pinehedron_bytes':['pinehedron-bytes-v1'],
        'smooth_fold_first':['smooth-fold-first-v1'],
        'smooth_fold_state':['smooth-fold-state-v1'],
        'smooth_fold_exponents':['smooth-fold-exponents-v1'],
        'vsshapes_modes':['vsshapes-modes-v1'],
        'vsshapes_first':['vsshapes-first-v1'],
        'vsshapes_state':['vsshapes-state-v1'],
        'four_d_base_rotation_pairs':['four-d-base-rotation-state-v1'],
        'complex_power_selectors':['complex-power-selector-points-v1'],
        'intpow_c_shared':['intpow-c-shared-v1'],
        'heightfield_state':['heightfield-state-v1'],
        'fold_stairs_state':['fold-stairs-state-v2'],
        'fold_stairs_selectors':['fold-stairs-selector-points-v2'],
        'julia_state_selectors':['julia-state-selector-points-v1'],
        'julia_state_persistent':['julia-state-persistent-v1'],
        'partly_julia_persistent':['partly-julia-persistent-v1'],
        'partly_julia_selectors':['partly-julia-selector-points-v1'],
        'transform_state_selectors':['transform-state-selector-points-v2'],
        'transform_state_persistent':['transform-state-persistent-v2'],
        'sine_approx_boundaries':['sine-approx-boundary-v2'],
        'sine_approx_selectors':['sine-approx-selector-points-v2'],
        'state_writeback_selectors':['state-writeback-selectors-points-v2'],
        'state_writeback_persistent':['state-writeback-persistent-v2'],
        'pinegon_state':['pinegon-state-v1'],
        'pinegon_bytes':['pinegon-bytes-v1'],
        'cayley_state':['cayley-state-v1'],
        'cayley_parameters':['cayley-parameters-v1'],
        'ifs_state':['ifs-state-v2'],
        'ifs_parameters':['ifs-parameters-v2','ifs-parameters-v4'],
        'gear_state':['gear-state-v1'],
        'gear_parameters':['gear-parameters-v2'],
        'logspiral_state':['logspiral-state-v1'],
        'logspiral_parameters':['logspiral-parameters-v1'],
        'iteration_rotation_counter':['iteration-rotation-counter-v2'],
        'iteration_rotation_state':['iteration-rotation-state-v2'],
        'iteration_rotation_parameters':['iteration-rotation-parameters-v2'],
        'random_transform_counter':['random-transform-counter-v1'],
        'random_transform_words':['random-transform-words-v1'],
        'random_transform_state':['random-transform-state-v1'],
        'random_transform_parameters':['random-transform-parameters-v1'],
        'discoball_state':['discoball-state-v1'],
        'discoball_parameters':['discoball-parameters-v1'],
        'sphcage_state':['sphcage-state-v1'],
        'sphcage_parameters':['sphcage-parameters-v1'],
        'riemann2_state':['riemann2-state-v1'],
        'riemann2_parameters':['riemann2-parameters-v1'],
        'hycinv_state':['hycinv-state-v1'],
        'hycinv_parameters':['hycinv-parameters-v1'],
        'gear2_state':['gear2-state-v1'],
        'gear2_parameters':['gear2-parameters-v1'],
        'hexgrid_state':['hexgrid-state-v2','hexgrid-family-state-v1'],
        'hexgrid_parameters':['hexgrid-parameters-v2','hexgrid-family-parameters-v1'],
    }.items():
        for phase in phases_for_profile:
            report=read(phase+'/summary.json')
            if report:
                profiles[profile]={**report,'evidence':phase}
                runs[phase]={'finished':report.get('finished',False),'counts':dict(Counter(r['status'] for r in report['results']))}
    conditional=read('conditional-points/summary.json')
    rows=[]
    for f in m['formulas']:
        name=f['name'];cases=[c for c in ch.get('cases',[]) if name in c['target_formulas']];ids={c['id'] for c in cases}
        downloaded_ids={c['id'] for c in downloaded.get('cases',[]) if name in c['target_formulas']}
        ir=[influence[c] for c in ids if c in influence];statuses=Counter(x['status'] for r in ir for x in r['formulas'] if x['name']==name)
        row={'formula':name,'asset':f['asset'],'downloaded_scenes':study.get('formula_usage',{}).get(name,0),'challenge_cases':len(cases),'cpp_cases_measured':len(ir),'cpp_errors':sum('error' in r for r in ir),'empty_geometry':sum(r.get('geometry_pixels')==0 for r in ir),'depth_influential':statuses['influential'],'depth_unchanged':statuses['no-measured-depth-influence'],'control_errors':statuses['control-error']}
        details={}
        for phase,rr in images.items():
            selected=[r for r in rr if r['scene'] in (set(f['cases']) if phase=='baseline' else downloaded_ids if phase=='downloaded-parity' else ids)]
            details[phase]=selected;count=Counter('error' if r['status'].endswith('error') else r['status'] for r in selected)
            for status in ['pass','fail','error']:row[phase+'_'+status]=count[status]
        pt=pointmap.get(name);row['point_cases']=pt['cases'] if pt else 0;row['point_failed_lanes']=pt['failed_lanes'] if pt else '';row['point_reference']=pt['reference'] if pt else 'unverified'
        current=latest_image_cases(details, {c['id']:c['sha256'] for c in cases})
        counts=Counter('error' if r['status'].endswith('error') else r['status'] for r in current.values())
        for status in ['pass','fail','error']:row['latest_image_'+status]=counts[status]
        row['latest_exact_rgb']=sum(r.get('exact_rgb',False) for r in current.values())
        row['image_challenges_pending']=len(ids-current.keys())
        br=[broad[c] for c in ids if c in broad];bc=Counter(r['status'] for r in br)
        for status in ['pass','fail','error','unverified']:row['state_point_'+status]=bc[status]
        cr=[r for r in conditional.get('results',[]) if r['name']==name]
        row['conditional_point_rows']=sum(r.get('cases',0) for r in cr)
        row['conditional_failed_cases']=sum(r['status']!='pass' for r in cr)
        profile_details={}
        for profile, report in profiles.items():
            selected=[r for r in report['results'] if r['name']==name]
            profile_details[profile]=selected
            for status in ['pass','fail','error','unverified']:
                row[profile+'_'+status]=sum(r['status']==status for r in selected)
            row[profile+'_rows']=sum(r.get('cases',r.get('rows',0)) for r in selected)
        row['complete']=False
        rows.append({**row,'cases':cases,'images':details,'point':pt,'state_points':br,'conditional_points':cr,'profiles':profile_details,'unverified':['discrete options without documented finite ranges and compound option types','arbitrary hybrid combinations and repeat ranges','independent Pascal parameter expansion, persistent state and initialization paths','maps and missing-map behavior','full Monte Carlo Pascal parity']})
    csvkeys=[k for k in rows[0] if k not in ['cases','images','point','state_points','conditional_points','profiles','unverified']]
    with (a.output/'formulas.csv').open('w',newline='',encoding='utf-8-sig') as out:
        w=csv.DictWriter(out,fieldnames=csvkeys,extrasaction='ignore');w.writeheader();w.writerows(rows)
    totals={'assets':len(rows),'challenge_cases':sum(r['challenge_cases'] for r in rows),'unique_challenge_cases':len(ch.get('cases',[])),'measured_cpp_cases':len(influence),'point_cases':sum(r['point_cases'] for r in rows),'point_formulas':sum(r['point_cases']>0 for r in rows),'point_failed_formulas':sum(bool(r['point_failed_lanes']) for r in rows),'cpp_errors':sum(r['cpp_errors'] for r in rows),'formulas_with_depth_influence':sum(r['depth_influential']>0 for r in rows),'downloaded_loaded':len(study.get('scenes',[])),'downloaded_errors':len(study.get('errors',[])),'downloaded_named_formulas':sum(bool(n) for n in study.get('formula_usage',{})),'complete':False}
    current_images=latest_image_cases(images, {c['id']:c['sha256'] for c in ch['cases']})
    totals['latest_image_challenges']=dict(Counter('error' if r['status'].endswith('error') else r['status'] for r in current_images.values()))
    totals['latest_image_challenges_missing']=len(ch['cases'])-len(current_images)
    totals['state_point_cases']=dict(Counter(r['status'] for r in broad.values()))
    totals['state_point_formula_groups']={
        'all_listed_cases_pass':sum(r['challenge_cases']>0 and r['state_point_pass']==r['challenge_cases'] for r in rows),
        'has_failed_cases':sum(r['state_point_fail']>0 for r in rows),
        'has_error_cases':sum(r['state_point_error']>0 for r in rows),
        'has_unverified_cases':sum(r['state_point_unverified']>0 for r in rows),
        'has_missing_cases':sum(sum(r['state_point_'+s] for s in ['pass','fail','error','unverified'])<r['challenge_cases'] for r in rows)}
    totals['conditional_point_rows']=sum(r['conditional_point_rows'] for r in rows)
    mc={}
    for phase in ['mc-volume-fixed','mc-vector-fixed','mc-final','mc-first-state-v3','mc-menger-v2-verified','mc-sampling-render-v2','mc-grid-wave-full-v1','mc-lens-render-v2','mc-subpixel-render-v2','mc-fold-stairs-v2','mc-camera-render-v1','mc-streams-render-v1','mc-thresholds-render-v1','mc-julia-render-v1','mc-refinement-v2','mc-normals-v1','mc-cayley-v1','mc-ifs-v5','mc-iteration-rotation-v2','mc-random-transform-v1','mc-hexgrid-family-v1','mc-riemann2-v1','mc-hycinv-v1','mc-mag-sign-v1','mc-tiling-v1','mc-limiter-v2','mc-camera-row-v1','mc-mandalay-v1','mc-celtic-v1','mc-sphereoffset4d-v1','mc-knots-v1','mc-kleinbot-v1','mc-general-v1','mc-priority-v1','mc-heightmap-v1','mc-frequency-v2','mc-rootcause-v1','mc-rootcause-v2','mc-contract-batch-v1','mc-shape-state-v1','mc-orbit-shapes-v3','radius-order-mc-v1','box-tetra-pine-mc-v1','gnarly-family-mc-v1','dodeca-wavy-mc-v1','axis-wave-box-mc-v1','map-icen-mc-v1','iq-hex-mc-v1','slice-coord-mc-v1','fold-kali-tube-mc-v1','fold-kali-tube-mc-v2','varyscale-pair-mc-v1']:
        report=read(phase+'/summary.json')
        if report:
            runs[phase]={'finished':report.get('finished',False),'counts':report.get('counts',{})}
            if report.get('finished',False):mc={**report,'evidence':phase}
    sampling=read('mc-sampling-v2/summary.json')
    lens=read('mc-lens-v2/summary.json')
    subpixel=read('mc-subpixel-v2/summary.json')
    codec=read('mc-codec-v4/summary.json')
    adaptive=read('mc-adaptive-v5/summary.json')
    halton=read('mc-halton-v1/summary.json')
    depth_codec=read('depth-codec-v1/summary.json')
    parameter_layouts=read('parameter-layouts-v1/summary.json')
    reference_math_audit={
        'original_rotation':read('rotation-original-source-v1/summary.json'),
        'loader_output_effect':read('rotation-loader-impact-v1/summary.json'),
        'scope':'The portable_math-patched Pascal application and immutable original ASM are different reference identities. Image acceptance against the former does not certify the latter.'}
    camera_row=read('camera-row-angle-v1/summary.json')
    ray_limiter=read('ray-step-limiter-v3/summary.json')
    ray_refinement=read('ray-refinement-v1/summary.json')
    native_numeric_kernels={n:read(n+'/summary.json') for n in ('smooth-iterations-v1','integer-power8-v1','radius-de-v2')}
    recent_layouts={n:read(n+'/summary.json') for n in ('orbit-shapes-layouts-v1','manowar-layouts-v2','room-fold-layouts-v2','box-tetra-pine-layouts-v1','gnarly-family-layouts-v2','dodeca-wavy-layouts-v2','axis-wave-box-layouts-v1','map-icen-layouts-v2','iq-hex-layouts-v1','slice-coord-layouts-v1','angle-impact-layouts-v1','angle-hex-layout-v1','fold-kali-tube-layouts-v1','logsp-layouts-v2','varyscale-pair-layouts-v1','amazingbox-sse2-layouts-v1','projection-reciprocal-layouts-v1','devil-citrus-layouts-v1','magnet-bicomplex-layouts-v2','angular-radius-layouts-v1','heightmap-family-layouts-v1','branch-julia-layouts-v1','ifs-apply-layouts-v2','fold-order-layouts-v2','knots-weird-layouts-v1','fold-poincare-layouts-v1','last-image-layouts-v1','remaining-points-layouts-v1')}
    camera_angles=read('mc-camera-angles-v2/summary.json')
    halton_shifts=read('mc-halton-shifts-v1/summary.json')
    mc_thresholds=read('mc-thresholds-v2/summary.json')
    selector_influence=read('randcubes-influence-v2/summary.json')
    vsshapes_views=read('vsshapes-view-qualification.json')
    vsshapes_influence=read('vsshapes-hybrid-influence-v1/summary.json')
    vsshapes_parity=read('vsshapes-hybrid-parity-v1/summary.json')
    combination_expected={}
    for manifest_name in ['generated-combination-manifest-v1.json','generated-combination-manifest-v2.json']:
        for case in read(manifest_name).get('cases',[]):
            if case['id'] in combination_expected and combination_expected[case['id']] != case['sha256']:
                raise ValueError('Conflicting combination scene: '+case['id'])
            combination_expected[case['id']]=case['sha256']
    combination_latest={}
    combination_attempts={}
    for phase in ['generated-combination-images-v1','generated-combination-retry-v1','generated-combination-images-v2']:
        report=read(phase+'/summary.json')
        if not report:continue
        combination_attempts[phase]={'finished':report.get('finished',False),
                                     'counts':report.get('counts',{}), 'timeouts':report.get('timeouts')}
        for result in report.get('results',[]):
            item={**result,'evidence':phase, 'cpp_sha256':report.get('cpp_sha256'),
                  'pascal_sha256':report.get('pascal_sha256')}
            if item.get('scene_sha256')!=combination_expected.get(item['scene']):
                item.update(status='error',error='Scene hash differs from combination manifest')
            combination_latest[item['scene']]=item
    combination_images={'scope':'Finite extra parameter combinations, separate from the 5520 default challenges. Historical timeout errors remain in attempts and retained run reports.',
                        'expected_cases':len(combination_expected),
                        'counts':dict(Counter(r['status'] for r in combination_latest.values())),
                        'missing_cases':sorted(set(combination_expected)-set(combination_latest)),
                        'attempts':combination_attempts,'results':list(combination_latest.values())}
    hybrid_qualification=read('formula-hybrid-qualification-v1.json')
    hybrid_parity=read('formula-hybrid-parity-v1/summary.json')
    # Keep candidate attempts separate: v2 Knots controls were invalid; v3
    # changes those scenes. Never merge different scene hashes under one ID.
    hybrid_profiles={}
    for version in (1,2,3,4,5):
        qualification=read('formula-hybrid-qualified-v5.json' if version == 5 else
                           f'formula-hybrid-qualification-v{version}.json')
        parity=read(f'formula-hybrid-parity-v{version}/summary.json')
        if qualification:
            hybrid_profiles[f'v{version}']={'qualification':qualification, 'parity':parity}
    profile_summary={p:{k:v for k,v in r.items() if k!='results'} for p,r in profiles.items()}
    # Group by source metadata, including algorithms with no passing evidence.
    algorithms={}
    for f,row in zip(m['formulas'],rows):
        meta=f['metadata']
        keys=['de_option:'+str(meta['de_option'])]+['option_type:'+str(t) for t in set(meta['option_types'])]
        for key in keys:
            group=algorithms.setdefault(key,{'formulas':0,'state_point_pass':0,'state_point_fail':0,'state_point_error':0,'state_point_unverified':0})
            group['formulas']+=1
            for status in ['pass','fail','error','unverified']:group['state_point_'+status]+=row['state_point_'+status]
    write_json(a.output/'coverage.json',{'scope':__doc__,'provenance_records':provenance_records,'totals':totals,'runs':runs,'profiles':profile_summary,'algorithms':algorithms,'evidence_sha256':evidence,'monte_carlo':{'counts':mc.get('counts',{}),'evidence':mc.get('evidence'),'binaries':mc.get('binaries'),'finished':mc.get('finished',False),'complete':False},'depth_codec':depth_codec,'independent_parameter_layouts':parameter_layouts,'mc_sampling':sampling,'mc_lens':lens,'mc_subpixel':subpixel,'mc_codec':codec,'mc_adaptive_kernel':adaptive,'mc_halton':halton,'camera_row_angle':camera_row,'ray_step_limiter':ray_limiter,'ray_refinement':ray_refinement,'recent_parameter_layouts':recent_layouts,'native_numeric_kernels':native_numeric_kernels,'mc_camera_angles':camera_angles,'mc_halton_shifts':halton_shifts,'mc_thresholds':mc_thresholds,'randcubes_influence':selector_influence,'vsshapes_views':vsshapes_views,'vsshapes_hybrid_influence':vsshapes_influence,'vsshapes_hybrid_parity':vsshapes_parity,'generated_combination_images':combination_images,'formula_hybrid_qualification':hybrid_qualification,'formula_hybrid_parity':hybrid_parity,'formulas':rows})
    write_json(a.output/'reference-math-audit.json', reference_math_audit)
    lines=['# External formula validation','', 'Status: incomplete. A PASS below is a finite measurement, not universal equivalence.','',str(totals),'','The CSV contains one row per shipped .m3f asset. coverage.json includes scene hashes, metrics, point evidence and explicit remaining obligations.','', 'Latest merges named image cases only; it does not replace failing early execution with a passing once-only case. Numerical thresholds are unchanged. Baseline images used the old templates; active-parity used early execution. Corrected-parity supersedes only its listed cases. CPP influence is a depth comparison, not a Pascal comparison. A sole-formula replacement control is weaker than omission.','', 'Point tests execute original x86 code for selected formulas; JIT source equations use an independent Python oracle. They compare one step and Julia/W outputs, not complete orbits.','', '| Formula | Baseline pass/fail/error | Active pass/fail/error | Latest pass/fail/error | Depth influence / tested | Point cases / failed lanes |','|---|---|---|---|---|---|']
    lines[2]='Status: INCOMPLETE. Latest means the latest listed measurement of the same scene; historical binaries differ. It does not certify the working tree. A PASS is finite evidence, not universal equivalence.'
    lines.insert(7,'Monte Carlo: '+str(mc.get('counts',{}))+'. Fixed samples only. State-point tests compare 20 lanes with original x86 code, sharing C++ parameter expansion. Conditional tests add six hybrid counters (26 lanes). Maps and pointer-bearing folding are unverified. Detailed state-point and pending-image counts are in CSV/JSON.')
    lines.insert(8,'Reference identity audit: [reference-math-audit.json](reference-math-audit.json). Original FSINCOS/BuildRotMatrix matches C++ in six selected matrices; the portable_math-patched Pascal application differs in two. Independent buffer substitution shows a formula-output effect. Do not change C++ to match patched trigonometry without identifying the intended reference.')
    for r in rows:
        groups=['/'.join(str(r[p+'_'+s]) for s in ['pass','fail','error']) for p in ['baseline','active-parity','latest_image']]
        lines.append('| '+r['formula']+' | '+' | '.join(groups)+f" | {r['depth_influential']} / {r['cpp_cases_measured']} | {r['point_cases']} / {r['point_failed_lanes']} |")
    lines += ['', '## Original-code state checks by formula', '', 'One-step default-input cases, including parameter variants. Counts use the latest listed result per identical scene; binary hashes are retained. They do not certify complete orbits or parameter expansion.', '', '| Formula | PASS | FAIL | ERROR | Unverified | Image challenges pending |', '|---|---:|---:|---:|---:|---:|']
    for r in rows:
        lines.append('| '+r['formula']+' | '+' | '.join(str(r['state_point_'+k]) for k in ['pass','fail','error','unverified'])+' | '+str(r['image_challenges_pending'])+' |')
    lines += ['', '## Monte Carlo sampling helpers', '', f"Independent extracted Pascal functions: {sampling.get('rows',0)} input rows, bit-exact={sampling.get('exact',False)}, failed output lanes={sampling.get('failures',[])}. This covers disc conversion and eight bokeh modes, not full transport, adaptive scheduling or GUI behavior."]
    lines += ['', f"Lens initialization and scaling: {lens.get('rows',0)} input rows, bit-exact={lens.get('exact',False)}, failed output lanes={lens.get('failures',[])}. Source statements are extracted independently; camera basis and complete rays are outside this helper check.", '', 'RandCubes selector scene usefulness (C++ only): '+str(selector_influence.get('summary', 'in progress'))+'. This does not establish Pascal image parity.']
    lines += ['', f"Uniform/Gaussian subpixel sampling and random consumption: {subpixel.get('rows',0)} input streams, bit-exact={subpixel.get('exact',False)}, failed lanes={subpixel.get('failures',[])}. This does not verify camera rays or full transport."]
    lines += ['', f"Packed MC codec: {codec.get('rows',0)} input rows, passed={codec.get('passed',False)}, failed lanes={codec.get('failures',[])}. Exhaustive decode: {codec.get('exhaustive_decode',{})}. Encoding requires exact codes; decode/noise allow fixed 2e-12 absolute/relative error and retain bit mismatches."]
    lines += ['', f"Adaptive CalcN decision kernel: {adaptive.get('rows',0)} input rows, exact={adaptive.get('exact',False)}, failed lanes={adaptive.get('failures',[])}. Original displaced neighbor-counter reads use explicit supplied tail bytes; missing bytes are rejected. This does not certify the full adaptive scheduler or progressive file resume."]
    lines += ['', f"Halton table: {halton.get('table_rows',0)} records (all four Word fields), plus {halton.get('inverse_rows',0)} radical-inverse inputs; bit-exact={halton.get('exact',False)}, failed lanes={halton.get('failures',[])}. Routine evidence and remaining gaps: [mc-function-coverage.md](mc-function-coverage.md)."]
    lines += ['', 'Camera angle expressions: '+str(camera_angles.get('rows',0))+' rows, exact='+str(camera_angles.get('exact',False))+'. This excludes view-vector construction and DOF. Halton shifts, Word conversion and RNG: '+str(halton_shifts.get('rows',0))+' rows, exact='+str(halton_shifts.get('exact',False))+'. All Word values at five shifts plus sampled pixel/seed streams; not every seed.']
    lines += ['', 'MC typed tolerance constants and refinement predicate: '+str(mc_thresholds.get('rows',0))+' inputs, exact='+str(mc_thresholds.get('exact',False))+'. This does not certify complete ray transport.']
    lines += ['', 'Regular camera row angle: '+str(camera_row.get('rows',0))+' independent Pascal inputs; passed='+str(camera_row.get('passed',False))+'. Typed ray-step limiter: '+str(ray_limiter.get('rows',0))+' inputs; passed='+str(ray_limiter.get('passed',False))+'. The FPC SSE2 evaluation order is covered; other Pascal compilers and all camera modes are not certified.']
    lines += ['', '## Persistent-state and selector profiles', '', 'These are separate measurements, not replacements for default-input results. Each profile records its frozen binary and oracle hashes in coverage.json.', '', '| Profile | Finished | Cases by status | Point rows |', '|---|---|---|---|']
    for profile,r in profiles.items():
        lines.append(f"| {profile} | {r.get('finished',False)} | {dict(Counter(x['status'] for x in r['results']))} | {sum(x.get('cases',x.get('rows',0)) for x in r['results'])} |")
    lines += ['', '## Source metadata groups', '', 'Counts overlap for option types. Registration and a passing point are not branch coverage.', '', '| Group | Formulas | State cases pass/fail/error/unverified |', '|---|---|---|']
    for key,g in sorted(algorithms.items()):
        counts='/'.join(str(g['state_point_'+s]) for s in ['pass','fail','error','unverified'])
        lines.append(f"| {key} | {g['formulas']} | {counts} |")
    lines += ['', 'ABoxVSShapes hybrid view qualification (C++ only): '+str(vsshapes_views.get('summary',{}))+'. Useful shape modes: '+str(vsshapes_views.get('useful_modes',[]))+'. Requires surface and background, varying depth/color and a measured omission-control effect. Qualified-view Pascal image results: '+str(dict(Counter(r['status'] for r in vsshapes_parity.get('results',[]))))+'. These remain separate from the default challenge cases and do not certify arbitrary hybrids.']
    lines += ['', 'Hybrid geometry qualification: '+str(hybrid_qualification.get('summary',{}))+'. Independently compared qualified images: '+str(hybrid_parity.get('counts',{}))+'. Omission controls establish measured geometry influence, not complete formula semantics.']
    write_json(a.output/'hybrid-coverage.json', {'scope':'Finite hybrid attempts remain separate by version and scene hash; unsuccessful qualification is not parity evidence.', 'profiles':hybrid_profiles})
    for version,profile in hybrid_profiles.items():
        lines += ['', 'Hybrid '+version+': qualification '+str(profile['qualification'].get('summary',{}))+'; Pascal images '+str(profile['parity'].get('counts',{}))+'.']
    lines += ['', 'Additional generated combination images: '+str(combination_images['counts'])+' of '+str(combination_images['expected_cases'])+'. Original timeouts and retries remain separate: '+str(combination_attempts)+'. These are finite additional cases, not exhaustive combinations.']
    (a.output/'coverage.md').write_text('\n'.join(lines)+'\n',encoding='utf-8');print(totals)
if __name__=='__main__':main()
