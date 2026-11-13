; Disassembly: _FoldingIcosa.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 290 bytes, decoded: 290 bytes (100.0%)
; DEoption: -1

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
0015:    D8 4E DC                fmul     dword ptr [esi - 0x24]
0018:    D9 C2                   fld      st(2)
001A:    D8 4E D8                fmul     dword ptr [esi - 0x28]
001D:    DE C1                   faddp    st(1)
001F:    D9 C3                   fld      st(3)
0021:    D8 4E D4                fmul     dword ptr [esi - 0x2c]
0024:    DE C1                   faddp    st(1)
0026:    D9 E1                   fabs    ; |ST(0)|
0028:    D9 C1                   fld      st(1)
002A:    D8 4E F4                fmul     dword ptr [esi - 0xc]
002D:    D9 C3                   fld      st(3)
002F:    D8 4E F0                fmul     dword ptr [esi - 0x10]
0032:    DE C1                   faddp    st(1)
0034:    D9 C4                   fld      st(4)
0036:    D8 4E EC                fmul     dword ptr [esi - 0x14]
0039:    DE C1                   faddp    st(1)
003B:    D9 E1                   fabs    ; |ST(0)|
003D:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
003F:    D8 4E E8                fmul     dword ptr [esi - 0x18]
0042:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0044:    D8 4E E4                fmul     dword ptr [esi - 0x1c]
0047:    DE C3                   faddp    st(3)
0049:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
004B:    D8 4E E0                fmul     dword ptr [esi - 0x20]
004E:    DE C2                   faddp    st(2)
0050:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0052:    D9 E1                   fabs    ; |ST(0)|
0054:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0056:    D9 C0                   fld      st(0)
0058:    DC 0E                   fmul     qword ptr [esi]
005A:    D9 C2                   fld      st(2)
005C:    DC 4E 08                fmul     qword ptr [esi + 8]
005F:    DE C1                   faddp    st(1)
0061:    D9 C3                   fld      st(3)
0063:    DC 4E 10                fmul     qword ptr [esi + 0x10]
0066:    DE C1                   faddp    st(1)
0068:    D9 E4                   ftst
006A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
006C:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
006E:    73 18                   jae      0x88    ; Conditional jump
0070:    D8 C0                   fadd     st(0)
0072:    DD 46 08                fld      qword ptr [esi + 8]
0075:    DD 06                   fld      qword ptr [esi]
0077:    DD 46 10                fld      qword ptr [esi + 0x10]
007A:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
007C:    DC CB                   fmul     st(3), st(0)
007E:    DC CA                   fmul     st(2), st(0)
0080:    DE C9                   fmulp    st(1)
0082:    DE EB                   fsubp    st(3)
0084:    DE EB                   fsubp    st(3)
0086:    DC EB                   fsub     st(3), st(0)
0088:    DD D8                   fstp     st(0)
008A:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
008C:    D9 E1                   fabs    ; |ST(0)|
008E:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0090:    D9 E1                   fabs    ; |ST(0)|
0092:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0094:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0096:    D9 C0                   fld      st(0)
0098:    DC 0E                   fmul     qword ptr [esi]
009A:    D9 C2                   fld      st(2)
009C:    DC 4E 08                fmul     qword ptr [esi + 8]
009F:    DE C1                   faddp    st(1)
00A1:    D9 C3                   fld      st(3)
00A3:    DC 4E 10                fmul     qword ptr [esi + 0x10]
00A6:    DE C1                   faddp    st(1)
00A8:    D9 E4                   ftst
00AA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00AC:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00AE:    73 18                   jae      0xc8    ; Conditional jump
00B0:    D8 C0                   fadd     st(0)
00B2:    DD 46 08                fld      qword ptr [esi + 8]
00B5:    DD 06                   fld      qword ptr [esi]
00B7:    DD 46 10                fld      qword ptr [esi + 0x10]
00BA:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00BC:    DC CB                   fmul     st(3), st(0)
00BE:    DC CA                   fmul     st(2), st(0)
00C0:    DE C9                   fmulp    st(1)
00C2:    DE EB                   fsubp    st(3)
00C4:    DE EB                   fsubp    st(3)
00C6:    DC EB                   fsub     st(3), st(0)
00C8:    DD D8                   fstp     st(0)
00CA:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00CC:    D9 E1                   fabs    ; |ST(0)|
00CE:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00D0:    D9 E1                   fabs    ; |ST(0)|
00D2:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00D4:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00D6:    D9 C0                   fld      st(0)
00D8:    DC 0E                   fmul     qword ptr [esi]
00DA:    D9 C2                   fld      st(2)
00DC:    DC 4E 08                fmul     qword ptr [esi + 8]
00DF:    DE C1                   faddp    st(1)
00E1:    D9 C3                   fld      st(3)
00E3:    DC 4E 10                fmul     qword ptr [esi + 0x10]
00E6:    DE C1                   faddp    st(1)
00E8:    D9 E4                   ftst
00EA:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00EC:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
00EE:    73 18                   jae      0x108    ; Conditional jump
00F0:    D8 C0                   fadd     st(0)
00F2:    DD 46 08                fld      qword ptr [esi + 8]
00F5:    DD 06                   fld      qword ptr [esi]
00F7:    DD 46 10                fld      qword ptr [esi + 0x10]
00FA:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00FC:    DC CB                   fmul     st(3), st(0)
00FE:    DC CA                   fmul     st(2), st(0)
0100:    DE C9                   fmulp    st(1)
0102:    DE EB                   fsubp    st(3)
0104:    DE EB                   fsubp    st(3)
0106:    DC EB                   fsub     st(3), st(0)
0108:    DD D8                   fstp     st(0)
010A:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
010C:    D9 E1                   fabs    ; |ST(0)|
010E:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0110:    D9 E1                   fabs    ; |ST(0)|
0112:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0114:    DD 1A                   fstp     qword ptr [edx]
0116:    DD 1B                   fstp     qword ptr [ebx]
0118:    DD 19                   fstp     qword ptr [ecx]
011A:    89 D8                   mov      eax, ebx
011C:    5E                      pop      esi
011D:    5B                      pop      ebx
011E:    5D                      pop      ebp
011F:    C2 08 00                ret      8
