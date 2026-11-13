"""Bounded source-derived full47 caller projection; no device/admission adapter.

Original controls define inputs. Original compiler instructions establish XMM
transport operations, never numerical target tables. Only exact elementary
caller arithmetic is admitted here, apart from a separately proved SQRTSD to
RN32 projection. Unmodelled controls remain present with an OPEN disposition.
"""
from fractions import Fraction
import hashlib
from itertools import combinations
import json
from pathlib import Path
import struct
import sys

ROOT = next(p for p in Path(__file__).resolve().parents if (p / 'AGENTS.md').is_file())
sys.path.insert(0, str(ROOT / 'comparison/tools'))
import native_formula_affine_v2_reference as affine
import build_original_translate_callers_oracle as original

STATE = affine.STATE
SCHEMA = 'mb3d.translate-callers-source-control.v2'
PREPARED = 'comparison/runs/formula-translate-t009-20261104/original-callers-prepared-v2/'
PROOF_BUNDLE = 'comparison/results/formula-campaign-004/group01/8065/t009/caller-oracle/model-v2/compiled-proof-closure.json'
PINS = {
    **original.SOURCE_PINS, **affine.ABI_PINS, **affine.DEPENDENCY_PINS,
    'comparison/tools/native_formula_affine_v2_reference.py': '807f8d041375412280e39a63c640305e41008fdfbe5b956cf8d22a540783b7f0',
    'comparison/tools/build_original_translate_callers_oracle.py': '45572a1a13a12756925f1176a4da84b9bd6cf55bda73374624312bcef71e473b',
    PREPARED+'controls.json': '6fb532b614e4d8480a2cad17cbf7ba9c460909a10ba6a056a9c457f703212e9c',
    PREPARED+'preparation.json': '2071cd651a1e51f75885ac9689f8f07ea179ab08e31f1f39b2738f19235793e3',
    PREPARED+'original_translate_callers_oracle.pas': '975b272ac4f23a8c079e4f9bdaa6355081885a1d80207294405b8560b5202cc6',
    PREPARED+'compile/original_translate_callers_oracle.s': 'e254180f26e749805a1da3005efbbc53b05fadd158242d683fdd865effebac1c',
    PROOF_BUNDLE: '41478095eb516965171d20c7bc116e356309ad8c2bb55e372e3f6c358ba4b19f',
}
# Packed immutable TIteration3Dext fields. c[3]/derivative[3]/SSE/current_slot
# are adapter fields, not extra bytes invented in the original record.
DOUBLES = {**{f'xyz[{i}]':24+8*i for i in range(4)},
           **{f'c[{i}]':56+8*i for i in range(3)},
           **{f'j[{i}]':80+8*i for i in range(3)}, 'j[3]':0,
           **{f'original_j[{i}]':376+8*i for i in range(4)},
           **{f'derivative[{i}]':288+8*i for i in range(3)},
           'rout':112,'rold':8,'rstop_d':16,'otrap':248,'vary_scale':256,
           'dfree1':272,'dfree2':280}
SINGLES = {'smooth':108,'rstop':128}
INTEGERS = {'iteration':120,'max_iterations':124,'first':264,'tmp':268,
            **{f'hybrid_counts[{i}]':132+4*i for i in range(6)}}


class ScopeError(ValueError):
    """No full47 numerical claim outside the explicitly proved source domain."""


def canonical(value):
    return json.dumps(value, sort_keys=True, separators=(',', ':'))


def exact_add(a, b):
    result = affine.arithmetic.add(a, b)
    if affine.fraction(result) != affine.fraction(a)+affine.fraction(b):
        raise ScopeError('rounded elementary caller sum remains OPEN')
    return result


def exact_mul(a, b):
    result = affine.arithmetic.multiply(a, b)
    if affine.fraction(result) != affine.fraction(a)*affine.fraction(b):
        raise ScopeError('rounded elementary caller product remains OPEN')
    return result


def radius(values):
    squares = [exact_mul(v, v) for v in values]
    for size in range(2,len(squares)+1):
        for subset in combinations(squares,size):
            value = sum(map(affine.fraction,subset),Fraction())
            if affine.fraction(affine.fp32(value)) != value:
                raise ScopeError('rounded radius reduction remains OPEN')
    result = squares[0]
    for q in squares[1:]: result = exact_add(result,q)
    return result


def exact_sqrt(value):
    result = affine.sqrt32(value)
    if affine.fraction(result)**2 != affine.fraction(value):
        raise ScopeError('rounded interpolation square root remains OPEN')
    return result


def low_smooth(rout, iteration):
    if affine.fraction(rout) < 0 or affine.binary64(affine.fraction(rout)) >> 32 > 0x3ff00000:
        raise ScopeError('logarithmic smoothing remains OPEN')
    return affine.fp32(iteration)


