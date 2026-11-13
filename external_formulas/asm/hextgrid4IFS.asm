; Disassembly: hextgrid4IFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 580 bytes, decoded: 580 bytes (100.0%)
; DEoption: 20

0000:    83 C4 9C                add      esp, -0x64
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
004B:    DD 47 08                fld      qword ptr [edi + 8]
004E:    DC 4F B8                fmul     qword ptr [edi - 0x48]
0051:    DD 54 24 44             fst      qword ptr [esp + 0x44]
0055:    D9 C0                   fld      st(0)
0057:    DC 4F 80                fmul     qword ptr [edi - 0x80]
005A:    DD 5C 24 54             fstp     qword ptr [esp + 0x54]
005E:    DC 4F 88                fmul     qword ptr [edi - 0x78]
0061:    DD 5C 24 4C             fstp     qword ptr [esp + 0x4c]
0065:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0069:    DD 44 24 08             fld      qword ptr [esp + 8]
006D:    DD 04 24                fld      qword ptr [esp]
0070:    D9 C0                   fld      st(0)
0072:    D8 4F 98                fmul     dword ptr [edi - 0x68]
0075:    D9 C2                   fld      st(2)
0077:    D8 4F 94                fmul     dword ptr [edi - 0x6c]
007A:    DE C1                   faddp    st(1)
007C:    D9 C3                   fld      st(3)
007E:    D8 4F 90                fmul     dword ptr [edi - 0x70]
0081:    DE C1                   faddp    st(1)
0083:    D9 C1                   fld      st(1)
0085:    D8 4F B0                fmul     dword ptr [edi - 0x50]
0088:    D9 C3                   fld      st(3)
008A:    D8 4F AC                fmul     dword ptr [edi - 0x54]
008D:    DE C1                   faddp    st(1)
008F:    D9 C4                   fld      st(4)
0091:    D8 4F A8                fmul     dword ptr [edi - 0x58]
0094:    DE C1                   faddp    st(1)
0096:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0098:    D8 4F A4                fmul     dword ptr [edi - 0x5c]
009B:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
009D:    D8 4F A0                fmul     dword ptr [edi - 0x60]
00A0:    DE C3                   faddp    st(3)
00A2:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
00A4:    D8 4F 9C                fmul     dword ptr [edi - 0x64]
00A7:    DE C2                   faddp    st(2)
00A9:    DD 1C 24                fstp     qword ptr [esp]
00AC:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00B0:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00B4:    DD 04 24                fld      qword ptr [esp]
00B7:    D9 E1                   fabs    ; |ST(0)|
00B9:    DD 54 24 18             fst      qword ptr [esp + 0x18]
00BD:    DD 14 24                fst      qword ptr [esp]
00C0:    DB 5C 24 2C             fistp    dword ptr [esp + 0x2c]
00C4:    DD 44 24 18             fld      qword ptr [esp + 0x18]
00C8:    DB 44 24 2C             fild     dword ptr [esp + 0x2c]
00CC:    DE E9                   fsubp    st(1)
00CE:    D9 E1                   fabs    ; |ST(0)|
00D0:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
00D4:    DB 44 24 2C             fild     dword ptr [esp + 0x2c]
00D8:    DC 4C 24 4C             fmul     qword ptr [esp + 0x4c]
00DC:    DC 64 24 08             fsub     qword ptr [esp + 8]
00E0:    D9 E1                   fabs    ; |ST(0)|
00E2:    DD 54 24 3C             fst      qword ptr [esp + 0x3c]
00E6:    DC 74 24 44             fdiv     qword ptr [esp + 0x44]
00EA:    D9 FC                   frndint    ; round(ST(0))
00EC:    DC 4C 24 44             fmul     qword ptr [esp + 0x44]
00F0:    D9 E0                   fchs    ; -ST(0)
00F2:    DC 44 24 3C             fadd     qword ptr [esp + 0x3c]
00F6:    D9 E1                   fabs    ; |ST(0)|
00F8:    DC 44 24 54             fadd     qword ptr [esp + 0x54]
00FC:    DD 44 24 18             fld      qword ptr [esp + 0x18]
0100:    D8 D1                   fcom     st(1)
0102:    9B                      wait
0103:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0105:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0107:    73 04                   jae      0x10d    ; Conditional jump
0109:    DD D8                   fstp     st(0)
010B:    EB 02                   jmp      0x10f
010D:    DD D9                   fstp     st(1)
010F:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0113:    DD 04 24                fld      qword ptr [esp]
0116:    DC 0F                   fmul     qword ptr [edi]
0118:    DD 44 24 08             fld      qword ptr [esp + 8]
011C:    DC 4F 10                fmul     qword ptr [edi + 0x10]
011F:    DE E9                   fsubp    st(1)
0121:    D9 E1                   fabs    ; |ST(0)|
0123:    DD 54 24 20             fst      qword ptr [esp + 0x20]
0127:    DB 5C 24 30             fistp    dword ptr [esp + 0x30]
012B:    DD 44 24 20             fld      qword ptr [esp + 0x20]
012F:    DB 44 24 30             fild     dword ptr [esp + 0x30]
0133:    DE E9                   fsubp    st(1)
0135:    D9 E1                   fabs    ; |ST(0)|
0137:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
013B:    DB 44 24 30             fild     dword ptr [esp + 0x30]
013F:    DC 4C 24 4C             fmul     qword ptr [esp + 0x4c]
0143:    DD 44 24 08             fld      qword ptr [esp + 8]
0147:    DC 0F                   fmul     qword ptr [edi]
0149:    DD 04 24                fld      qword ptr [esp]
014C:    DC 4F 10                fmul     qword ptr [edi + 0x10]
014F:    DE C1                   faddp    st(1)
0151:    DE C1                   faddp    st(1)
0153:    D9 E1                   fabs    ; |ST(0)|
0155:    DD 54 24 3C             fst      qword ptr [esp + 0x3c]
0159:    DC 74 24 44             fdiv     qword ptr [esp + 0x44]
015D:    D9 FC                   frndint    ; round(ST(0))
015F:    DC 4C 24 44             fmul     qword ptr [esp + 0x44]
0163:    D9 E0                   fchs    ; -ST(0)
0165:    DC 44 24 3C             fadd     qword ptr [esp + 0x3c]
0169:    D9 E1                   fabs    ; |ST(0)|
016B:    DC 44 24 54             fadd     qword ptr [esp + 0x54]
016F:    DD 44 24 20             fld      qword ptr [esp + 0x20]
0173:    D8 D1                   fcom     st(1)
0175:    9B                      wait
0176:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0178:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
017A:    73 04                   jae      0x180    ; Conditional jump
017C:    DD D8                   fstp     st(0)
017E:    EB 02                   jmp      0x182
0180:    DD D9                   fstp     st(1)
0182:    DD 5C 24 20             fstp     qword ptr [esp + 0x20]
0186:    DD 04 24                fld      qword ptr [esp]
0189:    DC 0F                   fmul     qword ptr [edi]
018B:    DD 44 24 08             fld      qword ptr [esp + 8]
018F:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0192:    DE C1                   faddp    st(1)
0194:    D9 E1                   fabs    ; |ST(0)|
0196:    DD 54 24 28             fst      qword ptr [esp + 0x28]
019A:    DB 5C 24 34             fistp    dword ptr [esp + 0x34]
019E:    DD 44 24 28             fld      qword ptr [esp + 0x28]
01A2:    DB 44 24 34             fild     dword ptr [esp + 0x34]
01A6:    DE E9                   fsubp    st(1)
01A8:    D9 E1                   fabs    ; |ST(0)|
01AA:    DD 5C 24 28             fstp     qword ptr [esp + 0x28]
01AE:    DB 44 24 34             fild     dword ptr [esp + 0x34]
01B2:    DC 4C 24 4C             fmul     qword ptr [esp + 0x4c]
01B6:    DD 44 24 08             fld      qword ptr [esp + 8]
01BA:    DC 0F                   fmul     qword ptr [edi]
01BC:    DD 04 24                fld      qword ptr [esp]
01BF:    DC 4F 10                fmul     qword ptr [edi + 0x10]
01C2:    D9 E0                   fchs    ; -ST(0)
01C4:    DE C1                   faddp    st(1)
01C6:    DE C1                   faddp    st(1)
01C8:    D9 E1                   fabs    ; |ST(0)|
01CA:    DD 54 24 3C             fst      qword ptr [esp + 0x3c]
01CE:    DC 74 24 44             fdiv     qword ptr [esp + 0x44]
01D2:    D9 FC                   frndint    ; round(ST(0))
01D4:    DC 4C 24 44             fmul     qword ptr [esp + 0x44]
01D8:    D9 E0                   fchs    ; -ST(0)
01DA:    DC 44 24 3C             fadd     qword ptr [esp + 0x3c]
01DE:    D9 E1                   fabs    ; |ST(0)|
01E0:    DC 44 24 54             fadd     qword ptr [esp + 0x54]
01E4:    DD 44 24 28             fld      qword ptr [esp + 0x28]
01E8:    D8 D1                   fcom     st(1)
01EA:    9B                      wait
01EB:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01ED:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
01EF:    73 04                   jae      0x1f5    ; Conditional jump
01F1:    DD D8                   fstp     st(0)
01F3:    EB 02                   jmp      0x1f7
01F5:    DD D9                   fstp     st(1)
01F7:    DD 5C 24 28             fstp     qword ptr [esp + 0x28]
01FB:    DD 44 24 18             fld      qword ptr [esp + 0x18]
01FF:    DD 44 24 20             fld      qword ptr [esp + 0x20]
0203:    DD 44 24 28             fld      qword ptr [esp + 0x28]
0207:    D8 D1                   fcom     st(1)
0209:    9B                      wait
020A:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
020C:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
020E:    73 04                   jae      0x214    ; Conditional jump
0210:    DD D9                   fstp     st(1)
0212:    EB 02                   jmp      0x216
0214:    DD D8                   fstp     st(0)
0216:    D8 D1                   fcom     st(1)
0218:    9B                      wait
0219:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
021B:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
021D:    73 04                   jae      0x223    ; Conditional jump
021F:    DD D9                   fstp     st(1)
0221:    EB 02                   jmp      0x225
0223:    DD D8                   fstp     st(0)
0225:    D8 C8                   fmul     st(0)
0227:    DD 44 24 10             fld      qword ptr [esp + 0x10]
022B:    D8 C8                   fmul     st(0)
022D:    DE C1                   faddp    st(1)
022F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0231:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0234:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0238:    75 03                   jne      0x23d    ; Conditional jump
023A:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
023D:    DD 5E E0                fstp     qword ptr [esi - 0x20]
0240:    83 C4 64                add      esp, 0x64
0243:    C3                      ret
