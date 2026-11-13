; Disassembly: SierpHilbert.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 289 bytes, decoded: 289 bytes (100.0%)
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
0076:    90                      nop
0077:    90                      nop
0078:    DD 46 88                fld      qword ptr [esi - 0x78]
007B:    D9 EE                   fldz    ; Load 0.0
007D:    D8 D9                   fcomp    st(1)
007F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0081:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0083:    73 0B                   jae      0x90    ; Conditional jump
0085:    D8 E2                   fsub     st(2)
0087:    D9 E1                   fabs    ; |ST(0)|
0089:    D9 E0                   fchs    ; -ST(0)
008B:    DC 46 88                fadd     qword ptr [esi - 0x78]
008E:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0090:    DD D8                   fstp     st(0)
0092:    90                      nop
0093:    90                      nop
0094:    DD 46 80                fld      qword ptr [esi - 0x80]
0097:    D9 EE                   fldz    ; Load 0.0
0099:    D8 D9                   fcomp    st(1)
009B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
009D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
009F:    73 0B                   jae      0xac    ; Conditional jump
00A1:    D8 E1                   fsub     st(1)
00A3:    D9 E1                   fabs    ; |ST(0)|
00A5:    D9 E0                   fchs    ; -ST(0)
00A7:    DC 46 80                fadd     qword ptr [esi - 0x80]
00AA:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00AC:    DD D8                   fstp     st(0)
00AE:    90                      nop
00AF:    90                      nop
00B0:    D9 D0                   fnop
00B2:    D9 C0                   fld      st(0)
00B4:    D8 4E 98                fmul     dword ptr [esi - 0x68]
00B7:    D9 C2                   fld      st(2)
00B9:    D8 4E 94                fmul     dword ptr [esi - 0x6c]
00BC:    DE C1                   faddp    st(1)
00BE:    D9 C3                   fld      st(3)
00C0:    D8 4E 90                fmul     dword ptr [esi - 0x70]
00C3:    DE C1                   faddp    st(1)
00C5:    D9 C1                   fld      st(1)
00C7:    D8 4E B0                fmul     dword ptr [esi - 0x50]
00CA:    D9 C3                   fld      st(3)
00CC:    D8 4E AC                fmul     dword ptr [esi - 0x54]
00CF:    DE C1                   faddp    st(1)
00D1:    D9 C4                   fld      st(4)
00D3:    D8 4E A8                fmul     dword ptr [esi - 0x58]
00D6:    DE C1                   faddp    st(1)
00D8:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00DA:    D8 4E A4                fmul     dword ptr [esi - 0x5c]
00DD:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00DF:    D8 4E A0                fmul     dword ptr [esi - 0x60]
00E2:    DE C3                   faddp    st(3)
00E4:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00E6:    D8 4E 9C                fmul     dword ptr [esi - 0x64]
00E9:    DE C2                   faddp    st(2)
00EB:    DD 46 F0                fld      qword ptr [esi - 0x10]
00EE:    DD 41 08                fld      qword ptr [ecx + 8]
00F1:    D8 C9                   fmul     st(1)
00F3:    DD 59 08                fstp     qword ptr [ecx + 8]
00F6:    DC CB                   fmul     st(3), st(0)
00F8:    DC CA                   fmul     st(2), st(0)
00FA:    DC C9                   fmul     st(1), st(0)
00FC:    D9 E8                   fld1    ; Load 1.0
00FE:    DE E9                   fsubp    st(1)
0100:    D9 C0                   fld      st(0)
0102:    DC 4E D8                fmul     qword ptr [esi - 0x28]
0105:    DE EC                   fsubp    st(4)
0107:    D9 C0                   fld      st(0)
0109:    DC 4E E0                fmul     qword ptr [esi - 0x20]
010C:    DE EB                   fsubp    st(3)
010E:    DC 4E E8                fmul     qword ptr [esi - 0x18]
0111:    DE E9                   fsubp    st(1)
0113:    DD 1B                   fstp     qword ptr [ebx]
0115:    DD 1A                   fstp     qword ptr [edx]
0117:    DD 19                   fstp     qword ptr [ecx]
0119:    89 D8                   mov      eax, ebx
011B:    5E                      pop      esi
011C:    5B                      pop      ebx
011D:    5D                      pop      ebp
011E:    C2 08 00                ret      8
