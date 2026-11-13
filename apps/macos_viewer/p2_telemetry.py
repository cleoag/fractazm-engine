"""Private P2 diagnostic contract. Synthetic validation is not runtime evidence.

One bounded, finalized stream belongs to one job. Event sequence is independent
of portion sequence. Producers keep raw GPU clocks separate from host clocks;
missing or incomparable durations remain unavailable. No renderer uses this yet.
"""
import json
import math
import statistics

VERSION = 1
MAX_QUANTUM = 1 << 20  # continuation_control.h: MaxAllowance
MAX_EVENTS = 8192
MAX_EVENT_BYTES = 4096
MAX_TRACE_BYTES = 8 * 1024 * 1024
MAX_REPLY_BYTES = 1024 * 1024
IDENTITY = ('request_id', 'job_id', 'epoch', 'phase', 'scene_revision',
            'camera_revision', 'quality_revision')
TERMINALS = {'completed', 'cancelled', 'refused', 'failed', 'hard_limit', 'deadline'}


class InvalidTrace(ValueError):
    pass


def require(condition, message):
    if not condition:
        raise InvalidTrace(message)


def integer(value, label, minimum=0):
    require(type(value) is int and minimum <= value <= (1 << 64) - 1, label)
    return value


def encoded(value):
    try:
        return json.dumps(value, allow_nan=False, separators=(',', ':'),
                          ensure_ascii=False).encode('utf-8')
    except (TypeError, ValueError) as error:
        raise InvalidTrace('invalid JSON value') from error


def stamp(value):
    require(isinstance(value, dict), 'timestamp object required')
    require(value.get('unit') == 'seconds', 'explicit seconds required')
    require(isinstance(value.get('domain'), str) and 0 < len(value['domain']) <= 128,
            'clock domain required')
    require('value' in value, 'explicit timestamp value or null required')
    number = value['value']
    if number is None:
        require(isinstance(value.get('reason'), str) and bool(value['reason'].strip()),
                'missing timestamp needs a reason')
    else:
        valid = type(number) in (int, float)
        try:
            valid = valid and math.isfinite(number) and number >= 0
        except OverflowError:
            valid = False
        require(valid, 'invalid timestamp')
        require(value.get('reason') is None, 'measured timestamp has missing-value reason')
    return value


def duration(start, end):
    start, end = stamp(start), stamp(end)
    if start['value'] is None or end['value'] is None:
        return {'seconds': None, 'reason': 'timestamp unavailable'}
    if start['domain'] != end['domain']:
        return {'seconds': None, 'reason': 'incomparable clock domains'}
    require(end['value'] >= start['value'], 'negative duration')
    seconds = float(end['value'] - start['value'])
    require(math.isfinite(seconds * 1000), 'duration cannot be represented in milliseconds')
    return {'seconds': seconds, 'reason': None}


def distribution(values):
    if not values:
        return {'count': 0, 'median_ms': None, 'p95_ms': None, 'max_ms': None}
    values = sorted(values)
    return {'count': len(values), 'median_ms': statistics.median(values) * 1000,
            'p95_ms': values[math.ceil(.95 * len(values)) - 1] * 1000,
            'max_ms': values[-1] * 1000}


def validate_reply_size(reply):
    require(isinstance(reply, dict), 'reply object required')
    require(len(encoded(reply)) <= MAX_REPLY_BYTES, 'reply exceeds 1 MiB')
    require('portion_events' not in reply, 'portion records belong in bounded trace sink')
    if 'trace' in reply:
        summary = reply['trace']
        require(isinstance(summary, dict) and 'events' not in summary,
                'reply trace must be a summary')
        require(len(encoded(summary)) <= MAX_EVENT_BYTES, 'trace summary too large')
    return True


