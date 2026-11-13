; Disassembly: _kamtorcut.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 352 bytes, decoded: 352 bytes (100.0%)
; DEoption: -1

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    56                      push     esi
0004:    53                      push     ebx
0005:    8B 75 08                mov      esi, dword ptr [ebp + 8]
0008:    8B D8                   mov      ebx, eax
000A:    8B 76 30                mov      esi, dword ptr [esi + 0x30]
000D:    DD 03                   fld      qword ptr [ebx]
000F:    D8 C8                   fmul     st(0)
0011:    DD 02                   fld      qword ptr [edx]
0013:    D8 C8                   fmul     st(0)
0015:    DE C1                   faddp    st(1)
0017:    D9 FA                   fsqrt    ; sqrt(ST(0))
0019:    DC 4E E8                fmul     qword ptr [esi - 0x18]
001C:    DD 01                   fld      qword ptr [ecx]
001E:    DC 4E F0                fmul     qword ptr [esi - 0x10]
0021:    DE C1                   faddp    st(1)
0023:    DD 19                   fstp     qword ptr [ecx]
0025:    DD 01                   fld      qword ptr [ecx]
0027:    D9 E1                   fabs    ; |ST(0)|
0029:    DD 46 08                fld      qword ptr [esi + 8]
002C:    90                      nop
002D:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
002F:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0031:    80 E4 41                and      ah, 0x41
0034:    7F 10                   jg       0x46    ; Conditional jump
0036:    DD 01                   fld      qword ptr [ecx]
0038:    D9 C0                   fld      st(0)
003A:    D8 C8                   fmul     st(0)
003C:    D9 E0                   fchs    ; -ST(0)
003E:    D9 E8                   fld1    ; Load 1.0
0040:    DE C1                   faddp    st(1)
0042:    D9 FA                   fsqrt    ; sqrt(ST(0))
0044:    EB 10                   jmp      0x56
0046:    DD 06                   fld      qword ptr [esi]
0048:    DD 06                   fld      qword ptr [esi]
004A:    DD 1A                   fstp     qword ptr [edx]
004C:    DD 1B                   fstp     qword ptr [ebx]
004E:    8B C3                   mov      eax, ebx
0050:    5B                      pop      ebx
0051:    5E                      pop      esi
0052:    5D                      pop      ebp
0053:    C2 08 00                ret      8
0056:    D9 D0                   fnop
0058:    DD 03                   fld      qword ptr [ebx]
005A:    DC 4E E0                fmul     qword ptr [esi - 0x20]
005D:    DD 02                   fld      qword ptr [edx]
005F:    DC 4E D8                fmul     qword ptr [esi - 0x28]
0062:    DE C1                   faddp    st(1)
0064:    DD 03                   fld      qword ptr [ebx]
0066:    D8 C8                   fmul     st(0)
0068:    DC 4E D0                fmul     qword ptr [esi - 0x30]
006B:    DE C1                   faddp    st(1)
006D:    DD 02                   fld      qword ptr [edx]
006F:    D8 C8                   fmul     st(0)
0071:    DC 4E C8                fmul     qword ptr [esi - 0x38]
0074:    DE C1                   faddp    st(1)
0076:    DD 03                   fld      qword ptr [ebx]
0078:    DC 0A                   fmul     qword ptr [edx]
007A:    DC 4E C0                fmul     qword ptr [esi - 0x40]
007D:    DE C1                   faddp    st(1)
007F:    DD 03                   fld      qword ptr [ebx]
0081:    D8 C8                   fmul     st(0)
0083:    DD 02                   fld      qword ptr [edx]
0085:    D8 C8                   fmul     st(0)
0087:    DE C1                   faddp    st(1)
0089:    D9 FA                   fsqrt    ; sqrt(ST(0))
008B:    DC 4E B8                fmul     qword ptr [esi - 0x48]
008E:    DE C1                   faddp    st(1)
0090:    D8 CA                   fmul     st(2)
0092:    90                      nop
0093:    90                      nop
0094:    DD 03                   fld      qword ptr [ebx]
0096:    DC 4E B0                fmul     qword ptr [esi - 0x50]
0099:    DD 02                   fld      qword ptr [edx]
009B:    DC 4E A8                fmul     qword ptr [esi - 0x58]
009E:    DE C1                   faddp    st(1)
00A0:    DD 03                   fld      qword ptr [ebx]
00A2:    D8 C8                   fmul     st(0)
00A4:    DC 4E A0                fmul     qword ptr [esi - 0x60]
00A7:    DE C1                   faddp    st(1)
00A9:    DD 02                   fld      qword ptr [edx]
00AB:    D8 C8                   fmul     st(0)
00AD:    DC 4E 98                fmul     qword ptr [esi - 0x68]
00B0:    DE C1                   faddp    st(1)
00B2:    DD 03                   fld      qword ptr [ebx]
00B4:    DC 0A                   fmul     qword ptr [edx]
00B6:    DC 4E 90                fmul     qword ptr [esi - 0x70]
00B9:    DE C1                   faddp    st(1)
00BB:    DD 03                   fld      qword ptr [ebx]
00BD:    D8 C8                   fmul     st(0)
00BF:    DD 02                   fld      qword ptr [edx]
00C1:    D8 C8                   fmul     st(0)
00C3:    DE C1                   faddp    st(1)
00C5:    D9 FA                   fsqrt    ; sqrt(ST(0))
00C7:    DC 4E 88                fmul     qword ptr [esi - 0x78]
00CA:    DE C1                   faddp    st(1)
00CC:    D8 CA                   fmul     st(2)
00CE:    DE C1                   faddp    st(1)
00D0:    D9 D0                   fnop
00D2:    DD 03                   fld      qword ptr [ebx]
00D4:    DC 4E E0                fmul     qword ptr [esi - 0x20]
00D7:    DD 02                   fld      qword ptr [edx]
00D9:    DC 4E D8                fmul     qword ptr [esi - 0x28]
00DC:    DE C1                   faddp    st(1)
00DE:    DD 03                   fld      qword ptr [ebx]
00E0:    D8 C8                   fmul     st(0)
00E2:    DC 4E D0                fmul     qword ptr [esi - 0x30]
00E5:    DE C1                   faddp    st(1)
00E7:    DD 02                   fld      qword ptr [edx]
00E9:    D8 C8                   fmul     st(0)
00EB:    DC 4E C8                fmul     qword ptr [esi - 0x38]
00EE:    DE C1                   faddp    st(1)
00F0:    DD 03                   fld      qword ptr [ebx]
00F2:    DC 0A                   fmul     qword ptr [edx]
00F4:    DC 4E C0                fmul     qword ptr [esi - 0x40]
00F7:    DE C1                   faddp    st(1)
00F9:    DD 03                   fld      qword ptr [ebx]
00FB:    D8 C8                   fmul     st(0)
00FD:    DD 02                   fld      qword ptr [edx]
00FF:    D8 C8                   fmul     st(0)
0101:    DE C1                   faddp    st(1)
0103:    D9 FA                   fsqrt    ; sqrt(ST(0))
0105:    DC 4E B8                fmul     qword ptr [esi - 0x48]
0108:    DE C1                   faddp    st(1)
010A:    D8 CA                   fmul     st(2)
010C:    90                      nop
010D:    90                      nop
010E:    DD 03                   fld      qword ptr [ebx]
0110:    DC 4E B0                fmul     qword ptr [esi - 0x50]
0113:    DD 02                   fld      qword ptr [edx]
0115:    DC 4E A8                fmul     qword ptr [esi - 0x58]
0118:    DE C1                   faddp    st(1)
011A:    DD 03                   fld      qword ptr [ebx]
011C:    D8 C8                   fmul     st(0)
011E:    DC 4E A0                fmul     qword ptr [esi - 0x60]
0121:    DE C1                   faddp    st(1)
0123:    DD 02                   fld      qword ptr [edx]
0125:    D8 C8                   fmul     st(0)
0127:    DC 4E 98                fmul     qword ptr [esi - 0x68]
012A:    DE C1                   faddp    st(1)
012C:    DD 03                   fld      qword ptr [ebx]
012E:    DC 0A                   fmul     qword ptr [edx]
0130:    DC 4E 90                fmul     qword ptr [esi - 0x70]
0133:    DE C1                   faddp    st(1)
0135:    DD 03                   fld      qword ptr [ebx]
0137:    D8 C8                   fmul     st(0)
0139:    DD 02                   fld      qword ptr [edx]
013B:    D8 C8                   fmul     st(0)
013D:    DE C1                   faddp    st(1)
013F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0141:    DC 4E 88                fmul     qword ptr [esi - 0x78]
0144:    DE C1                   faddp    st(1)
0146:    D8 CC                   fmul     st(4)
0148:    DE C1                   faddp    st(1)
014A:    D9 D0                   fnop
014C:    DD 1A                   fstp     qword ptr [edx]
014E:    DD 1B                   fstp     qword ptr [ebx]
0150:    DD C0                   ffree    st(0)
0152:    DD C1                   ffree    st(1)
0154:    D9 F7                   fincstp
0156:    D9 F7                   fincstp
0158:    8B C3                   mov      eax, ebx
015A:    5B                      pop      ebx
015B:    5E                      pop      esi
015C:    5D                      pop      ebp
015D:    C2 08 00                ret      8
