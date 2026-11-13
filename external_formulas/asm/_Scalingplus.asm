; Disassembly: _Scalingplus.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 53 bytes, decoded: 53 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 41 08                fld      qword ptr [ecx + 8]
000D:    DD 46 D8                fld      qword ptr [esi - 0x28]
0010:    DE C9                   fmulp    st(1)
0012:    DD 59 08                fstp     qword ptr [ecx + 8]
0015:    DD 01                   fld      qword ptr [ecx]
0017:    DD 46 E0                fld      qword ptr [esi - 0x20]
001A:    DE C9                   fmulp    st(1)
001C:    DD 19                   fstp     qword ptr [ecx]
001E:    DD 02                   fld      qword ptr [edx]
0020:    DD 46 E8                fld      qword ptr [esi - 0x18]
0023:    DE C9                   fmulp    st(1)
0025:    DD 1A                   fstp     qword ptr [edx]
0027:    DD 00                   fld      qword ptr [eax]
0029:    DD 46 F0                fld      qword ptr [esi - 0x10]
002C:    DE C9                   fmulp    st(1)
002E:    DD 18                   fstp     qword ptr [eax]
0030:    5E                      pop      esi
0031:    5D                      pop      ebp
0032:    C2 08 00                ret      8
