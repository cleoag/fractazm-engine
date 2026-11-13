; Disassembly: _Scaling.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 44 bytes, decoded: 44 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 41 08                fld      qword ptr [ecx + 8]
000D:    DD 01                   fld      qword ptr [ecx]
000F:    DD 02                   fld      qword ptr [edx]
0011:    DD 00                   fld      qword ptr [eax]
0013:    DD 46 F0                fld      qword ptr [esi - 0x10]
0016:    DC CC                   fmul     st(4), st(0)
0018:    DC CB                   fmul     st(3), st(0)
001A:    DC CA                   fmul     st(2), st(0)
001C:    DE C9                   fmulp    st(1)
001E:    DD 18                   fstp     qword ptr [eax]
0020:    DD 1A                   fstp     qword ptr [edx]
0022:    DD 19                   fstp     qword ptr [ecx]
0024:    DD 59 08                fstp     qword ptr [ecx + 8]
0027:    5E                      pop      esi
0028:    5D                      pop      ebp
0029:    C2 08 00                ret      8
