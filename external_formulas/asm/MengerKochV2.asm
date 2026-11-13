; Disassembly: MengerKochV2.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 307 bytes, decoded: 307 bytes (100.0%)
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
007D:    D9 EE                   fldz    ; Load 0.0
007F:    D8 D9                   fcomp    st(1)
0081:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0083:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0085:    73 0B                   jae      0x92    ; Conditional jump
0087:    D8 E2                   fsub     st(2)
0089:    D9 E1                   fabs    ; |ST(0)|
008B:    D9 E0                   fchs    ; -ST(0)
008D:    DC 46 88                fadd     qword ptr [esi - 0x78]
0090:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0092:    DD D8                   fstp     st(0)
0094:    90                      nop
0095:    90                      nop
0096:    DD 46 80                fld      qword ptr [esi - 0x80]
0099:    D9 EE                   fldz    ; Load 0.0
009B:    D8 D9                   fcomp    st(1)
009D:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
009F:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00A1:    73 0B                   jae      0xae    ; Conditional jump
00A3:    D8 E1                   fsub     st(1)
00A5:    D9 E1                   fabs    ; |ST(0)|
00A7:    D9 E0                   fchs    ; -ST(0)
00A9:    DC 46 80                fadd     qword ptr [esi - 0x80]
00AC:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00AE:    DD D8                   fstp     st(0)
00B0:    90                      nop
00B1:    90                      nop
00B2:    D9 D0                   fnop
00B4:    D9 C1                   fld      st(1)
00B6:    D8 4E 98                fmul     dword ptr [esi - 0x68]
00B9:    D9 C1                   fld      st(1)
00BB:    D8 4E 94                fmul     dword ptr [esi - 0x6c]
00BE:    DE C1                   faddp    st(1)
00C0:    D9 C3                   fld      st(3)
00C2:    D8 4E 90                fmul     dword ptr [esi - 0x70]
00C5:    DE C1                   faddp    st(1)
00C7:    D9 C2                   fld      st(2)
00C9:    D8 4E B0                fmul     dword ptr [esi - 0x50]
00CC:    D9 C2                   fld      st(2)
00CE:    D8 4E AC                fmul     dword ptr [esi - 0x54]
00D1:    DE C1                   faddp    st(1)
00D3:    D9 C4                   fld      st(4)
00D5:    D8 4E A8                fmul     dword ptr [esi - 0x58]
00D8:    DE C1                   faddp    st(1)
00DA:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00DC:    D8 4E A4                fmul     dword ptr [esi - 0x5c]
00DF:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00E1:    D8 4E A0                fmul     dword ptr [esi - 0x60]
00E4:    DE C2                   faddp    st(2)
00E6:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00E8:    D8 4E 9C                fmul     dword ptr [esi - 0x64]
00EB:    DE C1                   faddp    st(1)
00ED:    DD 46 F0                fld      qword ptr [esi - 0x10]
00F0:    DD 41 08                fld      qword ptr [ecx + 8]
00F3:    D8 C9                   fmul     st(1)
00F5:    DD 59 08                fstp     qword ptr [ecx + 8]
00F8:    DC CB                   fmul     st(3), st(0)
00FA:    DC CA                   fmul     st(2), st(0)
00FC:    DC C9                   fmul     st(1), st(0)
00FE:    D9 E8                   fld1    ; Load 1.0
0100:    DE E9                   fsubp    st(1)
0102:    D9 C0                   fld      st(0)
0104:    D9 C0                   fld      st(0)
0106:    DC 4E E8                fmul     qword ptr [esi - 0x18]
0109:    DE EC                   fsubp    st(4)
010B:    DC 4E E0                fmul     qword ptr [esi - 0x20]
010E:    DE EA                   fsubp    st(2)
0110:    DC 4E D8                fmul     qword ptr [esi - 0x28]
0113:    D9 C0                   fld      st(0)
0115:    DC 4E F8                fmul     qword ptr [esi - 8]
0118:    D8 DC                   fcomp    st(4)
011A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
011C:    80 E4 41                and      ah, 0x41
011F:    74 02                   je       0x123    ; Conditional jump
0121:    DC EB                   fsub     st(3), st(0)
0123:    DD D8                   fstp     st(0)
0125:    DD 1A                   fstp     qword ptr [edx]
0127:    DD 1B                   fstp     qword ptr [ebx]
0129:    DD 19                   fstp     qword ptr [ecx]
012B:    89 D8                   mov      eax, ebx
012D:    5E                      pop      esi
012E:    5B                      pop      ebx
012F:    5D                      pop      ebp
0130:    C2 08 00                ret      8
