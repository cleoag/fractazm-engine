; Disassembly: MengerIFSplus.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 223 bytes, decoded: 223 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    56                      push     esi
0005:    89 C3                   mov      ebx, eax
0007:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    DD 01                   fld      qword ptr [ecx]
000F:    D9 E1                   fabs    ; |ST(0)|
0011:    DD 02                   fld      qword ptr [edx]
0013:    D9 E1                   fabs    ; |ST(0)|
0015:    DD 03                   fld      qword ptr [ebx]
0017:    D9 E1                   fabs    ; |ST(0)|
0019:    D8 D1                   fcom     st(1)
001B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
001D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
001F:    73 02                   jae      0x23    ; Conditional jump
0021:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0023:    D8 D2                   fcom     st(2)
0025:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0027:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0029:    73 02                   jae      0x2d    ; Conditional jump
002B:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
002D:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
002F:    D8 D2                   fcom     st(2)
0031:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0033:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0035:    73 02                   jae      0x39    ; Conditional jump
0037:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0039:    90                      nop
003A:    DD 46 AC                fld      qword ptr [esi - 0x54]
003D:    DC C2                   fadd     st(2), st(0)
003F:    DD D8                   fstp     st(0)
0041:    DD 46 A4                fld      qword ptr [esi - 0x5c]
0044:    DC C1                   fadd     st(1), st(0)
0046:    DD D8                   fstp     st(0)
0048:    DD 46 9C                fld      qword ptr [esi - 0x64]
004B:    DC C3                   fadd     st(3), st(0)
004D:    DD D8                   fstp     st(0)
004F:    D9 C1                   fld      st(1)
0051:    D8 4E BC                fmul     dword ptr [esi - 0x44]
0054:    D9 C1                   fld      st(1)
0056:    D8 4E B8                fmul     dword ptr [esi - 0x48]
0059:    DE C1                   faddp    st(1)
005B:    D9 C3                   fld      st(3)
005D:    D8 4E B4                fmul     dword ptr [esi - 0x4c]
0060:    DE C1                   faddp    st(1)
0062:    D9 C2                   fld      st(2)
0064:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
0067:    D9 C2                   fld      st(2)
0069:    D8 4E D0                fmul     dword ptr [esi - 0x30]
006C:    DE C1                   faddp    st(1)
006E:    D9 C4                   fld      st(4)
0070:    D8 4E CC                fmul     dword ptr [esi - 0x34]
0073:    DE C1                   faddp    st(1)
0075:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0077:    D8 4E C8                fmul     dword ptr [esi - 0x38]
007A:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
007C:    D8 4E C4                fmul     dword ptr [esi - 0x3c]
007F:    DE C2                   faddp    st(2)
0081:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0083:    D8 4E C0                fmul     dword ptr [esi - 0x40]
0086:    DE C1                   faddp    st(1)
0088:    90                      nop
0089:    DD 46 AC                fld      qword ptr [esi - 0x54]
008C:    DC EA                   fsub     st(2), st(0)
008E:    DD D8                   fstp     st(0)
0090:    DD 46 A4                fld      qword ptr [esi - 0x5c]
0093:    DC E9                   fsub     st(1), st(0)
0095:    DD D8                   fstp     st(0)
0097:    DD 46 9C                fld      qword ptr [esi - 0x64]
009A:    DC EB                   fsub     st(3), st(0)
009C:    DD D8                   fstp     st(0)
009E:    90                      nop
009F:    DD 46 F0                fld      qword ptr [esi - 0x10]
00A2:    DD 41 08                fld      qword ptr [ecx + 8]
00A5:    D8 C9                   fmul     st(1)
00A7:    DD 59 08                fstp     qword ptr [ecx + 8]
00AA:    DC CA                   fmul     st(2), st(0)
00AC:    DC C9                   fmul     st(1), st(0)
00AE:    D9 C0                   fld      st(0)
00B0:    D9 E8                   fld1    ; Load 1.0
00B2:    DE E9                   fsubp    st(1)
00B4:    D9 C0                   fld      st(0)
00B6:    D9 C0                   fld      st(0)
00B8:    DC 4E E8                fmul     qword ptr [esi - 0x18]
00BB:    DE ED                   fsubp    st(5)
00BD:    DC 4E E0                fmul     qword ptr [esi - 0x20]
00C0:    DE EB                   fsubp    st(3)
00C2:    D8 F1                   fdiv     st(1)
00C4:    DC 4E D8                fmul     qword ptr [esi - 0x28]
00C7:    DC EC                   fsub     st(4), st(0)
00C9:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
00CB:    D9 E1                   fabs    ; |ST(0)|
00CD:    DE EC                   fsubp    st(4)
00CF:    DE CB                   fmulp    st(3)
00D1:    DD 1A                   fstp     qword ptr [edx]
00D3:    DD 1B                   fstp     qword ptr [ebx]
00D5:    DD 19                   fstp     qword ptr [ecx]
00D7:    89 D8                   mov      eax, ebx
00D9:    5E                      pop      esi
00DA:    5B                      pop      ebx
00DB:    5D                      pop      ebp
00DC:    C2 08 00                ret      8
