"""Original-source caller-unit diagnostics; deliberately NOT a collectable model.

Known memory fields are separate from unproved synthetic SSE transport. These
valid-ABI synthetic caller fixtures do not claim authored scene preparation or
HeaderTrafos selection of an analytic driver for an all-modifier scene. No
candidate formula/caller arithmetic or GPU observation supplies expectations.
"""
from fractions import Fraction
import hashlib
import json
from pathlib import Path

import native_formula_affine_v2_reference as affine

ROOT = Path(__file__).resolve().parents[2]
STATE = affine.STATE
SSE_FIELDS = tuple(f'sse[{i}]' for i in range(4))
MEMORY_FIELDS = tuple(k for k in STATE if k not in SSE_FIELDS)
PROFILES = {
    'numeric3d': (0, 0, 3, False, 3442),
    'analytic11': (1, 11, 3, True, 3697),
    'numeric4d': (2, 4, 4, False, 3540),
    'analytic4d5': (3, 5, 4, True, 3483),
    'analytic4d6': (3, 6, 4, True, 3483),
}
OPEN_PROFILES = ('interpolation', 'combined', 'difs', 'logarithmic_smoothing')
PINNED_PATHS = {
    **affine.SOURCE_PINS, **affine.ABI_PINS, **affine.DEPENDENCY_PINS,
    'comparison/tools/native_formula_affine_v2_reference.py': '807f8d041375412280e39a63c640305e41008fdfbe5b956cf8d22a540783b7f0',
    'mb3d/HeaderTrafos.pas': '52b3209608ba3360f4be7d821866c37a1845bc09f9aa2ec56d1912616f5d7f9d',
    'mb3d/Calc.pas': 'eb290ad9001a80ad185df553401ec55bc1bf662141ee3f7169b78dff6714c419',
    'src/render/native_metal/generated/inventory.json': '8bb63770c5c30c3960f80d21fbd9bd6729368cb03fc0e15da369066b0ec89034',
}


class ScopeError(ValueError):
    """Unsupported ordinary domains remain explicit requirements."""


def _finite(word):
    return affine.fraction(word)


def _add(a, b):
    return affine.arithmetic.add(a, b)


def _mul(a, b):
    return affine.arithmetic.multiply(a, b)


def exact_radius(s, dimension):
    squares = []
    for i in range(dimension):
        x = s[f'xyz[{i}]']; q = _mul(x, x)
        if _finite(q) != _finite(x)**2:
            raise ScopeError('rounded radius product remains OPEN')
        squares.append(q)
    # Every subset sum must be exact: no reduction/contraction assumption.
    from itertools import combinations
    for length in range(2, dimension+1):
        for subset in combinations(squares, length):
            value = sum(map(_finite, subset), Fraction())
            if _finite(affine.fp32(value)) != value:
                raise ScopeError('rounded radius reduction remains OPEN')
    return affine.fp32(sum(map(_finite, squares), Fraction()))


def smooth_low_branch(rout, iterations):
    """Actual x87 source compares Double's high DWORD, not exact Rout<=1.

    formulas.pas3396..3401 uses signed JG for the nonnegative finite radius.
    Logarithmic branches and their final register transport are not modelled.
    """
    if _finite(rout) < 0 or not 0 <= iterations <= 8:
        raise ScopeError('invalid bounded smoothing input')
    high = affine.binary64(_finite(rout)) >> 32
    if high > 0x3ff00000:
        raise ScopeError('logarithmic smoothing remains OPEN')
    return affine.fp32(iterations)


def program(profile, s, parameters):
    if profile not in PROFILES:
        raise ScopeError('caller profile remains OPEN')
    p = affine.program(parameters, s)
    p[1], p[6] = PROFILES[profile][:2]
    return p


def validate(case):
    if set(case) != {'profile', 'input'} or case['profile'] not in PROFILES:
        raise ScopeError('explicit supported caller-unit profile required')
    payload = case['input']
    if set(payload) != {'identity', 'kind', 'input_state_bits', 'parameter_bits'} or payload['identity'] != affine.IDENTITY or payload['kind'] != 'orbit':
        raise ScopeError('Translate orbit input required')
    def decode(x):
        if not isinstance(x, str) or not affine.re.fullmatch(r'0x[0-9a-f]{8}', x):
            raise ScopeError('canonical hex32 required')
        return int(x, 16)
    s = {k: decode(v) for k,v in payload['input_state_bits'].items()}
    affine._state(s)
    p = [decode(v) for v in payload['parameter_bits']]
    params = affine.prepared_translate_parameters(p)
    if p != program(case['profile'], s, params):
        raise ScopeError('noncanonical caller-unit program or typed aliases')
    if not 1 <= s['max_iterations'] <= 8 or not 1 <= s['hybrid_counts[0]'] <= 3 or any(s[f'hybrid_counts[{i}]'] for i in range(1,6)):
        raise ScopeError('bounded positive single-slot schedule required')
    if s['julia'] not in (0,1) or s['calc_smooth'] not in (0,1) or s['inside'] or _finite(s['rstop']) <= 0:
        raise ScopeError('bounded exterior caller scalar domain required')
    return s, p, params


