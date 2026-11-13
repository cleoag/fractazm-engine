"""Independent binary32 reference for four audited affine M3F CODE blocks.

Expected state is computed from the immutable x87 instruction semantics and
Pascal caller contracts, rounded to binary32 after each primitive operation.
This module neither imports translated formula bodies nor executes GPU code.
It is a bounded formula/caller fixture model, not an arbitrary orbit interpreter
and not a claim of Pascal/x87 parity or full-scene qualification.
"""
from __future__ import annotations

import argparse
from fractions import Fraction
import hashlib
import json
from pathlib import Path
import struct

from gpu_pipeline_proofs import STATE

ROOT = Path(__file__).resolve().parents[2]
# Transport descriptor: ID, original DE option, parameter count, wrapper traits,
# wrapper flags, analytic caller. Traits/flags are ABI metadata, not the oracle.
FORMULAS = {
    '_Translate': (8065, -1, 3, 0, 1, 1),
    '_TranslateC4d': (10001, -1, 8, 544, 0, 3),
    '_scaleC': (6003, -1, 3, 0, 1, 1),
    '_ScaleC4d': (9525, 5, 2, 0, 0, 3),
}
POINT_WRITES = {
    '_Translate': tuple(f'xyz[{i}]' for i in range(3)),
    '_TranslateC4d': tuple(f'{a}[{i}]' for a in ('xyz', 'j') for i in range(4)),
    '_scaleC': tuple(f'j[{i}]' for i in range(3)),
    '_ScaleC4d': tuple(f'{a}[{i}]' for a in ('xyz', 'j') for i in range(4)),
}


def word(value: int) -> str:
    return f'0x{value & 0xffffffff:08x}'


def fraction(bits: int) -> Fraction:
    """Decode a finite binary32 word exactly, including subnormals."""
    exponent = (bits >> 23) & 255
    if exponent == 255:
        raise ValueError('reference domain requires finite binary32 operands')
    mantissa = (bits & 0x7fffff) | (0x800000 if exponent else 0)
    shift = (exponent - 127 - 23) if exponent else -149
    value = Fraction(mantissa << shift) if shift >= 0 else Fraction(mantissa, 1 << -shift)
    return -value if bits >> 31 else value


def _nearest_even(numerator: int, denominator: int) -> int:
    quotient, remainder = divmod(numerator, denominator)
    twice = remainder * 2
    return quotient + int(twice > denominator or (twice == denominator and quotient & 1))


def binary32(value: Fraction | int, negative_zero: bool = False) -> int:
    """Round an exact rational to IEEE binary32, RN-even; never use host FP math."""
    value = Fraction(value)
    if not value:
        return 0x80000000 if negative_zero else 0
    sign = 0x80000000 if value < 0 else 0
    value = abs(value)
    n, d = value.numerator, value.denominator
    exponent = n.bit_length() - d.bit_length()
    if (n < d << exponent) if exponent >= 0 else (n << -exponent < d):
        exponent -= 1
    shift = 149 if exponent < -126 else 23 - exponent
    significand = _nearest_even(n << shift, d) if shift >= 0 else _nearest_even(n, d << -shift)
    if exponent < -126:
        return sign | significand
    if significand == 1 << 24:
        significand >>= 1
        exponent += 1
    if exponent > 127:
        return sign | 0x7f800000
    return sign | ((exponent + 127) << 23) | (significand & 0x7fffff)


def add(a: int, b: int) -> int:
    return binary32(fraction(a) + fraction(b), a == b == 0x80000000)


def multiply(a: int, b: int) -> int:
    return binary32(fraction(a) * fraction(b), bool((a ^ b) >> 31))


def _bits(value) -> int:
    # Fixture literals are exact integers/Fractions; strings such as '0x80000000'
    # deliberately denote raw words so signed zero cannot be lost.
    return int(value, 16) if isinstance(value, str) else binary32(value)


