; Disassembly: KleinBotTIFS.m3f
; Disassembler: Capstone 5.0.7 (CS_ARCH_X86, CS_MODE_32, Intel syntax)
; Code size: 366 bytes, decoded: 366 bytes (100.0%)
; DEoption: 20

0000:    83 C4 CE                add      esp, -0x32
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
004D:    DD 5C 24 18             fstp     qword ptr [esp + 0x18]
0051:    DD 04 24                fld      qword ptr [esp]
0054:    DD 44 24 08             fld      qword ptr [esp + 8]
0058:    D9 E4                   ftst
005A:    9B                      wait
005B:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
005D:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
005F:    73 09                   jae      0x6a    ; Conditional jump
0061:    DD 07                   fld      qword ptr [edi]
0063:    BA 01 00 00 00          mov      edx, 1
0068:    EB 09                   jmp      0x73
006A:    DD 07                   fld      qword ptr [edi]
006C:    D9 E0                   fchs    ; -ST(0)
006E:    BA FF FF FF FF          mov      edx, 0xffffffff
0073:    DD D9                   fstp     st(1)
0075:    DE C1                   faddp    st(1)
0077:    DD 1C 24                fstp     qword ptr [esp]
007A:    DD 04 24                fld      qword ptr [esp]
007D:    D8 C8                   fmul     st(0)
007F:    DD 44 24 08             fld      qword ptr [esp + 8]
0083:    D8 C8                   fmul     st(0)
0085:    DE C1                   faddp    st(1)
0087:    D9 FA                   fsqrt    ; sqrt(ST(0))
0089:    DC 27                   fsub     qword ptr [edi]
008B:    D8 C8                   fmul     st(0)
008D:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0091:    D8 C8                   fmul     st(0)
0093:    DE C1                   faddp    st(1)
0095:    D9 FA                   fsqrt    ; sqrt(ST(0))
0097:    DC 67 F0                fsub     qword ptr [edi - 0x10]
009A:    D9 E1                   fabs    ; |ST(0)|
009C:    DC 67 AC                fsub     qword ptr [edi - 0x54]
009F:    DD 5E E0                fstp     qword ptr [esi - 0x20]
00A2:    66 0F 11 04 24          movupd   xmmword ptr [esp], xmm0
00A7:    F2 0F 11 4C 24 10       movsd    qword ptr [esp + 0x10], xmm1
00AD:    83 FA 01                cmp      edx, 1
00B0:    75 47                   jne      0xf9    ; Conditional jump
00B2:    DD 04 24                fld      qword ptr [esp]
00B5:    D8 C8                   fmul     st(0)
00B7:    DD 44 24 08             fld      qword ptr [esp + 8]
00BB:    D8 C8                   fmul     st(0)
00BD:    DE C1                   faddp    st(1)
00BF:    D9 FA                   fsqrt    ; sqrt(ST(0))
00C1:    DD 44 24 08             fld      qword ptr [esp + 8]
00C5:    DD 04 24                fld      qword ptr [esp]
00C8:    D9 F3                   fpatan    ; atan2(ST(1), ST(0))
00CA:    DC 4F 20                fmul     qword ptr [edi + 0x20]
00CD:    DD 47 B8                fld      qword ptr [edi - 0x48]
00D0:    D8 C0                   fadd     st(0)
00D2:    DE C9                   fmulp    st(1)
00D4:    DC 47 F0                fadd     qword ptr [edi - 0x10]
00D7:    DD 5C 24 08             fstp     qword ptr [esp + 8]
00DB:    DC 67 08                fsub     qword ptr [edi + 8]
00DE:    D8 C8                   fmul     st(0)
00E0:    DD 44 24 10             fld      qword ptr [esp + 0x10]
00E4:    D8 C8                   fmul     st(0)
00E6:    DE C1                   faddp    st(1)
00E8:    D9 FA                   fsqrt    ; sqrt(ST(0))
00EA:    DC 64 24 08             fsub     qword ptr [esp + 8]
00EE:    DD 54 24 18             fst      qword ptr [esp + 0x18]
00F2:    D9 E1                   fabs    ; |ST(0)|
00F4:    DC 67 AC                fsub     qword ptr [edi - 0x54]
00F7:    EB 42                   jmp      0x13b
00F9:    DD 44 24 08             fld      qword ptr [esp + 8]
00FD:    DD 44 24 10             fld      qword ptr [esp + 0x10]
0101:    DD 04 24                fld      qword ptr [esp]
0104:    DC 47 08                fadd     qword ptr [edi + 8]
0107:    DD 1C 24                fstp     qword ptr [esp]
010A:    DD 5C 24 08             fstp     qword ptr [esp + 8]
010E:    DD 5C 24 10             fstp     qword ptr [esp + 0x10]
0112:    DD 04 24                fld      qword ptr [esp]
0115:    D8 C8                   fmul     st(0)
0117:    DD 44 24 08             fld      qword ptr [esp + 8]
011B:    D8 C8                   fmul     st(0)
011D:    DE C1                   faddp    st(1)
011F:    D9 FA                   fsqrt    ; sqrt(ST(0))
0121:    DC 47 B8                fadd     qword ptr [edi - 0x48]
0124:    DC 67 F0                fsub     qword ptr [edi - 0x10]
0127:    D8 C8                   fmul     st(0)
0129:    DD 44 24 10             fld      qword ptr [esp + 0x10]
012D:    D8 C8                   fmul     st(0)
012F:    DE C1                   faddp    st(1)
0131:    D9 FA                   fsqrt    ; sqrt(ST(0))
0133:    DC 67 B8                fsub     qword ptr [edi - 0x48]
0136:    D9 E1                   fabs    ; |ST(0)|
0138:    DC 67 AC                fsub     qword ptr [edi - 0x54]
013B:    DD 46 E0                fld      qword ptr [esi - 0x20]
013E:    DD 44 24 18             fld      qword ptr [esp + 0x18]
0142:    D9 C0                   fld      st(0)
0144:    D9 E1                   fabs    ; |ST(0)|
0146:    DE E1                   fsubrp   st(1)
0148:    DE C1                   faddp    st(1)
014A:    DD 5E E0                fstp     qword ptr [esi - 0x20]
014D:    DC 56 E0                fcom     qword ptr [esi - 0x20]
0150:    9B                      wait
0151:    DF E0                   fnstsw   ax    ; Store FPU status word in AX
0153:    D0 EC                   shr      ah, 1    ; Test FPU condition code (CF = C0 bit)
0155:    73 02                   jae      0x159    ; Conditional jump
0157:    EB 05                   jmp      0x15e
0159:    DD D8                   fstp     st(0)
015B:    DD 46 E0                fld      qword ptr [esi - 0x20]
015E:    83 7F B4 00             cmp      dword ptr [edi - 0x4c], 0
0162:    75 03                   jne      0x167    ; Conditional jump
0164:    DC 77 E0                fdiv     qword ptr [edi - 0x20]
0167:    DD 5E E0                fstp     qword ptr [esi - 0x20]
016A:    83 C4 32                add      esp, 0x32
016D:    C3                      ret
