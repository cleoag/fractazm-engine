"""Independent original _Scaling CODE and bounded analytic3D state reference.

Four scalar products are rounded to binary32, never inferred from candidate
outputs. Orbit radii use exact products and every subset sum, so their targets
are independent of contraction/grouping. No distance, arbitrary-hybrid, x87
parity or scene claim is made by this model.
"""
from fractions import Fraction
import hashlib
from itertools import combinations
from math import isqrt
from pathlib import Path
import re
import struct

import native_formula_reference as arithmetic
from gpu_pipeline_proofs import STATE

ROOT = Path(__file__).resolve().parents[2]
FORMULAS = {'_Scaling': (9452, 2, 1, 0, 1, 1)}
SOURCE_PINS = {
    'mb3d/M3Formulas/_Scaling.m3f': '6bf1874557dd4907396b3131f9d89427122051395de30bfee9815a5a636b10ab',
    'mb3d/CustomFormulas.pas': '9ed1c8289ebfb99202795af8daca6bef92650ba250a28c12435eb5beb4264c61',
    'mb3d/TypeDefinitions.pas': '5ef479844d3d5ffac309b141c9f80b62ebf0f9bf8b8782b706d9f451ef096747',
    'mb3d/formulas.pas': '3fefcd04fe6726831f3a7904f151cc3cc192610f4532f2889b9771e5953775bd',
}
ABI_PINS = {
    'src/render/native_metal/formula_abi.h': 'd69725ad200b889c8e7e2c3267d8fee96e37188b54e5cdc145a6cde3afb6350a',
    'src/core/external_formulas.h': 'b36cba7e6b37c6426a8d052f2434866397f6edc7edb8d799efd0ffccc751f3b6',
}
CODE_SHA256 = 'e6159d5c6632faf8103af8bd5cf05f1f103609930828bed384f1f63a53ae0b00'
CALLER_PROFILE = ('comparison/results/native-formula-group05-20261104/caller-profile.json', '3b9231389e037f59f9c16f425575836cbe621c34c59a41cd577aae63405993f6')
PRIMITIVE_SHA256 = 'a0964784744671549916c1b53b227ea085be6e8eef18d6fe84bd79994295d94e'
fp32, fraction, word = arithmetic.binary32, arithmetic.fraction, arithmetic.word
ONE, NEG_ZERO = 0x3f800000, 0x80000000


class ScopeError(ValueError):
    """Input lies outside the independently stated proof domain."""


def normal_or_zero(bits):
    value = fraction(bits)
    if value and bits & 0x7f800000 == 0:
        raise ScopeError('subnormal arithmetic outside reference domain')
    return value


def product(a, b):
    normal_or_zero(a)
    normal_or_zero(b)
    result = arithmetic.multiply(a, b)
    normal_or_zero(result)
    return result


def radius(s):
    squares = []
    for i in range(3):
        v = s[f'xyz[{i}]']
        square = product(v, v)
        if fraction(square) != fraction(v)**2:
            raise ScopeError('rounded radius product')
        squares.append(square)
    for n in (2, 3):
        for subset in combinations(squares, n):
            exact = sum(map(fraction, subset), Fraction())
            if normal_or_zero(fp32(exact)) != exact:
                raise ScopeError('rounded radius subset sum')
    return fp32(sum(map(fraction, squares), Fraction()))


def point(s, scale):
    # Original CODE 0x0a loads [ecx+8]=w; 0x13 loads PVar-16=Scale;
    # 0x16..0x1c multiply w,z,y,x; 0x1e..0x24 store x,y,z,w.
    # Deriv1..3 and J/C are neither read nor written.
    out = dict(s)
    for i in range(4):
        out[f'xyz[{i}]'] = product(s[f'xyz[{i}]'], scale)
    out['current_slot'] = 0
    return out


