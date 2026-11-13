; Disassembly: Sierpinski3.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 229 bytes, decoded: 229 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    56                      push     esi
0005:    89 C3                   mov      ebx, eax
0007:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    DD 01                   fld      qword ptr [ecx]
000F:    DD 02                   fld      qword ptr [edx]
0011:    DD 03                   fld      qword ptr [ebx]
0013:    D9 C0                   fld      st(0)
0015:    D8 4E BC                fmul     dword ptr [esi - 0x44]
0018:    D9 C2                   fld      st(2)
001A:    D8 4E B8                fmul     dword ptr [esi - 0x48]
001D:    DE C1                   faddp    st(1)
001F:    D9 C3                   fld      st(3)
0021:    D8 4E B4                fmul     dword ptr [esi - 0x4c]
0024:    DE C1                   faddp    st(1)
0026:    D9 C1                   fld      st(1)
0028:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
002B:    D9 C3                   fld      st(3)
002D:    D8 4E D0                fmul     dword ptr [esi - 0x30]
0030:    DE C1                   faddp    st(1)
0032:    D9 C4                   fld      st(4)
0034:    D8 4E CC                fmul     dword ptr [esi - 0x34]
0037:    DE C1                   faddp    st(1)
0039:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003B:    D8 4E C8                fmul     dword ptr [esi - 0x38]
003E:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0040:    D8 4E C4                fmul     dword ptr [esi - 0x3c]
0043:    DE C3                   faddp    st(3)
0045:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0047:    D8 4E C0                fmul     dword ptr [esi - 0x40]
004A:    DE C2                   faddp    st(2)
004C:    D9 E0                   fchs    ; -ST(0)
004E:    D8 D1                   fcom     st(1)
0050:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0052:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0054:    72 02                   jb       0x58    ; Conditional jump
0056:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0058:    D8 D2                   fcom     st(2)
005A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
005C:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
005E:    72 02                   jb       0x62    ; Conditional jump
0060:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0062:    D9 E0                   fchs    ; -ST(0)
0064:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0066:    D9 E0                   fchs    ; -ST(0)
0068:    D8 D2                   fcom     st(2)
006A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
006C:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
006E:    72 02                   jb       0x72    ; Conditional jump
0070:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0072:    D9 E0                   fchs    ; -ST(0)
0074:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0076:    D9 C0                   fld      st(0)
0078:    D8 4E 98                fmul     dword ptr [esi - 0x68]
007B:    D9 C2                   fld      st(2)
007D:    D8 4E 94                fmul     dword ptr [esi - 0x6c]
0080:    DE C1                   faddp    st(1)
0082:    D9 C3                   fld      st(3)
0084:    D8 4E 90                fmul     dword ptr [esi - 0x70]
0087:    DE C1                   faddp    st(1)
0089:    D9 C1                   fld      st(1)
008B:    D8 4E B0                fmul     dword ptr [esi - 0x50]
008E:    D9 C3                   fld      st(3)
0090:    D8 4E AC                fmul     dword ptr [esi - 0x54]
0093:    DE C1                   faddp    st(1)
0095:    D9 C4                   fld      st(4)
0097:    D8 4E A8                fmul     dword ptr [esi - 0x58]
009A:    DE C1                   faddp    st(1)
009C:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
009E:    D8 4E A4                fmul     dword ptr [esi - 0x5c]
00A1:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00A3:    D8 4E A0                fmul     dword ptr [esi - 0x60]
00A6:    DE C3                   faddp    st(3)
00A8:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00AA:    D8 4E 9C                fmul     dword ptr [esi - 0x64]
00AD:    DE C2                   faddp    st(2)
00AF:    DD 46 F0                fld      qword ptr [esi - 0x10]
00B2:    DD 41 08                fld      qword ptr [ecx + 8]
00B5:    D8 C9                   fmul     st(1)
00B7:    DD 59 08                fstp     qword ptr [ecx + 8]
00BA:    DC CB                   fmul     st(3), st(0)
00BC:    DC CA                   fmul     st(2), st(0)
00BE:    DC C9                   fmul     st(1), st(0)
00C0:    D9 E8                   fld1    ; Load 1.0
00C2:    DE E9                   fsubp    st(1)
00C4:    D9 C0                   fld      st(0)
00C6:    DC 4E D8                fmul     qword ptr [esi - 0x28]
00C9:    DE EC                   fsubp    st(4)
00CB:    D9 C0                   fld      st(0)
00CD:    DC 4E E0                fmul     qword ptr [esi - 0x20]
00D0:    DE EB                   fsubp    st(3)
00D2:    DC 4E E8                fmul     qword ptr [esi - 0x18]
00D5:    DE E9                   fsubp    st(1)
00D7:    DD 1B                   fstp     qword ptr [ebx]
00D9:    DD 1A                   fstp     qword ptr [edx]
00DB:    DD 19                   fstp     qword ptr [ecx]
00DD:    89 D8                   mov      eax, ebx
00DF:    5E                      pop      esi
00E0:    5B                      pop      ebx
00E1:    5D                      pop      ebp
00E2:    C2 08 00                ret      8
