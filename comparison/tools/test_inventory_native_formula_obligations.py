"""Host-only extraction regressions. Synthetic fixtures grant no formula credit."""
import copy
import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest import mock

import inventory_native_formula_obligations as inv


class OriginalDefaultsTests(unittest.TestCase):
    def test_all467_originals_extract_unique_typed_defaults(self):
        rows = inv.read(inv.ROOT/'comparison/contracts/native-gpu-acceptance-v4/inventory.json')['formulas']
        self.assertEqual(len(rows), 467)
        self.assertEqual(len({(r['namespace'], r['id']) for r in rows}), 467)
        for row in rows:
            with self.subTest(identity=row):
                d = inv.external_defaults(row) if row['namespace'] == 'external' else inv.internal_defaults(row)
                self.assertEqual(d['status'], 'INVENTORIED')
                json.loads(json.dumps(d, allow_nan=False))
                self.assertEqual(len(d['authored_option_types']), len(d['authored_values_binary64_hex']))
                self.assertLessEqual(len(d['authored_option_types']), 16)
                for r in d['preparation_sources']:
                    self.assertEqual(inv.ref(inv.ROOT/r['path']), r)

    def test_translate_absent_rstop_uses_original16_not_generator100(self):
        d = inv.external_defaults({'namespace': 'external', 'id': 8065, 'name': '_Translate'})
        self.assertEqual(d['parsed_metadata']['rstop'], 16)
        self.assertEqual(d['parsed_metadata']['de_option'], -1)
        self.assertEqual(d['authored_option_types'], [0, 0, 0])
        self.assertEqual(d['authored_values_binary64_hex'], ['0x0000000000000000']*3)

    def test_signed_zero_words_are_not_normalized(self):
        self.assertEqual(inv.words64([-0., 0.]), ['0x8000000000000000', '0x0000000000000000'])

    def test_compound_angles_expanded_and_raw_options_retained(self):
        d = inv.external_defaults({'namespace': 'external', 'id': 429, 'name': 'genIFS'})
        self.assertEqual(d['authored_option_types'], [0, 2, 2, 2, 1, 1, 1, 6, 6, 6])
        self.assertIn('.3SingleAngles', d['sections']['OPTIONS']['text'])
        raw = (inv.ROOT/d['source']['path']).read_bytes()
        for section in d['sections'].values():
            self.assertEqual(inv.digest(raw[section['byte_start']:section['byte_end']]), section['sha256'])

    def test_active_internal_test_defaults_exclude_commented_alternatives(self):
        d = inv.internal_defaults({'namespace': 'internal', 'id': 7, 'name': 'test'})
        self.assertEqual(d['authored_option_types'], [2, 2, 1, 1, 1, 1])
        self.assertEqual(d['authored_values_binary64_hex'], inv.words64([1., 2., 0., 1., 1., 0.]))
        e = inv.internal_defaults({'namespace': 'internal', 'id': 8, 'name': 'testIFS'})
        self.assertEqual(e['authored_option_types'], [0, 0, 0, 0, 0, 2, 2, 1, 1, 2])
        self.assertEqual(e['authored_values_binary64_hex'], inv.words64([0., 1., 0., 0., 1., 0., 0., 0., 1., 0.]))

    def test_pascal_comment_removal_preserves_literals_and_lines(self):
        text = "x='{not comment}'; {gone}\ny=2; //gone\nz='a''//b'; (*gone\n*)k=3;"
        result = inv.pascal_code(text)
        self.assertIn("'{not comment}'", result)
        self.assertIn("'a''//b'", result)
        self.assertNotIn('gone', result)
        self.assertEqual(text.count('\n'), result.count('\n'))

    def test_internal_nontrivial_types_and_defaults(self):
        d = inv.internal_defaults({'namespace': 'internal', 'id': 5, 'name': 'Bulbox'})
        self.assertEqual(d['authored_option_types'], [0, 7, 11, 0, 9, 9])
        self.assertEqual(d['authored_values_binary64_hex'], inv.words64([2., .5, 1., 1., 2., 2.]))
        d = inv.internal_defaults({'namespace': 'internal', 'id': 9, 'name': 'Aexion C'})
        self.assertEqual(d['authored_option_types'], [0, 0, 2, 2, 0, 0, 2, 2])


