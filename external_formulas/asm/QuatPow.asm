; Disassembly: QuatPow.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 531 bytes, decoded: 531 bytes (100.0%)
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
0010:    DD 41 08                fld      qword ptr [ecx + 8]
0013:    D8 C8                   fmul     st(0)
0015:    DD 02                   fld      qword ptr [edx]
0017:    D8 C8                   fmul     st(0)
0019:    DE C1                   faddp    st(1)
001B:    DD 01                   fld      qword ptr [ecx]
001D:    D8 C8                   fmul     st(0)
001F:    DE C1                   faddp    st(1)
0021:    D9 FA                   fsqrt    ; sqrt(ST(0))
0023:    D9 C0                   fld      st(0)
0025:    D9 E4                   ftst
0027:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0029:    D9 D0                   fnop
002B:    9E                      sahf
002C:    DD C0                   ffree    st(0)
002E:    D9 F7                   fincstp
0030:    74 51                   je       0x83    ; Conditional jump
0032:    D9 C0                   fld      st(0)
0034:    DD 03                   fld      qword ptr [ebx]
0036:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0038:    D9 C1                   fld      st(1)
003A:    DE F9                   fdivp    st(1)
003C:    DD 03                   fld      qword ptr [ebx]
003E:    D8 C8                   fmul     st(0)
0040:    D9 C2                   fld      st(2)
0042:    D8 C8                   fmul     st(0)
0044:    DE C1                   faddp    st(1)
0046:    D9 C0                   fld      st(0)
0048:    DD 07                   fld      qword ptr [edi]
004A:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
004C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
004E:    80 E4 41                and      ah, 0x41
0051:    74 0D                   je       0x60    ; Conditional jump
0053:    D9 C0                   fld      st(0)
0055:    D9 ED                   fldln2    ; Load ln(2)
0057:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0059:    D9 F1                   fyl2x
005B:    DC 4F 08                fmul     qword ptr [edi + 8]
005E:    EB 03                   jmp      0x63
0060:    DD 47 10                fld      qword ptr [edi + 0x10]
0063:    DD 1B                   fstp     qword ptr [ebx]
0065:    DD 02                   fld      qword ptr [edx]
0067:    D8 CA                   fmul     st(2)
0069:    DD 1A                   fstp     qword ptr [edx]
006B:    DD 01                   fld      qword ptr [ecx]
006D:    D8 CA                   fmul     st(2)
006F:    DD 19                   fstp     qword ptr [ecx]
0071:    DD 41 08                fld      qword ptr [ecx + 8]
0074:    D8 CA                   fmul     st(2)
0076:    DD 59 08                fstp     qword ptr [ecx + 8]
0079:    DD D8                   fstp     st(0)
007B:    DD D8                   fstp     st(0)
007D:    DD D8                   fstp     st(0)
007F:    EB 25                   jmp      0xa6
0081:    90                      nop
0082:    90                      nop
0083:    90                      nop
0084:    DD D8                   fstp     st(0)
0086:    DD 03                   fld      qword ptr [ebx]
0088:    DD 07                   fld      qword ptr [edi]
008A:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
008C:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
008E:    80 E4 41                and      ah, 0x41
0091:    74 0E                   je       0xa1    ; Conditional jump
0093:    DD 03                   fld      qword ptr [ebx]
0095:    D9 ED                   fldln2    ; Load ln(2)
0097:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0099:    D9 F1                   fyl2x
009B:    DD 1B                   fstp     qword ptr [ebx]
009D:    EB 07                   jmp      0xa6
009F:    90                      nop
00A0:    90                      nop
00A1:    DD 47 10                fld      qword ptr [edi + 0x10]
00A4:    DD 1B                   fstp     qword ptr [ebx]
00A6:    D9 D0                   fnop
00A8:    D9 D0                   fnop
00AA:    DD 41 08                fld      qword ptr [ecx + 8]
00AD:    DD 47 B8                fld      qword ptr [edi - 0x48]
00B0:    DE C9                   fmulp    st(1)
00B2:    DD 59 08                fstp     qword ptr [ecx + 8]
00B5:    DD 01                   fld      qword ptr [ecx]
00B7:    DD 47 C0                fld      qword ptr [edi - 0x40]
00BA:    DE C9                   fmulp    st(1)
00BC:    DD 19                   fstp     qword ptr [ecx]
00BE:    DD 02                   fld      qword ptr [edx]
00C0:    DD 47 C8                fld      qword ptr [edi - 0x38]
00C3:    DE C9                   fmulp    st(1)
00C5:    DD 1A                   fstp     qword ptr [edx]
00C7:    DD 03                   fld      qword ptr [ebx]
00C9:    DD 47 D0                fld      qword ptr [edi - 0x30]
00CC:    DE C9                   fmulp    st(1)
00CE:    DD 1B                   fstp     qword ptr [ebx]
00D0:    D9 D0                   fnop
00D2:    D9 D0                   fnop
00D4:    DD 03                   fld      qword ptr [ebx]
00D6:    90                      nop
00D7:    DC 4F F0                fmul     qword ptr [edi - 0x10]
00DA:    90                      nop
00DB:    90                      nop
00DC:    DD 02                   fld      qword ptr [edx]
00DE:    90                      nop
00DF:    DC 4F E8                fmul     qword ptr [edi - 0x18]
00E2:    D9 E0                   fchs    ; -ST(0)
00E4:    DE C1                   faddp    st(1)
00E6:    DD 01                   fld      qword ptr [ecx]
00E8:    90                      nop
00E9:    DC 4F E0                fmul     qword ptr [edi - 0x20]
00EC:    D9 E0                   fchs    ; -ST(0)
00EE:    DE C1                   faddp    st(1)
00F0:    DD 41 08                fld      qword ptr [ecx + 8]
00F3:    DC 4F D8                fmul     qword ptr [edi - 0x28]
00F6:    D9 E0                   fchs    ; -ST(0)
00F8:    DE C1                   faddp    st(1)
00FA:    D9 D0                   fnop
00FC:    DD 03                   fld      qword ptr [ebx]
00FE:    90                      nop
00FF:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0102:    90                      nop
0103:    90                      nop
0104:    DD 02                   fld      qword ptr [edx]
0106:    90                      nop
0107:    DC 4F F0                fmul     qword ptr [edi - 0x10]
010A:    90                      nop
010B:    90                      nop
010C:    DE C1                   faddp    st(1)
010E:    DD 01                   fld      qword ptr [ecx]
0110:    90                      nop
0111:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0114:    90                      nop
0115:    90                      nop
0116:    DE C1                   faddp    st(1)
0118:    DD 41 08                fld      qword ptr [ecx + 8]
011B:    DC 4F E0                fmul     qword ptr [edi - 0x20]
011E:    D9 E0                   fchs    ; -ST(0)
0120:    DE C1                   faddp    st(1)
0122:    D9 D0                   fnop
0124:    DD 03                   fld      qword ptr [ebx]
0126:    90                      nop
0127:    DC 4F E0                fmul     qword ptr [edi - 0x20]
012A:    90                      nop
012B:    90                      nop
012C:    DD 02                   fld      qword ptr [edx]
012E:    90                      nop
012F:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0132:    D9 E0                   fchs    ; -ST(0)
0134:    DE C1                   faddp    st(1)
0136:    DD 01                   fld      qword ptr [ecx]
0138:    90                      nop
0139:    DC 4F F0                fmul     qword ptr [edi - 0x10]
013C:    90                      nop
013D:    90                      nop
013E:    DE C1                   faddp    st(1)
0140:    DD 41 08                fld      qword ptr [ecx + 8]
0143:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0146:    90                      nop
0147:    90                      nop
0148:    DE C1                   faddp    st(1)
014A:    D9 D0                   fnop
014C:    DD 03                   fld      qword ptr [ebx]
014E:    90                      nop
014F:    DC 4F D8                fmul     qword ptr [edi - 0x28]
0152:    90                      nop
0153:    90                      nop
0154:    DD 02                   fld      qword ptr [edx]
0156:    90                      nop
0157:    DC 4F E0                fmul     qword ptr [edi - 0x20]
015A:    90                      nop
015B:    90                      nop
015C:    DE C1                   faddp    st(1)
015E:    DD 01                   fld      qword ptr [ecx]
0160:    90                      nop
0161:    DC 4F E8                fmul     qword ptr [edi - 0x18]
0164:    D9 E0                   fchs    ; -ST(0)
0166:    DE C1                   faddp    st(1)
0168:    DD 41 08                fld      qword ptr [ecx + 8]
016B:    DC 4F F0                fmul     qword ptr [edi - 0x10]
016E:    90                      nop
016F:    90                      nop
0170:    DE C1                   faddp    st(1)
0172:    D9 D0                   fnop
0174:    90                      nop
0175:    DD 59 08                fstp     qword ptr [ecx + 8]
0178:    DD 19                   fstp     qword ptr [ecx]
017A:    DD 1A                   fstp     qword ptr [edx]
017C:    DD 1B                   fstp     qword ptr [ebx]
017E:    DD 41 08                fld      qword ptr [ecx + 8]
0181:    D8 C8                   fmul     st(0)
0183:    DD 02                   fld      qword ptr [edx]
0185:    D8 C8                   fmul     st(0)
0187:    DE C1                   faddp    st(1)
0189:    DD 01                   fld      qword ptr [ecx]
018B:    D8 C8                   fmul     st(0)
018D:    DE C1                   faddp    st(1)
018F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0191:    D9 C0                   fld      st(0)
0193:    D9 FB                   fsincos    ; ST(0)=cos, push sin
0195:    D9 D0                   fnop
0197:    DD 03                   fld      qword ptr [ebx]
0199:    D9 EA                   fldl2e    ; Load log2(e)
019B:    DE C9                   fmulp    st(1)
019D:    D9 E8                   fld1    ; Load 1.0
019F:    D9 C1                   fld      st(1)
01A1:    D9 F8                   fprem    ; ST(0) = ST(0) mod ST(1)
01A3:    D9 F0                   f2xm1
01A5:    DE C1                   faddp    st(1)
01A7:    D9 FD                   fscale
01A9:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
01AB:    DD D8                   fstp     st(0)
01AD:    D9 C0                   fld      st(0)
01AF:    D8 CA                   fmul     st(2)
01B1:    DD 1B                   fstp     qword ptr [ebx]
01B3:    D8 CA                   fmul     st(2)
01B5:    D9 C3                   fld      st(3)
01B7:    DD 07                   fld      qword ptr [edi]
01B9:    DE D9                   fcompp    ; Compare ST(0) with ST(1) and pop both
01BB:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01BD:    80 E4 41                and      ah, 0x41
01C0:    74 02                   je       0x1c4    ; Conditional jump
01C2:    D8 F3                   fdiv     st(3)
01C4:    DD 02                   fld      qword ptr [edx]
01C6:    D8 C9                   fmul     st(1)
01C8:    DD 1A                   fstp     qword ptr [edx]
01CA:    DD 01                   fld      qword ptr [ecx]
01CC:    D8 C9                   fmul     st(1)
01CE:    DD 19                   fstp     qword ptr [ecx]
01D0:    DD 41 08                fld      qword ptr [ecx + 8]
01D3:    D8 C9                   fmul     st(1)
01D5:    DD 59 08                fstp     qword ptr [ecx + 8]
01D8:    DD D8                   fstp     st(0)
01DA:    DD D8                   fstp     st(0)
01DC:    DD D8                   fstp     st(0)
01DE:    DD D8                   fstp     st(0)
01E0:    D9 D0                   fnop
01E2:    D9 D0                   fnop
01E4:    DD 01                   fld      qword ptr [ecx]
01E6:    DD 02                   fld      qword ptr [edx]
01E8:    DD 03                   fld      qword ptr [ebx]
01EA:    DD 46 18                fld      qword ptr [esi + 0x18]
01ED:    DC 4F B0                fmul     qword ptr [edi - 0x50]
01F0:    DE C1                   faddp    st(1)
01F2:    DD 1B                   fstp     qword ptr [ebx]
01F4:    DD 46 20                fld      qword ptr [esi + 0x20]
01F7:    DC 4F B0                fmul     qword ptr [edi - 0x50]
01FA:    DE C1                   faddp    st(1)
01FC:    DD 1A                   fstp     qword ptr [edx]
01FE:    DD 46 28                fld      qword ptr [esi + 0x28]
0201:    DC 4F B0                fmul     qword ptr [edi - 0x50]
0204:    DE C1                   faddp    st(1)
0206:    DD 19                   fstp     qword ptr [ecx]
0208:    90                      nop
0209:    8B C3                   mov      eax, ebx
020B:    5F                      pop      edi
020C:    5E                      pop      esi
020D:    58                      pop      eax
020E:    5B                      pop      ebx
020F:    5D                      pop      ebp
0210:    C2 08 00                ret      8
