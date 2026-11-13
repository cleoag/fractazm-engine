; Disassembly: Benesi1Pow2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 85 bytes, decoded: 85 bytes (100.0%)

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
0015:    D9 C0                   fld      st(0)
0017:    D8 C2                   fadd     st(2)
0019:    D9 C0                   fld      st(0)
001B:    D9 FA                   fsqrt    ; sqrt(ST(0))
001D:    D8 CC                   fmul     st(4)
001F:    D8 C0                   fadd     st(0)
0021:    D8 F1                   fdiv     st(1)
0023:    DC CD                   fmul     st(5), st(0)
0025:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0027:    DE E2                   fsubrp   st(2)
0029:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
002B:    D8 C8                   fmul     st(0)
002D:    DE E3                   fsubrp   st(3)
002F:    DE C9                   fmulp    st(1)
0031:    D9 E1                   fabs    ; |ST(0)|
0033:    DD 46 20                fld      qword ptr [esi + 0x20]
0036:    D9 E1                   fabs    ; |ST(0)|
0038:    DE E1                   fsubrp   st(1)
003A:    DD 1A                   fstp     qword ptr [edx]
003C:    DC 46 18                fadd     qword ptr [esi + 0x18]
003F:    DD 18                   fstp     qword ptr [eax]
0041:    DE C9                   fmulp    st(1)
0043:    D8 C0                   fadd     st(0)
0045:    D9 E1                   fabs    ; |ST(0)|
0047:    DD 46 28                fld      qword ptr [esi + 0x28]
004A:    D9 E1                   fabs    ; |ST(0)|
004C:    DE E1                   fsubrp   st(1)
004E:    DD 19                   fstp     qword ptr [ecx]
0050:    5E                      pop      esi
0051:    5D                      pop      ebp
0052:    C2 08 00                ret      8
