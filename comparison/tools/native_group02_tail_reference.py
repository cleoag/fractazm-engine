"""Independent bounded Native FP32 targets for the final five group02 formulas.

Original M3F CODE and Pascal typed memory/caller contracts are authoritative.
Ideal RN32 transcendental targets come from rational interval proofs, not host
libm, translated formulas or GPU observations. Metal sin/cos/atan2 are not
universally correctly rounded: equality is a device obligation for this fixed
corpus, and a future one-ULP mismatch alone is not proof of a Metal defect.
Ambiguous arithmetic/contraction/interval inputs fail closed, never PASS.
"""
from __future__ import annotations

from fractions import Fraction
from functools import lru_cache
import hashlib
from itertools import permutations
from pathlib import Path
import re
import struct

import native_formula_reference as affine
from gpu_pipeline_proofs import STATE

ROOT = Path(__file__).resolve().parents[2]
SOURCE_PINS = {'mb3d/DivUtils.pas': '879221cb1d6abb371452f1f04afcf5315af238317dcb31b6359a145cb303da96',
 'mb3d/CustomFormulas.pas': '9ed1c8289ebfb99202795af8daca6bef92650ba250a28c12435eb5beb4264c61',
 'mb3d/Math3D.pas': '0b03d5d982be1285839831dfc36b2532725f5d679e2891b474d00d750b7b6727',
 'mb3d/TypeDefinitions.pas': '5ef479844d3d5ffac309b141c9f80b62ebf0f9bf8b8782b706d9f451ef096747',
 'mb3d/formulas.pas': '3fefcd04fe6726831f3a7904f151cc3cc192610f4532f2889b9771e5953775bd',
 'mb3d/M3Formulas/_invcylindrical.m3f': 'b4eccf41f6bf6e3aa582fb9f03230bec83b2a8775798b396f781739c01534b94',
 'mb3d/M3Formulas/_SinY.m3f': '10c0feae136b76c8620fb9c3eea1b43dd56bba0b0e13b6eee53b4dffe128d5ed',
 'mb3d/M3Formulas/_PolyFolding.m3f': '096ecfe77cd89a8177a1f0f2621660492e83bb28d5ee14cd71a85ca7d5583840',
 'mb3d/M3Formulas/_PolyFold-sym.m3f': 'b3b0469b9d299990f612eeaefdb23b39b1a5999ad52a11ef250d4b24e3e260c6',
 'mb3d/M3Formulas/_ngon.m3f': '4a05415fc034e58510978c8b751a198e2bcc0deab81b6ed2de57be1132c262f9'}
ABI_PINS = {'src/render/native_metal/formula_abi.h': 'd69725ad200b889c8e7e2c3267d8fee96e37188b54e5cdc145a6cde3afb6350a',
 'src/core/external_formulas.h': 'b36cba7e6b37c6426a8d052f2434866397f6edc7edb8d799efd0ffccc751f3b6'}
