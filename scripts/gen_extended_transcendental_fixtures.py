"""Optional reference maintenance; bootstrap uses the checked-in rows only.

Requires mpmath==1.3.0. Compute at 120 decimal digits independently of C++.
Trig references use Intel's documented internal Pi, not mathematical sin(x).
"""
import random
from pathlib import Path
import mpmath as mp

assert mp.__version__ == '1.3.0'
mp.mp.dps = 120
rng = random.Random(20261019)
intel_pi = mp.floor(mp.pi * 2**66) / 2**66

def bits(x):
    if not x:
        return '0, 0'
    sign = 0x8000 if x < 0 else 0
    m, e = mp.frexp(abs(x))
    s = int(mp.nint(m * 2**64))
    if s == 2**64:
        s >>= 1
        e += 1
    return f'0x{sign | (int(e) - 1 + 16383):04x}, 0x{s:016x}ULL'

rows = []
def row(op, a, b, expected):
    rows.append(f'    {{{op}, {{{bits(mp.mpf(a))}}}, {{{bits(mp.mpf(b))}}}, {{{bits(expected)}}}}},')

for a in [0.0, -0.0, .0625, .5, 1., 3., 25., 1e6, 1e12, float(2**63-1024)]+[rng.uniform(-100,100) for _ in range(32)]:
    for op, fn in [(0, mp.sin), (1, mp.cos)]:
        row(op, a, 0, fn(mp.mpf(a)*mp.pi/intel_pi))
for _ in range(64):
    a,b = rng.uniform(-10,10), rng.uniform(-10,10)
    row(2,a,b,mp.atan2(a,b))
for exponent in [-16000,-2000,-1022,-10,0,10,1024,2000,16000]:
    a = mp.mpf(1.25)*mp.power(2,exponent)
    row(3,a,0,mp.log(a,2))
for a in [-16000.25,-2000.25,-1022.25,-10.25,-.5,0,.5,10.25,1024.25,2000.25,16000.25]:
    row(4,a,0,mp.power(2,a))

target = Path(__file__).resolve().parents[1]/'tests/fixtures/extended_transcendental_reference.h'
target.write_text('// Independent mpmath 1.3.0, 120-digit references. Regenerate with\n'
    '// scripts/gen_extended_transcendental_fixtures.py; never from C++ output.\n'
    'struct ExtendedReferenceBits { uint16_t exponent; uint64_t significand; };\n'
    'struct ExtendedReference { int operation; ExtendedReferenceBits a,b,expected; };\n'
    'static constexpr ExtendedReference extendedReferences[] = {\n'+'\n'.join(rows)+'\n};\n')
