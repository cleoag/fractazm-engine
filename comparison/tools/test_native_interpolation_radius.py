"""Original interpolation denominator, including its nonzero RN32 epsilon."""
import importlib.util
from pathlib import Path
import random
import struct
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[2]
MODEL=ROOT/'comparison/results/formula-campaign-004/group01/8065/t009/caller-oracle/model-v3-draft/interpolation_reference.py'
import sys
sys.path.insert(0,str(MODEL.parent))
spec=importlib.util.spec_from_file_location('interpolation_source_test',MODEL)
source=importlib.util.module_from_spec(spec);spec.loader.exec_module(source)

HARNESS=r'''
#include "src/render/native_metal/interpolation_radius.h"
#include <cstdio>
int main(){
  using namespace mb3d_native;TranslateWord x;
  while(std::fread(&x,sizeof(x),1,stdin)==1){
    auto y=translate_add_bits(native_interpolation_denominator(translate_add_float(x)));
    if(std::fwrite(&y,sizeof(y),1,stdout)!=1)return 2;
  }
  return std::ferror(stdin)?3:0;
}
'''

class InterpolationRadiusTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=tempfile.TemporaryDirectory(prefix='mb3d-interpolation-radius-')
        root=Path(cls.temp.name);cpp=root/'probe.cpp';cpp.write_text(HARNESS);cls.binary=root/'probe'
        compiler=subprocess.check_output(['xcrun','--find','clang++'],text=True).strip()
        sdk=subprocess.check_output(['xcrun','--sdk','macosx','--show-sdk-path'],text=True).strip()
        subprocess.run([compiler,'-isysroot',sdk,'-std=c++20','-O2','-fno-fast-math','-ffp-contract=off',
                        '-I',str(ROOT),str(cpp),'-o',str(cls.binary)],check=True,capture_output=True)
    @classmethod
    def tearDownClass(cls):cls.temp.cleanup()
    def check_words(self,words):
        expected=[source.A.sqrt32(source.A.arithmetic.add(x,source.EPSILON)) for x in words]
        raw=subprocess.check_output([str(self.binary)],input=struct.pack('<%dI'%len(words),*words))
        actual=struct.unpack('<%dI'%len(words),raw)
        self.assertEqual(list(actual),expected)
    def test_zero_denominator_is_positive(self):
        self.assertNotEqual(source.A.sqrt32(source.EPSILON),0)
        self.check_words([0,0x80000000])
    def test_subnormal_and_scaling_boundaries(self):
        values={0,1,2,0x116c1,0x116c2,0x116c3,0x7ffffe,0x7fffff,0x800000,0x800001,
                0x0d7fffff,0x0d800000,0x0d800001,0x3f800000,0x7f7fffff}
        for power in range(1,255):
            values.update(((power<<23)-1,power<<23,(power<<23)+1))
        self.check_words(sorted(values))
    def test_deterministic_source_corpus(self):
        rng=random.Random(80650117)
        self.check_words([rng.randrange(0x0d800000 if i%2 else 0x7f800000) for i in range(4000)])

if __name__=='__main__':unittest.main()
