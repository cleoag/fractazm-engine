; Disassembly: _Folding1.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 80 bytes, decoded: 80 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0007:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000A:    DD 46 F0                fld      qword ptr [esi - 0x10]
000D:    DD 01                   fld      qword ptr [ecx]
000F:    D9 C0                   fld      st(0)
0011:    D8 E2                   fsub     st(2)
0013:    D9 E1                   fabs    ; |ST(0)|
0015:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0017:    D8 C2                   fadd     st(2)
0019:    D9 E1                   fabs    ; |ST(0)|
001B:    DE E1                   fsubrp   st(1)
001D:    DC 21                   fsub     qword ptr [ecx]
001F:    DD 19                   fstp     qword ptr [ecx]
0021:    DD 02                   fld      qword ptr [edx]
0023:    D9 C0                   fld      st(0)
0025:    D8 E2                   fsub     st(2)
0027:    D9 E1                   fabs    ; |ST(0)|
0029:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
002B:    D8 C2                   fadd     st(2)
002D:    D9 E1                   fabs    ; |ST(0)|
002F:    DE E1                   fsubrp   st(1)
0031:    DC 22                   fsub     qword ptr [edx]
0033:    DD 1A                   fstp     qword ptr [edx]
0035:    DD 00                   fld      qword ptr [eax]
0037:    D9 C0                   fld      st(0)
0039:    D8 E2                   fsub     st(2)
003B:    D9 E1                   fabs    ; |ST(0)|
003D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
003F:    D8 C2                   fadd     st(2)
0041:    D9 E1                   fabs    ; |ST(0)|
0043:    DE E1                   fsubrp   st(1)
0045:    DC 20                   fsub     qword ptr [eax]
0047:    DD 18                   fstp     qword ptr [eax]
0049:    DD D8                   fstp     st(0)
004B:    5E                      pop      esi
004C:    5D                      pop      ebp
004D:    C2 08 00                ret      8
