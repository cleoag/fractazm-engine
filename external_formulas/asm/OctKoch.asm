; Disassembly: OctKoch.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 342 bytes, decoded: 342 bytes (100.0%)
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
005E:    80 E4 41                and      ah, 0x41
0061:    74 02                   je       0x65    ; Conditional jump
0063:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0065:    D8 D2                   fcom     st(2)
0067:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0069:    80 E4 41                and      ah, 0x41
006C:    74 02                   je       0x70    ; Conditional jump
006E:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0070:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0072:    D8 D2                   fcom     st(2)
0074:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0076:    80 E4 41                and      ah, 0x41
0079:    74 02                   je       0x7d    ; Conditional jump
007B:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
007D:    DD 1A                   fstp     qword ptr [edx]
007F:    DD 1B                   fstp     qword ptr [ebx]
0081:    DD 19                   fstp     qword ptr [ecx]
0083:    D9 E8                   fld1    ; Load 1.0
0085:    DC 6E F0                fsubr    qword ptr [esi - 0x10]
0088:    DC 4E 80                fmul     qword ptr [esi - 0x80]
008B:    DD 03                   fld      qword ptr [ebx]
008D:    DE E1                   fsubrp   st(1)
008F:    DD 1B                   fstp     qword ptr [ebx]
0091:    D9 E8                   fld1    ; Load 1.0
0093:    DC 6E F0                fsubr    qword ptr [esi - 0x10]
0096:    DC 4E 88                fmul     qword ptr [esi - 0x78]
0099:    DD 03                   fld      qword ptr [ebx]
009B:    D8 C1                   fadd     st(1)
009D:    D9 E1                   fabs    ; |ST(0)|
009F:    DD 03                   fld      qword ptr [ebx]
00A1:    D8 E2                   fsub     st(2)
00A3:    D9 E1                   fabs    ; |ST(0)|
00A5:    D9 E0                   fchs    ; -ST(0)
00A7:    DC 23                   fsub     qword ptr [ebx]
00A9:    DE C1                   faddp    st(1)
00AB:    83 BE 7C FF FF FF 00    cmp      dword ptr [esi - 0x84], 0
00B2:    74 02                   je       0xb6    ; Conditional jump
00B4:    D9 E1                   fabs    ; |ST(0)|
00B6:    DD 1B                   fstp     qword ptr [ebx]
00B8:    DD D8                   fstp     st(0)
00BA:    DD 01                   fld      qword ptr [ecx]
00BC:    DD 03                   fld      qword ptr [ebx]
00BE:    DD 02                   fld      qword ptr [edx]
00C0:    D8 D1                   fcom     st(1)
00C2:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00C4:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00C6:    73 02                   jae      0xca    ; Conditional jump
00C8:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00CA:    D8 D2                   fcom     st(2)
00CC:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00CE:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00D0:    73 02                   jae      0xd4    ; Conditional jump
00D2:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00D4:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00D6:    D8 D2                   fcom     st(2)
00D8:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00DA:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00DC:    73 02                   jae      0xe0    ; Conditional jump
00DE:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00E0:    D9 C1                   fld      st(1)
00E2:    D8 4E 98                fmul     dword ptr [esi - 0x68]
00E5:    D9 C1                   fld      st(1)
00E7:    D8 4E 94                fmul     dword ptr [esi - 0x6c]
00EA:    DE C1                   faddp    st(1)
00EC:    D9 C3                   fld      st(3)
00EE:    D8 4E 90                fmul     dword ptr [esi - 0x70]
00F1:    DE C1                   faddp    st(1)
00F3:    D9 C2                   fld      st(2)
00F5:    D8 4E B0                fmul     dword ptr [esi - 0x50]
00F8:    D9 C2                   fld      st(2)
00FA:    D8 4E AC                fmul     dword ptr [esi - 0x54]
00FD:    DE C1                   faddp    st(1)
00FF:    D9 C4                   fld      st(4)
0101:    D8 4E A8                fmul     dword ptr [esi - 0x58]
0104:    DE C1                   faddp    st(1)
0106:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0108:    D8 4E A4                fmul     dword ptr [esi - 0x5c]
010B:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
010D:    D8 4E A0                fmul     dword ptr [esi - 0x60]
0110:    DE C2                   faddp    st(2)
0112:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0114:    D8 4E 9C                fmul     dword ptr [esi - 0x64]
0117:    DE C1                   faddp    st(1)
0119:    DD 46 F0                fld      qword ptr [esi - 0x10]
011C:    D9 E8                   fld1    ; Load 1.0
011E:    DE E9                   fsubp    st(1)
0120:    D9 C0                   fld      st(0)
0122:    D9 C0                   fld      st(0)
0124:    DC 4E E8                fmul     qword ptr [esi - 0x18]
0127:    DE EC                   fsubp    st(4)
0129:    DC 4E E0                fmul     qword ptr [esi - 0x20]
012C:    DE EA                   fsubp    st(2)
012E:    DC 4E D8                fmul     qword ptr [esi - 0x28]
0131:    DC EB                   fsub     st(3), st(0)
0133:    DD D8                   fstp     st(0)
0135:    DD 46 F0                fld      qword ptr [esi - 0x10]
0138:    DD 41 08                fld      qword ptr [ecx + 8]
013B:    D8 C9                   fmul     st(1)
013D:    DD 59 08                fstp     qword ptr [ecx + 8]
0140:    DC CB                   fmul     st(3), st(0)
0142:    DC CA                   fmul     st(2), st(0)
0144:    DC C9                   fmul     st(1), st(0)
0146:    DD D8                   fstp     st(0)
0148:    DD 1A                   fstp     qword ptr [edx]
014A:    DD 1B                   fstp     qword ptr [ebx]
014C:    DD 19                   fstp     qword ptr [ecx]
014E:    89 D8                   mov      eax, ebx
0150:    5E                      pop      esi
0151:    5B                      pop      ebx
0152:    5D                      pop      ebp
0153:    C2 08 00                ret      8
