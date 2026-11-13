; Disassembly: _Rpow3_4d.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 41 bytes, decoded: 41 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    DD 41 08                fld      qword ptr [ecx + 8]
000A:    DD 01                   fld      qword ptr [ecx]
000C:    DD 02                   fld      qword ptr [edx]
000E:    DD 00                   fld      qword ptr [eax]
0010:    DD 46 38                fld      qword ptr [esi + 0x38]
0013:    DC CC                   fmul     st(4), st(0)
0015:    DC CB                   fmul     st(3), st(0)
0017:    DC CA                   fmul     st(2), st(0)
0019:    DE C9                   fmulp    st(1)
001B:    DD 18                   fstp     qword ptr [eax]
001D:    DD 1A                   fstp     qword ptr [edx]
001F:    DD 19                   fstp     qword ptr [ecx]
0021:    DD 59 08                fstp     qword ptr [ecx + 8]
0024:    5E                      pop      esi
0025:    5D                      pop      ebp
0026:    C2 08 00                ret      8
