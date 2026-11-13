; Disassembly: Benesi2Pow2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 89 bytes, decoded: 89 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    DD 01                   fld      qword ptr [ecx]
0006:    DD 02                   fld      qword ptr [edx]
0008:    DD 00                   fld      qword ptr [eax]
000A:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000D:    D9 C1                   fld      st(1)
000F:    D8 CA                   fmul     st(2)
0011:    D9 C3                   fld      st(3)
0013:    D8 CC                   fmul     st(4)
0015:    D9 C2                   fld      st(2)
0017:    D8 CB                   fmul     st(3)
0019:    D9 C0                   fld      st(0)
001B:    D8 E2                   fsub     st(2)
001D:    D8 E3                   fsub     st(3)
001F:    DC 46 18                fadd     qword ptr [esi + 0x18]
0022:    DD 18                   fstp     qword ptr [eax]
0024:    DE C2                   faddp    st(2)
0026:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0028:    DE CA                   fmulp    st(2)
002A:    D9 C0                   fld      st(0)
002C:    D9 FA                   fsqrt    ; sqrt(ST(0))
002E:    DE CC                   fmulp    st(4)
0030:    D9 C0                   fld      st(0)
0032:    DE E3                   fsubrp   st(3)
0034:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0036:    D8 C0                   fadd     st(0)
0038:    D9 E1                   fabs    ; |ST(0)|
003A:    DD 46 20                fld      qword ptr [esi + 0x20]
003D:    D9 E1                   fabs    ; |ST(0)|
003F:    DE E1                   fsubrp   st(1)
0041:    DD 1A                   fstp     qword ptr [edx]
0043:    D8 C0                   fadd     st(0)
0045:    DE C9                   fmulp    st(1)
0047:    DE F1                   fdivrp   st(1)
0049:    D9 E1                   fabs    ; |ST(0)|
004B:    DD 46 28                fld      qword ptr [esi + 0x28]
004E:    D9 E1                   fabs    ; |ST(0)|
0050:    DE E1                   fsubrp   st(1)
0052:    DD 19                   fstp     qword ptr [ecx]
0054:    5E                      pop      esi
0055:    5D                      pop      ebp
0056:    C2 08 00                ret      8