class ContextTests(unittest.TestCase):
    def fixture(self):
        scene = next(s for s in inv.read(inv.ROOT/'comparison/contracts/native-gpu-acceptance-v4/inventory.json')['scenes']
                     if s['id'] == 'ecc0b336d49fa5e9')
        slots = []
        for s in scene['slots']:
            n, number = s['key'].split(':')
            slots.append(dict(s, namespace=n, id=int(number)))
        summary = dict(host_only=True, settings_overrides=[], precision='native', width=600, height=330,
                       original_iterations=400, programs=[{'max_iterations': 400}])
        return scene, summary, slots

    def test_original400_accepted_preview64_rejected(self):
        scene, summary, slots = self.fixture()
        inv.verify_original_settings(scene, summary, slots)
        summary['programs'][0]['max_iterations'] = 64
        with self.assertRaisesRegex(ValueError, 'iteration'):
            inv.verify_original_settings(scene, summary, slots)

    def test_combined_preserves_own_secondary_iteration_limit(self):
        scene, summary, slots = self.fixture()
        summary['original_secondary_iterations'] = 23
        summary['programs'].append({'max_iterations': 23})
        inv.verify_original_settings(scene, summary, slots)
        summary['programs'][1]['max_iterations'] = 400
        with self.assertRaisesRegex(ValueError, 'secondary'):
            inv.verify_original_settings(scene, summary, slots)

    def test_changed_dimensions_overrides_and_slot_bits_reject(self):
        for mutation in ('dimensions', 'overrides', 'parameters', 'counts'):
            scene, summary, slots = self.fixture()
            if mutation == 'dimensions': summary['width'] = 160
            if mutation == 'overrides': summary['settings_overrides'] = ['smoothing']
            if mutation == 'parameters':
                slots[0] = copy.deepcopy(slots[0]); slots[0]['option_values_binary64_hex'][0] = '0x0000000000000000'
            if mutation == 'counts': slots[0]['iterations'] += 1
            with self.subTest(mutation=mutation), self.assertRaises(ValueError):
                inv.verify_original_settings(scene, summary, slots)

    def test_addon_preserves_disabled_slots_signedzero_and_negative_counts(self):
        raw = bytearray(1136)
        for i in range(6):
            struct.pack_into('<iii', raw, 8+188*i, -2147483647 if i == 1 else 0, 20, 3)
            raw[8+188*i+12:8+188*i+22] = b'_Translate'
            struct.pack_into('<Q', raw, 8+188*i+60, 0x8000000000000000)
        identities = [{'namespace':'external', 'id':8065, 'name':'_Translate'}]
        slots = inv.loaded_slots(raw, identities)
        self.assertEqual(slots[1]['id'], 8065)
        self.assertEqual(slots[1]['formula_number'], 20)
        self.assertEqual(len(slots), 6)
        self.assertEqual(slots[1]['iterations'], -2147483647)
        self.assertEqual(slots[0]['option_values_binary64_hex'][0], '0x8000000000000000')
        with self.assertRaisesRegex(ValueError, 'ABI'):
            inv.loaded_slots(raw[:-1], identities)

    def test_exact_dedup_retains_all_scene_and_slot_links(self):
        row = dict(id='x', parameters_sha256='p', program_sha256='g', resources_sha256='r', settings_sha256='s',
                   formula_keys=['external:8065'], capability='mixed', scene_ids=['a'], scene_links=[{'scene_id':'a','slot':1}])
        other = dict(row, scene_ids=['b'], scene_links=[{'scene_id':'b','slot':2}])
        result = inv.deduplicate([copy.deepcopy(row), copy.deepcopy(other)])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0]['scene_ids'], ['a', 'b'])
        self.assertEqual([s['slot'] for s in result[0]['scene_links']], [1, 2])
        other['program_sha256'] = 'different'
        with self.assertRaisesRegex(ValueError, 'collision'):
            inv.deduplicate([copy.deepcopy(row), other])

    def test_parameter_or_program_or_resource_or_effect_change_changes_context(self):
        values = ['p', 'g', 'r', 's']
        initial = inv.context_identity(*values)
        for i in range(4):
            changed = list(values); changed[i] += 'x'
            self.assertNotEqual(initial, inv.context_identity(*changed))

    def test_all_ten_categories_present_without_na_or_credit(self):
        self.assertEqual(len(set(inv.CATEGORIES)), 10)
        self.assertIn('rounded_finite', inv.CATEGORIES)
        self.assertIn('scene_contexts', inv.CATEGORIES)