def _point(name: str, state: dict[str, int], parameters: list[int]) -> dict[str, int]:
    out = dict(state)
    if name == '_Translate':
        for i in range(3):
            out[f'xyz[{i}]'] = add(state[f'xyz[{i}]'], parameters[i])
    elif name == '_TranslateC4d':
        for i in range(4):
            out[f'xyz[{i}]'] = add(state[f'xyz[{i}]'], parameters[i])
            out[f'j[{i}]'] = add(state[f'j[{i}]'], parameters[i + 4])
    elif name == '_scaleC':
        for i in range(3):
            out[f'j[{i}]'] = multiply(state[f'j[{i}]'], parameters[i])
    elif name == '_ScaleC4d':
        for i in range(4):
            out[f'xyz[{i}]'] = multiply(state[f'xyz[{i}]'], parameters[0])
            out[f'j[{i}]'] = multiply(state[f'j[{i}]'], parameters[1])
    else:
        raise ValueError('formula outside independent reference scope')
    out['current_slot'] = 0  # Observer ABI exposes the selected callback slot.
    return out


def _radius(state: dict[str, int], four: bool) -> int:
    coordinates = [state[f'xyz[{i}]'] for i in range(4 if four else 3)]
    squares = [multiply(v, v) for v in coordinates]
    if any(fraction(square) != fraction(v) ** 2 for v, square in zip(coordinates, squares)):
        raise ValueError('orbit radius outside exact-arithmetic fixture domain')
    result = add(squares[0], squares[1])
    if fraction(result) != fraction(squares[0]) + fraction(squares[1]):
        raise ValueError('orbit radius sum outside exact-arithmetic fixture domain')
    for square in squares[2:]:
        exact = fraction(result) + fraction(square)
        result = add(result, square)
        if fraction(result) != exact:
            raise ValueError('orbit radius sum outside exact-arithmetic fixture domain')
    return result


def _orbit(name: str, state: dict[str, int], parameters: list[int], program: list[int]) -> dict[str, int]:
    """Restricted doHybridPasDE/doHybrid4DDEPas, no interpolation or smoothing.

    Identity rotation and exact dyadic orbit fixtures avoid unspecified FMA
    contraction effects. This models state only; sqrt-derived distance is not
    silently promoted to an independently checked output.
    """
    s = dict(state)
    four = program[1] == 3
    if four:
        # Math3D.pas Rotate4Dex evaluates all products/sums, even for identity.
        # Assigning XYZ=C/W=+0 would discard observable signed-zero behavior.
        for row in range(4):
            terms = [multiply(s[f'c[{i}]'], program[28 + row * 4 + i]) for i in range(3)]
            s[f'xyz[{row}]'] = add(add(terms[0], terms[1]), terms[2])
    else:
        for i in range(3):
            s[f'xyz[{i}]'] = s[f'c[{i}]']
        s['xyz[3]'] = binary32(1)
    for i in range(3):
        s[f'j[{i}]'] = s[f'original_j[{i}]'] if s['julia'] else s[f'xyz[{i}]']
    if four:
        s['j[3]'] = s['original_j[3]'] if s['julia'] else s['xyz[3]']
        s['derivative[0]'] = binary32(1)
    # The 3D caller fixtures enter with VaryScale=1 and J4=Ju4. Their
    # initialization is therefore not inferred from translated GPU code.
    s['first'] = 0
    s['iteration'] = 0
    s['rout'] = _radius(s, four)
    s['otrap'] = s['rout']
    count = s['hybrid_counts[0]']
    remaining = count
    s['tmp'] = remaining
    for _ in range(s['max_iterations']):
        s['rold'] = s['rout']
        if remaining == 0:
            remaining = count  # One slot repeats to itself.
        s = _point(name, s, parameters)
        remaining -= 1
        s['tmp'] = remaining
        s['iteration'] += 1
        s['rout'] = _radius(s, four)
        if fraction(s['rout']) < fraction(s['otrap']):
            s['otrap'] = s['rout']
        if fraction(s['rout']) > fraction(s['rstop']):
            break
    return s


