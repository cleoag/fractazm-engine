"""Source-derived RN32 controls for the isolated Translate add; no GPU credit."""
import hashlib
from pathlib import Path
import random
import struct
import subprocess
import tempfile
import unittest

import native_formula_reference as source

ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / 'src/render/native_metal/translate_add.metal.h'
THRESHOLD = 0x0d800000  # 2^-100, selected by proof, not device observations.
MAGNITUDES = (0, 1, 2, 3, 0x003fffff, 0x00400000, 0x007ffffe, 0x007fffff,
              0x00800000, 0x00800001, 0x00ffffff, 0x01000000, 0x01000001,
              THRESHOLD - 1, THRESHOLD, THRESHOLD + 1, 0x3f800000, 0x7f7fffff)


def authored_corpus():
    values = [v | sign for v in MAGNITUDES for sign in (0, 0x80000000)]
    return [{'id': f'translate-add-{i:04d}', 'a': source.word(a),
             'b': source.word(b), 'expected': source.word(source.add(a, b))}
            for i, (a, b) in enumerate((a, b) for a in values for b in values)]


def finite_pairs():
    pairs = [(int(r['a'], 16), int(r['b'], 16)) for r in authored_corpus()]
    # Adjacent normals and power boundaries throughout binary32, both signs,
    # test cancellation and unchanged normal results outside the small branch.
    for exponent in range(1, 255):
        base = exponent << 23
        for offset in (-1, 0, 1):
            for sign in (0, 0x80000000):
                pairs += [(base | sign, ((base + offset) | sign) ^ 0x80000000),
                          (base | sign, 1), (base | sign, 0x807fffff)]
    rng = random.Random(8065009)
    for i in range(20000):
        upper = THRESHOLD if i % 2 else 0x7f800000
        pairs.append((rng.randrange(upper) | (rng.randrange(2) << 31),
                      rng.randrange(upper) | (rng.randrange(2) << 31)))
    return pairs


HARNESS = r'''
#include "src/render/native_metal/translate_add.metal.h"
#include <cstdio>
int main() {
    using namespace mb3d_native;
    TranslateWord pair[2];
    while (std::fread(pair, sizeof(pair), 1, stdin) == 1) {
        float a = translate_add_float(pair[0]), b = translate_add_float(pair[1]);
        TranslateWord out[6] = {translate_add_bits(native_translate_add(a,b)),
                               translate_add_bits(a+b), 0, 0, 0, 0};
        if ((pair[0]&0x7fffffffu) < 0x0d800000u &&
            (pair[1]&0x7fffffffu) < 0x0d800000u) {
            out[2] = translate_add_scale24(pair[0]);
            out[3] = translate_add_scale24(pair[1]);
            volatile float sum = translate_add_float(out[2]) + translate_add_float(out[3]);
            out[4] = translate_add_bits(sum);
            out[5] = translate_add_unscale24(out[4]);
        }
        if (std::fwrite(out, sizeof(out), 1, stdout) != 1) return 2;
    }
    return std::ferror(stdin) ? 3 : 0;
}
'''


class TranslateAddTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary = tempfile.TemporaryDirectory(prefix='mb3d-translate-add-')
        directory = Path(cls.temporary.name)
        cpp = directory / 'check.cpp'; cpp.write_text(HARNESS)
        cls.binary = directory / 'check'
        compiler = subprocess.check_output(['xcrun', '--find', 'clang++'], text=True).strip()
        sdk = subprocess.check_output(['xcrun', '--sdk', 'macosx', '--show-sdk-path'], text=True).strip()
        compilation = subprocess.run([compiler, '-isysroot', sdk, '-std=c++20', '-O2', '-fno-fast-math', '-ffp-contract=off',
                                      '-I', str(ROOT), str(cpp), '-o', str(cls.binary)],
                                     capture_output=True, text=True, timeout=30)
        if compilation.returncode:
            raise RuntimeError(compilation.stderr)
        cls.pairs = finite_pairs()
        cls.results = cls.execute(cls.pairs)

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()

    @classmethod
    def execute(cls, pairs):
        raw = b''.join(struct.pack('<2I', *pair) for pair in pairs)
        out = subprocess.run([cls.binary], input=raw, capture_output=True,
                             check=True, timeout=30).stdout
        if len(out) != len(pairs) * 24:
            raise AssertionError('Incomplete isolated host output')
        return list(struct.iter_unpack('<6I', out))

    def test_original_three_adds_and_oracle_source_are_pinned(self):
        for name, expected in (
            ('mb3d/M3Formulas/_Translate.m3f', '3acdcb61881b9007cd1a66a33887d8a13ae7541bd3120472b582e2ac9a09b706'),
            ('comparison/tools/native_formula_reference.py', 'a0964784744671549916c1b53b227ea085be6e8eef18d6fe84bd79994295d94e')):
            self.assertEqual(hashlib.sha256((ROOT/name).read_bytes()).hexdigest(), expected)

    def test_predeclared_and_stratified_finite_pairs_match_independent_rational_add(self):
        for (a, b), out in zip(self.pairs, self.results):
            self.assertEqual(out[0], source.add(a, b), (hex(a), hex(b)))

    def test_scaled_operands_and_nonzero_result_are_normal_and_scaling_is_exact(self):
        checked = 0
        for (a, b), out in zip(self.pairs, self.results):
            if max(a & 0x7fffffff, b & 0x7fffffff) >= THRESHOLD:
                continue
            checked += 1
            for original, scaled in ((a, out[2]), (b, out[3])):
                self.assertEqual(source.fraction(scaled), source.fraction(original) * (1 << 24))
                self.assertTrue(not scaled & 0x7fffffff or scaled & 0x7f800000)
            self.assertEqual(out[4], source.add(out[2], out[3]))
            self.assertTrue(not out[4] & 0x7fffffff or out[4] & 0x7f800000)
            self.assertEqual(out[5], out[0])
            if 0 < (out[4] & 0x7fffffff) < (25 << 23):
                # A subnormal final result requires no second rounding.
                self.assertEqual(source.fraction(out[4]) / (1 << 24), source.fraction(out[0]))
        self.assertGreater(checked, 10000)

    def test_large_branch_is_unchanged_even_with_signed_zero_flush_hypothesis(self):
        def flush(word):
            return word & 0x80000000 if word & 0x7f800000 == 0 else word
        checked = 0
        for (a, b), out in zip(self.pairs, self.results):
            if max(a & 0x7fffffff, b & 0x7fffffff) < THRESHOLD:
                continue
            checked += 1
            self.assertEqual(out[0], out[1])
            self.assertEqual(flush(source.add(flush(a), flush(b))), source.add(a, b))
        self.assertGreater(checked, 10000)

    def test_signed_zeros_cancellation_and_normal_to_subnormal_result(self):
        pairs = [(0, 0), (0, 0x80000000), (0x80000000, 0), (0x80000000, 0x80000000),
                 (0x00800001, 0x80800000), (0x80800001, 0x00800000),
                 (0x00800000, 0x80800000), (0x3f800000, 0xbf800000)]
        self.assertEqual([r[0] for r in self.execute(pairs)], [0, 0, 0, 0x80000000, 1, 0x80000001, 0, 0])

    def test_infinity_nan_and_overflow_keep_the_original_hardware_expression(self):
        values = [0, 0x80000000, 0x3f800000, 0xbf800000, 0x7f7fffff, 0xff7fffff,
                  0x7f800000, 0xff800000, 0x7fc00001, 0xffc00123, 0x7f800001]
        pairs = [(a, b) for a in values for b in values
                 if ((a & 0x7f800000) == 0x7f800000 or (b & 0x7f800000) == 0x7f800000)]
        pairs += [(0x7f7fffff, 0x7f7fffff), (0xff7fffff, 0xff7fffff)]
        for pair, out in zip(pairs, self.execute(pairs)):
            self.assertEqual(out[0], out[1], tuple(map(hex, pair)))


if __name__ == '__main__':
    unittest.main()
