; Disassembly: _planeFold.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 66 bytes, decoded: 66 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 46 E0                fld      qword ptr [esi - 0x20]
000D:    DD 46 E8                fld      qword ptr [esi - 0x18]
0010:    DD 46 F0                fld      qword ptr [esi - 0x10]
0013:    DD 01                   fld      qword ptr [ecx]
0015:    DD 02                   fld      qword ptr [edx]
0017:    DD 00                   fld      qword ptr [eax]
0019:    D9 C0                   fld      st(0)
001B:    D8 CC                   fmul     st(4)
001D:    D9 C2                   fld      st(2)
001F:    D8 CE                   fmul     st(6)
0021:    DE C1                   faddp    st(1)
0023:    D9 C3                   fld      st(3)
0025:    D8 CF                   fmul     st(7)
0027:    DE C1                   faddp    st(1)
0029:    D8 C0                   fadd     st(0)
002B:    DC CC                   fmul     st(4), st(0)
002D:    DC CD                   fmul     st(5), st(0)
002F:    DE CE                   fmulp    st(6)
0031:    DE E3                   fsubrp   st(3)
0033:    DE E3                   fsubrp   st(3)
0035:    DE E3                   fsubrp   st(3)
0037:    DD 18                   fstp     qword ptr [eax]
0039:    DD 1A                   fstp     qword ptr [edx]
003B:    DD 19                   fstp     qword ptr [ecx]
003D:    5E                      pop      esi
003E:    5D                      pop      ebp
003F:    C2 08 00                ret      8
