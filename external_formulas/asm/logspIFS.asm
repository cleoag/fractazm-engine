; Disassembly: logspIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 441 bytes, decoded: 441 bytes (100.0%)
; DEoption: 20

0000:    83 C4 D8                add      esp, -0x28
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
004B:    83 C7 C4                add      edi, -0x3c
004E:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0052:    DD 44 24 08             fld      qword ptr [esp + 8]
0056:    DD 04 24                fld      qword ptr [esp]
0059:    D9 C0                   fld      st(0)
005B:    D8 4F C4                fmul     dword ptr [edi - 0x3c]
005E:    D9 C2                   fld      st(2)
0060:    D8 4F C0                fmul     dword ptr [edi - 0x40]
0063:    DE C1                   faddp    st(1)
0065:    D9 C3                   fld      st(3)
0067:    D8 4F BC                fmul     dword ptr [edi - 0x44]
006A:    DE C1                   faddp    st(1)
006C:    D9 C1                   fld      st(1)
006E:    D8 4F DC                fmul     dword ptr [edi - 0x24]
0071:    D9 C3                   fld      st(3)
0073:    D8 4F D8                fmul     dword ptr [edi - 0x28]
0076:    DE C1                   faddp    st(1)
0078:    D9 C4                   fld      st(4)
007A:    D8 4F D4                fmul     dword ptr [edi - 0x2c]
007D:    DE C1                   faddp    st(1)
007F:    D9 CA                   fxch     st(2)    ; Exchange ST(0) and st(2)
0081:    D8 4F D0                fmul     dword ptr [edi - 0x30]
0084:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
0086:    D8 4F CC                fmul     dword ptr [edi - 0x34]
0089:    DE C3                   faddp    st(3)
008B:    D9 CB                   fxch     st(3)    ; Exchange ST(0) and st(3)
008D:    D8 4F C8                fmul     dword ptr [edi - 0x38]
0090:    DE C2                   faddp    st(2)
0092:    DD 1C 24                fstp     qword ptr [esp]
0095:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0099:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
009D:    83 C7 3C                add      edi, 0x3c
00A0:    8B 57 BC                mov      edx, dword ptr [edi - 0x44]
00A3:    81 E2 00 00 00 80       and      edx, 0x80000000
00A9:    0F 84 72 00 00 00       je       0x121    ; Conditional jump
00AF:    DD 04 24                fld      qword ptr [esp]
00B2:    D9 E8                   fld1    ; Load 1.0
00B4:    DE C1                   faddp    st(1)
00B6:    D8 C8                   fmul     st(0)
00B8:    DD 44 24 08             fld      qword ptr [esp + 8]
00BC:    D8 C8                   fmul     st(0)
00BE:    DE C1                   faddp    st(1)
00C0:    D9 E8                   fld1    ; Load 1.0
00C2:    DE F1                   fdivrp   st(1)
00C4:    D9 C0                   fld      st(0)
00C6:    DD 04 24                fld      qword ptr [esp]
00C9:    D8 C8                   fmul     st(0)
00CB:    DD 44 24 08             fld      qword ptr [esp + 8]
00CF:    D8 C8                   fmul     st(0)
00D1:    DE C1                   faddp    st(1)
00D3:    D9 E8                   fld1    ; Load 1.0
00D5:    DE E9                   fsubp    st(1)
00D7:    DE C9                   fmulp    st(1)
00D9:    DD 44 24 08             fld      qword ptr [esp + 8]
00DD:    D9 E0                   fchs    ; -ST(0)
00DF:    D8 C0                   fadd     st(0)
00E1:    DE CA                   fmulp    st(2)
00E3:    DC 47 C0                fadd     qword ptr [edi - 0x40]
00E6:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00EA:    DD 1C 24                fstp     qword ptr [esp]
00ED:    DD 04 24                fld      qword ptr [esp]
00F0:    D8 8F 7C FF FF FF       fmul     dword ptr [edi - 0x84]
00F6:    DD 44 24 08             fld      qword ptr [esp + 8]
00FA:    D8 8F 78 FF FF FF       fmul     dword ptr [edi - 0x88]
0100:    D9 E0                   fchs    ; -ST(0)
0102:    DE C1                   faddp    st(1)
0104:    DD 04 24                fld      qword ptr [esp]
0107:    D8 8F 78 FF FF FF       fmul     dword ptr [edi - 0x88]
010D:    DD 44 24 08             fld      qword ptr [esp + 8]
0111:    D8 8F 7C FF FF FF       fmul     dword ptr [edi - 0x84]
0117:    DE C1                   faddp    st(1)
0119:    DD 1C 24                fstp     qword ptr [esp]
011C:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0120:    90                      nop
0121:    DD 04 24                fld      qword ptr [esp]
0124:    D8 C8                   fmul     st(0)
0126:    DD 44 24 08             fld      qword ptr [esp + 8]
012A:    D8 C8                   fmul     st(0)
012C:    DE C1                   faddp    st(1)
012E:    D9 FA                   fsqrt    ; sqrt(ST(0))
0130:    DD 47 F0                fld      qword ptr [edi - 0x10]
0133:    83 7F A4 01             cmp      dword ptr [edi - 0x5c], 1
0137:    75 04                   jne      0x13d    ; Conditional jump
0139:    DE F1                   fdivrp   st(1)
013B:    EB 40                   jmp      0x17d
013D:    83 7F A4 02             cmp      dword ptr [edi - 0x5c], 2
0141:    75 0A                   jne      0x14d    ; Conditional jump
0143:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0145:    D9 F1                   fyl2x
0147:    D8 C8                   fmul     st(0)
0149:    D9 E0                   fchs    ; -ST(0)
014B:    EB 30                   jmp      0x17d
014D:    83 7F A4 03             cmp      dword ptr [edi - 0x5c], 3
0151:    75 24                   jne      0x177    ; Conditional jump
0153:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0155:    D9 F1                   fyl2x
0157:    DD 54 24 18             fst      qword ptr [esp + 0x18]
015B:    D9 E1                   fabs    ; |ST(0)|
015D:    DC 47 08                fadd     qword ptr [edi + 8]
0160:    8B 54 24 1C             mov      edx, dword ptr [esp + 0x1c]
0164:    81 E2 00 00 00 80       and      edx, 0x80000000
016A:    74 02                   je       0x16e    ; Conditional jump
016C:    D9 E0                   fchs    ; -ST(0)
016E:    90                      nop
016F:    D9 E8                   fld1    ; Load 1.0
0171:    DE F1                   fdivrp   st(1)
0173:    EB 08                   jmp      0x17d
0175:    90                      nop
0176:    90                      nop
0177:    D9 C9                   fxch     st(1)    ; Exchange ST(0) and st(1)
0179:    D9 F1                   fyl2x
017B:    D9 D0                   fnop
017D:    DD 04 24                fld      qword ptr [esp]
0180:    DD 44 24 08             fld      qword ptr [esp + 8]
0184:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
0186:    DC 0F                   fmul     qword ptr [edi]
0188:    DA 4F B0                fimul    dword ptr [edi - 0x50]
018B:    DE C1                   faddp    st(1)
018D:    D9 C0                   fld      st(0)
018F:    D9 FC                   frndint    ; round(ST(0))
0191:    DE E1                   fsubrp   st(1)
0193:    D9 E1                   fabs    ; |ST(0)|
0195:    DC 4F A8                fmul     qword ptr [edi - 0x58]
0198:    DD 44 24 10             fld      qword ptr [esp + 0x10]
019C:    D8 C8                   fmul     st(0)
019E:    DE C1                   faddp    st(1)
01A0:    D9 FA                   fsqrt    ; sqrt(ST(0))
01A2:    DD 47 B8                fld      qword ptr [edi - 0x48]
01A5:    D9 E1                   fabs    ; |ST(0)|
01A7:    DE E9                   fsubp    st(1)
01A9:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
01AD:    75 03                   jne      0x1b2    ; Conditional jump
01AF:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
01B2:    DD 5E E0                fstp     qword ptr [esi - 0x20]
01B5:    83 C4 28                add      esp, 0x28
01B8:    C3                      ret