def projected_state(raw, *, sse=(0,0,0,0), slot=0):
    """Project source-authored/observed bytes; the model uses source input only."""
    if len(raw) != 456: raise ValueError('full packed456 state required')
    out = dict.fromkeys(STATE,0)
    for name,at in DOUBLES.items(): out[name] = affine.project_double(struct.unpack_from('<Q',raw,at)[0])
    for name,at in {**SINGLES,**INTEGERS}.items(): out[name] = struct.unpack_from('<I',raw,at)[0]
    out['julia'] = int(struct.unpack_from('<i',raw,208)[0] != 0)
    out['inside'] = int(struct.unpack_from('<i',raw,440)[0] != 0)
    out['calc_smooth'] = raw[204]
    # Explicit supplemental Native input values; preservation is an adapter
    # obligation, not a false claim that original has these memory fields.
    out['c[3]'] = affine.fp32(Fraction(3,8))
    out['derivative[3]'] = affine.fp32(Fraction(-5,8))
    for i,value in enumerate(sse): out[f'sse[{i}]'] = value
    out['current_slot'] = slot
    affine._state(out)
    return out


def validate(control):
    if not isinstance(control,dict): raise ScopeError('explicit original control required')
    declared = next((r for r in original.controls() if r['id']==control.get('id')),None)
    if declared is None or canonical(control) != canonical(declared):
        raise ScopeError('undeclared/altered caller control remains OPEN')
    return projected_state(original.state_bytes(control))


def evaluate_source(control):
    s = validate(control); out = dict(s)
    dimension, analytic, interpol = control['dimensions'], control['analytic'], control['interpolation']
    raw = original.state_bytes(control)
    if dimension == 4:
        matrix = struct.unpack_from('<16I',raw,312)
        for row in range(4):
            terms = [exact_mul(s[f'c[{i}]'],matrix[row*4+i]) for i in range(3)]
            out[f'xyz[{row}]'] = exact_add(exact_add(terms[0],terms[1]),terms[2])
    else:
        for i in range(3): out[f'xyz[{i}]'] = s[f'c[{i}]']
        out['xyz[3]'] = affine.ONE if analytic else 0
    for i in range(3 if dimension==3 else 4):
        out[f'j[{i}]'] = s[f'original_j[{i}]'] if control['julia'] else out[f'xyz[{i}]']
    if dimension==4 and analytic: out['derivative[0]'] = affine.ONE
    out['first'] = out['iteration'] = 0
    out['rout'] = out['otrap'] = radius([out[f'xyz[{i}]'] for i in range(dimension)])
    parameters = [[struct.unpack('<Q',struct.pack('<d',v))[0] for v in delta] for delta in control['deltas']]
    trace, work, slot = [], 0, 0
    if not interpol: out['tmp'] = s['hybrid_counts[0]'] & 0x7fffffff

    def callback(state, selected):
        nonlocal work
        trace.append({'slot':selected,'iteration':state['iteration'],'tmp':state['tmp']})
        work += 1
        result = affine.translate_step(state,parameters[selected],selected)
        for i,bits in enumerate(parameters[selected]):
            exact_add(state[f'xyz[{i}]'],affine.project_double(bits))
        return result

    while True:
        out['rold'] = out['rout']
        if interpol:
            before = dict(out)
            left = callback(out,0)
            # Source copies only XYZW and (for analytic4D) saves/restores Deriv1.
            out = dict(left)
            for i in range(4): out[f'xyz[{i}]'] = before[f'xyz[{i}]']
            out = callback(out,1); slot = 1
            weights = [struct.unpack('<I',struct.pack('<f',v))[0] for v in control['weights']]
            lengths = [exact_sqrt(radius([v[f'xyz[{i}]'] for i in range(dimension)])) for v in (left,out)]
            length = exact_add(exact_mul(lengths[0],weights[0]),exact_mul(lengths[1],weights[1]))
            for i in range(4):
                a,b = left[f'xyz[{i}]'],out[f'xyz[{i}]']
                if analytic and dimension==3 and i==3: a,b=a&0x7fffffff,b&0x7fffffff
                out[f'xyz[{i}]'] = exact_add(exact_mul(a,weights[0]),exact_mul(b,weights[1]))
            if analytic and dimension==4:
                out['derivative[0]'] = exact_add(exact_mul(left['derivative[0]']&0x7fffffff,weights[0]),exact_mul(out['derivative[0]']&0x7fffffff,weights[1]))
            blend = [out[f'xyz[{i}]'] for i in range(dimension)]
            blend_radius = radius(blend)
            # PE numeric interpolation preserves this pre-normalization partial
            # reduction in XMM1, rather than the eventual Rout=XX*XX.
            numeric_transport = radius(blend[:3 if dimension==3 else 2])
            if affine.fraction(blend_radius)==0:
                if affine.fraction(length)!=0: raise ScopeError('nonzero length over zero blend remains OPEN')
                scale = 0  # 0/sqrt(positive original 1e-40), in either width.
            else:
                # For this bounded subset 1e-40 is below half an Extended ulp;
                # the source x87 add/fsqrt/fdiv therefore has exact ratio1.
                if Fraction(1,10**40) >= affine.fraction(blend_radius)*Fraction(1,1<<65):
                    raise ScopeError('interpolation epsilon rounding remains OPEN')
                denominator = exact_sqrt(blend_radius)
                if length != denominator: raise ScopeError('non-unit interpolation normalization remains OPEN')
                scale = affine.ONE
            for i in range(dimension): out[f'xyz[{i}]'] = exact_mul(out[f'xyz[{i}]'],scale)
            out['iteration'] += 1
            out['rout'] = exact_mul(length,length)
        else:
            if out['tmp']==0:
                slot = (slot+1)%control['active_slots']
                out['tmp'] = s[f'hybrid_counts[{slot}]'] & 0x7fffffff
            out = callback(out,slot);out['tmp'] -= 1
            if s[f'hybrid_counts[{slot}]'] < 0x80000000:
                out['iteration'] += 1
                xyz = [out[f'xyz[{i}]'] for i in range(dimension)]
                out['rout'] = radius(xyz)
                numeric_transport = radius(xyz[:3])
        if affine.fraction(out['rout']) < affine.fraction(out['otrap']): out['otrap'] = out['rout']
        if out['iteration'] >= s['max_iterations'] or affine.fraction(out['rout']) > affine.fraction(s['rstop']): break
        if work>=8: raise ScopeError('bounded caller work exceeded')
    if control['smooth']: out['smooth'] = low_smooth(out['rout'],out['iteration'])
    if analytic:
        denominator = out['xyz[3]' if dimension==3 else 'derivative[0]'] & 0x7fffffff
        if denominator!=affine.ONE: raise ScopeError('non-unit analytic divisor remains OPEN')
        transport = [affine.sqrt_transport(out['rout']),0,denominator,0]
    else:
        transport = [out['rout'] if out['iteration']>=s['max_iterations'] else s['rstop'],0,numeric_transport,0]
    for i,value in enumerate(transport): out[f'sse[{i}]'] = value
    return {'schema':'mb3d.translate-caller-source-projection.v2','status':'SOURCE_FULL47_PENDING_INDEPENDENT_REVIEW',
            'qualification':False,'expected_state_bits':{k:affine.word(out[k]) for k in STATE},
            'expected_work':work,'trace':trace,'unproved_fields':[],
            'profile':'source_exact_elementary_and_RN32_return_projection',
            'original_pascal_parity':False,'collector_compatible':False}


