; Disassembly: CrossBiFold.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 326 bytes, decoded: 326 bytes (100.0%)
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
007A:    DD 46 F0                fld      qword ptr [esi - 0x10]
007D:    DD 41 08                fld      qword ptr [ecx + 8]
0080:    D8 C9                   fmul     st(1)
0082:    DD 59 08                fstp     qword ptr [ecx + 8]
0085:    DC CB                   fmul     st(3), st(0)
0087:    DC CA                   fmul     st(2), st(0)
0089:    DC C9                   fmul     st(1), st(0)
008B:    DD D8                   fstp     st(0)
008D:    DD 1A                   fstp     qword ptr [edx]
008F:    DD 1B                   fstp     qword ptr [ebx]
0091:    DD 19                   fstp     qword ptr [ecx]
0093:    D9 E8                   fld1    ; Load 1.0
0095:    DC 6E F0                fsubr    qword ptr [esi - 0x10]
0098:    DC 4E 80                fmul     qword ptr [esi - 0x80]
009B:    DD 03                   fld      qword ptr [ebx]
009D:    DE E1                   fsubrp   st(1)
009F:    DD 1B                   fstp     qword ptr [ebx]
00A1:    D9 E8                   fld1    ; Load 1.0
00A3:    DC 6E F0                fsubr    qword ptr [esi - 0x10]
00A6:    DC 4E 88                fmul     qword ptr [esi - 0x78]
00A9:    DD 03                   fld      qword ptr [ebx]
00AB:    D8 C1                   fadd     st(1)
00AD:    D9 E1                   fabs    ; |ST(0)|
00AF:    DD 03                   fld      qword ptr [ebx]
00B1:    D8 E2                   fsub     st(2)
00B3:    D9 E1                   fabs    ; |ST(0)|
00B5:    D9 E0                   fchs    ; -ST(0)
00B7:    DC 23                   fsub     qword ptr [ebx]
00B9:    DE C1                   faddp    st(1)
00BB:    DD 1B                   fstp     qword ptr [ebx]
00BD:    DD D8                   fstp     st(0)
00BF:    DD 01                   fld      qword ptr [ecx]
00C1:    DD 03                   fld      qword ptr [ebx]
00C3:    DD 02                   fld      qword ptr [edx]
00C5:    D8 D1                   fcom     st(1)
00C7:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00C9:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00CB:    73 02                   jae      0xcf    ; Conditional jump
00CD:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00CF:    D8 D2                   fcom     st(2)
00D1:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00D3:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00D5:    73 02                   jae      0xd9    ; Conditional jump
00D7:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00D9:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00DB:    D8 D2                   fcom     st(2)
00DD:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00DF:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00E1:    73 02                   jae      0xe5    ; Conditional jump
00E3:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00E5:    D9 C1                   fld      st(1)
00E7:    D8 4E 98                fmul     dword ptr [esi - 0x68]
00EA:    D9 C1                   fld      st(1)
00EC:    D8 4E 94                fmul     dword ptr [esi - 0x6c]
00EF:    DE C1                   faddp    st(1)
00F1:    D9 C3                   fld      st(3)
00F3:    D8 4E 90                fmul     dword ptr [esi - 0x70]
00F6:    DE C1                   faddp    st(1)
00F8:    D9 C2                   fld      st(2)
00FA:    D8 4E B0                fmul     dword ptr [esi - 0x50]
00FD:    D9 C2                   fld      st(2)
00FF:    D8 4E AC                fmul     dword ptr [esi - 0x54]
0102:    DE C1                   faddp    st(1)
0104:    D9 C4                   fld      st(4)
0106:    D8 4E A8                fmul     dword ptr [esi - 0x58]
0109:    DE C1                   faddp    st(1)
010B:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
010D:    D8 4E A4                fmul     dword ptr [esi - 0x5c]
0110:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0112:    D8 4E A0                fmul     dword ptr [esi - 0x60]
0115:    DE C2                   faddp    st(2)
0117:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0119:    D8 4E 9C                fmul     dword ptr [esi - 0x64]
011C:    DE C1                   faddp    st(1)
011E:    DD 46 F0                fld      qword ptr [esi - 0x10]
0121:    D9 E8                   fld1    ; Load 1.0
0123:    DE E9                   fsubp    st(1)
0125:    D9 C0                   fld      st(0)
0127:    D9 C0                   fld      st(0)
0129:    DC 4E D8                fmul     qword ptr [esi - 0x28]
012C:    DE EC                   fsubp    st(4)
012E:    DC 4E E0                fmul     qword ptr [esi - 0x20]
0131:    DE EA                   fsubp    st(2)
0133:    DC 4E E8                fmul     qword ptr [esi - 0x18]
0136:    DE E3                   fsubrp   st(3)
0138:    DD 1B                   fstp     qword ptr [ebx]
013A:    DD 1A                   fstp     qword ptr [edx]
013C:    DD 19                   fstp     qword ptr [ecx]
013E:    89 D8                   mov      eax, ebx
0140:    5E                      pop      esi
0141:    5B                      pop      ebx
0142:    5D                      pop      ebp
0143:    C2 08 00                ret      8
