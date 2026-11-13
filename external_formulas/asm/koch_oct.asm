; Disassembly: koch_oct.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 261 bytes, decoded: 261 bytes (100.0%)
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
0019:    D9 C0                   fld      st(0)
001B:    D8 4E BC                fmul     dword ptr [esi - 0x44]
001E:    D9 C2                   fld      st(2)
0020:    D8 4E B8                fmul     dword ptr [esi - 0x48]
0023:    DE C1                   faddp    st(1)
0025:    D9 C3                   fld      st(3)
0027:    D8 4E B4                fmul     dword ptr [esi - 0x4c]
002A:    DE C1                   faddp    st(1)
002C:    D9 C1                   fld      st(1)
002E:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
0031:    D9 C3                   fld      st(3)
0033:    D8 4E D0                fmul     dword ptr [esi - 0x30]
0036:    DE C1                   faddp    st(1)
0038:    D9 C4                   fld      st(4)
003A:    D8 4E CC                fmul     dword ptr [esi - 0x34]
003D:    DE C1                   faddp    st(1)
003F:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0041:    D8 4E C8                fmul     dword ptr [esi - 0x38]
0044:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0046:    D8 4E C4                fmul     dword ptr [esi - 0x3c]
0049:    DE C3                   faddp    st(3)
004B:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
004D:    D8 4E C0                fmul     dword ptr [esi - 0x40]
0050:    DE C2                   faddp    st(2)
0052:    90                      nop
0053:    90                      nop
0054:    D8 D1                   fcom     st(1)
0056:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0058:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
005A:    72 02                   jb       0x5e    ; Conditional jump
005C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
005E:    D8 D2                   fcom     st(2)
0060:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0062:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0064:    72 02                   jb       0x68    ; Conditional jump
0066:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0068:    90                      nop
0069:    90                      nop
006A:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
006C:    90                      nop
006D:    90                      nop
006E:    D8 D2                   fcom     st(2)
0070:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0072:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0074:    72 02                   jb       0x78    ; Conditional jump
0076:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0078:    90                      nop
0079:    90                      nop
007A:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
007C:    D9 C1                   fld      st(1)
007E:    D8 4E 98                fmul     dword ptr [esi - 0x68]
0081:    D9 C1                   fld      st(1)
0083:    D8 4E 94                fmul     dword ptr [esi - 0x6c]
0086:    DE C1                   faddp    st(1)
0088:    D9 C3                   fld      st(3)
008A:    D8 4E 90                fmul     dword ptr [esi - 0x70]
008D:    DE C1                   faddp    st(1)
008F:    D9 C2                   fld      st(2)
0091:    D8 4E B0                fmul     dword ptr [esi - 0x50]
0094:    D9 C2                   fld      st(2)
0096:    D8 4E AC                fmul     dword ptr [esi - 0x54]
0099:    DE C1                   faddp    st(1)
009B:    D9 C4                   fld      st(4)
009D:    D8 4E A8                fmul     dword ptr [esi - 0x58]
00A0:    DE C1                   faddp    st(1)
00A2:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00A4:    D8 4E A4                fmul     dword ptr [esi - 0x5c]
00A7:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00A9:    D8 4E A0                fmul     dword ptr [esi - 0x60]
00AC:    DE C2                   faddp    st(2)
00AE:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00B0:    D8 4E 9C                fmul     dword ptr [esi - 0x64]
00B3:    DE C1                   faddp    st(1)
00B5:    DD 46 F0                fld      qword ptr [esi - 0x10]
00B8:    DD 41 08                fld      qword ptr [ecx + 8]
00BB:    D8 C9                   fmul     st(1)
00BD:    DD 59 08                fstp     qword ptr [ecx + 8]
00C0:    DC CB                   fmul     st(3), st(0)
00C2:    DC CA                   fmul     st(2), st(0)
00C4:    DC C9                   fmul     st(1), st(0)
00C6:    90                      nop
00C7:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00C9:    DD 46 88                fld      qword ptr [esi - 0x78]
00CC:    D8 C1                   fadd     st(1)
00CE:    D9 E1                   fabs    ; |ST(0)|
00D0:    DD 46 88                fld      qword ptr [esi - 0x78]
00D3:    D8 E2                   fsub     st(2)
00D5:    D9 E1                   fabs    ; |ST(0)|
00D7:    D9 E0                   fchs    ; -ST(0)
00D9:    DE C1                   faddp    st(1)
00DB:    DE E1                   fsubrp   st(1)
00DD:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00DF:    90                      nop
00E0:    D9 E8                   fld1    ; Load 1.0
00E2:    DE E9                   fsubp    st(1)
00E4:    D9 C0                   fld      st(0)
00E6:    D9 C0                   fld      st(0)
00E8:    DC 4E E0                fmul     qword ptr [esi - 0x20]
00EB:    DE EC                   fsubp    st(4)
00ED:    DC 4E E8                fmul     qword ptr [esi - 0x18]
00F0:    DE EA                   fsubp    st(2)
00F2:    DC 4E D8                fmul     qword ptr [esi - 0x28]
00F5:    DE EB                   fsubp    st(3)
00F7:    DD 1B                   fstp     qword ptr [ebx]
00F9:    DD 1A                   fstp     qword ptr [edx]
00FB:    DD 19                   fstp     qword ptr [ecx]
00FD:    89 D8                   mov      eax, ebx
00FF:    5E                      pop      esi
0100:    5B                      pop      ebx
0101:    5D                      pop      ebp
0102:    C2 08 00                ret      8