def orbit(s, scale):
    # Original formulas.pas:3697–3750, DEoption=2, single positive slot,
    # maxIt>=1, no smoothing. w=1 is separate from preserved Deriv1..3.
    out = dict(s)
    for i in range(3):
        out[f'xyz[{i}]'] = s[f'c[{i}]']
        out[f'j[{i}]'] = s[f'original_j[{i}]'] if s['julia'] else s[f'c[{i}]']
    out['xyz[3]'] = ONE
    out['first'] = out['iteration'] = 0
    out['rout'] = out['otrap'] = radius(out)
    count = remaining = s['hybrid_counts[0]']
    out['tmp'] = remaining
    for _ in range(s['max_iterations']):
        out['rold'] = out['rout']
        if remaining == 0:
            remaining = count
        out = point(out, scale)
        remaining -= 1
        out['tmp'] = remaining
        out['iteration'] += 1
        out['rout'] = radius(out)
        if fraction(out['rout']) < fraction(out['otrap']):
            out['otrap'] = out['rout']
        if fraction(out['rout']) > fraction(s['rstop']):
            break
    if not fraction(out['xyz[3]']):
        raise ScopeError('zero denominator outside finite distance caller scope')
    # Independently compiled original DE2 epilogue returns distance in
    # XMM0.low and abs(w) in XMM1.low. Only exact rational square roots
    # and divisions are admitted here, before any device observations.
    value = fraction(out['rout'])
    a, b = isqrt(value.numerator), isqrt(value.denominator)
    if a*a != value.numerator or b*b != value.denominator:
        raise ScopeError('irrational distance transport outside exact-root domain')
    distance = Fraction(a,b)/abs(fraction(out['xyz[3]']))
    rounded = fp32(distance)
    if normal_or_zero(rounded) != distance:
        raise ScopeError('rounded distance transport outside exact domain')
    out['sse[0]'], out['sse[1]'] = rounded, 0
    out['sse[2]'], out['sse[3]'] = out['xyz[3]'] & 0x7fffffff, 0
    return out


def program(scale, s):
    # Existing packed ABI is transport only; original metadata is DEoption2,
    # one Double at PVar-16, with the loader's Double .5 at PVar-8.
    p = [0]*2048
    p[:15] = [1, 1, 6, 0, 0, 0, 2, s['max_iterations'], s['julia'], 0, 0, 256, 256, 0, 0]
    p[16], p[18], p[21], p[22] = s['rstop'], ONE, fp32(1 << 20), ONE
    p[24:28] = [s[f'original_j[{i}]'] for i in range(4)]
    for i in range(4):
        p[28+5*i] = ONE
    p[50] = s['hybrid_counts[0]']
    for slot in range(6):
        p[56+332*slot] = 1
    p[56:68] = [1, 1, 9452, 2, 1, 0, 1, 0, 0, 1, 0, 0]
    p[68] = scale
    for i, bits in enumerate((0x3f000000, scale)):
        normal_or_zero(bits)
        lo, hi = struct.unpack('<II', struct.pack('<d', struct.unpack('<f', struct.pack('<I', bits))[0]))
        at = 56+204+62-2*i
        p[at:at+2] = [lo, hi]
        p[56+268+i] = bits
        if i:
            p[132:134] = [lo, hi]
    return p


def evaluate(payload):
    if set(payload) != {'identity', 'kind', 'input_state_bits', 'parameter_bits'}:
        raise ValueError('wrong semantic input keys')
    if payload['identity'] != ['external', 9452, '_Scaling']:
        raise ValueError('wrong formula identity')
    if set(payload['input_state_bits']) != set(STATE) or len(payload['parameter_bits']) != 2048:
        raise ValueError('wrong state/program width')
    def decode(v):
        if not isinstance(v, str) or not re.fullmatch(r'0x[0-9a-f]{8}', v):
            raise ValueError('expected canonical hex32')
        return int(v, 16)
    s = {k: decode(v) for k, v in payload['input_state_bits'].items()}
    p = [decode(v) for v in payload['parameter_bits']]
    for lane in STATE[:33]:
        normal_or_zero(s[lane])
    if payload['kind'] not in ('point', 'orbit') or s['julia'] not in (0, 1) or s['inside'] or s['calc_smooth']:
        raise ScopeError('unsupported caller')
    if not 1 <= s['max_iterations'] <= 8 or not 1 <= s['hybrid_counts[0]'] <= 3:
        raise ScopeError('caller work bounds')
    scale = p[68]
    if p != program(scale, s):
        raise ValueError('prepared ABI or caller metadata differs')
    if payload['kind'] == 'orbit':
        if s['vary_scale'] != ONE or s['j[3]'] != s['original_j[3]']:
            raise ScopeError('entry state outside audited caller domain')
        out = orbit(s, scale)
    else:
        out = point(s, scale)
    for lane in STATE[:33]:
        normal_or_zero(out[lane])
    return {k: word(out[k]) for k in STATE}


def dependency_paths():
    return [Path(arithmetic.__file__).resolve()]


