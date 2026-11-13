; Disassembly: Menger3.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 249 bytes, decoded: 249 bytes (100.0%)
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
007A:    D9 C1                   fld      st(1)
007C:    D8 4E 98                fmul     dword ptr [esi - 0x68]
007F:    D9 C1                   fld      st(1)
0081:    D8 4E 94                fmul     dword ptr [esi - 0x6c]
0084:    DE C1                   faddp    st(1)
0086:    D9 C3                   fld      st(3)
0088:    D8 4E 90                fmul     dword ptr [esi - 0x70]
008B:    DE C1                   faddp    st(1)
008D:    D9 C2                   fld      st(2)
008F:    D8 4E B0                fmul     dword ptr [esi - 0x50]
0092:    D9 C2                   fld      st(2)
0094:    D8 4E AC                fmul     dword ptr [esi - 0x54]
0097:    DE C1                   faddp    st(1)
0099:    D9 C4                   fld      st(4)
009B:    D8 4E A8                fmul     dword ptr [esi - 0x58]
009E:    DE C1                   faddp    st(1)
00A0:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00A2:    D8 4E A4                fmul     dword ptr [esi - 0x5c]
00A5:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00A7:    D8 4E A0                fmul     dword ptr [esi - 0x60]
00AA:    DE C2                   faddp    st(2)
00AC:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00AE:    D8 4E 9C                fmul     dword ptr [esi - 0x64]
00B1:    DE C1                   faddp    st(1)
00B3:    DD 46 F0                fld      qword ptr [esi - 0x10]
00B6:    DD 41 08                fld      qword ptr [ecx + 8]
00B9:    D8 C9                   fmul     st(1)
00BB:    DD 59 08                fstp     qword ptr [ecx + 8]
00BE:    DC CB                   fmul     st(3), st(0)
00C0:    DC CA                   fmul     st(2), st(0)
00C2:    DC C9                   fmul     st(1), st(0)
00C4:    D9 E8                   fld1    ; Load 1.0
00C6:    DE E9                   fsubp    st(1)
00C8:    D9 C0                   fld      st(0)
00CA:    D9 C0                   fld      st(0)
00CC:    DC 4E E8                fmul     qword ptr [esi - 0x18]
00CF:    DE EC                   fsubp    st(4)
00D1:    DC 4E E0                fmul     qword ptr [esi - 0x20]
00D4:    DE EA                   fsubp    st(2)
00D6:    DC 4E D8                fmul     qword ptr [esi - 0x28]
00D9:    D9 C0                   fld      st(0)
00DB:    DC 4E F8                fmul     qword ptr [esi - 8]
00DE:    D8 DC                   fcomp    st(4)
00E0:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00E2:    80 E4 41                and      ah, 0x41
00E5:    74 02                   je       0xe9    ; Conditional jump
00E7:    DC EB                   fsub     st(3), st(0)
00E9:    DD D8                   fstp     st(0)
00EB:    DD 1A                   fstp     qword ptr [edx]
00ED:    DD 1B                   fstp     qword ptr [ebx]
00EF:    DD 19                   fstp     qword ptr [ecx]
00F1:    89 D8                   mov      eax, ebx
00F3:    5E                      pop      esi
00F4:    5B                      pop      ebx
00F5:    5D                      pop      ebp
00F6:    C2 08 00                ret      8