def validate_trace(trace):
    require(isinstance(trace, dict), 'trace object required')
    require(type(trace.get('version')) is int and trace['version'] == VERSION,
            'unsupported trace version')
    require(len(encoded(trace)) <= MAX_TRACE_BYTES, 'trace exceeds byte bound')
    identity = trace.get('identity')
    require(isinstance(identity, dict) and set(identity) == set(IDENTITY), 'job identity required')
    for key in IDENTITY:
        integer(identity[key], key)
    require(isinstance(trace.get('pipeline_cache'), str) and trace['pipeline_cache'] in {'cold', 'warm', 'unknown'}, 'cache provenance required')
    provenance = trace.get('provenance')
    require(isinstance(provenance, dict), 'run provenance required')
    for key in ('build_receipt_sha256', 'library_sha256'):
        digest = provenance.get(key)
        require(isinstance(digest, str) and len(digest) == 64 and
                all(c in '0123456789abcdef' for c in digest), 'invalid provenance digest')
    for key in ('device', 'scene', 'profile', 'contention'):
        require(isinstance(provenance.get(key), str) and 0 < len(provenance[key]) <= 256,
                'run cohort required')
    for key in ('width', 'height', 'workers'):
        integer(provenance.get(key), key, 1)
    dropped = integer(trace.get('dropped_events'), 'dropped event count')
    events = trace.get('events')
    require(isinstance(events, list) and 0 < len(events) <= MAX_EVENTS, 'bounded events required')
    active = None
    portions = 0
    terminal = None
    reply = False
    cancelling = False
    quiescent = None
    cancel_requested = None
    active_cancellation = False
    fault_outcome = None
    setup = []
    timings = []
    unavailable = []
    overruns = []
    previous = {}
    work = coverage = ready = total = 0
    for sequence, event in enumerate(events):
        require(isinstance(event, dict) and len(encoded(event)) <= MAX_EVENT_BYTES,
                'event exceeds bound')
        require(type(event.get('sequence')) is int and event['sequence'] == sequence,
                'noncontiguous event sequence')
        require(event.get('identity') == identity, 'foreign job event')
        for key in IDENTITY:
            integer(event['identity'][key], key)
        at = stamp(event.get('at'))
        if at['value'] is not None:
            require(at['value'] >= previous.get(at['domain'], 0), 'clock moved backwards')
            previous[at['domain']] = at['value']
        kind = event.get('kind')
        if kind != 'portion_complete':
            require('gpu_begin' not in event and 'gpu_end' not in event, 'GPU clocks read before completion')
        require(not reply, 'event after terminal reply')
        require(terminal is None or kind == 'reply', 'event after terminal outcome')
        if kind == 'begin':
            require(sequence == 0, 'duplicate begin')
            work = integer(event.get('semantic_work'), 'initial work')
            coverage = integer(event.get('committed_coverage'), 'initial coverage')
            total = integer(event.get('total_samples'), 'total samples')
        else:
            require(sequence > 0 and events[0].get('kind') == 'begin', 'missing begin')
            if kind == 'setup':
                require(active is None, 'setup overlaps an in-flight portion')
                require(isinstance(event.get('component'), str) and event['component'] in {'prepare', 'device', 'library', 'abi', 'pipeline'},
                        'unknown setup component')
                require(isinstance(event.get('status'), str) and event['status'] in {'ready', 'failed'}, 'setup status required')
                if event['status'] == 'failed':
                    require(bool(event.get('error')), 'failed setup needs error')
                    fault_outcome = 'failed'
                setup.append(dict(component=event['component'], status=event['status'],
                                  duration=duration(event.get('begin'), at)))
            elif kind == 'portion_submit':
                require(active is None and not cancelling and quiescent is None and fault_outcome is None,
                        'overlap or submission after cancellation/quiescence')
                require(integer(event.get('portion'), 'portion sequence') == portions,
                        'stale or skipped portion')
                quantum = integer(event.get('quantum'), 'positive quantum', 1)
                require(quantum <= MAX_QUANTUM, 'quantum exceeds control contract')
                require(isinstance(event.get('stage'), str) and bool(event['stage']), 'stage required')
                location = event.get('owner_sample')
                require(isinstance(location, dict), 'owner/sample attribution required')
                for key in ('owner', 'sample'):
                    if location.get(key) is not None:
                        integer(location[key], key)
                if location.get('owner') is None or location.get('sample') is None:
                    require(bool(location.get('reason')), 'missing owner/sample needs explanation')
                active = event
            elif kind == 'portion_complete':
                portion = integer(event.get('portion'), 'completion sequence')
                require(active is not None and portion == active['portion'],
                        'completion without matching submission')
                consumed = integer(event.get('consumed_quantum'), 'consumed quantum')
                require(consumed <= active['quantum'], 'allowance exceeded')
                require(isinstance(event.get('yield_reason'), str) and event['yield_reason'] in {'allowance', 'phase_complete', 'fault', 'cancelled'},
                        'yield reason required')
                require(event['yield_reason'] != 'allowance' or consumed > 0, 'yield made no progress')
                if event['yield_reason'] == 'fault':
                    fault_outcome = event.get('fault_outcome')
                    require(isinstance(fault_outcome, str) and fault_outcome in TERMINALS - {'completed', 'cancelled'},
                            'specific fault outcome required')
                new_work = integer(event.get('semantic_work'), 'semantic work')
                new_ready = integer(event.get('completed_samples'), 'completed samples')
                remaining = integer(event.get('remaining_samples'), 'remaining samples')
                require(new_work >= work and ready <= new_ready <= total and new_ready + remaining == total,
                        'work or sample accounting regressed')
                require(event.get('published') is False, 'portion cannot publish a phase image')
                require(integer(event.get('committed_coverage'), 'coverage') == coverage,
                        'pending portion contributed coverage')
                require(isinstance(event.get('stage'), str) and bool(event['stage']), 'end stage required')
                duration(active['at'], at)  # Validate ordering only within one domain.
                gpu = duration(event.get('gpu_begin'), event.get('gpu_end'))
                if gpu['seconds'] is None:
                    unavailable.append({'portion': portions, 'reason': gpu['reason']})
                else:
                    timings.append(gpu['seconds'])
                    if gpu['seconds'] > .010:
                        overruns.append({'portion': portions, 'seconds': gpu['seconds']})
                work, ready = new_work, new_ready
                portions += 1
                active = None
            elif kind == 'cancel_request':
                require(cancel_requested is None, 'duplicate cancellation request')
                cancel_requested = at
                active_cancellation = active is not None
            elif kind == 'cancel_observed':
                require(cancel_requested is not None and not cancelling, 'unattributed cancellation')
                duration(cancel_requested, at)
                cancelling = True
            elif kind == 'quiescent':
                require(active is None and quiescent is None, 'premature or duplicate quiescence')
                quiescent = at
            elif kind == 'terminal':
                require(active is None and quiescent is not None, 'terminal before quiescence')
                terminal = event.get('outcome')
                require(isinstance(terminal, str) and terminal in TERMINALS, 'terminal reason required')
                require(fault_outcome is None or terminal == fault_outcome, 'fault hidden by terminal reason')
                require(terminal != 'cancelled' or cancelling, 'unobserved cancellation')
                require(terminal != 'completed' or not cancelling, 'completion concealed observed cancellation')
                final_coverage = integer(event.get('committed_coverage'), 'final coverage')
                if terminal == 'completed':
                    require(ready == total and final_coverage >= coverage and event.get('published') is True,
                            'incomplete success')
                else:
                    require(final_coverage == coverage and event.get('published') is False,
                            'failed or cancelled phase published output')
            elif kind == 'reply':
                require(terminal is not None and event.get('outcome') == terminal, 'false terminal reply')
                reply = True
            else:
                raise InvalidTrace('unknown event kind')
    if not dropped:
        require(reply and terminal is not None, 'finalized trace lacks terminal reply')
    client = trace.get('client')
    require(isinstance(client, dict), 'client clock observations required')
    for name in ('input', 'reply', 'draw', 'present'):
        stamp(client.get(name))
    require(client['present']['value'] is None, 'P2 AppKit draw is not presentation acknowledgment')
    stop = duration(cancel_requested, quiescent) if cancel_requested and quiescent else {
        'seconds': None, 'reason': 'no complete cancellation observation'}
    return {'complete_evidence': not dropped and reply, 'terminal': terminal, 'portions': portions,
            'gpu': distribution(timings), 'unavailable_gpu': unavailable, 'portion_overruns': overruns,
            'stop': stop, 'active_cancellation': active_cancellation, 'warm_stop_overrun': trace['pipeline_cache'] == 'warm' and
            stop['seconds'] is not None and stop['seconds'] > .050,
            'request_to_reply': duration(client['input'], client['reply']),
            'input_to_draw': duration(client['input'], client['draw']),
            'input_to_present': duration(client['input'], client['present']), 'setup': setup,
            'gpu_timing_complete': not dropped and not unavailable and portions > 0 and len(timings) == portions}


def read_trace(path):
    # Bound the input before parsing. Duplicate keys could hide identities or
    # terminal reasons, so reject them rather than taking the last value.
    def unique(pairs):
        result = {}
        for key, value in pairs:
            require(key not in result, 'duplicate JSON key')
            result[key] = value
        return result
    with open(path, 'rb') as source:
        data = source.read(MAX_TRACE_BYTES + 1)
    require(len(data) <= MAX_TRACE_BYTES, 'trace input exceeds byte bound')
    try:
        result = json.loads(data, object_pairs_hook=unique)
    except (ValueError, UnicodeDecodeError) as error:
        raise InvalidTrace('invalid or ambiguous JSON trace') from error
    return validate_trace(result)


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('trace')
    args = parser.parse_args()
    try:
        print(json.dumps(read_trace(args.trace), indent=2, allow_nan=False))
    except (InvalidTrace, OSError) as error:
        parser.error(str(error))