def _program(name: str, params: list[int], *, julia: int, iterations: int, count: int, stop: int) -> list[int]:
    identity, option, size, traits, flags, driver = FORMULAS[name]
    if len(params) != size:
        raise ValueError('wrong parameter count')
    p = [0] * 2048
    p[:15] = [1, driver, 6, 0, 0, 0, 5 if driver == 3 else 2,
              iterations, julia, 0, 0, 256, 256, 0, 0]
    p[16], p[18], p[21], p[22] = stop, binary32(1), binary32(1 << 20), binary32(1)
    for i in range(4):
        p[28 + i * 4 + i] = binary32(1)
    p[50] = count
    for slot in range(6):
        p[56 + slot * 332] = 1
    base = 56
    p[base:base + 12] = [1, 1, identity, option & 0xffffffff, size, traits, flags, 0, 0, 1, 0, 0]
    p[base + 12:base + 12 + size] = params
    # Exact binary64 metadata and the source-defined negative PVar slots.
    # Converting a binary32 value to binary64 is exact, including signed zero.
    for i, bits in enumerate([binary32(Fraction(1, 2)), *params]):
        value = struct.unpack('<f', struct.pack('<I', bits))[0]
        lo, hi = struct.unpack('<II', struct.pack('<d', value))
        raw_offset = 204 + 62 - 2 * i
        p[base + raw_offset:base + raw_offset + 2] = [lo, hi]
        p[base + 268 + i] = bits
        if i:
            p[base + 76 + 2 * (i - 1):base + 78 + 2 * (i - 1)] = [lo, hi]
    return p


def evaluate(semantic_input: dict) -> dict[str, str]:
    """Evaluate the exact four-key observer input; reject out-of-domain callers."""
    if set(semantic_input) != {'identity', 'kind', 'input_state_bits', 'parameter_bits'}:
        raise ValueError('expected four-key semantic input')
    identity = semantic_input['identity']
    if len(identity) != 3 or identity[0] != 'external' or identity[2] not in FORMULAS:
        raise ValueError('unsupported identity')
    name = identity[2]
    if identity[1] != FORMULAS[name][0]:
        raise ValueError('formula name/ID mismatch')
    raw_state = semantic_input['input_state_bits']
    if set(raw_state) != set(STATE) or len(semantic_input['parameter_bits']) != 2048:
        raise ValueError('wrong state/program width')
    def decode(value):
        if not isinstance(value, str) or len(value) != 10 or not value.startswith('0x') or any(c not in '0123456789abcdef' for c in value[2:]):
            raise ValueError('expected 0x plus eight hex digits')
        return int(value, 16)
    s = {k: decode(raw_state[k]) for k in STATE}
    p = [decode(x) for x in semantic_input['parameter_bits']]
    for lane in STATE[:33]:
        fraction(s[lane])
    if semantic_input['kind'] not in ('point', 'orbit'):
        raise ValueError('unsupported observer kind')
    params = p[68:68 + FORMULAS[name][2]]
    for bits in params:
        fraction(bits)
    if not 1 <= s['max_iterations'] <= 8 or not 1 <= s['hybrid_counts[0]'] <= 3:
        raise ValueError('reference fixture bounds exceeded')
    expected_program = _program(name, params, julia=s['julia'], iterations=s['max_iterations'],
                                count=s['hybrid_counts[0]'], stop=s['rstop'])
    expected_program[24:28] = [s[f'original_j[{i}]'] for i in range(4)]
    if p != expected_program or s['julia'] not in (0, 1) or s['inside'] or s['calc_smooth']:
        raise ValueError('program outside independently audited single-slot caller domain')
    if semantic_input['kind'] == 'orbit':
        if p[1] == 1 and (s['vary_scale'] != binary32(1) or s['j[3]'] != s['original_j[3]']):
            raise ValueError('3D initialization outside audited caller domain')
        out = _orbit(name, s, params, p)
    else:
        out = _point(name, s, params)
    if any(out[lane] & 0x7f800000 == 0x7f800000 for lane in STATE[:33]):
        raise ValueError('nonfinite arithmetic result outside no-range-exit reference domain')
    return {k: word(out[k]) for k in STATE}


