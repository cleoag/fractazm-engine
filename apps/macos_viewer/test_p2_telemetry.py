#!/usr/bin/env python3
"""Synthetic P2 trace mutations; no GPU, latency or scene acceptance claims."""
import copy
import unittest
import json
import tempfile
from pathlib import Path
from unittest.mock import patch
import p2_telemetry as telemetry


def stamp(value, domain='host', reason=None):
    return dict(value=value, domain=domain, unit='seconds', reason=reason)


def fixture(cancel=False):
    identity = dict(zip(telemetry.IDENTITY, (1, 2, 3, 0, 4, 5, 6)))
    events = []
    def event(kind, at, **fields):
        events.append(dict(kind=kind, at=stamp(at), sequence=len(events),
                           identity=identity.copy(), **fields))
    event('begin', 1., semantic_work=10, committed_coverage=4, total_samples=1)
    event('setup', 1.1, begin=stamp(1.), component='pipeline', status='ready')
    event('portion_submit', 1.2, portion=0, quantum=7, stage='orbit',
          owner_sample=dict(owner=0, sample=0, reason=None))
    if cancel:
        event('cancel_request', 1.201)
        event('cancel_observed', 1.202)
    event('portion_complete', 1.21, portion=0, consumed_quantum=3,
          yield_reason='phase_complete', semantic_work=99, completed_samples=1,
          remaining_samples=0, published=False, committed_coverage=4, stage='complete',
          gpu_begin=stamp(10., 'metal'), gpu_end=stamp(10.008, 'metal'))
    event('quiescent', 1.22)
    event('terminal', 1.23, outcome='cancelled' if cancel else 'completed',
          published=not cancel, committed_coverage=4 if cancel else 5)
    event('reply', 1.24, outcome='cancelled' if cancel else 'completed')
    return dict(version=1, identity=identity, provenance=dict(build_receipt_sha256='0'*64,
                library_sha256='1'*64, device='synthetic', scene='synthetic', profile='synthetic',
                contention='synthetic', width=64, height=64, workers=4), pipeline_cache='warm', dropped_events=0,
                events=events, client=dict(input=stamp(20., 'client'), reply=stamp(20.3, 'client'),
                draw=stamp(None, 'client', 'not observed'),
                present=stamp(None, 'client', 'AppKit presentation acknowledgment unavailable')))


def find(trace, kind):
    return next(event for event in trace['events'] if event['kind'] == kind)


def resequence(trace):
    for number, event in enumerate(trace['events']):
        event['sequence'] = number


