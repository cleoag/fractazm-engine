"""Independent original-CODE Riemann2/TgladTetra finite binary32 targets.

Source-ordered RN32 scalar algebra is an explicitly bounded *target*, not a
claim that Metal contraction or precise division/sqrt universally gives those
bits. Riemann sine inputs must be exact zero and powers have exact identities.
Tglad includes meaningful finite interior cases, not just its guard sentinel.
No generated body, translated formula or device result supplies expectations.
"""
from __future__ import annotations
from fractions import Fraction
import hashlib
from pathlib import Path
import re
import struct
import native_formula_reference as affine
import native_group03_geometry_reference as geometry
import native_group02_tail_reference as tail
from gpu_pipeline_proofs import STATE
ROOT=Path(__file__).resolve().parents[2]
CALLER_PROFILE = {'path': 'comparison/results/native-formula-group04-20261101/caller-profile.json',
                  'sha256': '52a02f1b539b022d9975baf0ac027e3463d5441aa755c4bee143506aa1e1ed6e'}
fp32,fraction,word=affine.binary32,affine.fraction,affine.word
ONE,NEG_ZERO=0x3f800000,0x80000000
ScopeError=geometry.ScopeError
SOURCE_PINS = {'mb3d/CustomFormulas.pas': '9ed1c8289ebfb99202795af8daca6bef92650ba250a28c12435eb5beb4264c61',
 'mb3d/DivUtils.pas': '879221cb1d6abb371452f1f04afcf5315af238317dcb31b6359a145cb303da96',
 'mb3d/M3Formulas/Riemann2.m3f': '815b275151789f06d0308cc81df07be0dd3b2c9b1da13bebd3dac3f61a1bf457',
 'mb3d/M3Formulas/TgladTetra.m3f': '8ea4261e94c77ba5ad796d47e0cab56e2bf5d29e155a6070ed244ddf9b411c1a',
 'mb3d/Math3D.pas': '0b03d5d982be1285839831dfc36b2532725f5d679e2891b474d00d750b7b6727',
 'mb3d/TypeDefinitions.pas': '5ef479844d3d5ffac309b141c9f80b62ebf0f9bf8b8782b706d9f451ef096747',
 'mb3d/formulas.pas': '3fefcd04fe6726831f3a7904f151cc3cc192610f4532f2889b9771e5953775bd'}
ABI_PINS = {'src/core/external_formulas.h': 'b36cba7e6b37c6426a8d052f2434866397f6edc7edb8d799efd0ffccc751f3b6',
 'src/render/native_metal/formula_abi.h': 'd69725ad200b889c8e7e2c3267d8fee96e37188b54e5cdc145a6cde3afb6350a'}
DEPENDENCY_PINS = {'native_formula_reference.py': 'a0964784744671549916c1b53b227ea085be6e8eef18d6fe84bd79994295d94e',
 'native_group02_tail_reference.py': 'a8247e6156e22f8fcbb52f2e3fa7876f9c8683c8802ea9f435195b7ef9ae6cb6',
 'native_group03_geometry_reference.py': '1f95959a32f6f1f4edc95c15c087abf2a1d0f7d7607676f158f4b8bfc787c952'}
