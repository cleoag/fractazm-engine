; Disassembly: Benesi2Pow6.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 196 bytes, decoded: 196 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 01                   fld      qword ptr [ecx]
000D:    DD 02                   fld      qword ptr [edx]
000F:    DD 00                   fld      qword ptr [eax]
0011:    D9 C1                   fld      st(1)
0013:    D8 CA                   fmul     st(2)
0015:    D9 C3                   fld      st(3)
0017:    D8 CC                   fmul     st(4)
0019:    D8 C1                   fadd     st(1)
001B:    D9 C2                   fld      st(2)
001D:    D8 CB                   fmul     st(3)
001F:    DE C2                   faddp    st(2)
0021:    D9 C1                   fld      st(1)
0023:    D8 CA                   fmul     st(2)
0025:    D8 CA                   fmul     st(2)
0027:    D9 E8                   fld1    ; Load 1.0
0029:    DE F1                   fdivrp   st(1)
002B:    D9 C3                   fld      st(3)
002D:    D8 CC                   fmul     st(4)
002F:    D9 C0                   fld      st(0)
0031:    D8 CB                   fmul     st(3)
0033:    DC 0F                   fmul     qword ptr [edi]
0035:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0037:    DE E3                   fsubrp   st(3)
0039:    D9 C2                   fld      st(2)
003B:    D8 CB                   fmul     st(3)
003D:    DE E1                   fsubrp   st(1)
003F:    DE CA                   fmulp    st(2)
0041:    D9 C2                   fld      st(2)
0043:    D9 FA                   fsqrt    ; sqrt(ST(0))
0045:    D9 C6                   fld      st(6)
0047:    D8 C8                   fmul     st(0)
0049:    DE EC                   fsubp    st(4)
004B:    D8 C0                   fadd     st(0)
004D:    DE CE                   fmulp    st(6)
004F:    D9 C2                   fld      st(2)
0051:    D8 C8                   fmul     st(0)
0053:    D9 C6                   fld      st(6)
0055:    D8 C8                   fmul     st(0)
0057:    DE E9                   fsubp    st(1)
0059:    D9 C6                   fld      st(6)
005B:    D8 CC                   fmul     st(4)
005D:    D8 C0                   fadd     st(0)
005F:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0061:    DC 46 18                fadd     qword ptr [esi + 0x18]
0064:    DD 18                   fstp     qword ptr [eax]
0066:    D9 C2                   fld      st(2)
0068:    D8 CC                   fmul     st(4)
006A:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
006C:    DC CC                   fmul     st(4), st(0)
006E:    D8 CF                   fmul     st(7)
0070:    DE C1                   faddp    st(1)
0072:    D9 CE                   fxch     st(6)    ; Exchange ST(0) and st(6)
0074:    DE CA                   fmulp    st(2)
0076:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0078:    DE EA                   fsubp    st(2)
007A:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
007C:    D9 C0                   fld      st(0)
007E:    D8 C8                   fmul     st(0)
0080:    D9 C4                   fld      st(4)
0082:    D8 C8                   fmul     st(0)
0084:    DE E9                   fsubp    st(1)
0086:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
0088:    DE C9                   fmulp    st(1)
008A:    D8 C0                   fadd     st(0)
008C:    D9 C0                   fld      st(0)
008E:    D8 C8                   fmul     st(0)
0090:    D9 C4                   fld      st(4)
0092:    D8 C8                   fmul     st(0)
0094:    DE E1                   fsubrp   st(1)
0096:    D9 C1                   fld      st(1)
0098:    D8 CD                   fmul     st(5)
009A:    D8 C0                   fadd     st(0)
009C:    DE CD                   fmulp    st(5)
009E:    DE C9                   fmulp    st(1)
00A0:    DE C3                   faddp    st(3)
00A2:    DE CA                   fmulp    st(2)
00A4:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00A6:    D9 E1                   fabs    ; |ST(0)|
00A8:    DE C9                   fmulp    st(1)
00AA:    DD 46 28                fld      qword ptr [esi + 0x28]
00AD:    D9 E1                   fabs    ; |ST(0)|
00AF:    DE E1                   fsubrp   st(1)
00B1:    DD 19                   fstp     qword ptr [ecx]
00B3:    D9 E1                   fabs    ; |ST(0)|
00B5:    DD 46 20                fld      qword ptr [esi + 0x20]
00B8:    D9 E1                   fabs    ; |ST(0)|
00BA:    DE E1                   fsubrp   st(1)
00BC:    DD 1A                   fstp     qword ptr [edx]
00BE:    5F                      pop      edi
00BF:    5E                      pop      esi
00C0:    5D                      pop      ebp
00C1:    C2 08 00                ret      8
