; Disassembly: MengerIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 178 bytes, decoded: 178 bytes (100.0%)
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
0039:    D9 C1                   fld      st(1)
003B:    D8 4E BC                fmul     dword ptr [esi - 0x44]
003E:    D9 C1                   fld      st(1)
0040:    D8 4E B8                fmul     dword ptr [esi - 0x48]
0043:    DE C1                   faddp    st(1)
0045:    D9 C3                   fld      st(3)
0047:    D8 4E B4                fmul     dword ptr [esi - 0x4c]
004A:    DE C1                   faddp    st(1)
004C:    D9 C2                   fld      st(2)
004E:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
0051:    D9 C2                   fld      st(2)
0053:    D8 4E D0                fmul     dword ptr [esi - 0x30]
0056:    DE C1                   faddp    st(1)
0058:    D9 C4                   fld      st(4)
005A:    D8 4E CC                fmul     dword ptr [esi - 0x34]
005D:    DE C1                   faddp    st(1)
005F:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0061:    D8 4E C8                fmul     dword ptr [esi - 0x38]
0064:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0066:    D8 4E C4                fmul     dword ptr [esi - 0x3c]
0069:    DE C2                   faddp    st(2)
006B:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
006D:    D8 4E C0                fmul     dword ptr [esi - 0x40]
0070:    DE C1                   faddp    st(1)
0072:    DD 46 F0                fld      qword ptr [esi - 0x10]
0075:    DD 41 08                fld      qword ptr [ecx + 8]
0078:    D8 C9                   fmul     st(1)
007A:    DD 59 08                fstp     qword ptr [ecx + 8]
007D:    DC CA                   fmul     st(2), st(0)
007F:    DC C9                   fmul     st(1), st(0)
0081:    D9 C0                   fld      st(0)
0083:    D9 E8                   fld1    ; Load 1.0
0085:    DE E9                   fsubp    st(1)
0087:    D9 C0                   fld      st(0)
0089:    D9 C0                   fld      st(0)
008B:    DC 4E E8                fmul     qword ptr [esi - 0x18]
008E:    DE ED                   fsubp    st(5)
0090:    DC 4E E0                fmul     qword ptr [esi - 0x20]
0093:    DE EB                   fsubp    st(3)
0095:    D8 F1                   fdiv     st(1)
0097:    DC 4E D8                fmul     qword ptr [esi - 0x28]
009A:    DC EC                   fsub     st(4), st(0)
009C:    D9 CC                   fxch     st(4)    ; Exchange ST(0) and st(4)
009E:    D9 E1                   fabs    ; |ST(0)|
00A0:    DE EC                   fsubp    st(4)
00A2:    DE CB                   fmulp    st(3)
00A4:    DD 1A                   fstp     qword ptr [edx]
00A6:    DD 1B                   fstp     qword ptr [ebx]
00A8:    DD 19                   fstp     qword ptr [ecx]
00AA:    89 D8                   mov      eax, ebx
00AC:    5E                      pop      esi
00AD:    5B                      pop      ebx
00AE:    5D                      pop      ebp
00AF:    C2 08 00                ret      8