class BuildBindingTests(unittest.TestCase):
    """Structurally realistic synthetic build records; no compilation credit."""
    def fixture(self, root):
        source, build, provenance = root/'source', root/'build', root/'provenance'
        records = provenance/'records'
        for directory in (source, build, records): directory.mkdir(parents=True)
        src = source/'unit.cpp'; src.write_bytes(b'synthetic source')
        tool = root/'cxx'; tool.write_bytes(b'synthetic tool, never executable')
        ar = root/'ar'; ar.write_bytes(b'synthetic archive tool, never executable')
        capture = provenance/'capture.py'; capture.write_bytes(b'synthetic capture')
        context = provenance/'context.json'
        context.write_text(json.dumps(dict(root=str(source), build=str(build), records=str(records))))
        obj, archive, dump = build/'unit.o', build/'libunit.a', build/'mb3d_native_scene_context_dump'
        for path in (obj, archive, dump): path.write_bytes(('synthetic '+path.name).encode())
        depfile = build/'unit.o.d'; depfile.write_text(f'{obj}: {src}\n')
        def identity(path, classification=None):
            value = dict(path=str(path), sha256=inv.digest(path.read_bytes()))
            if classification: value['classification'] = classification
            return value
        definitions = [
            ('compile', tool, [str(tool), '-c', str(src), '-o', str(obj), '-MF', str(depfile)], obj,
             [identity(src, 'repository')]),
            ('link', ar, [str(ar), 'qc', str(archive), str(obj)], archive,
             [identity(obj, 'build_generated')]),
            ('link', tool, [str(tool), str(archive), '-o', str(dump)], dump,
             [identity(archive, 'build_generated')])]
        bindings = []
        for index, (kind, executable, argv, output, inputs) in enumerate(definitions):
            before = dict(schema='mb3d.actual-build-command.v1', kind=kind, argv=argv, expanded_argv=argv,
                cwd=str(build), pid=100+index, start_time_ns=10*index+1, context_sha256=inv.digest(context.read_bytes()),
                tool=identity(executable), explicit_inputs_before=inputs, output_existed_before=False)
            record = dict(before, end_time_ns=10*index+2, exit_code=0, capture_errors=[], output=identity(output))
            if index == 0:
                record.update(dependencies=[identity(src, 'repository')], depfile=identity(depfile))
            path = records/(str(index)+'.json'); path.write_text(json.dumps(record))
            (records/(str(index)+'.started.json')).write_text(json.dumps(before))
            bindings.append(dict(path=str(path.relative_to(root)), sha256=inv.digest(path.read_bytes())))
        manifest = root/'source-manifest.json'
        manifest.write_text(json.dumps(dict(source_files={'unit.cpp': inv.digest(src.read_bytes())},
                                            capture_sha256=inv.digest(capture.read_bytes()))))
        receipt = dict(schema='mb3d.host-scene-context.build.v1', status='PASS_CAPTURED_BUILD_CLOSURE',
            host_only=True, airs=0, objects=1, dependencies=1, producer_edges=2, commands=3,
            targets=['mb3d_native_scene_context_dump'], compiled_source_root=str(source),
            source_manifest_sha256=inv.digest(manifest.read_bytes()), capture_sha256=inv.digest(capture.read_bytes()),
            records=bindings, products=[dict(path=str(p.relative_to(root)), sha256=inv.digest(p.read_bytes()))
                                       for p in (obj, archive, dump)])
        path = root/'build-receipt.json'; path.write_text(json.dumps(receipt))
        return dump, path, receipt

    def update_record(self, root, receipt, index, mutate, update_started=False):
        path = root/receipt['records'][index]['path']; record = json.loads(path.read_text())
        mutate(record); path.write_text(json.dumps(record))
        if update_started:
            started = path.with_name(path.stem+'.started.json'); old = json.loads(started.read_text())
            started.write_text(json.dumps({k:record[k] for k in old}))
        receipt['records'][index]['sha256'] = inv.digest(path.read_bytes())

    def test_matching_full_closure_and_changed_binary_rejection(self):
        with tempfile.TemporaryDirectory() as temp:
            dump, path, receipt = self.fixture(Path(temp))
            inv.validate_host_build(dump, path)
            dump.write_bytes(b'changed')
            with self.assertRaisesRegex(ValueError, 'product'):
                inv.validate_host_build(dump, path)

    def test_failed_or_literal_json_and_capture_error_rejected_after_rehash(self):
        for mutation in ('literal', 'wrong_schema', 'failed', 'capture_error'):
            with self.subTest(mutation=mutation), tempfile.TemporaryDirectory() as temp:
                root = Path(temp); dump, path, receipt = self.fixture(root)
                if mutation == 'literal':
                    record_path = root/receipt['records'][0]['path']; record_path.write_text('synthetic command')
                    receipt['records'][0]['sha256'] = inv.digest(record_path.read_bytes())
                else:
                    field, value = {'wrong_schema':('schema','synthetic'), 'failed':('exit_code',1),
                                    'capture_error':('capture_errors',['input changed'])}[mutation]
                    self.update_record(root, receipt, 0, lambda r:r.update({field:value}))
                path.write_text(json.dumps(receipt))
                with self.assertRaises(ValueError): inv.validate_host_build(dump, path)

    def test_missing_producer_and_wrong_input_hash_rejected_after_rehash(self):
        for mutation in ('producer_time', 'producer_hash'):
            with self.subTest(mutation=mutation), tempfile.TemporaryDirectory() as temp:
                root = Path(temp); dump, path, receipt = self.fixture(root)
                if mutation == 'producer_time':
                    self.update_record(root, receipt, 0, lambda r:r.update(end_time_ns=99))
                else:
                    self.update_record(root, receipt, 1,
                        lambda r:r['explicit_inputs_before'][0].update(sha256='0'*64), True)
                path.write_text(json.dumps(receipt))
                with self.assertRaisesRegex(ValueError, 'producer|Producer'): inv.validate_host_build(dump, path)

    def test_missing_dependency_or_depfile_mismatch_rejected_after_rehash(self):
        for mutation in ('empty', 'wrong_hash', 'wrong_set'):
            with self.subTest(mutation=mutation), tempfile.TemporaryDirectory() as temp:
                root = Path(temp); dump, path, receipt = self.fixture(root)
                def change(r):
                    if mutation == 'empty': r['dependencies'] = []
                    elif mutation == 'wrong_hash': r['dependencies'][0]['sha256'] = '0'*64
                    else: r['dependencies'][0]['path'] = str(root/'build/unit.o')
                self.update_record(root, receipt, 0, change); path.write_text(json.dumps(receipt))
                with self.assertRaisesRegex(ValueError, '[Dd]ependen'): inv.validate_host_build(dump, path)

    def test_started_gaps_source_change_and_forged_counters_rejected(self):
        for mutation in ('missing_start', 'extra_start', 'source', 'counter', 'unlisted'):
            with self.subTest(mutation=mutation), tempfile.TemporaryDirectory() as temp:
                root = Path(temp); dump, path, receipt = self.fixture(root)
                if mutation == 'missing_start': (root/'provenance/records/0.started.json').unlink()
                if mutation == 'extra_start': (root/'provenance/records/orphan.started.json').write_text('{}')
                if mutation == 'source': (root/'source/unit.cpp').write_text('changed')
                if mutation == 'counter': receipt['objects'] = 134
                if mutation == 'unlisted': receipt['records'].pop()
                path.write_text(json.dumps(receipt))
                with self.assertRaises(ValueError): inv.validate_host_build(dump, path)

    def test_invalid_pid_and_duplicate_process_rejected(self):
        for mutation in ('invalid', 'duplicate'):
            with self.subTest(mutation=mutation), tempfile.TemporaryDirectory() as temp:
                root = Path(temp); dump, path, receipt = self.fixture(root)
                update = {'pid': 0} if mutation == 'invalid' else {'pid': 100, 'start_time_ns': 1}
                self.update_record(root, receipt, 2, lambda r:r.update(update), True)
                path.write_text(json.dumps(receipt))
                with self.assertRaisesRegex(ValueError, 'pid|process identity'):
                    inv.validate_host_build(dump, path)

    def test_other_target_air_or_changed_command_manifest_rejected(self):
        for mutation in ('target', 'air', 'command', 'manifest'):
            with self.subTest(mutation=mutation), tempfile.TemporaryDirectory() as temp:
                root = Path(temp); dump, path, receipt = self.fixture(root)
                if mutation == 'target': receipt['targets'] = ['mb3d_render']
                if mutation == 'air': receipt['airs'] = 7
                if mutation == 'command': (root/receipt['records'][0]['path']).write_bytes(b'changed')
                if mutation == 'manifest': (root/'source-manifest.json').write_bytes(b'changed')
                path.write_text(json.dumps(receipt))
                with self.assertRaises(ValueError): inv.validate_host_build(dump, path)


if __name__ == '__main__': unittest.main()
