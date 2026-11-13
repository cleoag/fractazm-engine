; Disassembly: Riemann.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 195 bytes, decoded: 195 bytes (100.0%)

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    57                      push     edi
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
000B:    DD 00                   fld      qword ptr [eax]
000D:    DD 02                   fld      qword ptr [edx]
000F:    DD 01                   fld      qword ptr [ecx]
0011:    DD 46 38                fld      qword ptr [esi + 0x38]
0014:    D9 FA                   fsqrt    ; sqrt(ST(0))
0016:    D9 E8                   fld1    ; Load 1.0
0018:    DE F1                   fdivrp   st(1)
001A:    DC CB                   fmul     st(3), st(0)
001C:    DC CA                   fmul     st(2), st(0)
001E:    DE C9                   fmulp    st(1)
0020:    D9 E8                   fld1    ; Load 1.0
0022:    DC EA                   fsub     st(2), st(0)
0024:    DC F2                   fdivr    st(2), st(0)
0026:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0028:    DC CB                   fmul     st(3), st(0)
002A:    DE C9                   fmulp    st(1)
002C:    D9 C2                   fld      st(2)
002E:    D8 CB                   fmul     st(3)
0030:    D8 E2                   fsub     st(2)
0032:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0034:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0036:    D8 C0                   fadd     st(0)
0038:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
003A:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
003C:    D9 C2                   fld      st(2)
003E:    D8 CB                   fmul     st(3)
0040:    D8 E2                   fsub     st(2)
0042:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0044:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0046:    D8 C0                   fadd     st(0)
0048:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
004A:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
004C:    DD 47 F0                fld      qword ptr [edi - 0x10]
004F:    DC CB                   fmul     st(3), st(0)
0051:    DE C9                   fmulp    st(1)
0053:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0055:    D8 C2                   fadd     st(2)
0057:    DE F9                   fdivp    st(1)
0059:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
005B:    D9 FB                   fsincos    ; ST(0)=cos, push sin
005D:    D8 C2                   fadd     st(2)
005F:    DE F9                   fdivp    st(1)
0061:    D9 C0                   fld      st(0)
0063:    D8 C9                   fmul     st(1)
0065:    D9 C3                   fld      st(3)
0067:    D8 CC                   fmul     st(4)
0069:    DE C1                   faddp    st(1)
006B:    D9 C0                   fld      st(0)
006D:    D8 C3                   fadd     st(3)
006F:    D8 FB                   fdivr    st(3)
0071:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0073:    DE E3                   fsubrp   st(3)
0075:    DC CB                   fmul     st(3), st(0)
0077:    DC CA                   fmul     st(2), st(0)
0079:    DE C9                   fmulp    st(1)
007B:    DD 47 F0                fld      qword ptr [edi - 0x10]
007E:    DC 4F F8                fmul     qword ptr [edi - 8]
0081:    DD 46 38                fld      qword ptr [esi + 0x38]
0084:    D9 ED                   fldln2    ; Load ln(2)
0086:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0088:    D9 F1                   fyl2x
008A:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
008C:    DE C9                   fmulp    st(1)
008E:    D9 EA                   fldl2e    ; Load log2(e)
0090:    DE C9                   fmulp    st(1)
0092:    D9 C0                   fld      st(0)
0094:    D9 FC                   frndint    ; round(ST(0))
0096:    DC E9                   fsub     st(1), st(0)
0098:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
009A:    D9 F0                   f2xm1
009C:    D9 E8                   fld1    ; Load 1.0
009E:    DE C1                   faddp    st(1)
00A0:    D9 FD                   fscale
00A2:    DD D9                   fstp     st(1)
00A4:    DC CB                   fmul     st(3), st(0)
00A6:    DC CA                   fmul     st(2), st(0)
00A8:    DE C9                   fmulp    st(1)
00AA:    D8 C0                   fadd     st(0)
00AC:    DC 46 18                fadd     qword ptr [esi + 0x18]
00AF:    DD 18                   fstp     qword ptr [eax]
00B1:    DC 46 20                fadd     qword ptr [esi + 0x20]
00B4:    DD 1A                   fstp     qword ptr [edx]
00B6:    D8 C0                   fadd     st(0)
00B8:    DC 46 28                fadd     qword ptr [esi + 0x28]
00BB:    DD 19                   fstp     qword ptr [ecx]
00BD:    5F                      pop      edi
00BE:    5E                      pop      esi
00BF:    5D                      pop      ebp
00C0:    C2 08 00                ret      8
