; Disassembly: WeirdCubeIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 674 bytes, decoded: 674 bytes (100.0%)
; DEoption: 20

0000:    83 C4 A0                add      esp, -0x60
0003:    66 0F 10 46 88          movupd   xmm0, xmmword ptr [esi - 0x78]
0008:    F2 0F 10 4E 98          movsd    xmm1, qword ptr [esi - 0x68]
000D:    66 0F 59 47 E0          mulpd    xmm0, xmmword ptr [edi - 0x20]
0012:    F2 0F 59 4F E0          mulsd    xmm1, qword ptr [edi - 0x20]
0017:    66 0F 58 47 C0          addpd    xmm0, xmmword ptr [edi - 0x40]
001C:    F2 0F 58 4F D0          addsd    xmm1, qword ptr [edi - 0x30]
0021:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0025:    74 19                   je       0x40    ; Conditional jump
0027:    F2 0F 10 6E 70          movsd    xmm5, qword ptr [esi + 0x70]
002C:    F2 0F 59 6F E0          mulsd    xmm5, qword ptr [edi - 0x20]
0031:    66 0F 11 46 88          movupd   xmmword ptr [esi - 0x78], xmm0
0036:    F2 0F 11 4E 98          movsd    qword ptr [esi - 0x68], xmm1
003B:    F2 0F 11 6E 70          movsd    qword ptr [esi + 0x70], xmm5
0040:    66 0F 11 04 24          movupd   xmmword ptr [esp], xmm0
0045:    F2 0F 11 4C 24 10       movsd    qword ptr [esp + 0x10], xmm1
004B:    D9 EE                   fldz    ; Load 0.0
004D:    DD 54 24 18             fst      qword ptr [esp + 0x18]
0051:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0055:    83 C7 D4                add      edi, -0x2c
0058:    DD 44 24 10             fld      qword ptr [esp + 0x10]
005C:    DD 44 24 08             fld      qword ptr [esp + 8]
0060:    DD 04 24                fld      qword ptr [esp]
0063:    D9 C0                   fld      st(0)
0065:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
0068:    D9 C2                   fld      st(2)
006A:    D8 4F C0                fmul     dword ptr [edi - 0x40]
006D:    DE C1                   faddp    st(1)
006F:    D9 C3                   fld      st(3)
0071:    D8 4F BC                fmul     dword ptr [edi - 0x44]
0074:    DE C1                   faddp    st(1)
0076:    D9 C1                   fld      st(1)
0078:    D8 4F DC                fmul     dword ptr [edi - 0x24]
007B:    D9 C3                   fld      st(3)
007D:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0080:    DE C1                   faddp    st(1)
0082:    D9 C4                   fld      st(4)
0084:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
0087:    DE C1                   faddp    st(1)
0089:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
008B:    D8 4F D0                fmul     dword ptr [edi - 0x30]
008E:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0090:    D8 4F CC                fmul     dword ptr [edi - 0x34]
0093:    DE C3                   faddp    st(3)
0095:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0097:    D8 4F C8                fmul     dword ptr [edi - 0x38]
009A:    DE C2                   faddp    st(2)
009C:    DD 1C 24                fstp     qword ptr [esp]
009F:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00A3:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00A7:    83 C7 2C                add      edi, 0x2c
00AA:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00AE:    D8 C8                   fmul     st(0)
00B0:    DD 44 24 08             fld      qword ptr [esp + 8]
00B4:    D8 C8                   fmul     st(0)
00B6:    DD 04 24                fld      qword ptr [esp]
00B9:    90                      nop
00BA:    D8 C8                   fmul     st(0)
00BC:    DD 5C 24 28             fstp     qword ptr [esp + 0x28]
00C0:    DD 5C 24 30             fstp     qword ptr [esp + 0x30]
00C4:    DD 5C 24 38             fstp     qword ptr [esp + 0x38]
00C8:    DD 04 24                fld      qword ptr [esp]
00CB:    DC 4C 24 08             fmul     qword ptr [esp + 8]
00CF:    DD 44 24 08             fld      qword ptr [esp + 8]
00D3:    DC 4C 24 10             fmul     qword ptr [esp + 0x10]
00D7:    DD 04 24                fld      qword ptr [esp]
00DA:    DC 4C 24 10             fmul     qword ptr [esp + 0x10]
00DE:    DD 5C 24 50             fstp     qword ptr [esp + 0x50]
00E2:    DD 5C 24 48             fstp     qword ptr [esp + 0x48]
00E6:    DD 5C 24 40             fstp     qword ptr [esp + 0x40]
00EA:    D9 D0                   fnop
00EC:    DD 44 24 40             fld      qword ptr [esp + 0x40]
00F0:    D8 C8                   fmul     st(0)
00F2:    DD 44 24 48             fld      qword ptr [esp + 0x48]
00F6:    D8 C8                   fmul     st(0)
00F8:    DD 44 24 50             fld      qword ptr [esp + 0x50]
00FC:    D8 C8                   fmul     st(0)
00FE:    DE C1                   faddp    st(1)
0100:    DE C1                   faddp    st(1)
0102:    DC 4F 08                fmul     qword ptr [edi + 8]
0105:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0109:    90                      nop
010A:    DD 04 24                fld      qword ptr [esp]
010D:    DC 4C 24 28             fmul     qword ptr [esp + 0x28]
0111:    DD 44 24 08             fld      qword ptr [esp + 8]
0115:    DC 44 24 10             fadd     qword ptr [esp + 0x10]
0119:    DE C9                   fmulp    st(1)
011B:    DC 0F                   fmul     qword ptr [edi]
011D:    90                      nop
011E:    D9 E4                   ftst
0120:    9B                      wait
0121:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0123:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0125:    73 0A                   jae      0x131    ; Conditional jump
0127:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
012B:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
012F:    EB 08                   jmp      0x139
0131:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
0135:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0139:    DD 44 24 08             fld      qword ptr [esp + 8]
013D:    DC 4C 24 30             fmul     qword ptr [esp + 0x30]
0141:    DD 04 24                fld      qword ptr [esp]
0144:    90                      nop
0145:    DC 44 24 10             fadd     qword ptr [esp + 0x10]
0149:    DE C9                   fmulp    st(1)
014B:    DC 0F                   fmul     qword ptr [edi]
014D:    90                      nop
014E:    D9 E4                   ftst
0150:    9B                      wait
0151:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0153:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0155:    73 0A                   jae      0x161    ; Conditional jump
0157:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
015B:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
015F:    EB 08                   jmp      0x169
0161:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
0165:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0169:    DD 44 24 10             fld      qword ptr [esp + 0x10]
016D:    DC 4C 24 38             fmul     qword ptr [esp + 0x38]
0171:    DD 04 24                fld      qword ptr [esp]
0174:    90                      nop
0175:    DC 44 24 08             fadd     qword ptr [esp + 8]
0179:    DE C9                   fmulp    st(1)
017B:    DC 0F                   fmul     qword ptr [edi]
017D:    90                      nop
017E:    D9 E4                   ftst
0180:    9B                      wait
0181:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0183:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0185:    73 0A                   jae      0x191    ; Conditional jump
0187:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
018B:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
018F:    EB 08                   jmp      0x199
0191:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
0195:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0199:    DD 04 24                fld      qword ptr [esp]
019C:    DC 4C 24 48             fmul     qword ptr [esp + 0x48]
01A0:    DC 4F B8                fmul     qword ptr [edi - 0x48]
01A3:    D9 E4                   ftst
01A5:    9B                      wait
01A6:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01A8:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01AA:    73 0A                   jae      0x1b6    ; Conditional jump
01AC:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
01B0:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
01B4:    EB 08                   jmp      0x1be
01B6:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
01BA:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
01BE:    DD 44 24 28             fld      qword ptr [esp + 0x28]
01C2:    DC 4F 18                fmul     qword ptr [edi + 0x18]
01C5:    DC 4F B8                fmul     qword ptr [edi - 0x48]
01C8:    90                      nop
01C9:    D9 E8                   fld1    ; Load 1.0
01CB:    DE C1                   faddp    st(1)
01CD:    DC 4C 24 48             fmul     qword ptr [esp + 0x48]
01D1:    DC 4F 20                fmul     qword ptr [edi + 0x20]
01D4:    D9 E4                   ftst
01D6:    9B                      wait
01D7:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01D9:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01DB:    73 0A                   jae      0x1e7    ; Conditional jump
01DD:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
01E1:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
01E5:    EB 08                   jmp      0x1ef
01E7:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
01EB:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
01EF:    DD 44 24 30             fld      qword ptr [esp + 0x30]
01F3:    DC 4F 18                fmul     qword ptr [edi + 0x18]
01F6:    DC 4F B8                fmul     qword ptr [edi - 0x48]
01F9:    D9 E8                   fld1    ; Load 1.0
01FB:    DE C1                   faddp    st(1)
01FD:    DC 4C 24 50             fmul     qword ptr [esp + 0x50]
0201:    DC 4F 20                fmul     qword ptr [edi + 0x20]
0204:    D9 E4                   ftst
0206:    9B                      wait
0207:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0209:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
020B:    73 0A                   jae      0x217    ; Conditional jump
020D:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
0211:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0215:    EB 08                   jmp      0x21f
0217:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
021B:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
021F:    DD 44 24 38             fld      qword ptr [esp + 0x38]
0223:    DC 4F 18                fmul     qword ptr [edi + 0x18]
0226:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0229:    D9 E8                   fld1    ; Load 1.0
022B:    DE C1                   faddp    st(1)
022D:    DC 4C 24 40             fmul     qword ptr [esp + 0x40]
0231:    DC 4F 20                fmul     qword ptr [edi + 0x20]
0234:    D9 E4                   ftst
0236:    9B                      wait
0237:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0239:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
023B:    73 0A                   jae      0x247    ; Conditional jump
023D:    DC 6C 24 20             fsubr    qword ptr [esp + 0x20]
0241:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0245:    EB 08                   jmp      0x24f
0247:    DC 44 24 18             fadd     qword ptr [esp + 0x18]
024B:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
024F:    D9 D0                   fnop
0251:    DD 44 24 18             fld      qword ptr [esp + 0x18]
0255:    D9 FA                   fsqrt    ; sqrt(ST(0))
0257:    D9 FA                   fsqrt    ; sqrt(ST(0))
0259:    DD 44 24 20             fld      qword ptr [esp + 0x20]
025D:    D9 FA                   fsqrt    ; sqrt(ST(0))
025F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0261:    D9 E0                   fchs    ; -ST(0)
0263:    DE C1                   faddp    st(1)
0265:    DC 4F 28                fmul     qword ptr [edi + 0x28]
0268:    90                      nop
0269:    DD 04 24                fld      qword ptr [esp]
026C:    D8 C8                   fmul     st(0)
026E:    DD 44 24 08             fld      qword ptr [esp + 8]
0272:    D8 C8                   fmul     st(0)
0274:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0278:    D8 C8                   fmul     st(0)
027A:    DE C1                   faddp    st(1)
027C:    DE C1                   faddp    st(1)
027E:    D9 FA                   fsqrt    ; sqrt(ST(0))
0280:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0283:    D8 D1                   fcom     st(1)
0285:    9B                      wait
0286:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0288:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
028A:    73 04                   jae      0x290    ; Conditional jump
028C:    DD D8                   fstp     st(0)
028E:    EB 02                   jmp      0x292
0290:    DD D9                   fstp     st(1)
0292:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0296:    75 03                   jne      0x29b    ; Conditional jump
0298:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
029B:    DD 5E E0                fstp     qword ptr [esi - 0x20]
029E:    83 C4 60                add      esp, 0x60
02A1:    C3                      ret
