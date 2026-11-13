; Disassembly: ABoxPlatinum.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 512 bytes, decoded: 512 bytes (100.0%)
; DEoption: 2

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    53                      push     ebx
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    89 C3                   mov      ebx, eax
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    83 C4 E0                add      esp, -0x20
0010:    83 C6 C0                add      esi, -0x40
0013:    DD 46 18                fld      qword ptr [esi + 0x18]
0016:    DD 03                   fld      qword ptr [ebx]
0018:    DC 4E B4                fmul     qword ptr [esi - 0x4c]
001B:    DD 01                   fld      qword ptr [ecx]
001D:    DC 4E BC                fmul     qword ptr [esi - 0x44]
0020:    DE E9                   fsubp    st(1)
0022:    DD 03                   fld      qword ptr [ebx]
0024:    DC 4E BC                fmul     qword ptr [esi - 0x44]
0027:    DD 01                   fld      qword ptr [ecx]
0029:    DC 4E B4                fmul     qword ptr [esi - 0x4c]
002C:    DE C1                   faddp    st(1)
002E:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0032:    D9 C0                   fld      st(0)
0034:    DC 4E C4                fmul     qword ptr [esi - 0x3c]
0037:    DD 02                   fld      qword ptr [edx]
0039:    DC 4E CC                fmul     qword ptr [esi - 0x34]
003C:    DE E9                   fsubp    st(1)
003E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0040:    DC 4E CC                fmul     qword ptr [esi - 0x34]
0043:    DD 02                   fld      qword ptr [edx]
0045:    DC 4E C4                fmul     qword ptr [esi - 0x3c]
0048:    DE C1                   faddp    st(1)
004A:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
004C:    D9 C0                   fld      st(0)
004E:    D8 E3                   fsub     st(3)
0050:    D9 E1                   fabs    ; |ST(0)|
0052:    D8 C1                   fadd     st(1)
0054:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0056:    D8 C3                   fadd     st(3)
0058:    D9 E1                   fabs    ; |ST(0)|
005A:    DE E1                   fsubrp   st(1)
005C:    D9 C0                   fld      st(0)
005E:    DC 4E C4                fmul     qword ptr [esi - 0x3c]
0061:    D9 C2                   fld      st(2)
0063:    DC 4E CC                fmul     qword ptr [esi - 0x34]
0066:    DE E9                   fsubp    st(1)
0068:    D9 C0                   fld      st(0)
006A:    DC 4E B4                fmul     qword ptr [esi - 0x4c]
006D:    DD 44 24 08             fld      qword ptr [esp + 8]
0071:    DC 4E BC                fmul     qword ptr [esi - 0x44]
0074:    DE E9                   fsubp    st(1)
0076:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0078:    DC 4E CC                fmul     qword ptr [esi - 0x34]
007B:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
007D:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
007F:    DC 4E C4                fmul     qword ptr [esi - 0x3c]
0082:    DE C1                   faddp    st(1)
0084:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0086:    DC 4E BC                fmul     qword ptr [esi - 0x44]
0089:    DD 44 24 08             fld      qword ptr [esp + 8]
008D:    DC 4E B4                fmul     qword ptr [esi - 0x4c]
0090:    DE C1                   faddp    st(1)
0092:    D9 C2                   fld      st(2)
0094:    DC 4E 94                fmul     qword ptr [esi - 0x6c]
0097:    D9 C1                   fld      st(1)
0099:    DC 4E 9C                fmul     qword ptr [esi - 0x64]
009C:    DE E9                   fsubp    st(1)
009E:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00A0:    DC 4E 94                fmul     qword ptr [esi - 0x6c]
00A3:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00A5:    DC 4E 9C                fmul     qword ptr [esi - 0x64]
00A8:    DE C3                   faddp    st(3)
00AA:    D9 C0                   fld      st(0)
00AC:    DC 4E A4                fmul     qword ptr [esi - 0x5c]
00AF:    D9 C2                   fld      st(2)
00B1:    DC 4E AC                fmul     qword ptr [esi - 0x54]
00B4:    DE E9                   fsubp    st(1)
00B6:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
00B8:    DC 4E A4                fmul     qword ptr [esi - 0x5c]
00BB:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00BD:    DC 4E AC                fmul     qword ptr [esi - 0x54]
00C0:    DE C1                   faddp    st(1)
00C2:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00C4:    D9 C0                   fld      st(0)
00C6:    D8 E4                   fsub     st(4)
00C8:    D9 E1                   fabs    ; |ST(0)|
00CA:    D8 C1                   fadd     st(1)
00CC:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00CE:    D8 C4                   fadd     st(4)
00D0:    D9 E1                   fabs    ; |ST(0)|
00D2:    DE E1                   fsubrp   st(1)
00D4:    D9 C0                   fld      st(0)
00D6:    DC 4E A4                fmul     qword ptr [esi - 0x5c]
00D9:    D9 C2                   fld      st(2)
00DB:    DC 4E AC                fmul     qword ptr [esi - 0x54]
00DE:    DE E9                   fsubp    st(1)
00E0:    D9 C0                   fld      st(0)
00E2:    DC 4E 94                fmul     qword ptr [esi - 0x6c]
00E5:    D9 C4                   fld      st(4)
00E7:    DC 4E 9C                fmul     qword ptr [esi - 0x64]
00EA:    DE E9                   fsubp    st(1)
00EC:    DD 46 AC                fld      qword ptr [esi - 0x54]
00EF:    DE CB                   fmulp    st(3)
00F1:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00F3:    DC 4E A4                fmul     qword ptr [esi - 0x5c]
00F6:    DE C2                   faddp    st(2)
00F8:    DC 4E 9C                fmul     qword ptr [esi - 0x64]
00FB:    DD 46 94                fld      qword ptr [esi - 0x6c]
00FE:    DE CC                   fmulp    st(4)
0100:    DE C3                   faddp    st(3)
0102:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0104:    D9 C0                   fld      st(0)
0106:    D8 E4                   fsub     st(4)
0108:    D9 E1                   fabs    ; |ST(0)|
010A:    D8 C1                   fadd     st(1)
010C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
010E:    D8 C4                   fadd     st(4)
0110:    D9 E1                   fabs    ; |ST(0)|
0112:    DE E1                   fsubrp   st(1)
0114:    8B 46 90                mov      eax, dword ptr [esi - 0x70]
0117:    90                      nop
0118:    A9 04 00 00 00          test     eax, 4
011D:    74 02                   je       0x121    ; Conditional jump
011F:    D9 E1                   fabs    ; |ST(0)|
0121:    A9 02 00 00 00          test     eax, 2
0126:    74 06                   je       0x12e    ; Conditional jump
0128:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
012A:    D9 E1                   fabs    ; |ST(0)|
012C:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
012E:    A9 01 00 00 00          test     eax, 1
0133:    74 06                   je       0x13b    ; Conditional jump
0135:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0137:    D9 E1                   fabs    ; |ST(0)|
0139:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
013B:    DC 46 DC                fadd     qword ptr [esi - 0x24]
013E:    DD 46 E4                fld      qword ptr [esi - 0x1c]
0141:    DE C2                   faddp    st(2)
0143:    DD 46 EC                fld      qword ptr [esi - 0x14]
0146:    DE C3                   faddp    st(3)
0148:    D9 C0                   fld      st(0)
014A:    D8 C8                   fmul     st(0)
014C:    D9 C2                   fld      st(2)
014E:    D8 C8                   fmul     st(0)
0150:    DE C1                   faddp    st(1)
0152:    D9 C3                   fld      st(3)
0154:    D8 C8                   fmul     st(0)
0156:    DE C1                   faddp    st(1)
0158:    DC 4E D4                fmul     qword ptr [esi - 0x2c]
015B:    DC 56 20                fcom     qword ptr [esi + 0x20]
015E:    9B                      wait
015F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0161:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0163:    73 07                   jae      0x16c    ; Conditional jump
0165:    DD D8                   fstp     st(0)
0167:    DD 46 28                fld      qword ptr [esi + 0x28]
016A:    EB 17                   jmp      0x183
016C:    D9 E8                   fld1    ; Load 1.0
016E:    D8 D1                   fcom     st(1)
0170:    9B                      wait
0171:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0173:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0175:    72 07                   jb       0x17e    ; Conditional jump
0177:    DD D8                   fstp     st(0)
0179:    DC 7E 30                fdivr    qword ptr [esi + 0x30]
017C:    EB 05                   jmp      0x183
017E:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
0180:    DD 46 30                fld      qword ptr [esi + 0x30]
0183:    DD 41 08                fld      qword ptr [ecx + 8]
0186:    D8 C9                   fmul     st(1)
0188:    DD 59 08                fstp     qword ptr [ecx + 8]
018B:    DC CB                   fmul     st(3), st(0)
018D:    DC CA                   fmul     st(2), st(0)
018F:    DE C9                   fmulp    st(1)
0191:    DD 46 30                fld      qword ptr [esi + 0x30]
0194:    DD 46 DC                fld      qword ptr [esi - 0x24]
0197:    D8 C9                   fmul     st(1)
0199:    DE EA                   fsubp    st(2)
019B:    DD 46 E4                fld      qword ptr [esi - 0x1c]
019E:    D8 C9                   fmul     st(1)
01A0:    DE EB                   fsubp    st(3)
01A2:    DC 4E EC                fmul     qword ptr [esi - 0x14]
01A5:    DE EB                   fsubp    st(3)
01A7:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
01A9:    D9 C0                   fld      st(0)
01AB:    D8 4E FC                fmul     dword ptr [esi - 4]
01AE:    D9 C2                   fld      st(2)
01B0:    D8 4E F8                fmul     dword ptr [esi - 8]
01B3:    DE C1                   faddp    st(1)
01B5:    D9 C3                   fld      st(3)
01B7:    D8 4E F4                fmul     dword ptr [esi - 0xc]
01BA:    DE C1                   faddp    st(1)
01BC:    D9 C1                   fld      st(1)
01BE:    D8 4E 14                fmul     dword ptr [esi + 0x14]
01C1:    D9 C3                   fld      st(3)
01C3:    D8 4E 10                fmul     dword ptr [esi + 0x10]
01C6:    DE C1                   faddp    st(1)
01C8:    D9 C4                   fld      st(4)
01CA:    D8 4E 0C                fmul     dword ptr [esi + 0xc]
01CD:    DE C1                   faddp    st(1)
01CF:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
01D1:    D8 4E 08                fmul     dword ptr [esi + 8]
01D4:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
01D6:    D8 4E 04                fmul     dword ptr [esi + 4]
01D9:    DE C3                   faddp    st(3)
01DB:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
01DD:    D8 0E                   fmul     dword ptr [esi]
01DF:    8B 75 08                mov      esi, dword ptr [ebp + 8]
01E2:    DE C2                   faddp    st(2)
01E4:    DC 46 18                fadd     qword ptr [esi + 0x18]
01E7:    DD 1B                   fstp     qword ptr [ebx]
01E9:    DC 46 20                fadd     qword ptr [esi + 0x20]
01EC:    DD 1A                   fstp     qword ptr [edx]
01EE:    DC 46 28                fadd     qword ptr [esi + 0x28]
01F1:    DD 19                   fstp     qword ptr [ecx]
01F3:    DD D8                   fstp     st(0)
01F5:    83 C4 20                add      esp, 0x20
01F8:    89 D8                   mov      eax, ebx
01FA:    5B                      pop      ebx
01FB:    5E                      pop      esi
01FC:    5D                      pop      ebp
01FD:    C2 08 00                ret      8