CODE_AUDIT = {'Riemann2': {'bytes': 371,
              'instructions': [{'bytes': '55', 'instruction': 'push ebp', 'offset': '0x0'},
                               {'bytes': '8bec', 'instruction': 'mov ebp, esp', 'offset': '0x1'},
                               {'bytes': '81ec28000000', 'instruction': 'sub esp, 0x28', 'offset': '0x3'},
                               {'bytes': '53', 'instruction': 'push ebx', 'offset': '0x9'},
                               {'bytes': '56', 'instruction': 'push esi', 'offset': '0xa'},
                               {'bytes': '57', 'instruction': 'push edi', 'offset': '0xb'},
                               {'bytes': '8b7508',
                                'instruction': 'mov esi, dword ptr [ebp + 8]',
                                'offset': '0xc'},
                               {'bytes': '8b7e30',
                                'instruction': 'mov edi, dword ptr [esi + 0x30]',
                                'offset': '0xf'},
                               {'bytes': '8bd8', 'instruction': 'mov ebx, eax', 'offset': '0x12'},
                               {'bytes': 'd9d0', 'instruction': 'fnop ', 'offset': '0x14'},
                               {'bytes': 'dd01', 'instruction': 'fld qword ptr [ecx]', 'offset': '0x16'},
                               {'bytes': 'dd02', 'instruction': 'fld qword ptr [edx]', 'offset': '0x18'},
                               {'bytes': 'dd03', 'instruction': 'fld qword ptr [ebx]', 'offset': '0x1a'},
                               {'bytes': 'd9c0', 'instruction': 'fld st(0)', 'offset': '0x1c'},
                               {'bytes': 'd84fdc',
                                'instruction': 'fmul dword ptr [edi - 0x24]',
                                'offset': '0x1e'},
                               {'bytes': 'd9c2', 'instruction': 'fld st(2)', 'offset': '0x21'},
                               {'bytes': 'd84fd8',
                                'instruction': 'fmul dword ptr [edi - 0x28]',
                                'offset': '0x23'},
                               {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x26'},
                               {'bytes': 'd9c3', 'instruction': 'fld st(3)', 'offset': '0x28'},
                               {'bytes': 'd84fd4',
                                'instruction': 'fmul dword ptr [edi - 0x2c]',
                                'offset': '0x2a'},
                               {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x2d'},
                               {'bytes': 'd9c1', 'instruction': 'fld st(1)', 'offset': '0x2f'},
                               {'bytes': 'd84ff4',
                                'instruction': 'fmul dword ptr [edi - 0xc]',
                                'offset': '0x31'},
                               {'bytes': 'd9c3', 'instruction': 'fld st(3)', 'offset': '0x34'},
                               {'bytes': 'd84ff0',
                                'instruction': 'fmul dword ptr [edi - 0x10]',
                                'offset': '0x36'},
                               {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x39'},
                               {'bytes': 'd9c4', 'instruction': 'fld st(4)', 'offset': '0x3b'},
                               {'bytes': 'd84fec',
                                'instruction': 'fmul dword ptr [edi - 0x14]',
                                'offset': '0x3d'},
                               {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x40'},
                               {'bytes': 'd9ca', 'instruction': 'fxch st(2)', 'offset': '0x42'},
                               {'bytes': 'd84fe8',
                                'instruction': 'fmul dword ptr [edi - 0x18]',
                                'offset': '0x44'},
                               {'bytes': 'd9cb', 'instruction': 'fxch st(3)', 'offset': '0x47'},
                               {'bytes': 'd84fe4',
                                'instruction': 'fmul dword ptr [edi - 0x1c]',
                                'offset': '0x49'},
                               {'bytes': 'dec3', 'instruction': 'faddp st(3)', 'offset': '0x4c'},
                               {'bytes': 'd9cb', 'instruction': 'fxch st(3)', 'offset': '0x4e'},
                               {'bytes': 'd84fe0',
                                'instruction': 'fmul dword ptr [edi - 0x20]',
                                'offset': '0x50'},
                               {'bytes': 'dec2', 'instruction': 'faddp st(2)', 'offset': '0x53'},
                               {'bytes': 'dd1b', 'instruction': 'fstp qword ptr [ebx]', 'offset': '0x55'},
                               {'bytes': 'dd1a', 'instruction': 'fstp qword ptr [edx]', 'offset': '0x57'},
                               {'bytes': 'dd19', 'instruction': 'fstp qword ptr [ecx]', 'offset': '0x59'},
                               {'bytes': 'd9d0', 'instruction': 'fnop ', 'offset': '0x5b'},
                               {'bytes': 'dd03', 'instruction': 'fld qword ptr [ebx]', 'offset': '0x5d'},
                               {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0x5f'},
                               {'bytes': 'dd02', 'instruction': 'fld qword ptr [edx]', 'offset': '0x61'},
                               {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0x63'},
                               {'bytes': 'dd01', 'instruction': 'fld qword ptr [ecx]', 'offset': '0x65'},
                               {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0x67'},
                               {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x69'},
                               {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x6b'},
                               {'bytes': 'd9fa', 'instruction': 'fsqrt ', 'offset': '0x6d'},
                               {'bytes': 'dd55f0',
                                'instruction': 'fst qword ptr [ebp - 0x10]',
                                'offset': '0x6f'},
                               {'bytes': 'dd47cc',
                                'instruction': 'fld qword ptr [edi - 0x34]',
                                'offset': '0x72'},
                               {'bytes': 'def1', 'instruction': 'fdivrp st(1)', 'offset': '0x75'},
                               {'bytes': '90', 'instruction': 'nop ', 'offset': '0x77'},
                               {'bytes': 'dd03', 'instruction': 'fld qword ptr [ebx]', 'offset': '0x78'},
                               {'bytes': 'd8c9', 'instruction': 'fmul st(1)', 'offset': '0x7a'},
                               {'bytes': 'dd1b', 'instruction': 'fstp qword ptr [ebx]', 'offset': '0x7c'},
                               {'bytes': 'dd02', 'instruction': 'fld qword ptr [edx]', 'offset': '0x7e'},
                               {'bytes': 'd8c9', 'instruction': 'fmul st(1)', 'offset': '0x80'},
                               {'bytes': 'dd1a', 'instruction': 'fstp qword ptr [edx]', 'offset': '0x82'},
                               {'bytes': 'dd01', 'instruction': 'fld qword ptr [ecx]', 'offset': '0x84'},
                               {'bytes': 'd8c9', 'instruction': 'fmul st(1)', 'offset': '0x86'},
                               {'bytes': 'dd19', 'instruction': 'fstp qword ptr [ecx]', 'offset': '0x88'},
                               {'bytes': 'ddd8', 'instruction': 'fstp st(0)', 'offset': '0x8a'},
                               {'bytes': '90', 'instruction': 'nop ', 'offset': '0x8c'},
                               {'bytes': '90', 'instruction': 'nop ', 'offset': '0x8d'},
                               {'bytes': '90', 'instruction': 'nop ', 'offset': '0x8e'},
                               {'bytes': '90', 'instruction': 'nop ', 'offset': '0x8f'},
                               {'bytes': 'd9e8', 'instruction': 'fld1 ', 'offset': '0x90'},
                               {'bytes': 'dc23', 'instruction': 'fsub qword ptr [ebx]', 'offset': '0x92'},
                               {'bytes': 'd9c0', 'instruction': 'fld st(0)', 'offset': '0x94'},
                               {'bytes': 'dd4794',
                                'instruction': 'fld qword ptr [edi - 0x6c]',
                                'offset': '0x96'},
                               {'bytes': 'dae9', 'instruction': 'fucompp ', 'offset': '0x99'},
                               {'bytes': 'dfe0', 'instruction': 'fnstsw ax', 'offset': '0x9b'},
                               {'bytes': 'd0ec', 'instruction': 'shr ah, 1', 'offset': '0x9d'},
                               {'bytes': '7209', 'instruction': 'jb 0xaa', 'offset': '0x9f'},
                               {'bytes': 'ddd8', 'instruction': 'fstp st(0)', 'offset': '0xa1'},
                               {'bytes': 'd9e8', 'instruction': 'fld1 ', 'offset': '0xa3'},
                               {'bytes': '90', 'instruction': 'nop ', 'offset': '0xa5'},
                               {'bytes': '90', 'instruction': 'nop ', 'offset': '0xa6'},
                               {'bytes': '90', 'instruction': 'nop ', 'offset': '0xa7'},
                               {'bytes': 'eb05', 'instruction': 'jmp 0xaf', 'offset': '0xa8'},
                               {'bytes': 'd9e8', 'instruction': 'fld1 ', 'offset': '0xaa'},
                               {'bytes': 'def1', 'instruction': 'fdivrp st(1)', 'offset': '0xac'},
                               {'bytes': '90', 'instruction': 'nop ', 'offset': '0xae'},
                               {'bytes': 'dd01', 'instruction': 'fld qword ptr [ecx]', 'offset': '0xaf'},
                               {'bytes': 'd8c9', 'instruction': 'fmul st(1)', 'offset': '0xb1'},
                               {'bytes': 'dd5de8',
                                'instruction': 'fstp qword ptr [ebp - 0x18]',
                                'offset': '0xb3'},
                               {'bytes': 'dd02', 'instruction': 'fld qword ptr [edx]', 'offset': '0xb6'},
                               {'bytes': 'd8c9', 'instruction': 'fmul st(1)', 'offset': '0xb8'},
                               {'bytes': 'dd5de0',
                                'instruction': 'fstp qword ptr [ebp - 0x20]',
                                'offset': '0xba'},
                               {'bytes': 'ddd8', 'instruction': 'fstp st(0)', 'offset': '0xbd'},
                               {'bytes': 'dd45e8',
                                'instruction': 'fld qword ptr [ebp - 0x18]',
                                'offset': '0xbf'},
                               {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0xc2'},
                               {'bytes': 'dd45e0',
                                'instruction': 'fld qword ptr [ebp - 0x20]',
                                'offset': '0xc4'},
                               {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0xc7'},
                               {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0xc9'},
                               {'bytes': 'd9e8', 'instruction': 'fld1 ', 'offset': '0xcb'},
                               {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0xcd'},
                               {'bytes': 'dd5dd8',
                                'instruction': 'fstp qword ptr [ebp - 0x28]',
                                'offset': '0xcf'},
                               {'bytes': '90', 'instruction': 'nop ', 'offset': '0xd2'},
                               {'bytes': 'dd45e8',
                                'instruction': 'fld qword ptr [ebp - 0x18]',
                                'offset': '0xd3'},
                               {'bytes': 'dc0f', 'instruction': 'fmul qword ptr [edi]', 'offset': '0xd6'},
                               {'bytes': 'dc47b4',
                                'instruction': 'fadd qword ptr [edi - 0x4c]',
                                'offset': '0xd8'},
                               {'bytes': 'd9fe', 'instruction': 'fsin ', 'offset': '0xdb'},
                               {'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0xdd'},
                               {'bytes': 'dd479c',
                                'instruction': 'fld qword ptr [edi - 0x64]',
                                'offset': '0xdf'},
                               {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0xe2'},
                               {'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0xe4'},
                               {'bytes': 'dd5de8',
                                'instruction': 'fstp qword ptr [ebp - 0x18]',
                                'offset': '0xe6'},
                               {'bytes': 'dd45e0',
                                'instruction': 'fld qword ptr [ebp - 0x20]',
                                'offset': '0xe9'},
                               {'bytes': 'dc0f', 'instruction': 'fmul qword ptr [edi]', 'offset': '0xec'},
                               {'bytes': 'dc47ac',
                                'instruction': 'fadd qword ptr [edi - 0x54]',
                                'offset': '0xee'},
                               {'bytes': 'd9fe', 'instruction': 'fsin ', 'offset': '0xf1'},
                               {'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0xf3'},
                               {'bytes': 'dd479c',
                                'instruction': 'fld qword ptr [edi - 0x64]',
                                'offset': '0xf5'},
                               {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0xf8'},
                               {'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0xfa'},
                               {'bytes': 'dd5de0',
                                'instruction': 'fstp qword ptr [ebp - 0x20]',
                                'offset': '0xfc'},
                               {'bytes': '90', 'instruction': 'nop ', 'offset': '0xff'},
                               {'bytes': 'dd45f0',
                                'instruction': 'fld qword ptr [ebp - 0x10]',
                                'offset': '0x100'},
                               {'bytes': 'dd45d8',
                                'instruction': 'fld qword ptr [ebp - 0x28]',
                                'offset': '0x103'},
                               {'bytes': 'dc4fc4',
                                'instruction': 'fmul qword ptr [edi - 0x3c]',
                                'offset': '0x106'},
                               {'bytes': 'd9c9', 'instruction': 'fxch st(1)', 'offset': '0x109'},
                               {'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0x10b'},
                               {'bytes': 'd9f1', 'instruction': 'fyl2x ', 'offset': '0x10d'},
                               {'bytes': 'd9e8', 'instruction': 'fld1 ', 'offset': '0x10f'},
                               {'bytes': 'd9c1', 'instruction': 'fld st(1)', 'offset': '0x111'},
                               {'bytes': 'd9f8', 'instruction': 'fprem ', 'offset': '0x113'},
                               {'bytes': 'd9f0', 'instruction': 'f2xm1 ', 'offset': '0x115'},
                               {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x117'},
                               {'bytes': 'd9fd', 'instruction': 'fscale ', 'offset': '0x119'},
                               {'bytes': 'd9c9', 'instruction': 'fxch st(1)', 'offset': '0x11b'},
                               {'bytes': 'ddd8', 'instruction': 'fstp st(0)', 'offset': '0x11d'},
                               {'bytes': 'dc47bc',
                                'instruction': 'fadd qword ptr [edi - 0x44]',
                                'offset': '0x11f'},
                               {'bytes': 'dd5df0',
                                'instruction': 'fstp qword ptr [ebp - 0x10]',
                                'offset': '0x122'},
                               {'bytes': '90', 'instruction': 'nop ', 'offset': '0x125'},
                               {'bytes': 'dd4708',
                                'instruction': 'fld qword ptr [edi + 8]',
                                'offset': '0x126'},
                               {'bytes': 'dd45e8',
                                'instruction': 'fld qword ptr [ebp - 0x18]',
                                'offset': '0x129'},
                               {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0x12c'},
                               {'bytes': 'dd45e0',
                                'instruction': 'fld qword ptr [ebp - 0x20]',
                                'offset': '0x12e'},
                               {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0x131'},
                               {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x133'},
                               {'bytes': 'd9e8', 'instruction': 'fld1 ', 'offset': '0x135'},
                               {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x137'},
                               {'bytes': 'def9', 'instruction': 'fdivp st(1)', 'offset': '0x139'},
                               {'bytes': 'dd55d8',
                                'instruction': 'fst qword ptr [ebp - 0x28]',
                                'offset': '0x13b'},
                               {'bytes': '90', 'instruction': 'nop ', 'offset': '0x13e'},
                               {'bytes': 'd9e0', 'instruction': 'fchs ', 'offset': '0x13f'},
                               {'bytes': 'd9e8', 'instruction': 'fld1 ', 'offset': '0x141'},
                               {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x143'},
                               {'bytes': 'dc4df0',
                                'instruction': 'fmul qword ptr [ebp - 0x10]',
                                'offset': '0x145'},
                               {'bytes': '90', 'instruction': 'nop ', 'offset': '0x148'},
                               {'bytes': 'dd45f0',
                                'instruction': 'fld qword ptr [ebp - 0x10]',
                                'offset': '0x149'},
                               {'bytes': 'dc4de0',
                                'instruction': 'fmul qword ptr [ebp - 0x20]',
                                'offset': '0x14c'},
                               {'bytes': 'dc4dd8',
                                'instruction': 'fmul qword ptr [ebp - 0x28]',
                                'offset': '0x14f'},
                               {'bytes': '90', 'instruction': 'nop ', 'offset': '0x152'},
                               {'bytes': 'dd45f0',
                                'instruction': 'fld qword ptr [ebp - 0x10]',
                                'offset': '0x153'},
                               {'bytes': 'dc4de8',
                                'instruction': 'fmul qword ptr [ebp - 0x18]',
                                'offset': '0x156'},
                               {'bytes': 'dc4dd8',
                                'instruction': 'fmul qword ptr [ebp - 0x28]',
                                'offset': '0x159'},
                               {'bytes': '90', 'instruction': 'nop ', 'offset': '0x15c'},
                               {'bytes': 'dd19', 'instruction': 'fstp qword ptr [ecx]', 'offset': '0x15d'},
                               {'bytes': 'dd1a', 'instruction': 'fstp qword ptr [edx]', 'offset': '0x15f'},
                               {'bytes': 'dc47a4',
                                'instruction': 'fadd qword ptr [edi - 0x5c]',
                                'offset': '0x161'},
                               {'bytes': 'dd1b', 'instruction': 'fstp qword ptr [ebx]', 'offset': '0x164'},
                               {'bytes': 'd9d0', 'instruction': 'fnop ', 'offset': '0x166'},
                               {'bytes': '8bc3', 'instruction': 'mov eax, ebx', 'offset': '0x168'},
                               {'bytes': '5f', 'instruction': 'pop edi', 'offset': '0x16a'},
                               {'bytes': '5e', 'instruction': 'pop esi', 'offset': '0x16b'},
                               {'bytes': '5b', 'instruction': 'pop ebx', 'offset': '0x16c'},
                               {'bytes': '8be5', 'instruction': 'mov esp, ebp', 'offset': '0x16d'},
                               {'bytes': '5d', 'instruction': 'pop ebp', 'offset': '0x16f'},
                               {'bytes': 'c20800', 'instruction': 'ret 8', 'offset': '0x170'}],
              'sha256': '7361727764ee755fe87a8212e0128c86caaf3a3c9176915ae98ab389c682521a'},
 'TgladTetra': {'bytes': 503,
                'instructions': [{'bytes': '55', 'instruction': 'push ebp', 'offset': '0x0'},
                                 {'bytes': '8bec', 'instruction': 'mov ebp, esp', 'offset': '0x1'},
                                 {'bytes': '81ec60000000', 'instruction': 'sub esp, 0x60', 'offset': '0x3'},
                                 {'bytes': '53', 'instruction': 'push ebx', 'offset': '0x9'},
                                 {'bytes': '56', 'instruction': 'push esi', 'offset': '0xa'},
                                 {'bytes': '57', 'instruction': 'push edi', 'offset': '0xb'},
                                 {'bytes': '8b7508',
                                  'instruction': 'mov esi, dword ptr [ebp + 8]',
                                  'offset': '0xc'},
                                 {'bytes': '8b7e30',
                                  'instruction': 'mov edi, dword ptr [esi + 0x30]',
                                  'offset': '0xf'},
                                 {'bytes': '8bd8', 'instruction': 'mov ebx, eax', 'offset': '0x12'},
                                 {'bytes': 'd9d0', 'instruction': 'fnop ', 'offset': '0x14'},
                                 {'bytes': '8a67f4',
                                  'instruction': 'mov ah, byte ptr [edi - 0xc]',
                                  'offset': '0x16'},
                                 {'bytes': '80e407', 'instruction': 'and ah, 7', 'offset': '0x19'},
                                 {'bytes': 'dd03', 'instruction': 'fld qword ptr [ebx]', 'offset': '0x1c'},
                                 {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0x1e'},
                                 {'bytes': 'dd02', 'instruction': 'fld qword ptr [edx]', 'offset': '0x20'},
                                 {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0x22'},
                                 {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x24'},
                                 {'bytes': 'd9c0', 'instruction': 'fld st(0)', 'offset': '0x26'},
                                 {'bytes': 'dd01', 'instruction': 'fld qword ptr [ecx]', 'offset': '0x28'},
                                 {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0x2a'},
                                 {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x2c'},
                                 {'bytes': 'c1e010', 'instruction': 'shl eax, 0x10', 'offset': '0x2e'},
                                 {'bytes': 'dc5730',
                                  'instruction': 'fcom qword ptr [edi + 0x30]',
                                  'offset': '0x31'},
                                 {'bytes': '9b', 'instruction': 'wait ', 'offset': '0x34'},
                                 {'bytes': 'dfe0', 'instruction': 'fnstsw ax', 'offset': '0x35'},
                                 {'bytes': 'd0ec', 'instruction': 'shr ah, 1', 'offset': '0x37'},
                                 {'bytes': '0f83ab010000', 'instruction': 'jae 0x1ea', 'offset': '0x39'},
                                 {'bytes': 'c1e810', 'instruction': 'shr eax, 0x10', 'offset': '0x3f'},
                                 {'bytes': 'd9fa', 'instruction': 'fsqrt ', 'offset': '0x42'},
                                 {'bytes': 'dd55b8',
                                  'instruction': 'fst qword ptr [ebp - 0x48]',
                                  'offset': '0x44'},
                                 {'bytes': 'dd01', 'instruction': 'fld qword ptr [ecx]', 'offset': '0x47'},
                                 {'bytes': 'def1', 'instruction': 'fdivrp st(1)', 'offset': '0x49'},
                                 {'bytes': 'dd5df8',
                                  'instruction': 'fstp qword ptr [ebp - 8]',
                                  'offset': '0x4b'},
                                 {'bytes': 'd9fa', 'instruction': 'fsqrt ', 'offset': '0x4e'},
                                 {'bytes': 'dc22', 'instruction': 'fsub qword ptr [edx]', 'offset': '0x50'},
                                 {'bytes': 'dc33', 'instruction': 'fdiv qword ptr [ebx]', 'offset': '0x52'},
                                 {'bytes': 'dd55f0',
                                  'instruction': 'fst qword ptr [ebp - 0x10]',
                                  'offset': '0x54'},
                                 {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0x57'},
                                 {'bytes': 'dd55d8',
                                  'instruction': 'fst qword ptr [ebp - 0x28]',
                                  'offset': '0x59'},
                                 {'bytes': 'd9e8', 'instruction': 'fld1 ', 'offset': '0x5c'},
                                 {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x5e'},
                                 {'bytes': 'd9e8', 'instruction': 'fld1 ', 'offset': '0x60'},
                                 {'bytes': 'def1', 'instruction': 'fdivrp st(1)', 'offset': '0x62'},
                                 {'bytes': '90', 'instruction': 'nop ', 'offset': '0x64'},
                                 {'bytes': 'd9e8', 'instruction': 'fld1 ', 'offset': '0x65'},
                                 {'bytes': 'dc45f8',
                                  'instruction': 'fadd qword ptr [ebp - 8]',
                                  'offset': '0x67'},
                                 {'bytes': 'd9e8', 'instruction': 'fld1 ', 'offset': '0x6a'},
                                 {'bytes': 'dc65f8',
                                  'instruction': 'fsub qword ptr [ebp - 8]',
                                  'offset': '0x6c'},
                                 {'bytes': 'def1', 'instruction': 'fdivrp st(1)', 'offset': '0x6f'},
                                 {'bytes': 'd9fa', 'instruction': 'fsqrt ', 'offset': '0x71'},
                                 {'bytes': '90', 'instruction': 'nop ', 'offset': '0x73'},
                                 {'bytes': 'dec9', 'instruction': 'fmulp st(1)', 'offset': '0x74'},
                                 {'bytes': 'd9c0', 'instruction': 'fld st(0)', 'offset': '0x76'},
                                 {'bytes': 'd8c0', 'instruction': 'fadd st(0)', 'offset': '0x78'},
                                 {'bytes': 'dc4df0',
                                  'instruction': 'fmul qword ptr [ebp - 0x10]',
                                  'offset': '0x7a'},
                                 {'bytes': 'dd1b', 'instruction': 'fstp qword ptr [ebx]', 'offset': '0x7d'},
                                 {'bytes': '90', 'instruction': 'nop ', 'offset': '0x7f'},
                                 {'bytes': 'd9e8', 'instruction': 'fld1 ', 'offset': '0x80'},
                                 {'bytes': 'dc65d8',
                                  'instruction': 'fsub qword ptr [ebp - 0x28]',
                                  'offset': '0x82'},
                                 {'bytes': 'dec9', 'instruction': 'fmulp st(1)', 'offset': '0x85'},
                                 {'bytes': 'dd1a', 'instruction': 'fstp qword ptr [edx]', 'offset': '0x87'},
                                 {'bytes': 'd9d0', 'instruction': 'fnop ', 'offset': '0x89'},
                                 {'bytes': '8a47f0',
                                  'instruction': 'mov al, byte ptr [edi - 0x10]',
                                  'offset': '0x8b'},
                                 {'bytes': '2407', 'instruction': 'and al, 7', 'offset': '0x8e'},
                                 {'bytes': '90', 'instruction': 'nop ', 'offset': '0x90'},
                                 {'bytes': 'dd03', 'instruction': 'fld qword ptr [ebx]', 'offset': '0x91'},
                                 {'bytes': 'd9c0', 'instruction': 'fld st(0)', 'offset': '0x93'},
                                 {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0x95'},
                                 {'bytes': 'dec9', 'instruction': 'fmulp st(1)', 'offset': '0x97'},
                                 {'bytes': 'dd02', 'instruction': 'fld qword ptr [edx]', 'offset': '0x99'},
                                 {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0x9b'},
                                 {'bytes': 'dc4f08',
                                  'instruction': 'fmul qword ptr [edi + 8]',
                                  'offset': '0x9d'},
                                 {'bytes': 'dc0b', 'instruction': 'fmul qword ptr [ebx]', 'offset': '0xa0'},
                                 {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0xa2'},
                                 {'bytes': 'c1e010', 'instruction': 'shl eax, 0x10', 'offset': '0xa4'},
                                 {'bytes': 'dc5730',
                                  'instruction': 'fcom qword ptr [edi + 0x30]',
                                  'offset': '0xa7'},
                                 {'bytes': '9b', 'instruction': 'wait ', 'offset': '0xaa'},
                                 {'bytes': 'dfe0', 'instruction': 'fnstsw ax', 'offset': '0xab'},
                                 {'bytes': 'd0ec', 'instruction': 'shr ah, 1', 'offset': '0xad'},
                                 {'bytes': '0f8337010000', 'instruction': 'jae 0x1ec', 'offset': '0xaf'},
                                 {'bytes': 'c1e810', 'instruction': 'shr eax, 0x10', 'offset': '0xb5'},
                                 {'bytes': 'dd5de0',
                                  'instruction': 'fstp qword ptr [ebp - 0x20]',
                                  'offset': '0xb8'},
                                 {'bytes': 'dd02', 'instruction': 'fld qword ptr [edx]', 'offset': '0xbb'},
                                 {'bytes': 'd9c0', 'instruction': 'fld st(0)', 'offset': '0xbd'},
                                 {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0xbf'},
                                 {'bytes': 'dec9', 'instruction': 'fmulp st(1)', 'offset': '0xc1'},
                                 {'bytes': 'dd03', 'instruction': 'fld qword ptr [ebx]', 'offset': '0xc3'},
                                 {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0xc5'},
                                 {'bytes': 'dc4f08',
                                  'instruction': 'fmul qword ptr [edi + 8]',
                                  'offset': '0xc7'},
                                 {'bytes': 'dc0a', 'instruction': 'fmul qword ptr [edx]', 'offset': '0xca'},
                                 {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0xcc'},
                                 {'bytes': 'dd55d8',
                                  'instruction': 'fst qword ptr [ebp - 0x28]',
                                  'offset': '0xce'},
                                 {'bytes': 'dc4f20',
                                  'instruction': 'fmul qword ptr [edi + 0x20]',
                                  'offset': '0xd1'},
                                 {'bytes': 'dd5dd0',
                                  'instruction': 'fstp qword ptr [ebp - 0x30]',
                                  'offset': '0xd4'},
                                 {'bytes': 'dd45d8',
                                  'instruction': 'fld qword ptr [ebp - 0x28]',
                                  'offset': '0xd7'},
                                 {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0xda'},
                                 {'bytes': 'dd5dd8',
                                  'instruction': 'fstp qword ptr [ebp - 0x28]',
                                  'offset': '0xdc'},
                                 {'bytes': 'dd45e0',
                                  'instruction': 'fld qword ptr [ebp - 0x20]',
                                  'offset': '0xdf'},
                                 {'bytes': 'dc4718',
                                  'instruction': 'fadd qword ptr [edi + 0x18]',
                                  'offset': '0xe2'},
                                 {'bytes': 'dd4710',
                                  'instruction': 'fld qword ptr [edi + 0x10]',
                                  'offset': '0xe5'},
                                 {'bytes': 'dc65e0',
                                  'instruction': 'fsub qword ptr [ebp - 0x20]',
                                  'offset': '0xe8'},
                                 {'bytes': 'dec9', 'instruction': 'fmulp st(1)', 'offset': '0xeb'},
                                 {'bytes': 'dc65d8',
                                  'instruction': 'fsub qword ptr [ebp - 0x28]',
                                  'offset': '0xed'},
                                 {'bytes': 'dd5dc8',
                                  'instruction': 'fstp qword ptr [ebp - 0x38]',
                                  'offset': '0xf0'},
                                 {'bytes': 'dd45e0',
                                  'instruction': 'fld qword ptr [ebp - 0x20]',
                                  'offset': '0xf3'},
                                 {'bytes': 'dc4718',
                                  'instruction': 'fadd qword ptr [edi + 0x18]',
                                  'offset': '0xf6'},
                                 {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0xf9'},
                                 {'bytes': 'dc45d8',
                                  'instruction': 'fadd qword ptr [ebp - 0x28]',
                                  'offset': '0xfb'},
                                 {'bytes': 'dd4718',
                                  'instruction': 'fld qword ptr [edi + 0x18]',
                                  'offset': '0xfe'},
                                 {'bytes': 'd9e0', 'instruction': 'fchs ', 'offset': '0x101'},
                                 {'bytes': 'def1', 'instruction': 'fdivrp st(1)', 'offset': '0x103'},
                                 {'bytes': 'dd5dc0',
                                  'instruction': 'fstp qword ptr [ebp - 0x40]',
                                  'offset': '0x105'},
                                 {'bytes': '90', 'instruction': 'nop ', 'offset': '0x108'},
                                 {'bytes': 'dd45c8',
                                  'instruction': 'fld qword ptr [ebp - 0x38]',
                                  'offset': '0x109'},
                                 {'bytes': 'dc0b', 'instruction': 'fmul qword ptr [ebx]', 'offset': '0x10c'},
                                 {'bytes': 'dd45d0',
                                  'instruction': 'fld qword ptr [ebp - 0x30]',
                                  'offset': '0x10e'},
                                 {'bytes': 'dc0a', 'instruction': 'fmul qword ptr [edx]', 'offset': '0x111'},
                                 {'bytes': 'd9e0', 'instruction': 'fchs ', 'offset': '0x113'},
                                 {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x115'},
                                 {'bytes': 'dc4dc0',
                                  'instruction': 'fmul qword ptr [ebp - 0x40]',
                                  'offset': '0x117'},
                                 {'bytes': '90', 'instruction': 'nop ', 'offset': '0x11a'},
                                 {'bytes': 'dd45c8',
                                  'instruction': 'fld qword ptr [ebp - 0x38]',
                                  'offset': '0x11b'},
                                 {'bytes': 'dc0a', 'instruction': 'fmul qword ptr [edx]', 'offset': '0x11e'},
                                 {'bytes': 'dd45d0',
                                  'instruction': 'fld qword ptr [ebp - 0x30]',
                                  'offset': '0x120'},
                                 {'bytes': 'dc0b', 'instruction': 'fmul qword ptr [ebx]', 'offset': '0x123'},
                                 {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x125'},
                                 {'bytes': 'dc4dc0',
                                  'instruction': 'fmul qword ptr [ebp - 0x40]',
                                  'offset': '0x127'},
                                 {'bytes': '90', 'instruction': 'nop ', 'offset': '0x12a'},
                                 {'bytes': 'dd1a', 'instruction': 'fstp qword ptr [edx]', 'offset': '0x12b'},
                                 {'bytes': 'dd1b', 'instruction': 'fstp qword ptr [ebx]', 'offset': '0x12d'},
                                 {'bytes': 'dd45b8',
                                  'instruction': 'fld qword ptr [ebp - 0x48]',
                                  'offset': '0x12f'},
                                 {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0x132'},
                                 {'bytes': 'dd5db8',
                                  'instruction': 'fstp qword ptr [ebp - 0x48]',
                                  'offset': '0x134'},
                                 {'bytes': 'fec8', 'instruction': 'dec al', 'offset': '0x137'},
                                 {'bytes': '0f8f52ffffff', 'instruction': 'jg 0x91', 'offset': '0x139'},
                                 {'bytes': 'd9d0', 'instruction': 'fnop ', 'offset': '0x13f'},
                                 {'bytes': 'dd03', 'instruction': 'fld qword ptr [ebx]', 'offset': '0x141'},
                                 {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0x143'},
                                 {'bytes': 'dd02', 'instruction': 'fld qword ptr [edx]', 'offset': '0x145'},
                                 {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0x147'},
                                 {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x149'},
                                 {'bytes': 'dd55c0',
                                  'instruction': 'fst qword ptr [ebp - 0x40]',
                                  'offset': '0x14b'},
                                 {'bytes': 'd9fa', 'instruction': 'fsqrt ', 'offset': '0x14e'},
                                 {'bytes': 'dd55e8',
                                  'instruction': 'fst qword ptr [ebp - 0x18]',
                                  'offset': '0x150'},
                                 {'bytes': 'dc22', 'instruction': 'fsub qword ptr [edx]', 'offset': '0x153'},
                                 {'bytes': 'dc33', 'instruction': 'fdiv qword ptr [ebx]', 'offset': '0x155'},
                                 {'bytes': 'dd55f0',
                                  'instruction': 'fst qword ptr [ebp - 0x10]',
                                  'offset': '0x157'},
                                 {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0x15a'},
                                 {'bytes': 'dd55d8',
                                  'instruction': 'fst qword ptr [ebp - 0x28]',
                                  'offset': '0x15c'},
                                 {'bytes': 'd9e8', 'instruction': 'fld1 ', 'offset': '0x15f'},
                                 {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x161'},
                                 {'bytes': 'd9e8', 'instruction': 'fld1 ', 'offset': '0x163'},
                                 {'bytes': 'def1', 'instruction': 'fdivrp st(1)', 'offset': '0x165'},
                                 {'bytes': 'dd55e0',
                                  'instruction': 'fst qword ptr [ebp - 0x20]',
                                  'offset': '0x167'},
                                 {'bytes': 'd9e8', 'instruction': 'fld1 ', 'offset': '0x16a'},
                                 {'bytes': 'dc65d8',
                                  'instruction': 'fsub qword ptr [ebp - 0x28]',
                                  'offset': '0x16c'},
                                 {'bytes': 'dec9', 'instruction': 'fmulp st(1)', 'offset': '0x16f'},
                                 {'bytes': 'dd5dc8',
                                  'instruction': 'fstp qword ptr [ebp - 0x38]',
                                  'offset': '0x171'},
                                 {'bytes': '90', 'instruction': 'nop ', 'offset': '0x174'},
                                 {'bytes': 'dd45f0',
                                  'instruction': 'fld qword ptr [ebp - 0x10]',
                                  'offset': '0x175'},
                                 {'bytes': 'd8c0', 'instruction': 'fadd st(0)', 'offset': '0x178'},
                                 {'bytes': 'dc4de0',
                                  'instruction': 'fmul qword ptr [ebp - 0x20]',
                                  'offset': '0x17a'},
                                 {'bytes': 'dd5dd0',
                                  'instruction': 'fstp qword ptr [ebp - 0x30]',
                                  'offset': '0x17d'},
                                 {'bytes': '90', 'instruction': 'nop ', 'offset': '0x180'},
                                 {'bytes': 'd9e8', 'instruction': 'fld1 ', 'offset': '0x181'},
                                 {'bytes': 'dc45c0',
                                  'instruction': 'fadd qword ptr [ebp - 0x40]',
                                  'offset': '0x183'},
                                 {'bytes': 'dd4728',
                                  'instruction': 'fld qword ptr [edi + 0x28]',
                                  'offset': '0x186'},
                                 {'bytes': 'def1', 'instruction': 'fdivrp st(1)', 'offset': '0x189'},
                                 {'bytes': 'dd55d8',
                                  'instruction': 'fst qword ptr [ebp - 0x28]',
                                  'offset': '0x18b'},
                                 {'bytes': 'dc4de8',
                                  'instruction': 'fmul qword ptr [ebp - 0x18]',
                                  'offset': '0x18e'},
                                 {'bytes': 'dd5de0',
                                  'instruction': 'fstp qword ptr [ebp - 0x20]',
                                  'offset': '0x191'},
                                 {'bytes': '90', 'instruction': 'nop ', 'offset': '0x194'},
                                 {'bytes': '90', 'instruction': 'nop ', 'offset': '0x195'},
                                 {'bytes': 'dd45b8',
                                  'instruction': 'fld qword ptr [ebp - 0x48]',
                                  'offset': '0x196'},
                                 {'bytes': 'dc4dd0',
                                  'instruction': 'fmul qword ptr [ebp - 0x30]',
                                  'offset': '0x199'},
                                 {'bytes': 'dc4de0',
                                  'instruction': 'fmul qword ptr [ebp - 0x20]',
                                  'offset': '0x19c'},
                                 {'bytes': '90', 'instruction': 'nop ', 'offset': '0x19f'},
                                 {'bytes': 'dd45b8',
                                  'instruction': 'fld qword ptr [ebp - 0x48]',
                                  'offset': '0x1a0'},
                                 {'bytes': 'dc4dc8',
                                  'instruction': 'fmul qword ptr [ebp - 0x38]',
                                  'offset': '0x1a3'},
                                 {'bytes': 'dc4de0',
                                  'instruction': 'fmul qword ptr [ebp - 0x20]',
                                  'offset': '0x1a6'},
                                 {'bytes': '90', 'instruction': 'nop ', 'offset': '0x1a9'},
                                 {'bytes': 'dd45d8',
                                  'instruction': 'fld qword ptr [ebp - 0x28]',
                                  'offset': '0x1aa'},
                                 {'bytes': 'd9e8', 'instruction': 'fld1 ', 'offset': '0x1ad'},
                                 {'bytes': 'dee9', 'instruction': 'fsubp st(1)', 'offset': '0x1af'},
                                 {'bytes': 'dc4db8',
                                  'instruction': 'fmul qword ptr [ebp - 0x48]',
                                  'offset': '0x1b1'},
                                 {'bytes': '90', 'instruction': 'nop ', 'offset': '0x1b4'},
                                 {'bytes': '90', 'instruction': 'nop ', 'offset': '0x1b5'},
                                 {'bytes': 'dd19', 'instruction': 'fstp qword ptr [ecx]', 'offset': '0x1b6'},
                                 {'bytes': 'dd1a', 'instruction': 'fstp qword ptr [edx]', 'offset': '0x1b8'},
                                 {'bytes': 'dd1b', 'instruction': 'fstp qword ptr [ebx]', 'offset': '0x1ba'},
                                 {'bytes': 'd9d0', 'instruction': 'fnop ', 'offset': '0x1bc'},
                                 {'bytes': 'fecc', 'instruction': 'dec ah', 'offset': '0x1be'},
                                 {'bytes': '80fc00', 'instruction': 'cmp ah, 0', 'offset': '0x1c0'},
                                 {'bytes': '0f8f53feffff', 'instruction': 'jg 0x1c', 'offset': '0x1c3'},
                                 {'bytes': 'dd03', 'instruction': 'fld qword ptr [ebx]', 'offset': '0x1c9'},
                                 {'bytes': 'dc4618',
                                  'instruction': 'fadd qword ptr [esi + 0x18]',
                                  'offset': '0x1cb'},
                                 {'bytes': 'dd02', 'instruction': 'fld qword ptr [edx]', 'offset': '0x1ce'},
                                 {'bytes': 'dc4620',
                                  'instruction': 'fadd qword ptr [esi + 0x20]',
                                  'offset': '0x1d0'},
                                 {'bytes': 'dd01', 'instruction': 'fld qword ptr [ecx]', 'offset': '0x1d3'},
                                 {'bytes': 'dc4628',
                                  'instruction': 'fadd qword ptr [esi + 0x28]',
                                  'offset': '0x1d5'},
                                 {'bytes': 'dd19', 'instruction': 'fstp qword ptr [ecx]', 'offset': '0x1d8'},
                                 {'bytes': 'dd1a', 'instruction': 'fstp qword ptr [edx]', 'offset': '0x1da'},
                                 {'bytes': 'dd1b', 'instruction': 'fstp qword ptr [ebx]', 'offset': '0x1dc'},
                                 {'bytes': '90', 'instruction': 'nop ', 'offset': '0x1de'},
                                 {'bytes': '8bc3', 'instruction': 'mov eax, ebx', 'offset': '0x1df'},
                                 {'bytes': '5f', 'instruction': 'pop edi', 'offset': '0x1e1'},
                                 {'bytes': '5e', 'instruction': 'pop esi', 'offset': '0x1e2'},
                                 {'bytes': '5b', 'instruction': 'pop ebx', 'offset': '0x1e3'},
                                 {'bytes': '89ec', 'instruction': 'mov esp, ebp', 'offset': '0x1e4'},
                                 {'bytes': '5d', 'instruction': 'pop ebp', 'offset': '0x1e6'},
                                 {'bytes': 'c20800', 'instruction': 'ret 8', 'offset': '0x1e7'},
                                 {'bytes': 'ddd8', 'instruction': 'fstp st(0)', 'offset': '0x1ea'},
                                 {'bytes': 'ddd8', 'instruction': 'fstp st(0)', 'offset': '0x1ec'},
                                 {'bytes': 'dd4730',
                                  'instruction': 'fld qword ptr [edi + 0x30]',
                                  'offset': '0x1ee'},
                                 {'bytes': 'd9c0', 'instruction': 'fld st(0)', 'offset': '0x1f1'},
                                 {'bytes': 'd9c0', 'instruction': 'fld st(0)', 'offset': '0x1f3'},
                                 {'bytes': 'ebe1', 'instruction': 'jmp 0x1d8', 'offset': '0x1f5'}],
                'sha256': '802e07e07f1631cd2cbba92301c8c2dfe3584b70b377cc615b23e43091acc835'}}

# Transport descriptors only, not the semantic oracle. Constants reside at
# original positive PVar offsets; mutable J is dynamically appended by caller.
FORMULAS={'Riemann2':(10263,0,19,0,1,0),'TgladTetra':(10269,0,9,0,1,0)}
LAYOUT={'Riemann2':tuple((-12-4*i,'single') for i in range(9))+tuple((-52-8*i,'double') for i in range(8))+((0,'constant'),(8,'constant')),
        'TgladTetra':((-12,'int32'),(-16,'int32'))+tuple((8*i,'constant') for i in range(7))}
DECIMALS={'Riemann2':['3.1415926535898','2.0'],
          'TgladTetra':['0.5','-3.0','2.8284271247461901','0.3535533905932738','3.1819805153394639','2.000','1e10']}
CONSTANTS={name:[tail.binary64(Fraction(v)) for v in values] for name,values in DECIMALS.items()}
NARROW_CONSTANTS={name:list(map(tail.narrow64,values)) for name,values in CONSTANTS.items()}

def finite(bits):
    value=fraction(bits)
    if value and bits&0x7f800000==0:raise ScopeError('subnormal outside bounded projection domain')
    return value

def checked(bits):finite(bits);return bits

def add(a,b):return checked(affine.add(a,b))
def neg(a):return a^NEG_ZERO
def sub(a,b):return add(a,neg(b))
def mul(a,b):return checked(affine.multiply(a,b))
def div(a,b):
    if not finite(b):raise ScopeError('singular projection denominator')
    return checked(fp32(finite(a)/finite(b),bool((a^b)&NEG_ZERO)))

def sqrt_rn(a):
    """Correct RN32 sqrt by exact rational comparisons, no host/device sqrt."""
    value=finite(a)
    if value<0:raise ScopeError('negative radicand')
    if not value:return a
    lo,hi=0,0x7f7fffff
    while lo<hi:
        mid=(lo+hi+1)//2
        if fraction(mid)**2<=value:lo=mid
        else:hi=mid-1
    if fraction(lo)**2==value:return checked(lo)
    midpoint=(fraction(lo)+fraction(lo+1))/2
    return checked(lo if value<midpoint**2 or value==midpoint**2 and lo%2==0 else lo+1)

def exact_power(base,exponent):
    """Original FYL2X/F2XM1/FSCALE only on provable identity/exact exp2 inputs."""
    b,e=finite(base),finite(exponent)
    if b<=0:raise ScopeError('nonpositive power base')
    if e==0 or b==1:return ONE
    # A positive dyadic whose numerator and denominator each are powers of2.
    if b.numerator&(b.numerator-1) or b.denominator&(b.denominator-1):
        raise ScopeError('power base outside exact power-of-two domain')
    exponent2=e*((b.numerator.bit_length()-1)-(b.denominator.bit_length()-1))
    if exponent2.denominator!=1 or not -100<=exponent2<=100:
        raise ScopeError('power exponent outside exact integer-exp2 domain')
    n=int(exponent2)
    return fp32(Fraction(1<<n) if n>=0 else Fraction(1,1<<-n))

def zero_sine(value,unrounded_argument):
    # If product+shift contracts, exact cancellation must still hold.
    if finite(value)!=0 or unrounded_argument!=0:
        raise ScopeError('nonzero sine requires a separately proved transcendental domain')
    return value

def riemann(xyz,params,trace):
    # CODE0x1c..0x59 writes Z first, thenY, thenX from immutable XYZ inputs.
    x,y,z=geometry.matvec(params[:9],xyz)
    r=sqrt_rn(add(mul(x,x),add(mul(y,y),mul(z,z))))
    scale,power,rshift,ssh,tsh,xshift,fold,smallest=params[9:17]
    f=div(scale,r);x,y,z=[mul(v,f) for v in (x,y,z)]
    d=sub(ONE,x)
    # FUCOMPP compares Smallest against d, JB when Smallest<d; equality
    # takes the discontinuity branch. All admitted values are ordered/finite.
    use_reciprocal=finite(smallest)<finite(d)
    q=div(ONE,d) if use_reciprocal else ONE
    trace.append({'op':'riemann_denominator','reciprocal':use_reciprocal})
    s,t=mul(z,q),mul(y,q)
    w=add(add(mul(s,s),mul(t,t)),ONE)
    pi,two=params[17:19]
    def wave(v,shift):
        argument=add(mul(v,pi),shift)
        sine=zero_sine(argument,finite(v)*finite(pi)+finite(shift))
        return add(sine&0x7fffffff,fold)&0x7fffffff
    s,t=wave(s,ssh),wave(t,tsh)
    rr=add(exact_power(r,mul(w,power)),rshift)
    inv=div(two,add(add(mul(s,s),mul(t,t)),ONE))
    return [add(mul(add(neg(inv),ONE),rr),xshift),mul(mul(rr,t),inv),mul(mul(rr,s),inv)]

def tglad(xyz,j,params,trace):
    count,mapcount=(max(1,v&7) for v in params[:2])
    half,minus3,h,k,hplusk,two,limit=params[2:9]
    x,y,z=xyz
    for outer in range(count):
        xy=add(mul(x,x),mul(y,y));radius2=add(xy,mul(z,z))
        trace.append({'op':'tglad_outer','index':outer,'radius2':word(radius2)})
        if finite(radius2)>=finite(limit):
            trace.append({'op':'tglad_radius_guard'})
            return [limit]*3  # Original jump0x1ea skips mutable-J addition.
        r=sqrt_rn(radius2);angle=div(z,r)
        ratio=div(sub(sqrt_rn(xy),y),x);ratio2=mul(ratio,ratio)
        inv=div(ONE,add(ratio2,ONE))
        mag=sqrt_rn(div(sub(ONE,angle),add(ONE,angle)))
        factor=mul(mag,inv)
        x,y=mul(add(factor,factor),ratio),mul(sub(ONE,ratio2),factor)
        for inner in range(mapcount):
            # Opcode order: x*x*x + ((y*y)*(-3))*x, not a reassociated cube.
            S=add(mul(mul(x,x),x),mul(mul(mul(y,y),minus3),x))
            trace.append({'op':'tglad_map','outer':outer,'index':inner,'S':word(S)})
            if finite(S)>=finite(limit):
                trace.append({'op':'tglad_S_guard'})
                return [limit]*3
            T=add(mul(mul(y,y),y),mul(mul(mul(x,x),minus3),y))
            B=mul(T,hplusk);T2=mul(T,T)
            A=sub(mul(add(S,k),sub(h,S)),T2)
            C=div(neg(k),add(mul(add(S,k),add(S,k)),T2))
            nx=mul(add(mul(A,x),neg(mul(B,y))),C)
            ny=mul(add(mul(A,y),mul(B,x)),C)
            x,y=nx,ny;r=mul(r,r)
        xy=add(mul(x,x),mul(y,y));mag=sqrt_rn(xy)
        ratio=div(sub(mag,y),x);ratio2=mul(ratio,ratio)
        inv=div(ONE,add(ratio2,ONE));cy=mul(sub(ONE,ratio2),inv)
        sx=mul(add(ratio,ratio),inv);d=div(two,add(ONE,xy));sf=mul(d,mag)
        x,y,z=mul(mul(r,sx),sf),mul(mul(r,cy),sf),mul(sub(d,ONE),r)
    return [add(v,c) for v,c in zip((x,y,z),j)]

def _point(name,state,params,trace=None):
    out=dict(state);trace=[] if trace is None else trace
    xyz=[state[f'xyz[{i}]'] for i in range(3)]
    result=riemann(xyz,params,trace) if name=='Riemann2' else tglad(xyz,[state[f'j[{i}]'] for i in range(3)],params,trace)
    for i,v in enumerate(result):out[f'xyz[{i}]']=checked(v)
    out['current_slot']=0
    return out

def radius(state):
    x,y,z=[state[f'xyz[{i}]'] for i in range(3)]
    return add(add(mul(x,x),mul(y,y)),mul(z,z))

def _orbit(name,state,params):
    # Original doHybridPas with pinned NEW FPC i386 SSE2 caller transport:
    # W=0, J comes from C or originalJu; no analytic distance or 4D initialization.
    s=dict(state)
    for i in range(3):
        s[f'xyz[{i}]']=s[f'c[{i}]']
        s[f'j[{i}]']=s[f'original_j[{i}]'] if s['julia'] else s[f'c[{i}]']
    s['xyz[3]']=0;s['first']=s['iteration']=0;s['rout']=s['otrap']=radius(s)
    s['sse[0]']=add(mul(s['xyz[0]'],s['xyz[0]']),mul(s['xyz[1]'],s['xyz[1]']))
    s['sse[1]']=s['sse[3]']=0;s['sse[2]']=s['rout']
    count=remaining=s['hybrid_counts[0]'];s['tmp']=remaining
    for _ in range(s['max_iterations']):
        s['rold']=s['rout']
        if remaining==0:remaining=count
        s=_point(name,s,params);remaining-=1;s['tmp']=remaining;s['iteration']+=1;s['rout']=radius(s)
        if finite(s['rout'])<finite(s['otrap']):s['otrap']=s['rout']
        # In the captured PE maxIt JGE bypasses the RStop load, even if
        # bailout is also true; only continuation/bailout-first loads RStop.
        s['sse[0]']=s['rout'] if s['iteration']>=s['max_iterations'] else s['rstop']
        s['sse[1]']=s['sse[3]']=0;s['sse[2]']=s['rout']
        if finite(s['rout'])>finite(s['rstop']):break
    return s

def _program(name,params,state):
    ident,option,count,traits,flags,driver=FORMULAS[name]
    if len(params)!=count:raise ValueError('wrong projection parameter count')
    p=[0]*2048
    p[:15]=[1,driver,6,0,0,0,0,state['max_iterations'],state['julia'],0,0,16384,4096,0,0]
    p[16],p[18],p[21],p[22]=state['rstop'],ONE,fp32(1<<20),ONE
    p[24:28]=[state[f'original_j[{i}]'] for i in range(4)]
    for i in range(4):p[28+5*i]=ONE
    p[50]=state['hybrid_counts[0]']
    for slot in range(6):p[56+slot*332]=1
    b=56;p[b:b+12]=[1,1,ident,option,count,traits,flags,0,0,1,0,0]
    memory=bytearray(512);struct.pack_into('<Q',memory,248,0x3fe0000000000000)
    ci=0
    for i,((offset,kind),v) in enumerate(zip(LAYOUT[name],params)):
        if kind=='int32':
            if type(v) is not int or not -(1<<31)<=v<(1<<31):raise ValueError('int32 parameter out of range')
            original,prepared=tail.binary64(v),fp32(v);struct.pack_into('<i',memory,256+offset,v)
        else:
            original=CONSTANTS[name][ci] if kind=='constant' else geometry.wide32(v)
            if kind=='constant':ci+=1
            prepared=tail.narrow64(original)
            if prepared!=v:raise ValueError('original decimal constant or real metadata changed')
            struct.pack_into('<I' if kind=='single' else '<Q',memory,256+offset,v if kind=='single' else original)
        p[b+12+i]=prepared;p[b+76+2*i:b+78+2*i]=[original&0xffffffff,original>>32]
    p[b+204:b+268]=struct.unpack('<64I',memory[:256])
    for i in range(32):
        p[b+268+i]=tail.narrow64(struct.unpack_from('<Q',memory,248-8*i)[0])
        p[b+300+i]=tail.narrow64(struct.unpack_from('<Q',memory,256+8*i)[0])
    return p

def _parameters(name,p):
    values=p[68:68+FORMULAS[name][2]]
    for i,(_,kind) in enumerate(LAYOUT[name]):
        if kind=='int32':
            value=tail.fraction64(p[132+2*i]|p[133+2*i]<<32)
            if value.denominator!=1 or not -(1<<31)<=value<(1<<31):raise ValueError('typed integer metadata')
            values[i]=int(value)
    return values

def evaluate(payload):
    if set(payload)!={'identity','kind','input_state_bits','parameter_bits'}:raise ValueError('unexpected semantic fields')
    identity=payload['identity']
    if not isinstance(identity,list) or len(identity)!=3 or identity[0]!='external' or identity[2] not in FORMULAS or identity[1]!=FORMULAS[identity[2]][0]:raise ValueError('projection identity')
    def decode(v):
        if not isinstance(v,str) or not re.fullmatch(r'0x[0-9a-f]{8}',v):raise ValueError('expected hex32')
        return int(v,16)
    if set(payload['input_state_bits'])!=set(STATE) or len(payload['parameter_bits'])!=2048:raise ValueError('wrong state/program ABI')
    s={k:decode(v) for k,v in payload['input_state_bits'].items()};p=list(map(decode,payload['parameter_bits']));name=identity[2]
    for lane in STATE[:33]:finite(s[lane])
    if payload['kind'] not in ('point','orbit') or s['julia'] not in (0,1) or s['inside'] or s['calc_smooth'] or not 1<=s['max_iterations']<=4 or not 1<=s['hybrid_counts[0]']<=3:raise ScopeError('unsupported bounded3D caller')
    params=_parameters(name,p)
    if p!=_program(name,params,s):raise ValueError('typed original PVar/constants/program ABI differs')
    if payload['kind']=='orbit' and (s['vary_scale']!=ONE or s['j[3]']!=s['original_j[3]']):raise ScopeError('unsupported3D entry state')
    out=_orbit(name,s,params) if payload['kind']=='orbit' else _point(name,s,params)
    for lane in STATE[:33]:finite(out[lane])
    return {k:word(out[k]) for k in STATE}

def dependency_paths():return [Path(m.__file__).resolve() for m in (affine,geometry,tail)]
def manifest():
    if hashlib.sha256((ROOT/CALLER_PROFILE['path']).read_bytes()).hexdigest() != CALLER_PROFILE['sha256']:
        raise ValueError('pinned new FPC caller proof changed')
    def verify(pins):
        out={}
        for path,h in pins.items():
            data=(ROOT/path).read_bytes()
            if hashlib.sha256(data.replace(b'\r\n',b'\n')).hexdigest()!=h:raise ValueError('pinned original/ABI/dependency changed: '+path)
            out[path]=hashlib.sha256(data).hexdigest()
        return out
    sources,abi=verify(SOURCE_PINS),verify(ABI_PINS)
    dependencies=verify({'comparison/tools/'+k:v for k,v in DEPENDENCY_PINS.items()})
    for name,audit in CODE_AUDIT.items():
        text=(ROOT/'mb3d/M3Formulas'/f'{name}.m3f').read_text();code=bytes.fromhex(re.sub(r'\s+','',re.search(r'\[CODE\](.*?)\[END\]',text,re.S)[1]))
        if len(code)!=audit['bytes'] or hashlib.sha256(code).hexdigest()!=audit['sha256']:raise ValueError('original CODE differs')
        for row in audit['instructions']:
            start=int(row['offset'],16);insn=bytes.fromhex(row['bytes'])
            if code[start:start+len(insn)]!=insn:raise ValueError('original instruction bytes differ')
    return {'schema':'mb3d.native-group04-projection-reference.v2','caller_profile':dict(CALLER_PROFILE),'semantic_sources':sources,'abi_sources':abi,'code_audit':CODE_AUDIT,'dependencies':dependencies,
            'arithmetic':'Original source-ordered scalar RN32 targets; rational RN32 sqrt/division; provably zero sin and exact power identities only',
            'model_limits':['All47 state words; finite normal-or-zero point and bounded single-slot original doHybridPas driver0 orbits.',
            'Point SSE lanes are preserved; orbit transport follows the pinned NEW FPC i386 SSE2 PE profile, not the unrecreated historical executable.',
            'Positive counted iterations set SSE0=Rout at maxIt (also simultaneous bailout), otherwise RStop; SSE2=Rout and SSE1/3=zero.',
            'Prepared matrices are explicit ABI fixtures; only signed-zero Euler preparation is derived. No arbitrary-angle host preparation claim.',
            'Authored real parameters are exact widened binary32; original literal decimal constants are binary64 first, then binary32.',
            'Riemann sine arguments must be exactly zero even before product/add contraction. Nonzero sin is rejected. Power uses only identity or exact integer exp2 inputs.',
            'Tglad interior expectations are source-ordered RN32 targets, not a universal FMA/sqrt/division contract. Device mismatches remain diagnostic and require explanation before admission.',
            'Rounded orbit radii are source-ordered targets, not universally contraction-invariant. No tolerance, learned device expected values or sentinel-only completeness claim.',
            'Tglad masked byte counters execute max(1,int32&7) rounds; early finite guards return1e10 without mutableJ addition.',
            'No mixed-formula,4D,distance scalar,full-scene,CPU/x87 parity or nonfinite/singular projection claim.'],
            'model_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}

def _state():return geometry._state()
def xyz_state(values,lane='xyz'):
    s=_state()
    for i,v in enumerate(values):s[f'{lane}[{i}]']=fp32(v)
    return s

def riemann_params(scale=ONE,power=ONE,rshift=0,ssh=0,tsh=0,xshift=0,fold=0,smallest=0,matrix=None):
    return list(matrix if matrix is not None else geometry.prepare_zero_angles([0,0,0]))+[scale,power,rshift,ssh,tsh,xshift,fold,smallest]+NARROW_CONSTANTS['Riemann2']

def build_cases():
    sources=manifest()['semantic_sources'];cases=[]
    def emit(name,variant,s,params,kind='point'):
        payload={'identity':['external',FORMULAS[name][0],name],'kind':kind,'input_state_bits':{k:word(s[k]) for k in STATE},'parameter_bits':[word(v) for v in _program(name,params,s)]}
        reads={f'xyz[{i}]' for i in range(3)};writes=set(reads)|{'current_slot'}
        if name=='TgladTetra':reads|={f'j[{i}]' for i in range(3)}
        if kind=='orbit':
            reads|={f'{lane}[{i}]' for lane in ('c','original_j') for i in range(4)}|{'julia','max_iterations','rstop','hybrid_counts[0]'}
            writes|={f'xyz[{i}]' for i in range(4)}|{f'j[{i}]' for i in range(3)}|{'rout','rold','otrap','first','iteration','tmp'}|{f'sse[{i}]' for i in range(4)}
        cases.append({'id':f'{FORMULAS[name][0]}-{variant}','identity':payload['identity'],'variant':variant,'input':payload,'expected_state_bits':evaluate(payload),'read_lanes':[k for k in STATE if k in reads],'write_lanes':[k for k in STATE if k in writes],'source_hashes':sources})
    for x in (-4,-2,-1,Fraction(-1,2),Fraction(1,2),1,2,4):
        emit('Riemann2','axis_'+str(x).replace('/','over'),xyz_state((x,0,0)),riemann_params(power=fp32(2)))
    for scale in (-2,0,Fraction(1,2),1,2):
        emit('Riemann2','scale_'+str(scale).replace('/','over'),xyz_state((2,0,0)),riemann_params(scale=fp32(scale)))
    # Nonzero projected s/t are compensated by authored shifts; zero sine
    # is proved from exact rational PI*s+shift, not approximate sin(PI).
    pi=NARROW_CONSTANTS['Riemann2'][0]
    for axis in (1,2):
        for sign in (-1,1):
            v=[0,0,0];v[axis]=sign;shift=neg(mul(pi,fp32(sign)))
            emit('Riemann2',f'projected_axis{axis}_sign{sign}',xyz_state(v),riemann_params(ssh=shift if axis==2 else 0,tsh=shift if axis==1 else 0))
    for r in (Fraction(1,2),2,4):
        emit('Riemann2','nonunit_projected_radius_'+str(r).replace('/','over'),xyz_state((0,0,r)),riemann_params(ssh=neg(pi)))
    for threshold,q in ((Fraction(-3,4),-2),(Fraction(-1,2),1),(Fraction(-1,4),1)):
        # Normalized(1.5,0,2) has denominator-.5. Wrong equality/branch
        # changes the nonzero sine argument, unlike a zero transverse axis.
        emit('Riemann2','observable_threshold_'+str(threshold).replace('/','over'),xyz_state((3,0,4)),riemann_params(scale=fp32(Fraction(5,2)),power=0,smallest=fp32(threshold),ssh=neg(mul(pi,fp32(2*q)))))
    for smallest in (fp32(Fraction(-1,2)),0,fp32(Fraction(1,2))):
        # x normalized1/2 gives d1/2: strict-below vs equal/above branch.
        emit('Riemann2','threshold_'+word(smallest),xyz_state((2,0,0)),riemann_params(scale=fp32(Fraction(1,2)),smallest=smallest))
    for fold in (-1,Fraction(1,2),1,2):
        emit('Riemann2','fold_'+str(fold).replace('/','over'),xyz_state((1,0,0)),riemann_params(fold=fp32(fold),rshift=fp32(Fraction(1,4)),xshift=fp32(-2)))
    for exponent in (0,-1,Fraction(1,2),3):
        emit('Riemann2','power_'+str(exponent).replace('/','over'),xyz_state((4,0,0)),riemann_params(power=fp32(exponent)))
    emit('Riemann2','prepared_axis_permutation',xyz_state((0,2,0)),riemann_params(matrix=list(map(fp32,(0,1,0,0,0,1,1,0,0)))))
    s=xyz_state((-1,0,0));s['xyz[1]']=s['xyz[2]']=NEG_ZERO
    emit('Riemann2','negative_zero_angles_and_inputs',s,riemann_params(matrix=geometry.prepare_zero_angles([NEG_ZERO,0,NEG_ZERO])))
    for i,(x,y,z) in enumerate(((1,0,0),(-1,0,0),(3,4,0),(3,-4,0),(-3,4,0),(-3,-4,0),(4,0,3),(4,0,-3))):
        emit('TgladTetra',f'interior_{i}',xyz_state((x,y,z)),[0,0]+NARROW_CONSTANTS['TgladTetra'])
    for a,b in ((1,1),(2,1),(1,2),(2,2),(3,1),(1,3),(7,1),(1,7),(16777217,16777218),(16777219,1),(1,16777219),(-2147483646,1),(1,-2147483646),(-2147483648,-2147483647),(2147483647,1),(-1,1),(8,9)):
        emit('TgladTetra',f'masked_counts_{a}_{b}',xyz_state((1,0,0)),[a,b]+NARROW_CONSTANTS['TgladTetra'])
    for x in (1,-1):
        emit('TgladTetra',f'finite_S_guard_sign{x}',xyz_state((x,0,-2048)),[0,0]+NARROW_CONSTANTS['TgladTetra'])
    for x in (100000,100001,-100000):
        emit('TgladTetra',f'finite_radius_guard_{x}',xyz_state((x,0,0)),[7,7]+NARROW_CONSTANTS['TgladTetra'])
    for sy,sz in ((0,NEG_ZERO),(NEG_ZERO,0),(NEG_ZERO,NEG_ZERO)):
        s=xyz_state((1,0,0));s['xyz[1]'],s['xyz[2]']=sy,sz
        for i in range(3):s[f'j[{i}]']=0
        emit('TgladTetra',f'signed_zero_y{sy}_z{sz}',s,[0,0]+NARROW_CONSTANTS['TgladTetra'])
    s=xyz_state((1,0,0))
    for i,v in enumerate((4,-2,8)):s[f'j[{i}]']=fp32(v)
    emit('TgladTetra','mutable_j_distinct_from_c_original',s,[0,0]+NARROW_CONSTANTS['TgladTetra'])
    for name in FORMULAS:
        for variant in ('orbit_mandel','orbit_julia','orbit_repeat','orbit_escape','orbit_boundary'):
            s=xyz_state((1,0,0),'c');s['max_iterations']=2;s['julia']=int(variant!='orbit_mandel');s['hybrid_counts[0]']=2 if variant=='orbit_repeat' else 1
            for i in range(3):s[f'original_j[{i}]']=0
            if variant=='orbit_escape':s['rstop']=fp32(Fraction(1,16))
            if variant=='orbit_boundary':s['rstop']=ONE
            params=riemann_params() if name=='Riemann2' else [0,0]+NARROW_CONSTANTS['TgladTetra']
            emit(name,variant,s,params,'orbit')
        s=xyz_state((1,0,0));s['first']=0x80000000;s['iteration']=0x7fffffff;s['tmp']=0xffffffff
        emit(name,'integer_state_sentinels',s,riemann_params() if name=='Riemann2' else [0,0]+NARROW_CONSTANTS['TgladTetra'])
    return cases
