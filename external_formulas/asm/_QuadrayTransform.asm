; Disassembly: _QuadrayTransform.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 92 bytes, decoded: 92 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    DD 01                   fld      qword ptr [ecx]
0009:    DD 02                   fld      qword ptr [edx]
000B:    DD 00                   fld      qword ptr [eax]
000D:    D9 C0                   fld      st(0)
000F:    D8 C2                   fadd     st(2)
0011:    D8 C3                   fadd     st(3)
0013:    DD 18                   fstp     qword ptr [eax]
0015:    D9 C2                   fld      st(2)
0017:    D8 E1                   fsub     st(1)
0019:    D8 E2                   fsub     st(2)
001B:    DD 1A                   fstp     qword ptr [edx]
001D:    D9 C1                   fld      st(1)
001F:    D8 E1                   fsub     st(1)
0021:    D8 E3                   fsub     st(3)
0023:    DD 19                   fstp     qword ptr [ecx]
0025:    DE E1                   fsubrp   st(1)
0027:    DE E1                   fsubrp   st(1)
0029:    DD 59 08                fstp     qword ptr [ecx + 8]
002C:    DD 46 28                fld      qword ptr [esi + 0x28]
002F:    DD 46 20                fld      qword ptr [esi + 0x20]
0032:    DD 46 18                fld      qword ptr [esi + 0x18]
0035:    D9 C0                   fld      st(0)
0037:    D8 C2                   fadd     st(2)
0039:    D8 C3                   fadd     st(3)
003B:    DD 5E 18                fstp     qword ptr [esi + 0x18]
003E:    D9 C2                   fld      st(2)
0040:    D8 E1                   fsub     st(1)
0042:    D8 E2                   fsub     st(2)
0044:    DD 5E 20                fstp     qword ptr [esi + 0x20]
0047:    D9 C1                   fld      st(1)
0049:    D8 E1                   fsub     st(1)
004B:    D8 E3                   fsub     st(3)
004D:    DD 5E 28                fstp     qword ptr [esi + 0x28]
0050:    DE E1                   fsubrp   st(1)
0052:    DE E1                   fsubrp   st(1)
0054:    DD 5E C8                fstp     qword ptr [esi - 0x38]
0057:    5E                      pop      esi
0058:    5D                      pop      ebp
0059:    C2 08 00                ret      8
