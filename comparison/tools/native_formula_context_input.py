"""Strict input binding for the first actual mixed formula context (T008).

This adapter observes an orbit of the original prepared LightBulbMoon program.
It does not claim frame/effect qualification or the numerical correctness of its
three formula bodies. Unsupported contexts remain explicit, rather than being
converted to isolated slot-zero fixtures.
"""
from pathlib import Path
import json
import struct

from qualify_native_metal_all import ROOT, artifact, require, sha

INVENTORY = 'comparison/results/formula-restart-20261104/obligations/formulas/external-8065.json'
INVENTORY_SHA = '78c21049f05a648fb9bf034c76107186a678c27534e60264c10a26a188c3e74c'
CONTEXT_ID = '7705031ddcc6a8937acbe973f3e016c5852de2e642b374c96a19f07a120ee4fd'
INDEX_SHA = '6e8e84fa26debe25c3ccd2895557e7d4dc980547396b1e6d98987c19216e0384'
PROGRAM_SHA = '04e705d6a1826330c6ef1101d54f0013bb112e7e8cb080463791b91520a462f0'
IDENTITY = ['external', 8065, '_Translate']


def context(root=ROOT):
    root = Path(root)
    inv = json.loads(artifact(root, {'path': INVENTORY, 'sha256': INVENTORY_SHA}).read_text())
    require(inv['identity'] == IDENTITY and inv['context_ids'] == [CONTEXT_ID],
            'Unreviewed mixed context inventory')
    index = inv['scene_contexts_index']
    require(index['sha256'] == INDEX_SHA, 'Unreviewed scene context index')
    index_value = json.loads(artifact(root, index).read_text())
    rows = inv['contexts']
    require(len(rows) == 1 and rows[0]['id'] == CONTEXT_ID, 'Missing mixed context')
    row = rows[0]
    # The immutable extraction index, not a caller-supplied inventory, owns it.
    authoritative = [c for c in index_value['contexts'] if c['id'] == CONTEXT_ID]
    require(authoritative == rows, 'Context differs from captured original inventory')
    require(row['capability'] == 'mixed' and len(row['program_artifacts']) == 1 and
            row['program_artifacts'][0]['sha256'] == PROGRAM_SHA and
            row['prepared_summary']['map_count'] == 0, 'Unsupported context capability/resources')
    return inv, row


def descriptor(root=ROOT):
    return {'schema': 'mb3d.native-formula-context-input.v1',
            'inventory': {'path': INVENTORY, 'sha256': INVENTORY_SHA},
            'context_id': CONTEXT_ID, 'program_index': 0, 'target_slot': 1}


def dependency_paths(root=ROOT):
    root = Path(root)
    inv, row = context(root)
    refs = [inv['scene_contexts_index'], *inv['sources'], *inv['defaults'],
            *row['artifacts'], row['host'], row['build_receipt']]
    return sorted({Path(__file__).resolve(), root / INVENTORY,
                   *(artifact(root, ref) for ref in refs)})


def validate(case, program, state, root=ROOT):
    value = case.get('context')
    require(isinstance(value, dict) and value == descriptor(root) and
            type(value['target_slot']) is int and type(value['program_index']) is int,
            'Unsupported/malformed explicit mixed context descriptor')
    require(case['identity'] == IDENTITY and case['kind'] == 'orbit' and
            'resources' not in case, 'Mixed context requires its actual resource-free orbit')
    _, row = context(root)
    raw = artifact(root, row['program_artifacts'][0]).read_bytes()
    require(len(raw) == 8192 and struct.pack('<2048I', *program) == raw,
            'Mixed program differs from original scene settings/parameters')
    # Orbit-varying state and poison/carry lanes remain model inputs. Immutable
    # scene controls must match MakeFormulaPointState's actual assignments.
    pairs = [(36, 7), (26, 16), (43, 8), (44, 9), (45, 10)]
    pairs += [(12+i, 24+i) for i in range(4)]
    pairs += [(37+i, 50+i) for i in range(6)]
    require(all(state[s] == program[p] for s, p in pairs),
            'Mixed orbit state overrides original scene caller controls')
    require(program[3:6] == [0, 0, 2] and program[1] == 1 and
            program[13] == 0 and program[7] == 400,
            'Unsupported mixed driver/bounds/iteration limit')
    return value['target_slot']


def validate_schedule(reference_case):
    """A reviewed source trace must reach the target and both neighbours.

    These are independent model assertions; device work must separately match
    them. The trace is not inferred from a program merely containing slot1.
    """
    visits = reference_case.get('expected_slot_visits')
    work = reference_case.get('expected_work')
    require(isinstance(visits, list) and len(visits) == 6 and
            all(type(n) is int and n >= 0 for n in visits) and
            all(visits[i] > 0 for i in (0, 1, 2)) and visits[3:] == [0, 0, 0] and
            type(work) is int and 0 < work <= 411 and work == sum(visits),
            'Mixed reference must independently reach target and both neighbours')
    prefix = [0]*30 + [1] + [2]*5
    schedule = [prefix[i % 36] for i in range(work)]
    expected_visits = [schedule.count(i) for i in range(6)]
    targets = reference_case.get('expected_state_bits', {})
    require(visits == expected_visits and
            targets.get('iteration') == f'0x{work-visits[1]:08x}' and
            targets.get('current_slot') == f'0x{schedule[-1]:08x}',
            'Mixed source work/visits/final state contradict original count schedule')