def source_manifest() -> dict:
    audit_path = ROOT / 'tests/fixtures/native_affine_reference/manifest.json'
    audit = json.loads(audit_path.read_text())
    semantic = list(audit['sources'])
    for path in semantic:
        actual = hashlib.sha256((ROOT / path).read_bytes()).hexdigest()
        if actual not in audit['sources'][path].values():
            raise ValueError('immutable source changed since independent affine audit: ' + path)
    return {
        'schema': 'mb3d.native-affine-reference.v1',
        'original_audit_sha256': hashlib.sha256(audit_path.read_bytes()).hexdigest(),
        'arithmetic': 'Independent exact rational add/multiply rounded binary32 RN-even; signed zero explicit',
        'semantic_sources': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in semantic},
        'abi_sources': {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in (
            'src/render/native_metal/formula_abi.h', 'src/core/external_formulas.h')},
        'caller_source': 'formulas.pas doHybridPasDE / doHybrid4DDEPas; Math3D.pas Rotate4Dex/mCopyVec; TypeDefinitions.pas TIteration3Dext',
        'original_C_mapping': 'M3F esi+24/+32/+40 and esi-56 are J1/J2/J3/J4; C1/C2/C3 remain immutable',
        'model_limits': ['Four affine formulas only', 'Single enabled slot0, analytic 3D/4D, identity rotation',
                         'No smoothing/interpolation/DE combination/range exit; no distance-value proof',
                         'No CPU x87/Pascal parity or original-size scene credit',
                         'Transport metadata is pinned separately; output arithmetic does not use translated bodies'],
        'model_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
    }


