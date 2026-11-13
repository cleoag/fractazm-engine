; Disassembly: _Rpow3.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 33 bytes, decoded: 33 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    DD 01                   fld      qword ptr [ecx]
0009:    DD 02                   fld      qword ptr [edx]
000B:    DD 00                   fld      qword ptr [eax]
000D:    DD 46 38                fld      qword ptr [esi + 0x38]
0010:    DC CB                   fmul     st(3), st(0)
0012:    DC CA                   fmul     st(2), st(0)
0014:    DE C9                   fmulp    st(1)
0016:    DD 18                   fstp     qword ptr [eax]
0018:    DD 1A                   fstp     qword ptr [edx]
001A:    DD 19                   fstp     qword ptr [ecx]
001C:    5E                      pop      esi
001D:    5D                      pop      ebp
001E:    C2 08 00                ret      8
