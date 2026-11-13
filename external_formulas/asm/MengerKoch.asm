; Disassembly: MengerKoch.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 267 bytes, decoded: 267 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    56                      push     esi
0005:    89 C3                   mov      ebx, eax
0007:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    B8 FF FF FF 7F          mov      eax, 0x7fffffff
0012:    21 41 04                and      dword ptr [ecx + 4], eax
0015:    21 42 04                and      dword ptr [edx + 4], eax
0018:    21 43 04                and      dword ptr [ebx + 4], eax
001B:    DD 01                   fld      qword ptr [ecx]
001D:    DD 02                   fld      qword ptr [edx]
001F:    DD 03                   fld      qword ptr [ebx]
0021:    D9 C0                   fld      st(0)
0023:    D8 4E BC                fmul     dword ptr [esi - 0x44]
0026:    D9 C2                   fld      st(2)
0028:    D8 4E B8                fmul     dword ptr [esi - 0x48]
002B:    DE C1                   faddp    st(1)
002D:    D9 C3                   fld      st(3)
002F:    D8 4E B4                fmul     dword ptr [esi - 0x4c]
0032:    DE C1                   faddp    st(1)
0034:    D9 C1                   fld      st(1)
0036:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
0039:    D9 C3                   fld      st(3)
003B:    D8 4E D0                fmul     dword ptr [esi - 0x30]
003E:    DE C1                   faddp    st(1)
0040:    D9 C4                   fld      st(4)
0042:    D8 4E CC                fmul     dword ptr [esi - 0x34]
0045:    DE C1                   faddp    st(1)
0047:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0049:    D8 4E C8                fmul     dword ptr [esi - 0x38]
004C:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
004E:    D8 4E C4                fmul     dword ptr [esi - 0x3c]
0051:    DE C3                   faddp    st(3)
0053:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0055:    D8 4E C0                fmul     dword ptr [esi - 0x40]
0058:    DE C2                   faddp    st(2)
005A:    D8 D1                   fcom     st(1)
005C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
005E:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0060:    73 02                   jae      0x64    ; Conditional jump
0062:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0064:    D8 D2                   fcom     st(2)
0066:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0068:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
006A:    73 02                   jae      0x6e    ; Conditional jump
006C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
006E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0070:    D8 D2                   fcom     st(2)
0072:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0074:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0076:    73 02                   jae      0x7a    ; Conditional jump
0078:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
007A:    DD 46 88                fld      qword ptr [esi - 0x78]
007D:    D8 E2                   fsub     st(2)
007F:    D9 E1                   fabs    ; |ST(0)|
0081:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0083:    DD D8                   fstp     st(0)
0085:    DC 66 80                fsub     qword ptr [esi - 0x80]
0088:    D9 E1                   fabs    ; |ST(0)|
008A:    D9 D0                   fnop
008C:    D9 C1                   fld      st(1)
008E:    D8 4E 98                fmul     dword ptr [esi - 0x68]
0091:    D9 C1                   fld      st(1)
0093:    D8 4E 94                fmul     dword ptr [esi - 0x6c]
0096:    DE C1                   faddp    st(1)
0098:    D9 C3                   fld      st(3)
009A:    D8 4E 90                fmul     dword ptr [esi - 0x70]
009D:    DE C1                   faddp    st(1)
009F:    D9 C2                   fld      st(2)
00A1:    D8 4E B0                fmul     dword ptr [esi - 0x50]
00A4:    D9 C2                   fld      st(2)
00A6:    D8 4E AC                fmul     dword ptr [esi - 0x54]
00A9:    DE C1                   faddp    st(1)
00AB:    D9 C4                   fld      st(4)
00AD:    D8 4E A8                fmul     dword ptr [esi - 0x58]
00B0:    DE C1                   faddp    st(1)
00B2:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00B4:    D8 4E A4                fmul     dword ptr [esi - 0x5c]
00B7:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00B9:    D8 4E A0                fmul     dword ptr [esi - 0x60]
00BC:    DE C2                   faddp    st(2)
00BE:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00C0:    D8 4E 9C                fmul     dword ptr [esi - 0x64]
00C3:    DE C1                   faddp    st(1)
00C5:    DD 46 F0                fld      qword ptr [esi - 0x10]
00C8:    DD 41 08                fld      qword ptr [ecx + 8]
00CB:    D8 C9                   fmul     st(1)
00CD:    DD 59 08                fstp     qword ptr [ecx + 8]
00D0:    DC CB                   fmul     st(3), st(0)
00D2:    DC CA                   fmul     st(2), st(0)
00D4:    DC C9                   fmul     st(1), st(0)
00D6:    D9 E8                   fld1    ; Load 1.0
00D8:    DE E9                   fsubp    st(1)
00DA:    D9 C0                   fld      st(0)
00DC:    D9 C0                   fld      st(0)
00DE:    DC 4E E8                fmul     qword ptr [esi - 0x18]
00E1:    DE EC                   fsubp    st(4)
00E3:    DC 4E E0                fmul     qword ptr [esi - 0x20]
00E6:    DE EA                   fsubp    st(2)
00E8:    DC 4E D8                fmul     qword ptr [esi - 0x28]
00EB:    D9 C0                   fld      st(0)
00ED:    DC 4E F8                fmul     qword ptr [esi - 8]
00F0:    D8 DC                   fcomp    st(4)
00F2:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00F4:    80 E4 41                and      ah, 0x41
00F7:    74 02                   je       0xfb    ; Conditional jump
00F9:    DC EB                   fsub     st(3), st(0)
00FB:    DD D8                   fstp     st(0)
00FD:    DD 1A                   fstp     qword ptr [edx]
00FF:    DD 1B                   fstp     qword ptr [ebx]
0101:    DD 19                   fstp     qword ptr [ecx]
0103:    89 D8                   mov      eax, ebx
0105:    5E                      pop      esi
0106:    5B                      pop      ebx
0107:    5D                      pop      ebp
0108:    C2 08 00                ret      8