def build_cases() -> list[dict]:
    """40 bounded cases: 24 nontrivial point fixtures and 16 caller orbits."""
    cases = []
    provenance = source_manifest()
    variants = ['asymmetric', 'signed_zero', 'rounding_ties', 'large_c_cancellation', 'cancellation', 'integer_sentinels']
    for name, (identity, option, size, traits, flags, driver) in FORMULAS.items():
        for index, variant in enumerate(variants + ['orbit_mandel', 'orbit_julia', 'orbit_escape', 'orbit_repeat']):
            orbit = index >= 6
            state = {lane: binary32(Fraction(i + 1, 8)) if i < 33 else i + 11 for i, lane in enumerate(STATE)}
            state.update(iteration=19, first=17, tmp=23, max_iterations=4,
                         julia=0, inside=0, calc_smooth=0, current_slot=5)
            for i in range(6):
                state[f'hybrid_counts[{i}]'] = 1 if i == 0 else 0
            for i, value in enumerate((Fraction(3, 8), Fraction(-1, 2), Fraction(5, 8), Fraction(-3, 4))):
                state[f'xyz[{i}]'] = binary32(value)
                state[f'c[{i}]'] = binary32(value)
                state[f'j[{i}]'] = binary32(value * 2)
                state[f'original_j[{i}]'] = binary32(value * -2)
            state['rstop'] = binary32(1 << 16)
            state['rstop_d'] = binary32(Fraction(1, 8))
            params = ([Fraction(1, 8), Fraction(-1, 4), Fraction(3, 8)] if name == '_Translate' else
                      [Fraction(1, 8), Fraction(-1, 4), Fraction(3, 8), Fraction(-1, 2),
                       Fraction(5, 8), Fraction(-3, 4), Fraction(7, 8), -1] if name == '_TranslateC4d' else
                      [-2, Fraction(1, 2), 0] if name == '_scaleC' else [Fraction(-1, 2), 2])
            params = [_bits(v) for v in params]
            if variant == 'signed_zero':
                for i in range(4):
                    state[f'xyz[{i}]'] = 0x80000000 if i & 1 else 0
                    state[f'j[{i}]'] = 0 if i & 1 else 0x80000000
                params = ([0x80000000] * size if name.startswith('_Translate') else
                          [_bits(-1 if i & 1 else 0) for i in range(size)])
            elif variant == 'rounding_ties':
                for i in range(4):
                    state[f'xyz[{i}]'] = 0x3f800000 + i
                    state[f'j[{i}]'] = 0xbf800000 + i
                params = [binary32(Fraction(1, 1 << 24))] * size if name.startswith('_Translate') else [0x3f800001] * size
            elif variant == 'large_c_cancellation':
                for i in range(4):
                    state[f'xyz[{i}]'] = binary32((1, -1, Fraction(3, 4), Fraction(-1, 2))[i])
                    state[f'j[{i}]'] = binary32((1 << 25) * (-1 if i & 1 else 1))
                params = [binary32(0 if name.startswith('_Translate') else 1)] * size
            elif variant == 'cancellation':
                for i in range(4):
                    state[f'xyz[{i}]'] = binary32((-1 if i & 1 else 1) * (1 << 24))
                    state[f'j[{i}]'] = state[f'xyz[{i}]']
                params = [binary32((1 if i & 1 else -1) * (1 << 24)) for i in range(size)] if name.startswith('_Translate') else [binary32(-1)] * size
            elif variant == 'integer_sentinels':
                state['first'], state['iteration'], state['tmp'] = 0x80000000, 0x7fffffff, 0xffffffff
            elif orbit:
                state['julia'] = int(variant in ('orbit_julia', 'orbit_repeat'))
                state['max_iterations'] = {'orbit_mandel': 4, 'orbit_julia': 5, 'orbit_escape': 4, 'orbit_repeat': 4}[variant]
                state['hybrid_counts[0]'] = {'orbit_mandel': 1, 'orbit_julia': 2, 'orbit_escape': 3, 'orbit_repeat': 2}[variant]
                if variant == 'orbit_escape':
                    state['rstop'] = binary32(Fraction(1, 16))
                if variant == 'orbit_repeat':
                    # Exact first-step radius; >= would falsely stop at step1.
                    state['rstop'] = binary32({'_Translate': Fraction(29, 16),
                        '_TranslateC4d': Fraction(33, 16), '_scaleC': Fraction(25, 32),
                        '_ScaleC4d': Fraction(25, 128)}[name])
                if variant == 'orbit_mandel' and driver == 3:
                    for i in range(3):
                        state[f'c[{i}]'] |= 0x80000000
                if driver == 1:
                    state['vary_scale'] = binary32(1)
                    state['j[3]'] = state['original_j[3]']
            program = _program(name, params, julia=state['julia'], iterations=state['max_iterations'],
                               count=state['hybrid_counts[0]'], stop=state['rstop'])
            program[24:28] = [state[f'original_j[{i}]'] for i in range(4)]
            semantic = {'identity': ['external', identity, name], 'kind': 'orbit' if orbit else 'point',
                        'input_state_bits': {k: word(state[k]) for k in STATE},
                        'parameter_bits': [word(x) for x in program]}
            writes = set(POINT_WRITES[name]) | {'current_slot'}
            reads = set(POINT_WRITES[name])
            if orbit:
                writes |= {f'xyz[{i}]' for i in range(4)} | {f'j[{i}]' for i in range(3)} | {'rout', 'rold', 'otrap', 'first', 'iteration', 'tmp'}
                if driver == 3:
                    writes |= {'j[3]', 'derivative[0]'}
                reads |= {f'{a}[{i}]' for a in ('c', 'original_j') for i in range(4)} | {'julia', 'max_iterations', 'rstop', 'hybrid_counts[0]'}
            cases.append({'id': f'{identity}-{variant}', 'identity': semantic['identity'], 'variant': variant,
                          'input': semantic, 'expected_state_bits': evaluate(semantic),
                          'read_lanes': [k for k in STATE if k in reads],
                          'write_lanes': [k for k in STATE if k in writes],
                          'source_hashes': provenance['semantic_sources']})
    return cases


# Public spelling for collectors; provenance is separate from semantic input.
manifest = source_manifest


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--input', type=Path, help='evaluate one exact semantic input JSON')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        parser.error('output already exists')
    result = ({'expected_state_bits': evaluate(json.loads(args.input.read_text())), 'reference': manifest()}
              if args.input else {'reference': manifest(), 'cases': build_cases()})
    args.output.write_text(json.dumps(result, indent=2) + '\n')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