class TraceTests(unittest.TestCase):
    def reject(self, trace):
        with self.assertRaises(telemetry.InvalidTrace):
            telemetry.validate_trace(trace)

    def test_complete(self):
        report = telemetry.validate_trace(fixture())
        self.assertTrue(report['complete_evidence']); self.assertTrue(report['gpu_timing_complete'])
        self.assertEqual(report['gpu']['count'], 1)
        self.assertAlmostEqual(report['gpu']['max_ms'], 8)
        self.assertAlmostEqual(report['request_to_reply']['seconds'], .3)
        self.assertIsNone(report['input_to_present']['seconds'])

    def test_multiple_portions_preserve_accounting_and_sequence(self):
        trace = fixture(); first = find(trace, 'portion_complete')
        first.update(yield_reason='allowance', completed_samples=0, remaining_samples=1)
        submit = copy.deepcopy(find(trace, 'portion_submit')); submit.update(portion=1, at=stamp(1.215))
        complete = copy.deepcopy(first); complete.update(portion=1, at=stamp(1.225), semantic_work=100,
            yield_reason='phase_complete', completed_samples=1, remaining_samples=0,
            gpu_begin=stamp(10.012, 'metal'), gpu_end=stamp(10.020, 'metal'))
        trace['events'][4:4] = [submit, complete]
        for kind, value in [('quiescent', 1.23), ('terminal', 1.24), ('reply', 1.25)]:
            find(trace, kind)['at'] = stamp(value)
        resequence(trace)
        self.assertEqual(telemetry.validate_trace(trace)['portions'], 2)
        complete['semantic_work'] = 98; self.reject(trace)
        complete['semantic_work'] = 100; submit['portion'] = 0; self.reject(trace)

    def test_setup_is_not_gpu_time(self):
        report = telemetry.validate_trace(fixture())
        self.assertAlmostEqual(report['setup'][0]['duration']['seconds'], .1)
        self.assertAlmostEqual(report['gpu']['max_ms'], 8)

    def test_identity(self):
        for value in (99, True):
            trace = fixture(); find(trace, 'portion_complete')['identity']['request_id'] = value
            self.reject(trace)

    def test_sequence(self):
        for value in (1, 3, 2.0, True):
            trace = fixture(); trace['events'][2]['sequence'] = value; self.reject(trace)

    def test_overlapping_submission(self):
        trace = fixture(); trace['events'].insert(3, copy.deepcopy(trace['events'][2])); resequence(trace)
        self.reject(trace)

    def test_stale_or_boolean_completion(self):
        for value in (1, False):
            trace = fixture(); find(trace, 'portion_complete')['portion'] = value; self.reject(trace)

    def test_missing_submission(self):
        trace = fixture(); del trace['events'][2]; resequence(trace); self.reject(trace)

    def test_quanta(self):
        for value in (0, -1, True, 1.5, (1 << 20) + 1, 1 << 64):
            trace = fixture(); find(trace, 'portion_submit')['quantum'] = value; self.reject(trace)
        trace = fixture(); find(trace, 'portion_complete')['consumed_quantum'] = 8; self.reject(trace)

    def test_allowance_must_make_progress(self):
        trace = fixture(); find(trace, 'portion_complete').update(yield_reason='allowance', consumed_quantum=0)
        self.reject(trace)

    def test_pending_cannot_publish(self):
        trace = fixture(); find(trace, 'portion_complete')['published'] = True; self.reject(trace)
        trace = fixture(); find(trace, 'portion_complete')['committed_coverage'] = 5; self.reject(trace)

    def test_work_and_remaining_samples(self):
        for field, value in [('semantic_work', 9), ('completed_samples', 2), ('remaining_samples', 1)]:
            trace = fixture(); find(trace, 'portion_complete')[field] = value; self.reject(trace)

    def test_no_gpu_read_before_completion(self):
        trace = fixture(); find(trace, 'portion_submit')['gpu_begin'] = stamp(10., 'metal'); self.reject(trace)

    def test_clock_validation(self):
        for value in (True, float('nan'), float('inf'), -1, 10**400, 1.e308):
            trace = fixture(); find(trace, 'portion_complete')['gpu_end']['value'] = value; self.reject(trace)
        trace = fixture(); find(trace, 'portion_complete')['gpu_end']['value'] = 9.; self.reject(trace)

    def test_explicit_null_and_units(self):
        for mutation in ('value', 'reason', 'unit'):
            trace = fixture(); del trace['client']['present'][mutation]; self.reject(trace)

    def test_unknown_gpu_time_is_not_zero(self):
        trace = fixture(); find(trace, 'portion_complete')['gpu_end'] = stamp(None, 'metal', 'device timer unavailable')
        report = telemetry.validate_trace(trace)
        self.assertFalse(report['gpu_timing_complete']); self.assertEqual(report['gpu']['count'], 0)
        self.assertIsNone(report['gpu']['max_ms']); self.assertEqual(len(report['unavailable_gpu']), 1)

    def test_cross_domain_is_unavailable(self):
        trace = fixture(); trace['client']['reply']['domain'] = 'cpp_steady'
        report = telemetry.validate_trace(trace)
        self.assertIsNone(report['request_to_reply']['seconds'])
        self.assertIn('incomparable', report['request_to_reply']['reason'])

    def test_draw_cannot_be_present(self):
        trace = fixture(); trace['client']['present'] = stamp(20.4, 'client'); self.reject(trace)

    def test_active_cancellation(self):
        report = telemetry.validate_trace(fixture(True))
        self.assertEqual(report['terminal'], 'cancelled')
        self.assertTrue(report['active_cancellation'])
        self.assertAlmostEqual(report['stop']['seconds'], .019)

    def test_cancel_prevents_later_submission(self):
        trace = fixture(True); extra = copy.deepcopy(find(trace, 'portion_submit'))
        extra.update(portion=1, at=stamp(1.215)); trace['events'].insert(6, extra); resequence(trace)
        self.reject(trace)

    def test_observed_cancellation_wins_over_success(self):
        trace = fixture(True)
        find(trace, 'terminal').update(outcome='completed', published=True, committed_coverage=5)
        find(trace, 'reply')['outcome'] = 'completed'
        self.reject(trace)

    def test_partial_owner_attribution_still_checks_types(self):
        trace = fixture()
        find(trace, 'portion_submit')['owner_sample'] = dict(owner=True, sample=None, reason='pooled samples')
        self.reject(trace)

    def test_fault_is_not_cancellation(self):
        trace = fixture(True); find(trace, 'portion_complete').update(yield_reason='fault', fault_outcome='failed')
        self.reject(trace)
        find(trace, 'terminal')['outcome'] = 'failed'; find(trace, 'reply')['outcome'] = 'failed'
        self.assertEqual(telemetry.validate_trace(trace)['terminal'], 'failed')

    def test_cancelled_phase_has_no_image(self):
        trace = fixture(True); find(trace, 'terminal')['published'] = True; self.reject(trace)
        trace = fixture(True); find(trace, 'terminal')['committed_coverage'] = 5; self.reject(trace)

    def test_quiescence_requires_completion(self):
        trace = fixture(); del trace['events'][3]; resequence(trace); self.reject(trace)

    def test_exactly_one_terminal(self):
        trace = fixture(); trace['events'].insert(-1, copy.deepcopy(find(trace, 'terminal')))
        resequence(trace); self.reject(trace)
        trace = fixture(); trace['events'].pop(); self.reject(trace)

    def test_reply_matches_terminal(self):
        trace = fixture(); find(trace, 'reply')['outcome'] = 'cancelled'; self.reject(trace)

    def test_truncated_stream_never_complete(self):
        trace = fixture(); trace['events'] = trace['events'][:3]; trace['dropped_events'] = 4
        report = telemetry.validate_trace(trace)
        self.assertFalse(report['complete_evidence']); self.assertFalse(report['gpu_timing_complete'])

    def test_trace_bounds(self):
        trace = fixture()
        with patch.object(telemetry, 'MAX_EVENTS', 6): self.reject(trace)
        with patch.object(telemetry, 'MAX_TRACE_BYTES', 10): self.reject(trace)
        trace['events'][0]['extra'] = 'x' * telemetry.MAX_EVENT_BYTES; self.reject(trace)

    def test_reply_bounds(self):
        self.assertTrue(telemetry.validate_reply_size({'trace': {'events_written': 7}}))
        for reply in ({'portion_events': []}, {'trace': {'events': []}},
                      {'payload': 'é' * telemetry.MAX_REPLY_BYTES}):
            with self.assertRaises(telemetry.InvalidTrace): telemetry.validate_reply_size(reply)

    def test_nearest_rank_and_overrun(self):
        values = [i / 1000 for i in range(1, 22)]
        report = telemetry.distribution(values)
        self.assertEqual(report, dict(count=21, median_ms=11., p95_ms=20., max_ms=21.))
        trace = fixture(); find(trace, 'portion_complete')['gpu_end']['value'] = 10.011
        self.assertEqual(len(telemetry.validate_trace(trace)['portion_overruns']), 1)

    def test_provenance(self):
        for key, value in [('width', 0), ('workers', True), ('library_sha256', 'bad'), ('device', None)]:
            trace = fixture(); trace['provenance'][key] = value; self.reject(trace)

    def test_warm_stop_overrun_and_cold_cohort(self):
        trace = fixture(True)
        for kind, value in [('quiescent', 1.27), ('terminal', 1.28), ('reply', 1.29)]:
            find(trace, kind)['at'] = stamp(value)
        self.assertTrue(telemetry.validate_trace(trace)['warm_stop_overrun'])
        trace['pipeline_cache'] = 'cold'
        report = telemetry.validate_trace(trace)
        self.assertFalse(report['warm_stop_overrun']); self.assertGreater(report['stop']['seconds'], .05)

    def test_idle_cancellation_does_not_prove_active_stop(self):
        trace = fixture(True)
        trace['events'] = [event for event in trace['events'] if not event['kind'].startswith('portion_')]
        resequence(trace)
        self.assertFalse(telemetry.validate_trace(trace)['active_cancellation'])

    def test_missing_owner_sample_requires_reason(self):
        trace = fixture(); find(trace, 'portion_submit')['owner_sample'] = dict(owner=None, sample=None)
        self.reject(trace)
        find(trace, 'portion_submit')['owner_sample']['reason'] = 'pooled owners; canonical ledger separate'
        self.assertTrue(telemetry.validate_trace(trace)['complete_evidence'])

    def test_file_decoder_and_duplicate_keys(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp)/'trace.json'; path.write_text(json.dumps(fixture()))
            self.assertTrue(telemetry.read_trace(path)['complete_evidence'])
            path.write_text('{"version":1,"version":2}')
            with self.assertRaises(telemetry.InvalidTrace): telemetry.read_trace(path)
            path.write_bytes(b'x' * (telemetry.MAX_TRACE_BYTES + 1))
            with self.assertRaises(telemetry.InvalidTrace): telemetry.read_trace(path)

    def test_wrong_types_are_validation_errors(self):
        for kind, field in [('setup', 'status'), ('setup', 'component'),
                            ('portion_complete', 'yield_reason'), ('terminal', 'outcome')]:
            trace = fixture(); find(trace, kind)[field] = []; self.reject(trace)

    def test_failed_pipeline_is_not_successful_setup(self):
        trace = fixture(); find(trace, 'setup')['status'] = 'failed'; self.reject(trace)
        find(trace, 'setup')['error'] = 'compiler failure'
        self.reject(trace)  # A later successful image cannot conceal setup failure.
        trace['events'] = [event for event in trace['events'] if not event['kind'].startswith('portion_')]
        find(trace, 'terminal').update(outcome='failed', published=False, committed_coverage=4)
        find(trace, 'reply')['outcome'] = 'failed'; resequence(trace)
        self.assertEqual(telemetry.validate_trace(trace)['setup'][0]['status'], 'failed')


if __name__ == '__main__':
    unittest.main()
