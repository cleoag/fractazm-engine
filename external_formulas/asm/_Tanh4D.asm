; Disassembly: _Tanh4D.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 488 bytes, decoded: 488 bytes (100.0%)
; DEoption: 4

0000:    55                      push     ebp
0001:    8B EC                   mov      ebp, esp
0003:    53                      push     ebx
0004:    50                      push     eax
0005:    56                      push     esi
0006:    57                      push     edi
0007:    90                      nop
0008:    8B 75 08                mov      esi, dword ptr [ebp + 8]
000B:    8B D8                   mov      ebx, eax
000D:    8B 7E 30                mov      edi, dword ptr [esi + 0x30]
0010:    DD 03                   fld      qword ptr [ebx]
0012:    90                      nop
0013:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0016:    90                      nop
0017:    90                      nop
0018:    DD 02                   fld      qword ptr [edx]
001A:    90                      nop
001B:    DC 4F E8                fmul     qword ptr [edi - 0x18]
001E:    D9 E0                   fchs    ; -ST(0)
0020:    DE C1                   faddp    st(1)
0022:    DD 01                   fld      qword ptr [ecx]
0024:    90                      nop
0025:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0028:    D9 E0                   fchs    ; -ST(0)
002A:    DE C1                   faddp    st(1)
002C:    DD 41 08                fld      qword ptr [ecx + 8]
002F:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0032:    D9 E0                   fchs    ; -ST(0)
0034:    DE C1                   faddp    st(1)
0036:    D9 D0                   fnop
0038:    DD 03                   fld      qword ptr [ebx]
003A:    90                      nop
003B:    DC 4F E8                fmul     qword ptr [edi - 0x18]
003E:    90                      nop
003F:    90                      nop
0040:    DD 02                   fld      qword ptr [edx]
0042:    90                      nop
0043:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0046:    90                      nop
0047:    90                      nop
0048:    DE C1                   faddp    st(1)
004A:    DD 01                   fld      qword ptr [ecx]
004C:    90                      nop
004D:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0050:    90                      nop
0051:    90                      nop
0052:    DE C1                   faddp    st(1)
0054:    DD 41 08                fld      qword ptr [ecx + 8]
0057:    DC 4F E0                fmul     qword ptr [edi - 0x20]
005A:    D9 E0                   fchs    ; -ST(0)
005C:    DE C1                   faddp    st(1)
005E:    D9 D0                   fnop
0060:    DD 03                   fld      qword ptr [ebx]
0062:    90                      nop
0063:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0066:    90                      nop
0067:    90                      nop
0068:    DD 02                   fld      qword ptr [edx]
006A:    90                      nop
006B:    DC 4F D8                fmul     qword ptr [edi - 0x28]
006E:    D9 E0                   fchs    ; -ST(0)
0070:    DE C1                   faddp    st(1)
0072:    DD 01                   fld      qword ptr [ecx]
0074:    90                      nop
0075:    DC 4F F0                fmul     qword ptr [edi - 0x10]
0078:    90                      nop
0079:    90                      nop
007A:    DE C1                   faddp    st(1)
007C:    DD 41 08                fld      qword ptr [ecx + 8]
007F:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0082:    90                      nop
0083:    90                      nop
0084:    DE C1                   faddp    st(1)
0086:    D9 D0                   fnop
0088:    DD 03                   fld      qword ptr [ebx]
008A:    90                      nop
008B:    DC 4F D8                fmul     qword ptr [edi - 0x28]
008E:    90                      nop
008F:    90                      nop
0090:    DD 02                   fld      qword ptr [edx]
0092:    90                      nop
0093:    DC 4F E0                fmul     qword ptr [edi - 0x20]
0096:    90                      nop
0097:    90                      nop
0098:    DE C1                   faddp    st(1)
009A:    DD 01                   fld      qword ptr [ecx]
009C:    90                      nop
009D:    DC 4F E8                fmul     qword ptr [edi - 0x18]
00A0:    D9 E0                   fchs    ; -ST(0)
00A2:    DE C1                   faddp    st(1)
00A4:    DD 41 08                fld      qword ptr [ecx + 8]
00A7:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00AA:    90                      nop
00AB:    90                      nop
00AC:    DE C1                   faddp    st(1)
00AE:    D9 D0                   fnop
00B0:    90                      nop
00B1:    DD 59 08                fstp     qword ptr [ecx + 8]
00B4:    DD 19                   fstp     qword ptr [ecx]
00B6:    DD 1A                   fstp     qword ptr [edx]
00B8:    DD 1B                   fstp     qword ptr [ebx]
00BA:    DD 41 08                fld      qword ptr [ecx + 8]
00BD:    D8 C8                   fmul     st(0)
00BF:    DD 02                   fld      qword ptr [edx]
00C1:    D8 C8                   fmul     st(0)
00C3:    DE C1                   faddp    st(1)
00C5:    DD 01                   fld      qword ptr [ecx]
00C7:    D8 C8                   fmul     st(0)
00C9:    DE C1                   faddp    st(1)
00CB:    D9 FA                   fsqrt    ; sqrt(ST(0))
00CD:    D9 C0                   fld      st(0)
00CF:    D9 FB                   fsincos    ; ST(0)=cos, push sin
00D1:    D9 D0                   fnop
00D3:    DD 03                   fld      qword ptr [ebx]
00D5:    D9 EA                   fldl2e    ; Load log2(e)
00D7:    DE C9                   fmulp    st(1)
00D9:    D9 E8                   fld1    ; Load 1.0
00DB:    D9 C1                   fld      st(1)
00DD:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
00DF:    D9 F0                   f2xm1
00E1:    DE C1                   faddp    st(1)
00E3:    D9 FD                   fscale
00E5:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
00E7:    DD D8                   fstp     st(0)
00E9:    D9 C0                   fld      st(0)
00EB:    D8 CA                   fmul     st(2)
00ED:    DD 1B                   fstp     qword ptr [ebx]
00EF:    D8 CA                   fmul     st(2)
00F1:    D9 C3                   fld      st(3)
00F3:    DD 07                   fld      qword ptr [edi]
00F5:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
00F7:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
00F9:    80 E4 41                and      ah, 0x41
00FC:    74 02                   je       0x100    ; Conditional jump
00FE:    D8 F3                   fdiv     st(3)
0100:    DD 02                   fld      qword ptr [edx]
0102:    D8 C9                   fmul     st(1)
0104:    DD 1A                   fstp     qword ptr [edx]
0106:    DD 01                   fld      qword ptr [ecx]
0108:    D8 C9                   fmul     st(1)
010A:    DD 19                   fstp     qword ptr [ecx]
010C:    DD 41 08                fld      qword ptr [ecx + 8]
010F:    D8 C9                   fmul     st(1)
0111:    DD 59 08                fstp     qword ptr [ecx + 8]
0114:    DD D8                   fstp     st(0)
0116:    DD D8                   fstp     st(0)
0118:    DD D8                   fstp     st(0)
011A:    DD D8                   fstp     st(0)
011C:    90                      nop
011D:    DD 41 08                fld      qword ptr [ecx + 8]
0120:    D8 C8                   fmul     st(0)
0122:    DD 02                   fld      qword ptr [edx]
0124:    D8 C8                   fmul     st(0)
0126:    DE C1                   faddp    st(1)
0128:    DD 01                   fld      qword ptr [ecx]
012A:    D8 C8                   fmul     st(0)
012C:    DE C1                   faddp    st(1)
012E:    DD 03                   fld      qword ptr [ebx]
0130:    D9 E8                   fld1    ; Load 1.0
0132:    DE C1                   faddp    st(1)
0134:    D8 C8                   fmul     st(0)
0136:    DE C1                   faddp    st(1)
0138:    D9 E8                   fld1    ; Load 1.0
013A:    DE F1                   fdivrp   st(1)
013C:    D9 D0                   fnop
013E:    DD 03                   fld      qword ptr [ebx]
0140:    D9 E8                   fld1    ; Load 1.0
0142:    DE C1                   faddp    st(1)
0144:    DD 03                   fld      qword ptr [ebx]
0146:    D9 E8                   fld1    ; Load 1.0
0148:    D9 E0                   fchs    ; -ST(0)
014A:    DE C1                   faddp    st(1)
014C:    DE C9                   fmulp    st(1)
014E:    DD 02                   fld      qword ptr [edx]
0150:    D8 C8                   fmul     st(0)
0152:    DE C1                   faddp    st(1)
0154:    DD 01                   fld      qword ptr [ecx]
0156:    D8 C8                   fmul     st(0)
0158:    DE C1                   faddp    st(1)
015A:    DD 41 08                fld      qword ptr [ecx + 8]
015D:    D8 C8                   fmul     st(0)
015F:    DE C1                   faddp    st(1)
0161:    D9 D0                   fnop
0163:    DD 03                   fld      qword ptr [ebx]
0165:    D9 E8                   fld1    ; Load 1.0
0167:    DE C1                   faddp    st(1)
0169:    DC 0A                   fmul     qword ptr [edx]
016B:    DD 02                   fld      qword ptr [edx]
016D:    DD 03                   fld      qword ptr [ebx]
016F:    D9 E8                   fld1    ; Load 1.0
0171:    D9 E0                   fchs    ; -ST(0)
0173:    DE C1                   faddp    st(1)
0175:    DE C9                   fmulp    st(1)
0177:    D9 E0                   fchs    ; -ST(0)
0179:    DE C1                   faddp    st(1)
017B:    D9 D0                   fnop
017D:    DD 03                   fld      qword ptr [ebx]
017F:    D9 E8                   fld1    ; Load 1.0
0181:    DE C1                   faddp    st(1)
0183:    DC 09                   fmul     qword ptr [ecx]
0185:    DD 03                   fld      qword ptr [ebx]
0187:    D9 E8                   fld1    ; Load 1.0
0189:    D9 E0                   fchs    ; -ST(0)
018B:    DE C1                   faddp    st(1)
018D:    DC 09                   fmul     qword ptr [ecx]
018F:    D9 E0                   fchs    ; -ST(0)
0191:    DE C1                   faddp    st(1)
0193:    D9 D0                   fnop
0195:    DD 03                   fld      qword ptr [ebx]
0197:    D9 E8                   fld1    ; Load 1.0
0199:    DE C1                   faddp    st(1)
019B:    DC 49 08                fmul     qword ptr [ecx + 8]
019E:    DD 03                   fld      qword ptr [ebx]
01A0:    D9 E8                   fld1    ; Load 1.0
01A2:    D9 E0                   fchs    ; -ST(0)
01A4:    DE C1                   faddp    st(1)
01A6:    DC 49 08                fmul     qword ptr [ecx + 8]
01A9:    D9 E0                   fchs    ; -ST(0)
01AB:    DE C1                   faddp    st(1)
01AD:    D9 D0                   fnop
01AF:    D9 C4                   fld      st(4)
01B1:    D8 CC                   fmul     st(4)
01B3:    DC 4F D0                fmul     qword ptr [edi - 0x30]
01B6:    DD 1B                   fstp     qword ptr [ebx]
01B8:    D9 C4                   fld      st(4)
01BA:    D8 CB                   fmul     st(3)
01BC:    DC 4F D0                fmul     qword ptr [edi - 0x30]
01BF:    DD 1A                   fstp     qword ptr [edx]
01C1:    D9 C4                   fld      st(4)
01C3:    D8 CA                   fmul     st(2)
01C5:    DC 4F D0                fmul     qword ptr [edi - 0x30]
01C8:    DD 19                   fstp     qword ptr [ecx]
01CA:    D9 C4                   fld      st(4)
01CC:    D8 C9                   fmul     st(1)
01CE:    DC 4F D0                fmul     qword ptr [edi - 0x30]
01D1:    DD 59 08                fstp     qword ptr [ecx + 8]
01D4:    DD D8                   fstp     st(0)
01D6:    DD D8                   fstp     st(0)
01D8:    DD D8                   fstp     st(0)
01DA:    DD D8                   fstp     st(0)
01DC:    DD D8                   fstp     st(0)
01DE:    8B C3                   mov      eax, ebx
01E0:    5F                      pop      edi
01E1:    5E                      pop      esi
01E2:    58                      pop      eax
01E3:    5B                      pop      ebx
01E4:    5D                      pop      ebp
01E5:    C2 08 00                ret      8