def build_source_cases():
    result=[]
    for row in original.controls():
        try: expected=evaluate_source(row)
        except ScopeError as error:
            expected={'status':'OPEN_MODEL_DOMAIN','reason':str(error),'qualification':False}
        result.append({'id':row['id'],'schema':SCHEMA,'control':row,
                       'initial_state_bits':{k:affine.word(v) for k,v in validate(row).items()},'expected':expected})
    return result


def manifest():
    for path,expected in PINS.items():
        if hashlib.sha256((ROOT/path).read_bytes()).hexdigest()!=expected:
            raise ScopeError('source/proof pin changed: '+path)
    if original.controls()!=json.loads((ROOT/PREPARED/'controls.json').read_text()):
        raise ScopeError('original control inventory changed')
    proof=json.loads((ROOT/PROOF_BUNDLE).read_text())
    for path,expected in proof['pins'].items():
        if hashlib.sha256((ROOT/path).read_bytes()).hexdigest()!=expected:
            raise ScopeError('original compiled-proof dependency changed: '+path)
    review=json.loads((ROOT/proof['independent_review']['path']).read_text())
    if review['status']!='PASS_SCOPED_ORIGINAL_COMPILE_AND_OBSERVATION' or review['qualification'] is not False:
        raise ScopeError('independent original operations/transport proof missing')
    return {'schema':'mb3d.translate-callers-source-manifest.v2','qualification':False,
            'collector_compatible':False,'model_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
            'pins':PINS,'data_dependencies':proof['pins'],'actual_pe_review':proof['independent_review'],
            'open_domains':['logarithmic smoothing','rounded interpolation roots/normalization',
                            'arbitrary inputs/matrices/caller selection','combined','dIFS/noVecIni','nonfinite/range'],
            'adapter_fields':{'c[3]':'explicit input3/8 preserved','derivative[3]':'explicit input-5/8 preserved',
                              'sse':'source-compiled final register transport','current_slot':'last callback position'}}
