; Disassembly: _ScaleC4d.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 81 bytes, decoded: 81 bytes (100.0%)
; DEoption: 5

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 41 08                fld      qword ptr [ecx + 8]
000E:    DD 01                   fld      qword ptr [ecx]
0010:    DD 02                   fld      qword ptr [edx]
0012:    DD 00                   fld      qword ptr [eax]
0014:    DD 47 F0                fld      qword ptr [edi - 0x10]
0017:    DC CC                   fmul     st(4), st(0)
0019:    DC CB                   fmul     st(3), st(0)
001B:    DC CA                   fmul     st(2), st(0)
001D:    DE C9                   fmulp    st(1)
001F:    DD 18                   fstp     qword ptr [eax]
0021:    DD 1A                   fstp     qword ptr [edx]
0023:    DD 19                   fstp     qword ptr [ecx]
0025:    DD 59 08                fstp     qword ptr [ecx + 8]
0028:    DD 46 C8                fld      qword ptr [esi - 0x38]
002B:    DD 46 28                fld      qword ptr [esi + 0x28]
002E:    DD 46 20                fld      qword ptr [esi + 0x20]
0031:    DD 46 18                fld      qword ptr [esi + 0x18]
0034:    DD 47 E8                fld      qword ptr [edi - 0x18]
0037:    DC CC                   fmul     st(4), st(0)
0039:    DC CB                   fmul     st(3), st(0)
003B:    DC CA                   fmul     st(2), st(0)
003D:    DE C9                   fmulp    st(1)
003F:    DD 5E 18                fstp     qword ptr [esi + 0x18]
0042:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0045:    DD 5E 28                fstp     qword ptr [esi + 0x28]
0048:    DD 5E C8                fstp     qword ptr [esi - 0x38]
004B:    5F                      pop      edi
004C:    5E                      pop      esi
004D:    5D                      pop      ebp
004E:    C2 08 00                ret      8