def manifest():
    def check(pins):
        out = {}
        for path, pin in pins.items():
            data = (ROOT/path).read_bytes()
            if hashlib.sha256(data.replace(b'\r\n', b'\n')).hexdigest() != pin:
                raise ValueError('pinned source changed: '+path)
            out[path] = hashlib.sha256(data).hexdigest()
        return out
    sources, abi = check(SOURCE_PINS), check(ABI_PINS)
    caller = check({CALLER_PROFILE[0]: CALLER_PROFILE[1]})
    text = (ROOT/'mb3d/M3Formulas/_Scaling.m3f').read_text()
    code = bytes.fromhex(re.search(r'\[CODE\](.*?)\[END\]', text, re.S)[1])
    if len(code) != 44 or hashlib.sha256(code).hexdigest() != CODE_SHA256:
        raise ValueError('original CODE changed')
    dep = dependency_paths()[0]
    dependencies = check({str(dep.relative_to(ROOT)): PRIMITIVE_SHA256})
    return {'schema': 'mb3d.native-group05-scaling-reference.v1',
            'semantic_sources': sources, 'abi_sources': abi, 'caller_profile': caller,
            'code_audit': {'_Scaling': {'bytes': 44, 'sha256': CODE_SHA256,
                'reads': ['x', 'y', 'z', 'w', 'Double PVar-16'],
                'writes': ['x', 'y', 'z', 'w'], 'operations': 'four signed scalar multiplications'}},
            'dependencies': dependencies,
            'arithmetic': 'Independent rational binary32 RN-even multiplication, explicit signed zero; exact all-subset orbit radius',
            'model_limits': ['All47 state words, _Scaling9452 only',
                'One positive-count analytic3D slot with DEoption2; count1..3, iterations1..8',
                'Double parameter exactly widened from finite normal binary32 or signed zero',
                'Nonfinite and subnormal input/output excluded; zero-w orbit result rejected',
                'SSE return transport requires exact rational sqrt and distance; no general distance scalar, smoothing, arbitrary hybrids, resources, x87 parity or scene credit'],
            'model_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}


def state():
    s = {k: fp32(Fraction(i+1, 32)) if i < 33 else i+11 for i,k in enumerate(STATE)}
    s.update(iteration=19, first=17, tmp=23, max_iterations=4, julia=0,
             inside=0, calc_smooth=0, current_slot=5)
    for i in range(6):
        s[f'hybrid_counts[{i}]'] = 1 if i == 0 else 0
    for i, v in enumerate((Fraction(3, 8), Fraction(-1, 2), Fraction(5, 8), Fraction(-3, 4))):
        for lane, factor in (('xyz', 1), ('c', 1), ('j', 2), ('original_j', -2)):
            s[f'{lane}[{i}]'] = fp32(v*factor)
    s['vary_scale'], s['rstop'] = ONE, fp32(1 << 16)
    s['j[3]'] = s['original_j[3]']
    return s


def build_cases():
    cases = []
    pins = manifest()['semantic_sources']
    def append(tag, s, scale, kind='point'):
        semantic = {'identity': ['external', 9452, '_Scaling'], 'kind': kind,
                    'input_state_bits': {k: word(s[k]) for k in STATE},
                    'parameter_bits': [word(v) for v in program(scale, s)]}
        reads = {f'xyz[{i}]' for i in range(4)}
        writes = reads | {'current_slot'}
        if kind == 'orbit':
            reads |= {f'{a}[{i}]' for a in ('c', 'original_j') for i in range(3)} | {'julia','max_iterations','rstop','hybrid_counts[0]'}
            writes |= {f'sse[{i}]' for i in range(4)} | {f'j[{i}]' for i in range(3)} | {'rout','rold','otrap','first','iteration','tmp'}
        cases.append({'id': '9452-'+tag, 'identity': semantic['identity'], 'variant': tag,
                      'input': semantic, 'expected_state_bits': evaluate(semantic),
                      'read_lanes': [k for k in STATE if k in reads],
                      'write_lanes': [k for k in STATE if k in writes], 'source_hashes': pins})
    scales = [fp32(v) for v in (-2, Fraction(-1,2), 0, Fraction(1,2), 1, 2)] + [NEG_ZERO, 0x3f800001]
    for i, scale in enumerate(scales):
        append(f'point-scale-{i}', state(), scale)
        s = state()
        for j in range(4): s[f'xyz[{j}]'] = NEG_ZERO if j%2 else 0
        append(f'point-zeros-{i}', s, scale)
    s = state()
    for j in range(4): s[f'xyz[{j}]'] = 0x3f800001+j
    append('point-rounded-products', s, 0x3f800001)
    s = state(); s.update(first=0x80000000,iteration=0x7fffffff,tmp=0xffffffff)
    append('point-integer-sentinels', s, fp32(-2))
    for i, scale in enumerate([fp32(v) for v in (-2,Fraction(-1,2),Fraction(1,2),1,2)]):
        for julia in (0,1):
            for count in (1,2,3):
                s = state(); s['julia']=julia;s['hybrid_counts[0]']=count
                for k,v in enumerate((3,4,12)):s[f'c[{k}]']=fp32(v)
                append(f'orbit-{i}-j{julia}-c{count}',s,scale,'orbit')
    for tag, stop in [('escape',Fraction(338)),('bailout-equality',Fraction(676))]:
        s=state();s['rstop']=fp32(stop)
        for k,v in enumerate((3,4,12)):s[f'c[{k}]']=fp32(v)
        append('orbit-'+tag,s,fp32(2),'orbit')
    return cases
