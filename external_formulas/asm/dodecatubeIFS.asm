; Disassembly: dodecatubeIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 466 bytes, decoded: 466 bytes (100.0%)
; DEoption: 20

0000:    83 C4 E4                add      esp, -0x1c
0003:    66 0F 10 46 90          movupd   xmm0, xmmword ptr [esi - 0x70]
0008:    F2 0F 10 4E 88          movsd    xmm1, qword ptr [esi - 0x78]
000D:    66 0F 59 47 D0          mulpd    xmm0, xmmword ptr [edi - 0x30]
0012:    F2 0F 59 4F D0          mulsd    xmm1, qword ptr [edi - 0x30]
0017:    66 0F 58 47 C0          addpd    xmm0, xmmword ptr [edi - 0x40]
001C:    F2 0F 58 4F B8          addsd    xmm1, qword ptr [edi - 0x48]
0021:    83 7F 94 00             cmp      dword ptr [edi - 0x6c], 0
0025:    74 19                   je       0x40    ; Conditional jump
0027:    F2 0F 10 6E 70          movsd    xmm5, qword ptr [esi + 0x70]
002C:    F2 0F 59 6F D0          mulsd    xmm5, qword ptr [edi - 0x30]
0031:    66 0F 11 46 90          movupd   xmmword ptr [esi - 0x70], xmm0
0036:    F2 0F 11 4E 88          movsd    qword ptr [esi - 0x78], xmm1
003B:    F2 0F 11 6E 70          movsd    qword ptr [esi + 0x70], xmm5
0040:    66 0F 11 04 24          movupd   xmmword ptr [esp], xmm0
0045:    F2 0F 11 4C 24 10       movsd    qword ptr [esp + 0x10], xmm1
004B:    BA 02 00 00 00          mov      edx, 2
0050:    DD 04 24                fld      qword ptr [esp]
0053:    D9 E1                   fabs    ; |ST(0)|
0055:    DD 14 24                fst      qword ptr [esp]
0058:    DC 0F                   fmul     qword ptr [edi]
005A:    DD 44 24 08             fld      qword ptr [esp + 8]
005E:    D9 E1                   fabs    ; |ST(0)|
0060:    DD 54 24 08             fst      qword ptr [esp + 8]
0064:    DC 4F 08                fmul     qword ptr [edi + 8]
0067:    DD 44 24 10             fld      qword ptr [esp + 0x10]
006B:    D9 E1                   fabs    ; |ST(0)|
006D:    DD 54 24 10             fst      qword ptr [esp + 0x10]
0071:    DC 4F 10                fmul     qword ptr [edi + 0x10]
0074:    DE C1                   faddp    st(1)
0076:    DE C1                   faddp    st(1)
0078:    D9 C0                   fld      st(0)
007A:    D9 E1                   fabs    ; |ST(0)|
007C:    DE C1                   faddp    st(1)
007E:    D9 E0                   fchs    ; -ST(0)
0080:    D9 C0                   fld      st(0)
0082:    DC 0F                   fmul     qword ptr [edi]
0084:    DC 04 24                fadd     qword ptr [esp]
0087:    DD 1C 24                fstp     qword ptr [esp]
008A:    D9 C0                   fld      st(0)
008C:    DC 4F 08                fmul     qword ptr [edi + 8]
008F:    DC 44 24 08             fadd     qword ptr [esp + 8]
0093:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0097:    D9 C0                   fld      st(0)
0099:    DC 4F 10                fmul     qword ptr [edi + 0x10]
009C:    DC 44 24 10             fadd     qword ptr [esp + 0x10]
00A0:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00A4:    DD D8                   fstp     st(0)
00A6:    DD 04 24                fld      qword ptr [esp]
00A9:    DC 4F 08                fmul     qword ptr [edi + 8]
00AC:    DD 44 24 08             fld      qword ptr [esp + 8]
00B0:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00B3:    D9 E0                   fchs    ; -ST(0)
00B5:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00B9:    DC 0F                   fmul     qword ptr [edi]
00BB:    D9 E0                   fchs    ; -ST(0)
00BD:    DE C1                   faddp    st(1)
00BF:    DE C1                   faddp    st(1)
00C1:    D9 C0                   fld      st(0)
00C3:    D9 E1                   fabs    ; |ST(0)|
00C5:    DE C1                   faddp    st(1)
00C7:    D9 C0                   fld      st(0)
00C9:    DC 4F 08                fmul     qword ptr [edi + 8]
00CC:    D9 E0                   fchs    ; -ST(0)
00CE:    DC 04 24                fadd     qword ptr [esp]
00D1:    DD 1C 24                fstp     qword ptr [esp]
00D4:    D9 C0                   fld      st(0)
00D6:    DC 4F 10                fmul     qword ptr [edi + 0x10]
00D9:    DC 44 24 08             fadd     qword ptr [esp + 8]
00DD:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00E1:    D9 C0                   fld      st(0)
00E3:    DC 0F                   fmul     qword ptr [edi]
00E5:    DC 44 24 10             fadd     qword ptr [esp + 0x10]
00E9:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
00ED:    DD D8                   fstp     st(0)
00EF:    4A                      dec      edx
00F0:    0F 8F 5A FF FF FF       jg       0x50    ; Conditional jump
00F6:    DD 04 24                fld      qword ptr [esp]
00F9:    D9 E1                   fabs    ; |ST(0)|
00FB:    DD 44 24 08             fld      qword ptr [esp + 8]
00FF:    D9 E1                   fabs    ; |ST(0)|
0101:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0105:    D9 E1                   fabs    ; |ST(0)|
0107:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
010B:    DD 5C 24 08             fstp     qword ptr [esp + 8]
010F:    DD 1C 24                fstp     qword ptr [esp]
0112:    DD 04 24                fld      qword ptr [esp]
0115:    DC 4F B0                fmul     qword ptr [edi - 0x50]
0118:    DD 44 24 10             fld      qword ptr [esp + 0x10]
011C:    DC 4F A8                fmul     qword ptr [edi - 0x58]
011F:    DE C1                   faddp    st(1)
0121:    DD 04 24                fld      qword ptr [esp]
0124:    DC 4F A8                fmul     qword ptr [edi - 0x58]
0127:    DD 44 24 10             fld      qword ptr [esp + 0x10]
012B:    DC 4F B0                fmul     qword ptr [edi - 0x50]
012E:    D9 E0                   fchs    ; -ST(0)
0130:    DE C1                   faddp    st(1)
0132:    DD 1C 24                fstp     qword ptr [esp]
0135:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
0139:    DD 04 24                fld      qword ptr [esp]
013C:    DC 4F A0                fmul     qword ptr [edi - 0x60]
013F:    DD 44 24 08             fld      qword ptr [esp + 8]
0143:    DC 4F 98                fmul     qword ptr [edi - 0x68]
0146:    DE C1                   faddp    st(1)
0148:    DD 04 24                fld      qword ptr [esp]
014B:    DC 4F 98                fmul     qword ptr [edi - 0x68]
014E:    DD 44 24 08             fld      qword ptr [esp + 8]
0152:    DC 4F A0                fmul     qword ptr [edi - 0x60]
0155:    D9 E0                   fchs    ; -ST(0)
0157:    DE C1                   faddp    st(1)
0159:    DD 1C 24                fstp     qword ptr [esp]
015C:    DD 5C 24 08             fstp     qword ptr [esp + 8]
0160:    DD 44 24 08             fld      qword ptr [esp + 8]
0164:    DC 67 E8                fsub     qword ptr [edi - 0x18]
0167:    DD 54 24 08             fst      qword ptr [esp + 8]
016B:    D8 C8                   fmul     st(0)
016D:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0171:    DC 67 E0                fsub     qword ptr [edi - 0x20]
0174:    DD 54 24 10             fst      qword ptr [esp + 0x10]
0178:    D8 C8                   fmul     st(0)
017A:    DE C1                   faddp    st(1)
017C:    D9 FA                   fsqrt    ; sqrt(ST(0))
017E:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0181:    DD 04 24                fld      qword ptr [esp]
0184:    D9 E1                   fabs    ; |ST(0)|
0186:    DC 67 8C                fsub     qword ptr [edi - 0x74]
0189:    D8 C8                   fmul     st(0)
018B:    DD 44 24 08             fld      qword ptr [esp + 8]
018F:    D8 C8                   fmul     st(0)
0191:    DE C1                   faddp    st(1)
0193:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0197:    D8 C8                   fmul     st(0)
0199:    DE C1                   faddp    st(1)
019B:    D9 FA                   fsqrt    ; sqrt(ST(0))
019D:    DC 67 84                fsub     qword ptr [edi - 0x7c]
01A0:    D8 D1                   fcom     st(1)
01A2:    9B                      wait
01A3:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
01A5:    80 E4 41                and      ah, 0x41
01A8:    74 0A                   je       0x1b4    ; Conditional jump
01AA:    DD D9                   fstp     st(1)
01AC:    DD 87 74 FF FF FF       fld      qword ptr [edi - 0x8c]
01B2:    EB 08                   jmp      0x1bc
01B4:    DD D8                   fstp     st(0)
01B6:    DD 87 7C FF FF FF       fld      qword ptr [edi - 0x84]
01BC:    DD 9E 80 00 00 00       fstp     qword ptr [esi + 0x80]
01C2:    83 7F 94 00             cmp      dword ptr [edi - 0x6c], 0
01C6:    75 03                   jne      0x1cb    ; Conditional jump
01C8:    DC 77 D0                fdiv     qword ptr [edi - 0x30]
01CB:    DD 5E E0                fstp     qword ptr [esi - 0x20]
01CE:    83 C4 1C                add      esp, 0x1c
01D1:    C3                      ret