def evaluate_source(case):
    """Return only43 source memory words; no evaluate/build_cases adapter."""
    s, p, params = validate(case)
    dimension, analytic = PROFILES[case['profile']][2:4]
    out = dict(s)
    if dimension == 4:
        # Bounded identity matrix still executes all source products/adds;
        # it is not a coordinate copy and cannot erase signed-zero semantics.
        for row in range(4):
            products = [_mul(s[f'c[{col}]'], p[28+row*4+col]) for col in range(3)]
            out[f'xyz[{row}]'] = _add(_add(products[0],products[1]),products[2])
        for i in range(4):
            out[f'j[{i}]'] = s[f'original_j[{i}]'] if s['julia'] else out[f'xyz[{i}]']
        if analytic:
            out['derivative[0]'] = affine.ONE
    else:
        for i in range(3):
            out[f'xyz[{i}]'] = s[f'c[{i}]']
            out[f'j[{i}]'] = s[f'original_j[{i}]'] if s['julia'] else s[f'c[{i}]']
        out['xyz[3]'] = affine.ONE if analytic else 0
    out['first'] = out['iteration'] = 0
    out['rout'] = out['otrap'] = exact_radius(out,dimension)
    remaining = count = s['hybrid_counts[0]']; out['tmp'] = count
    trace = []
    for _ in range(s['max_iterations']):
        out['rold'] = out['rout']
        if remaining == 0:
            remaining = count
        out = affine.translate_step(out, params)
        remaining -= 1;out['tmp'] = remaining;out['iteration'] += 1
        out['rout'] = exact_radius(out,dimension)
        if _finite(out['rout']) < _finite(out['otrap']):
            out['otrap'] = out['rout']
        trace.append({k:affine.word(out[k]) for k in MEMORY_FIELDS})
        if _finite(out['rout']) > _finite(s['rstop']):
            break
    if s['calc_smooth']:
        out['smooth'] = smooth_low_branch(out['rout'],out['iteration'])
    return {
        'schema':'mb3d.translate-caller-source-partial.v1',
        'status':'PARTIAL_SOURCE_MODEL_NOT_ADMISSIBLE',
        'qualification':False,
        'known_fields':list(MEMORY_FIELDS), 'unproved_fields':list(SSE_FIELDS),
        'expected_memory_bits':{k:affine.word(out[k]) for k in MEMORY_FIELDS},
        'expected_work':out['iteration'], 'trace':trace,
        'compiled_transport_required':True,
        'limitations':['Synthetic caller-unit fixture, not authored scene selection/preparation',
                      'Positive Translate-only counts; actual negative neighbour schedules remain separate',
                      'Identity4D matrix only; no authored angle-preparation claim',
                      'Exact radius domain; logarithmic smoothing, nonfinite/range and distance scalar OPEN',
                      'No synthetic SSE expectation; cannot satisfy47-word admission'],
    }


def build_source_cases():
    cases=[]
    def append(profile,tag,s,values):
        params=[affine.binary64(v) for v in values]
        case={'profile':profile,'input':{'identity':affine.IDENTITY,'kind':'orbit',
              'input_state_bits':{k:affine.word(s[k]) for k in STATE},
              'parameter_bits':list(map(affine.word,program(profile,s,params)))}}
        cases.append({'id':f'translate-caller-{profile}-{tag}','source_case':case,
                      'expected':evaluate_source(case)})
    for profile in PROFILES:
        for julia in (0,1):
            for count in (1,3):
                for limit in (1,4):
                    s=affine.state();s['rstop']=affine.fp32(1024);s['julia']=julia;s['hybrid_counts[0]']=count;s['max_iterations']=limit
                    for i,v in enumerate((3,4,0)):s[f'c[{i}]']=affine.fp32(v)
                    append(profile,f'j{julia}-c{count}-n{limit}',s,(1,0,0))
            s=affine.state();s['rstop']=affine.fp32(16);s['julia']=julia;s['max_iterations']=2;s['calc_smooth']=1
            for i in range(3):s[f'c[{i}]']=affine.fp32(Fraction(1,4))
            append(profile,f'smooth-low-j{julia}',s,(0,0,0))
        s=affine.state();s['rstop']=affine.fp32(25);s['max_iterations']=2
        for i,v in enumerate((3,4,0)):s[f'c[{i}]']=affine.fp32(v)
        append(profile,'bailout-equality',s,(0,0,0))
        s=affine.state();s['rstop']=affine.fp32(16);s['max_iterations']=2
        for i in range(3):s[f'c[{i}]']=affine.NEG_ZERO
        append(profile,'signed-zero',s,(0,0,0))
    return cases


def manifest():
    for name,expected in PINNED_PATHS.items():
        if hashlib.sha256((ROOT/name).read_bytes()).hexdigest()!=expected:
            raise ScopeError('source/helper pin changed: '+name)
    # Verify descriptor against actual metadata, independent of formula body.
    text=(ROOT/'src/render/native_metal/generated/formula_metadata.h').read_text()
    if 'case 8065u: slot.traits=0u; slot.wrapper_flags=1u; slot.difs_style=0u;' not in text:
        raise ScopeError('Translate metadata changed')
    rows=json.loads((ROOT/'src/render/native_metal/generated/inventory.json').read_text())['formulas']
    row=next(r for r in rows if r['id']==8065)
    if (row['de_option'],row['param_count'],row['traits'],row['adds_c'],row['difs_style'])!=(-1,3,0,1,0):
        raise ScopeError('Translate descriptor identity changed')
    return {'schema':'mb3d.translate-caller-source-manifest.v1','qualification':False,
            'model_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
            'pins':PINNED_PATHS,'profiles':{k:list(v) for k,v in PROFILES.items()},
            'known_fields':list(MEMORY_FIELDS),'unproved_fields':list(SSE_FIELDS),
            'open_profiles':list(OPEN_PROFILES),'collector_compatible':False}