CODE_AUDIT = {'_invcylindrical': {'sha256': 'b385474c471abd9dbb408d2cc71e625994e67b733655d9b8a27415c1958023c9',
                     'bytes': 21,
                     'key_offsets': [{'offset': '0x0005', 'bytes': 'd9fb', 'instruction': 'fsincos '},
                                     {'offset': '0x0009',
                                      'bytes': 'dcca',
                                      'instruction': 'fmul st(2), st(0)'},
                                     {'offset': '0x000b', 'bytes': 'dec9', 'instruction': 'fmulp st(1)'},
                                     {'offset': '0x000d',
                                      'bytes': 'dd18',
                                      'instruction': 'fstp qword ptr [eax]'},
                                     {'offset': '0x000f',
                                      'bytes': 'dd1a',
                                      'instruction': 'fstp qword ptr [edx]'}]},
 '_SinY': {'sha256': 'ee59910528f3a8c7874d4a8f98b208555f0db053e1be60d463f525470057216f',
           'bytes': 43,
           'key_offsets': [{'offset': '0x000e', 'bytes': '83e603', 'instruction': 'and esi, 3'},
                           {'offset': '0x0014',
                            'bytes': 'd867f0',
                            'instruction': 'fsub dword ptr [edi - 0x10]'},
                           {'offset': '0x0017',
                            'bytes': 'd84fec',
                            'instruction': 'fmul dword ptr [edi - 0x14]'},
                           {'offset': '0x001a', 'bytes': 'd9fe', 'instruction': 'fsin '},
                           {'offset': '0x001c',
                            'bytes': 'd84fe8',
                            'instruction': 'fmul dword ptr [edi - 0x18]'},
                           {'offset': '0x001f',
                            'bytes': 'd847e4',
                            'instruction': 'fadd dword ptr [edi - 0x1c]'},
                           {'offset': '0x0022',
                            'bytes': 'dd1cf0',
                            'instruction': 'fstp qword ptr [eax + esi*8]'}]},
 '_PolyFolding': {'sha256': '87d5d9181884baf5cd93f603a438185d6fa5b374b95d2cf6b77c944cf69249f8',
                  'bytes': 102,
                  'key_offsets': [{'offset': '0x0010',
                                   'bytes': 'dc47d8',
                                   'instruction': 'fadd qword ptr [edi - 0x28]'},
                                  {'offset': '0x0015',
                                   'bytes': 'dc47e0',
                                   'instruction': 'fadd qword ptr [edi - 0x20]'},
                                  {'offset': '0x0018', 'bytes': 'd9f3', 'instruction': 'fpatan '},
                                  {'offset': '0x001d',
                                   'bytes': 'dc47e8',
                                   'instruction': 'fadd qword ptr [edi - 0x18]'},
                                  {'offset': '0x0020',
                                   'bytes': 'dc4f08',
                                   'instruction': 'fmul qword ptr [edi + 8]'},
                                  {'offset': '0x0023', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                                  {'offset': '0x0025',
                                   'bytes': 'dc37',
                                   'instruction': 'fdiv qword ptr [edi]'},
                                  {'offset': '0x0027',
                                   'bytes': 'dc4ff0',
                                   'instruction': 'fmul qword ptr [edi - 0x10]'},
                                  {'offset': '0x002a', 'bytes': 'd9fc', 'instruction': 'frndint '},
                                  {'offset': '0x002c', 'bytes': 'd9e0', 'instruction': 'fchs '},
                                  {'offset': '0x002e',
                                   'bytes': 'dc77f0',
                                   'instruction': 'fdiv qword ptr [edi - 0x10]'},
                                  {'offset': '0x0031',
                                   'bytes': 'dc0f',
                                   'instruction': 'fmul qword ptr [edi]'},
                                  {'offset': '0x0036',
                                   'bytes': 'dc4f08',
                                   'instruction': 'fmul qword ptr [edi + 8]'},
                                  {'offset': '0x0039', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                                  {'offset': '0x003b', 'bytes': 'd9fb', 'instruction': 'fsincos '},
                                  {'offset': '0x0041', 'bytes': 'd8c9', 'instruction': 'fmul st(1)'},
                                  {'offset': '0x0045', 'bytes': 'd8cb', 'instruction': 'fmul st(3)'},
                                  {'offset': '0x0047', 'bytes': 'dee1', 'instruction': 'fsubrp st(1)'},
                                  {'offset': '0x004d', 'bytes': 'd8cb', 'instruction': 'fmul st(3)'},
                                  {'offset': '0x0051', 'bytes': 'd8cb', 'instruction': 'fmul st(3)'},
                                  {'offset': '0x0053', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                                  {'offset': '0x0057',
                                   'bytes': 'dd1a',
                                   'instruction': 'fstp qword ptr [edx]'},
                                  {'offset': '0x0059',
                                   'bytes': 'dd1b',
                                   'instruction': 'fstp qword ptr [ebx]'},
                                  {'offset': '0x005b', 'bytes': 'ddd8', 'instruction': 'fstp st(0)'},
                                  {'offset': '0x005d', 'bytes': 'ddd8', 'instruction': 'fstp st(0)'}]},
 '_PolyFold-sym': {'sha256': 'b56901c7627f7073d138fc74d1bc58d2aef16be7155f1714be913636a906e17e',
                   'bytes': 131,
                   'key_offsets': [{'offset': '0x0010',
                                    'bytes': 'dc47d8',
                                    'instruction': 'fadd qword ptr [edi - 0x28]'},
                                   {'offset': '0x0013',
                                    'bytes': 'dd12',
                                    'instruction': 'fst qword ptr [edx]'},
                                   {'offset': '0x0017',
                                    'bytes': 'dc47e0',
                                    'instruction': 'fadd qword ptr [edi - 0x20]'},
                                   {'offset': '0x001a',
                                    'bytes': 'dd13',
                                    'instruction': 'fst qword ptr [ebx]'},
                                   {'offset': '0x001c', 'bytes': 'd9f3', 'instruction': 'fpatan '},
                                   {'offset': '0x0021',
                                    'bytes': 'dc47e8',
                                    'instruction': 'fadd qword ptr [edi - 0x18]'},
                                   {'offset': '0x0024',
                                    'bytes': 'dc4f08',
                                    'instruction': 'fmul qword ptr [edi + 8]'},
                                   {'offset': '0x0027', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                                   {'offset': '0x0029',
                                    'bytes': 'dc37',
                                    'instruction': 'fdiv qword ptr [edi]'},
                                   {'offset': '0x002b',
                                    'bytes': 'dc4ff0',
                                    'instruction': 'fmul qword ptr [edi - 0x10]'},
                                   {'offset': '0x0032',
                                    'bytes': 'db1c24',
                                    'instruction': 'fistp dword ptr [esp]'},
                                   {'offset': '0x0035',
                                    'bytes': 'db0424',
                                    'instruction': 'fild dword ptr [esp]'},
                                   {'offset': '0x0038', 'bytes': 'd9e0', 'instruction': 'fchs '},
                                   {'offset': '0x003a',
                                    'bytes': 'dc77f0',
                                    'instruction': 'fdiv qword ptr [edi - 0x10]'},
                                   {'offset': '0x003d',
                                    'bytes': 'dc0f',
                                    'instruction': 'fmul qword ptr [edi]'},
                                   {'offset': '0x0042',
                                    'bytes': 'dc4f08',
                                    'instruction': 'fmul qword ptr [edi + 8]'},
                                   {'offset': '0x0045', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                                   {'offset': '0x0047', 'bytes': 'd9fb', 'instruction': 'fsincos '},
                                   {'offset': '0x004d', 'bytes': 'd8c9', 'instruction': 'fmul st(1)'},
                                   {'offset': '0x0051', 'bytes': 'd8cb', 'instruction': 'fmul st(3)'},
                                   {'offset': '0x0053', 'bytes': 'dee1', 'instruction': 'fsubrp st(1)'},
                                   {'offset': '0x0059', 'bytes': 'd8cb', 'instruction': 'fmul st(3)'},
                                   {'offset': '0x005d', 'bytes': 'd8cb', 'instruction': 'fmul st(3)'},
                                   {'offset': '0x005f', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                                   {'offset': '0x0064', 'bytes': '83e001', 'instruction': 'and eax, 1'},
                                   {'offset': '0x0069', 'bytes': 'd9e0', 'instruction': 'fchs '},
                                   {'offset': '0x006e',
                                    'bytes': 'dc67d8',
                                    'instruction': 'fsub qword ptr [edi - 0x28]'},
                                   {'offset': '0x0071',
                                    'bytes': 'dd1a',
                                    'instruction': 'fstp qword ptr [edx]'},
                                   {'offset': '0x0073',
                                    'bytes': 'dc67e0',
                                    'instruction': 'fsub qword ptr [edi - 0x20]'},
                                   {'offset': '0x0076',
                                    'bytes': 'dd1b',
                                    'instruction': 'fstp qword ptr [ebx]'},
                                   {'offset': '0x0078', 'bytes': 'ddd8', 'instruction': 'fstp st(0)'},
                                   {'offset': '0x007a', 'bytes': 'ddd8', 'instruction': 'fstp st(0)'}]},
 '_ngon': {'sha256': '0541eb385832a4eb6f6c04b1774bfc11f49251ca01e7bc21e6b412f47b0f2b15',
           'bytes': 151,
           'key_offsets': [{'offset': '0x0013',
                            'bytes': 'dc47d8',
                            'instruction': 'fadd qword ptr [edi - 0x28]'},
                           {'offset': '0x0016', 'bytes': 'dd12', 'instruction': 'fst qword ptr [edx]'},
                           {'offset': '0x001a',
                            'bytes': 'dc47e0',
                            'instruction': 'fadd qword ptr [edi - 0x20]'},
                           {'offset': '0x001d', 'bytes': 'dd13', 'instruction': 'fst qword ptr [ebx]'},
                           {'offset': '0x001f', 'bytes': 'd9f3', 'instruction': 'fpatan '},
                           {'offset': '0x0024',
                            'bytes': 'dc47e8',
                            'instruction': 'fadd qword ptr [edi - 0x18]'},
                           {'offset': '0x0027', 'bytes': 'dc4f08', 'instruction': 'fmul qword ptr [edi + 8]'},
                           {'offset': '0x002a', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                           {'offset': '0x002c', 'bytes': 'dc37', 'instruction': 'fdiv qword ptr [edi]'},
                           {'offset': '0x002e',
                            'bytes': 'dc4ff0',
                            'instruction': 'fmul qword ptr [edi - 0x10]'},
                           {'offset': '0x0031',
                            'bytes': 'dd54240c',
                            'instruction': 'fst qword ptr [esp + 0xc]'},
                           {'offset': '0x0037', 'bytes': 'db1c24', 'instruction': 'fistp dword ptr [esp]'},
                           {'offset': '0x003a', 'bytes': 'db0424', 'instruction': 'fild dword ptr [esp]'},
                           {'offset': '0x003d', 'bytes': 'd9e0', 'instruction': 'fchs '},
                           {'offset': '0x003f',
                            'bytes': 'dc77f0',
                            'instruction': 'fdiv qword ptr [edi - 0x10]'},
                           {'offset': '0x0042', 'bytes': 'dc0f', 'instruction': 'fmul qword ptr [edi]'},
                           {'offset': '0x0047', 'bytes': 'dc4f08', 'instruction': 'fmul qword ptr [edi + 8]'},
                           {'offset': '0x004a', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                           {'offset': '0x004c', 'bytes': 'd9fb', 'instruction': 'fsincos '},
                           {'offset': '0x0052', 'bytes': 'd8c9', 'instruction': 'fmul st(1)'},
                           {'offset': '0x0056', 'bytes': 'd8cb', 'instruction': 'fmul st(3)'},
                           {'offset': '0x0058', 'bytes': 'dee1', 'instruction': 'fsubrp st(1)'},
                           {'offset': '0x0062', 'bytes': '85c0', 'instruction': 'test eax, eax'},
                           {'offset': '0x0066', 'bytes': 'db0424', 'instruction': 'fild dword ptr [esp]'},
                           {'offset': '0x0069', 'bytes': 'd9e0', 'instruction': 'fchs '},
                           {'offset': '0x006b', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                           {'offset': '0x0070', 'bytes': '83e001', 'instruction': 'and eax, 1'},
                           {'offset': '0x0075', 'bytes': 'd9e0', 'instruction': 'fchs '},
                           {'offset': '0x007a',
                            'bytes': 'dc77f0',
                            'instruction': 'fdiv qword ptr [edi - 0x10]'},
                           {'offset': '0x007d', 'bytes': 'dc0f', 'instruction': 'fmul qword ptr [edi]'},
                           {'offset': '0x007f',
                            'bytes': 'dc67d8',
                            'instruction': 'fsub qword ptr [edi - 0x28]'},
                           {'offset': '0x0082', 'bytes': 'dd1a', 'instruction': 'fstp qword ptr [edx]'},
                           {'offset': '0x0084',
                            'bytes': 'dc67e0',
                            'instruction': 'fsub qword ptr [edi - 0x20]'},
                           {'offset': '0x0087', 'bytes': 'dd1b', 'instruction': 'fstp qword ptr [ebx]'},
                           {'offset': '0x0089', 'bytes': 'ddd8', 'instruction': 'fstp st(0)'},
                           {'offset': '0x008b', 'bytes': 'ddd8', 'instruction': 'fstp st(0)'}]}}
PRIMITIVE_LF_SHA256 = 'a0964784744671549916c1b53b227ea085be6e8eef18d6fe84bd79994295d94e'

FORMULAS = {
    '_invcylindrical': (4522, -1, 0, 0, 1, 1),
    '_SinY': (7010, -1, 5, 512, 1, 1),
    '_PolyFolding': (7065, -1, 7, 0, 1, 1),
    '_PolyFold-sym': (7076, -1, 7, 0, 1, 1),
    '_ngon': (7081, -1, 8, 0, 1, 1),
}
LAYOUT = {
    '_invcylindrical': (),
    '_SinY': ((-12, 'int32'), (-16, 'single'), (-20, 'single'), (-24, 'single'), (-28, 'single')),
    '_PolyFolding': tuple((-16-8*i, 'double') for i in range(5))+((0, 'constant'), (8, 'constant')),
    '_PolyFold-sym': tuple((-16-8*i, 'double') for i in range(5))+((0, 'constant'), (8, 'constant')),
    '_ngon': tuple((-16-8*i, 'double') for i in range(5))+((-52, 'int32'), (0, 'constant'), (8, 'constant')),
}
fp32, fraction, word = affine.binary32, affine.fraction, affine.word
ONE, NEG_ZERO, INT_MIN = 0x3f800000, 0x80000000, -(1 << 31)


class ScopeError(ValueError):
    """No proof for this input in the declared bounded reference domain."""


def normal_or_zero(bits):
    try:
        value = fraction(bits)
    except ValueError as error:
        raise ScopeError('nonfinite arithmetic remains outside this finite model') from error
    if value and bits & 0x7f800000 == 0:
        raise ScopeError('subnormal arithmetic remains outside this domain')
    return value


def rn(value, negative_zero=False):
    bits = fp32(value, negative_zero)
    normal_or_zero(bits)
    return bits


def add(a, b):
    normal_or_zero(a); normal_or_zero(b)
    result = affine.add(a, b); normal_or_zero(result)
    return result


def mul(a, b):
    normal_or_zero(a); normal_or_zero(b)
    result = affine.multiply(a, b); normal_or_zero(result)
    return result


def neg(a): return a ^ NEG_ZERO

def sub(a, b): return add(a, neg(b))


def divide(a, b):
    value = normal_or_zero(b)
    if not value:
        raise ScopeError('singular division is not a successful finite case')
    return rn(normal_or_zero(a)/value, bool((a^b) >> 31))


def fused(a, b, c):
    product = normal_or_zero(a)*normal_or_zero(b)
    value = product+normal_or_zero(c)
    negative_zero = product == 0 and bool((a^b) >> 31) and c == NEG_ZERO
    return rn(value, negative_zero)


def sum_products(terms):
    """Require a singleton over grouping, one-product FMA and full-sum RN.

    The full-sum result is an extra conservative control, not a claim that
    hardware implements an unlimited dot product. Every rounded leaf is also
    checked for normal/zero finite arithmetic. This applies independently to
    each fixed radius, rather than assuming all squared-radius sums are exact.
    """
    terms = tuple(terms)
    if not 1 <= len(terms) <= 3:
        raise ScopeError('only sums of one to three products are covered')
    rounded = tuple(mul(a, b) for a, b in terms)
    exact = tuple(normal_or_zero(a)*normal_or_zero(b) for a, b in terms)
    def possibilities(order):
        if len(order) == 1:
            return {rounded[order[0]]}
        result = set()
        for split in range(1, len(order)):
            left, right = order[:split], order[split:]
            lo, ro = possibilities(left), possibilities(right)
            result.update(add(a, b) for a in lo for b in ro)
            if len(left) == 1:
                a, b = terms[left[0]]
                result.update(fused(a, b, c) for c in ro)
            if len(right) == 1:
                a, b = terms[right[0]]
                result.update(fused(a, b, c) for c in lo)
        return result
    outputs = set()
    for order in permutations(range(len(terms))):
        outputs.update(possibilities(order))
    outputs.add(rn(sum(exact, Fraction()), all(v == NEG_ZERO for v in rounded)))
    if len(outputs) != 1:
        raise ScopeError('arithmetic depends on grouping/FMA in this input')
    return outputs.pop()


def radius(state):
    return sum_products([(state[f'xyz[{i}]'], state[f'xyz[{i}]']) for i in range(3)])


def rounded_interval(interval, negative_zero=False):
    lo, hi = interval
    if lo > hi:
        raise ValueError('inverted interval')
    a, b = rn(lo, negative_zero), rn(hi, negative_zero)
    if a != b:
        raise ScopeError('interval has no unique RN32 target')
    return a


def atan_series_interval(value, terms=96):
    """Alternating rational series with its next-term remainder enclosure."""
    value = Fraction(value)
    if abs(value) > Fraction(1, 2) or terms < 1:
        raise ScopeError('arctangent-series proof domain exceeded')
    total = Fraction()
    power = value
    for k in range(terms):
        total += power/(2*k+1)
        power *= -value*value
    other = total+power/(2*terms+1)
    return min(total, other), max(total, other)


@lru_cache(maxsize=1)
def pi_interval():
    # Machin's identity, pi=16*atan(1/5)-4*atan(1/239).
    a, b = atan_series_interval(Fraction(1, 5)), atan_series_interval(Fraction(1, 239))
    return 16*a[0]-4*b[1], 16*a[1]-4*b[0]


@lru_cache(maxsize=256)
def trig_interval(bits, cosine=False):
    value = normal_or_zero(bits)
    if abs(value) > 16:
        raise ScopeError('trig argument outside bounded Taylor proof')
    if not value:
        return (Fraction(1), Fraction(1)) if cosine else (Fraction(), Fraction())
    term = Fraction(1) if cosine else value
    total = Fraction()
    for k in range(96):
        total += term
        denominator = (2*k+1)*(2*k+2) if cosine else (2*k+2)*(2*k+3)
        following = -term*value*value/denominator
        # After the peak, every subsequent magnitude decreases. The tail is
        # bounded by the next term even when the initial terms were increasing.
        if abs(following) <= abs(term):
            interval = min(total, total+following), max(total, total+following)
            try:
                rounded_interval(interval)
                return interval
            except ScopeError:
                pass
        term = following
    raise ScopeError('Taylor remainder did not isolate a unique RN32 target')


def sin_bits(bits):
    if bits in (0, NEG_ZERO): return bits
    return rounded_interval(trig_interval(bits))


def cos_bits(bits):
    return rounded_interval(trig_interval(bits, True))


def atan2_axis(y, x):
    """Original FPATAN quadrants, including all signed-zero axis cases."""
    fy, fx = normal_or_zero(y), normal_or_zero(x)
    if not fy:
        if not (x & NEG_ZERO): return y
        pi = pi_interval()
        return rounded_interval((-pi[1], -pi[0]) if y & NEG_ZERO else pi)
    if not fx:
        pi = pi_interval()
        interval = pi[0]/2, pi[1]/2
        return rounded_interval((-interval[1], -interval[0]) if y & NEG_ZERO else interval)
    raise ScopeError('general atan2 remains OPEN outside the sector-zero proof')


def nearest_integer(bits):
    value = normal_or_zero(bits)
    magnitude = abs(value)
    result = affine._nearest_even(magnitude.numerator, magnitude.denominator)
    return -result if value < 0 else result


def frndint(bits):
    # FRNDINT retains a real result; do not impose FISTP's int32 range.
    return rn(nearest_integer(bits), bool(bits & NEG_ZERO))


def fistp_int32(bits):
    result = nearest_integer(bits)
    return result if -(1 << 31) <= result < (1 << 31) else INT_MIN


def binary64(value, negative_zero=False):
    value = Fraction(value)
    if not value: return (1 << 63) if negative_zero else 0
    sign = (1 << 63) if value < 0 else 0
    value = abs(value); n, d = value.numerator, value.denominator
    exponent = n.bit_length()-d.bit_length()
    if (n < d << exponent) if exponent >= 0 else (n << -exponent < d): exponent -= 1
    shift = 1074 if exponent < -1022 else 52-exponent
    mantissa = affine._nearest_even(n << shift, d) if shift >= 0 else affine._nearest_even(n, d << -shift)
    if exponent < -1022: return sign | mantissa
    if mantissa == 1 << 53: mantissa >>= 1; exponent += 1
    if exponent > 1023: raise ScopeError('binary64 preparation overflow')
    return sign | ((exponent+1023) << 52) | (mantissa & ((1 << 52)-1))


def fraction64(bits):
    exponent = (bits >> 52) & 2047
    if exponent == 2047: raise ScopeError('nonfinite binary64 numeric parameter')
    mantissa = (bits & ((1 << 52)-1)) | ((1 << 52) if exponent else 0)
    shift = exponent-1075 if exponent else -1074
    value = Fraction(mantissa << shift) if shift >= 0 else Fraction(mantissa, 1 << -shift)
    return -value if bits >> 63 else value


def narrow64(bits):
    exponent, payload, sign = (bits >> 52) & 2047, bits & ((1 << 52)-1), (bits >> 32) & NEG_ZERO
    if exponent == 2047:
        # Packed raw int32/Single aliases may legitimately encode nonfinite
        # doubles. They are preserved, never used as arithmetic parameters.
        return sign | 0x7f800000 | ((0x400000 | (payload >> 29)) if payload else 0)
    return fp32(fraction64(bits), bool(sign))


def wide32(bits): return binary64(normal_or_zero(bits), bool(bits & NEG_ZERO))


# Original literal Double constants; neither is replaced by ideal 2*pi/pi/180.
CONSTANTS = {0: binary64(Fraction('6.283185307')), 8: binary64(Fraction('0.017453293'))}
TWO_PI, DEG = narrow64(CONSTANTS[0]), narrow64(CONSTANTS[8])


def _program(name, params, state):
    ident, option, count, traits, flags, driver = FORMULAS[name]
    if len(params) != count: raise ValueError('wrong prepared parameter count')
    p = [0]*2048
    p[:15] = [1, driver, 6, 0, 0, 0, 2, state['max_iterations'], state['julia'], 0, 0, 16384, 4096, 0, 0]
    p[16], p[18], p[21], p[22] = state['rstop'], ONE, fp32(1 << 20), ONE
    p[24:28] = [state[f'original_j[{i}]'] for i in range(4)]
    for i in range(4): p[28+5*i] = ONE
    p[50] = state['hybrid_counts[0]']
    for slot in range(6): p[56+slot*332] = 1
    base = 56
    p[base:base+12] = [1, 1, ident, option & 0xffffffff, count, traits, flags, 0, 0, 1, 0, 0]
    memory = bytearray(512)
    struct.pack_into('<Q', memory, 248, 0x3fe0000000000000)
    for i, ((offset, kind), value) in enumerate(zip(LAYOUT[name], params)):
        if kind == 'int32':
            if type(value) is not int or not INT_MIN <= value < (1 << 31):
                raise ScopeError('authored integer outside int32 storage domain')
            struct.pack_into('<i', memory, 256+offset, value)
            original, prepared = binary64(value), fp32(value)
        else:
            original = CONSTANTS[offset] if kind == 'constant' else wide32(value)
            prepared = narrow64(original)
            if prepared != value: raise ValueError('original constant differs')
            struct.pack_into('<I' if kind == 'single' else '<Q', memory, 256+offset,
                             value if kind == 'single' else original)
        p[base+12+i] = prepared
        p[base+76+2*i:base+78+2*i] = [original & 0xffffffff, original >> 32]
    p[base+204:base+268] = struct.unpack('<64I', memory[:256])
    for i in range(32):
        p[base+268+i] = narrow64(struct.unpack_from('<Q', memory, 248-8*i)[0])
        p[base+300+i] = narrow64(struct.unpack_from('<Q', memory, 256+8*i)[0])
    return p


def _parameters(name, program):
    values = program[68:68+FORMULAS[name][2]]
    for i, (_, kind) in enumerate(LAYOUT[name]):
        if kind == 'int32':
            exact = fraction64(program[56+76+2*i] | program[56+77+2*i] << 32)
            if exact.denominator != 1 or not INT_MIN <= exact < (1 << 31):
                raise ValueError('int32 metadata is not an exact signed integer')
            values[i] = int(exact)
    return values


def _polygon(name, state, params):
    order, shift, dx, dy, skew = params[:5]
    if not normal_or_zero(order): raise ScopeError('zero polygon order is singular')
    x, y = state['xyz[0]'], state['xyz[1]']
    xs, ys = add(x, dx), add(y, dy)
    if name != '_ngon' and order == fp32(Fraction(1, 2)) and shift == skew == 0:
        # |RN32(atan2)|<=RN32(pi); dividing by source TWO_PI and multiplying
        # by .5 lies in[-.25,.25]. Its rounded sector is zero regardless of
        # the non-axis atan2 value; the later +0 makes theta positive zero.
        bound = mul(divide(rounded_interval(pi_interval()), TWO_PI), order)
        if fraction(bound) >= Fraction(1, 2): raise ScopeError('sector-zero bound failed')
        q, sector_i, sector_bits, theta = None, 0, 0, 0
    else:
        angle = atan2_axis(ys, xs)
        combined = sum_products([(add(skew, shift), DEG), (angle, ONE)])
        q = mul(divide(combined, TWO_PI), order)
        if name == '_PolyFolding':
            sector_bits, sector_i = frndint(q), None
        else:
            sector_i = fistp_int32(q)
            sector_bits = fp32(sector_i)  # Original FILD int0 is +0, never -0.
        theta = sum_products([(divide(neg(sector_bits), order), TWO_PI), (shift, DEG)])
    sine, cosine = sin_bits(theta), cos_bits(theta)
    if name == '_PolyFolding':
        rx = sum_products([(y, sine), (neg(x), cosine)])
        ry = sum_products([(x, sine), (y, cosine)])
    else:
        rx = sub(sum_products([(ys, sine), (neg(xs), cosine)]), dx)
        if name == '_PolyFold-sym':
            ry = sum_products([(xs, sine), (ys, cosine)])
            if sector_i & 1 == 0: ry = neg(ry)
            ry = sub(ry, dy)
        else:
            residue = q if params[5] != 0 else sub(q, sector_bits)
            if params[5] == 0 and sector_i & 1 == 0: residue = neg(residue)
            ry = sum_products([(divide(residue, order), TWO_PI), (neg(dy), ONE)])
    out = dict(state)
    out['xyz[0]'], out['xyz[1]'] = rx, ry
    return out


def _point(name, state, params):
    out = dict(state)
    if name == '_invcylindrical':
        cosine, sine = cos_bits(state['xyz[1]']), sin_bits(state['xyz[1]'])
        out['xyz[0]'] = mul(state['xyz[0]'], cosine)
        out['xyz[1]'] = mul(state['xyz[0]'], sine)
    elif name == '_SinY':
        lane = f'xyz[{params[0] & 3}]'
        argument = mul(sub(state[lane], params[1]), params[2])
        out[lane] = sum_products([(sin_bits(argument), params[3]), (params[4], ONE)])
    elif name in ('_PolyFolding', '_PolyFold-sym', '_ngon'):
        out = _polygon(name, state, params)
    else: raise ValueError('formula outside group02 tail')
    out['current_slot'] = 0
    return out


def _orbit(name, state, params):
    s = dict(state)
    for i in range(3):
        s[f'xyz[{i}]'] = s[f'c[{i}]']
        s[f'j[{i}]'] = s[f'original_j[{i}]'] if s['julia'] else s[f'c[{i}]']
    s['xyz[3]'] = ONE
    s['first'] = s['iteration'] = 0
    s['rout'] = s['otrap'] = radius(s)
    remaining = count = s['hybrid_counts[0]']; s['tmp'] = remaining
    for _ in range(s['max_iterations']):
        s['rold'] = s['rout']
        if remaining == 0: remaining = count
        s = _point(name, s, params)
        remaining -= 1; s['tmp'] = remaining; s['iteration'] += 1
        s['rout'] = radius(s)
        if fraction(s['rout']) < fraction(s['otrap']): s['otrap'] = s['rout']
        if fraction(s['rout']) > fraction(s['rstop']): break
    return s


def evaluate(payload):
    if set(payload) != {'identity', 'kind', 'input_state_bits', 'parameter_bits'}:
        raise ValueError('expected exact semantic input fields')
    identity = payload['identity']
    if not isinstance(identity, list) or len(identity) != 3 or identity[0] != 'external' or identity[2] not in FORMULAS or identity[1] != FORMULAS[identity[2]][0]:
        raise ValueError('wrong tail formula identity')
    name = identity[2]
    def decode(value):
        if not isinstance(value, str) or not re.fullmatch(r'0x[0-9a-f]{8}', value): raise ValueError('expected hex32')
        return int(value, 16)
    if set(payload['input_state_bits']) != set(STATE) or len(payload['parameter_bits']) != 2048:
        raise ValueError('wrong ABI width')
    s = {k: decode(v) for k, v in payload['input_state_bits'].items()}
    p = [decode(v) for v in payload['parameter_bits']]
    for lane in STATE[:33]: normal_or_zero(s[lane])
    if payload['kind'] not in ('point', 'orbit') or s['julia'] not in (0, 1) or s['inside'] or s['calc_smooth']:
        raise ScopeError('caller outside bounded analytic3D model')
    if not 1 <= s['max_iterations'] <= 8 or not 1 <= s['hybrid_counts[0]'] <= 3:
        raise ScopeError('caller work bounds exceeded')
    params = _parameters(name, p)
    if p != _program(name, params, s): raise ValueError('typed prepared parameter ABI differs')
    if payload['kind'] == 'orbit':
        if s['vary_scale'] != ONE or s['j[3]'] != s['original_j[3]']: raise ScopeError('unsupported 3D entry state')
        out = _orbit(name, s, params)
    else: out = _point(name, s, params)
    for lane in STATE[:33]: normal_or_zero(out[lane])
    return {k: word(out[k]) for k in STATE}


def dependency_paths(): return [Path(affine.__file__).resolve()]


def primitive_identity(data):
    if hashlib.sha256(data.replace(b'\r\n', b'\n')).hexdigest() != PRIMITIVE_LF_SHA256:
        raise ValueError('frozen primitive dependency changed')
    return hashlib.sha256(data).hexdigest()


def manifest():
    def verified(pins):
        result = {}
        for path, pin in pins.items():
            data = (ROOT/path).read_bytes()
            if hashlib.sha256(data.replace(b'\r\n', b'\n')).hexdigest() != pin:
                raise ValueError('original source/ABI pin changed: '+path)
            result[path] = hashlib.sha256(data).hexdigest()
        return result
    sources, abi = verified(SOURCE_PINS), verified(ABI_PINS)
    for name, audit in CODE_AUDIT.items():
        text = (ROOT/'mb3d/M3Formulas'/f'{name}.m3f').read_text()
        code = bytes.fromhex(re.sub(r'\s+', '', re.search(r'\[CODE\](.*?)\[END\]', text, re.S)[1]))
        if len(code) != audit['bytes'] or hashlib.sha256(code).hexdigest() != audit['sha256']:
            raise ValueError('original CODE changed')
        for instruction in audit['key_offsets']:
            at, expected = int(instruction['offset'], 16), bytes.fromhex(instruction['bytes'])
            if code[at:at+len(expected)] != expected: raise ValueError('opcode audit changed')
    dep = dependency_paths()[0]
    return {'schema': 'mb3d.native-group02-tail-reference.v1', 'semantic_sources': sources,
            'abi_sources': abi, 'code_audit': CODE_AUDIT,
            'dependencies': {str(dep.relative_to(ROOT)): primitive_identity(dep.read_bytes())},
            'arithmetic': 'Independent rational interval ideal-RN32 targets; singleton grouping/FMA controls',
            'model_limits': [
                'Fixed finite five-formula corpus; all47 state lanes; bounded single-slot analytic3D orbits',
                'Ideal-RN32 sin/cos targets from rational Taylor remainder bounds, axis atan2 from Machin pi bounds',
                'Metal transcendental functions are not guaranteed universally RN32; a one-ULP difference alone is not a backend defect',
                'General atan2 remains OPEN except a proven zero-sector bound for two polygon formulas',
                'Original decimal Double constants preserved; authored real doubles restricted to widened binary32',
                'Original release DivUtils.pas CW0x133F selects nearest-even and masks FISTP invalid exceptions',
                'Typed int32 fields preserve exact metadata/raw bytes; FISTP finite overflow yields INT_MIN, unlike real FRNDINT',
                'Each radius and sum-product expression must have one result across groupings/FMA/full-sum controls',
                'Nonfinite/subnormal arithmetic, zero order and unresolved interval/contraction cases fail closed',
                'No arbitrary hybrid/angle/argument, distance scalar, x87 image parity or full-scene qualification'],
            'model_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}


def _state():
    s = {k: fp32(Fraction(i+1, 32)) for i, k in enumerate(STATE)}
    s.update(iteration=19, first=17, tmp=23, max_iterations=4, julia=0, inside=0, calc_smooth=0, current_slot=5)
    for i in range(6): s[f'hybrid_counts[{i}]'] = 1 if i == 0 else 0
    for i, value in enumerate((Fraction(3, 8), Fraction(-1, 2), Fraction(5, 8), Fraction(-3, 4))):
        for a, factor in (('xyz', 1), ('c', 1), ('j', 2), ('original_j', -2)): s[f'{a}[{i}]'] = fp32(value*factor)
    s['j[3]'] = s['original_j[3]']
    s['rstop'], s['vary_scale'] = fp32(1 << 16), ONE
    return s


def polygon_params(name, order=Fraction(1, 2), shift=0, dx=0, dy=0, skew=0, flag=0):
    params = [fp32(v) for v in (order, shift, dx, dy, skew)]
    if name == '_ngon': params.append(flag)
    return params+[TWO_PI, DEG]


def build_cases():
    sources, cases = manifest()['semantic_sources'], []
    def emit(name, variant, state, params, kind='point'):
        semantic = {'identity': ['external', FORMULAS[name][0], name], 'kind': kind,
                    'input_state_bits': {k: word(state[k]) for k in STATE},
                    'parameter_bits': [word(v) for v in _program(name, params, state)]}
        touched = {f'xyz[{params[0] & 3}]'} if name == '_SinY' else {'xyz[0]', 'xyz[1]'}
        reads, writes = set(touched), touched | {'current_slot'}
        if kind == 'orbit':
            writes |= {f'xyz[{i}]' for i in range(4)} | {f'j[{i}]' for i in range(3)}
            writes |= {'rout', 'rold', 'otrap', 'first', 'iteration', 'tmp'}
            reads |= {f'{a}[{i}]' for a in ('c', 'original_j') for i in range(4)}
            reads |= {'julia', 'max_iterations', 'rstop', 'hybrid_counts[0]'}
        cases.append({'id': f'{FORMULAS[name][0]}-{variant}', 'identity': semantic['identity'], 'variant': variant,
                      'input': semantic, 'expected_state_bits': evaluate(semantic),
                      'read_lanes': [k for k in STATE if k in reads], 'write_lanes': [k for k in STATE if k in writes],
                      'source_hashes': sources})
    tiny = fp32(Fraction(1, 1 << 20))
    for variant, x, y in [('tiny_double', fp32(2), tiny), ('tiny_triple', fp32(3), tiny),
                          ('negative_radius', fp32(-2), tiny), ('negative_angle', fp32(3), neg(tiny)),
                          ('tiny_upper', ONE, fp32(Fraction(1, 1 << 13))), ('positive_zero', fp32(2), 0),
                          ('negative_zero', fp32(2), NEG_ZERO), ('negative_zero_radius', NEG_ZERO, tiny),
                          ('literal_two_pi', ONE, TWO_PI), ('negative_literal_two_pi', ONE, neg(TWO_PI))]:
        s = _state(); s['xyz[0]'], s['xyz[1]'] = x, y
        emit('_invcylindrical', variant, s, [])
    for axis in range(4):
        s = _state(); s[f'xyz[{axis}]'] = tiny
        emit('_SinY', f'tiny_axis_{axis}', s, [axis, 0, ONE, fp32(2), 0])
    for index in (4, 5, -1, -2, 16777217, -16777217, 16777219, 2147483647, INT_MIN, INT_MIN+1):
        emit('_SinY', f'raw_index_{index}', _state(), [index, 0, 0, ONE, fp32(2)])
    s = _state(); s['xyz[1]'] = fp32(Fraction(5, 8))
    emit('_SinY', 'offset_scaled_tiny', s, [1, sub(s['xyz[1]'], tiny), fp32(2), fp32(-2), fp32(Fraction(1, 4))])
    for zero in (0, NEG_ZERO):
        s = _state(); s['xyz[1]'] = zero
        emit('_SinY', f'signed_zero_{zero}', s, [1, 0, ONE, ONE, zero])
    for name in ('_PolyFolding', '_PolyFold-sym'):
        emit(name, 'sector_zero_dense', _state(), polygon_params(name))
        emit(name, 'sector_zero_shifted', _state(), polygon_params(name, dx=Fraction(1, 4), dy=Fraction(1, 8)))
        for sign in (1, -1):
            s = _state(); s['xyz[0]'], s['xyz[1]'] = 0, fp32(sign)
            emit(name, f'half_sector_{sign}', s, polygon_params(name, order=2))
            s = _state(); s['xyz[0]'], s['xyz[1]'] = fp32(-1), 0 if sign == 1 else NEG_ZERO
            emit(name, f'negative_x_axis_{sign}', s, polygon_params(name, order=1))
    for name in ('_PolyFold-sym', '_ngon'):
        for variant, x, y in [('odd_sector', 0, ONE), ('even_sector', fp32(-1), 0)]:
            s = _state(); s['xyz[0]'], s['xyz[1]'] = x, y
            emit(name, variant, s, polygon_params(name, order=4))
    for name in ('_PolyFolding', '_PolyFold-sym', '_ngon'):
        for variant, order, skew in [('integer_below', (1 << 31)-128, 360),
                                     ('integer_positive_overflow', 1 << 31, 360),
                                     ('integer_negative_edge', 1 << 31, -360)]:
            s = _state(); s['xyz[0]'], s['xyz[1]'] = ONE, 0
            emit(name, variant, s, polygon_params(name, order=order, skew=skew))
    for flag in (1, -1, 2147483647):
        s = _state(); s['xyz[0]'], s['xyz[1]'] = ONE, 0
        emit('_ngon', f'no_yfold_{flag}', s, polygon_params('_ngon', order=1 << 31, skew=360, flag=flag))
    s = _state(); s['xyz[0]'], s['xyz[1]'] = ONE, 0
    params = polygon_params('_ngon', order=-2)  # q=-0; integer FILD reload is +0.
    emit('_ngon', 'signed_zero_integer_reload', s, params)
    for name in FORMULAS:
        for variant in ('orbit_mandel', 'orbit_julia', 'orbit_escape', 'orbit_repeat'):
            s = _state(); s['julia'] = int(variant in ('orbit_julia', 'orbit_repeat'))
            s['hybrid_counts[0]'] = 2 if variant == 'orbit_repeat' else 1
            if name == '_invcylindrical':
                s['c[0]'], s['c[1]'], s['c[2]'] = fp32(3 if s['julia'] else 2), tiny, 0
                params = []
            elif name == '_SinY':
                s['c[0]'], s['c[1]'], s['c[2]'] = 0, tiny, 0
                params = [1, 0, ONE, fp32(Fraction(1, 2)), 0]
            elif name == '_ngon':
                s['c[0]'], s['c[1]'], s['c[2]'] = ONE, 0, fp32(Fraction(1, 2))
                params = polygon_params(name, order=2)
            else: params = polygon_params(name)
            initial = radius({**s, **{f'xyz[{i}]': s[f'c[{i}]'] for i in range(3)}})
            if variant == 'orbit_escape': s['rstop'] = fp32(Fraction(1, 1 << 50))
            if variant == 'orbit_repeat': s['rstop'] = initial
            emit(name, variant, s, params, 'orbit')
    s = _state()
    emit('_SinY', 'orbit_w_write', s, [3, 0, 0, ONE, fp32(2)], 'orbit')
    return cases
